#include "config.h"
#include "math_3d.h"
#include "rasterizer.h"
#include "clipping_fill.h"
#include "components_mesh.h"
#include "wiring_table.h"
#include "robot_kinematics.h"
#include "ui_manager.h"

#include <GL/glut.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iostream>

// =========================================================================
// BIẾN TOÀN CỤC CHƯƠNG TRÌNH
// =========================================================================

// Framebuffer và cửa sổ xén hình hiện hành
MauSac g_frameBuffer[CUA_SO_CAO][CUA_SO_RONG];
int g_clipMinX = 0, g_clipMaxX = CUA_SO_RONG - 1;
int g_clipMinY = 0, g_clipMaxY = CUA_SO_CAO - 1;

// Danh mục dữ liệu
std::vector<LinhKien> g_dsLinhKien;
std::vector<DayDien>  g_bangDay;
KhungXe2Tang          g_khungXe;

// Hệ thống mô phỏng động học
DongHocXe       g_xeRobot;
CungCapTayGap   g_tayGap;
VatTheTuongTac  g_vatThe;

// Quản lý giao diện UI
TrinhQuanLyGiaoDien g_ui;

// Trạng thái kiểm thử độc lập linh kiện trong Chế độ 2 (Phần B Mục 2)
float g_gocQuayMoTo   = 0.0f; // Góc quay của mô-tơ/bánh xe (độ)
float g_tocDoQuayMoTo = 0.0f; // Vận tốc góc mô-tơ (độ/giây)
bool  g_congTacPinBat = true; // Trạng thái công tắc nguồn (BAT/TAT)

// Quản lý tương tác chuột
int  g_chuotCuX = 0, g_chuotCuY = 0;
bool g_dangKeoChuotTrai  = false;
bool g_dangKeoChuotPhai  = false;
int  g_sliderDangKeo     = -1; // Chỉ số slider đang tương tác

// Đồng hồ tính dt
auto g_thoiGianTruoc = std::chrono::high_resolution_clock::now();

// =========================================================================
// HÀM TÍNH TOÁN MA TRẬN THẾ GIỚI CỦA LINH KIỆN TRONG CÂY PHÂN CẤP
// =========================================================================

MaTran4x4 tinhMaTranTheGioiLinhKien(const LinhKien& lk, const MaTran4x4& matWorldCar, float tExp) {
    MaTran4x4 matBase;

    // Các linh kiện thuộc cánh tay robot di chuyển theo các khớp phân cấp
    if (lk.id == LK_SERVO_DE) {
        matBase = g_tayGap.matWorldBase;
    } else if (lk.id == LK_SERVO_KHUU) {
        matBase = g_tayGap.matWorldElbow;
    } else if (lk.id == LK_SERVO_KEP) {
        matBase = g_tayGap.matWorldWrist;
    } else {
        // Các linh kiện gắn trên khung gầm xe
        matBase = lk.viTriLapDat * matWorldCar;
    }

    // Áp dụng phép tịnh tiến bóc tách không gian (Exploded View):
    // M_world = M_hierarchy * Tr(t_exp * v_exp) (Mục 5.1 Đề cương)
    if (tExp > 1e-4f) {
        Diem3D vExp = lk.vectorTachRoi * tExp;
        matBase = matBase * taoMaTranTinhTien(vExp.x, vExp.y, vExp.z);
    }

    return matBase;
}

// =========================================================================
// RENDER CẢNH 3D TRUNG TÂM (CHẾ ĐỘ 1, 2, 3)
// Áp dụng:
// 1. Back-face Culling trên mặt phẳng chiếu 2D
// 2. Lambertian Flat Shading với hướng sáng L = normalize(0.4, 0.8, 0.5)
// 3. Sutherland-Hodgman Polygon Clipping theo CenterViewport
// 4. Định tuyến dây 3D máng cáp + bám xương cánh tay + Bézier bậc 2
// 5. Global Painter's Algorithm sắp xếp đồng thời cả Mặt đa giác và Đoạn dây
// =========================================================================

enum KieuPhanTu3D { PHAN_TU_MAT, PHAN_TU_DOAN_DAY };

struct PhanTuCanVe3D {
    KieuPhanTu3D kieu;
    std::vector<Diem2D> poly2D; // Dùng khi là Mặt
    Diem2D pA, pB;              // Dùng khi là Đoạn dây
    float doSauZ;
    MauSac mauSac;
    int doDay;
    bool laKhungDay;
};

// Hàm phụ trợ nạp các mặt của một Part vào danh sách Painter's Algorithm
void themPartVaoDanhSachRender(
    const Part& part,
    const MaTran4x4& matParent,
    int idLinhKien,
    const Diem3D& camTarget,
    float thetaRad, float phiRad, float R_cam, float D_proj,
    float cx, float cy,
    const Diem2D& wmin, const Diem2D& wmax,
    const Diem3D& huongSang,
    std::vector<PhanTuCanVe3D>& danhSach)
{
    MaTran4x4 matPart = part.bienDoiCucBo * matParent;

    std::vector<Diem3D> vWorld, vView;
    std::vector<Diem2D> vScreen;
    vWorld.reserve(part.mesh.dinh.size());
    vView.reserve(part.mesh.dinh.size());
    vScreen.reserve(part.mesh.dinh.size());

    for (const auto& d : part.mesh.dinh) {
        Diem3D pw = matPart.bienDoi(d);
        Diem3D pv = chuyenHeToaDoQuanSat(pw, camTarget, thetaRad, phiRad, R_cam);
        Diem2D ps = chieuLenManHinh(pv, D_proj, true, cx, cy);
        vWorld.push_back(pw);
        vView.push_back(pv);
        vScreen.push_back(ps);
    }

    for (const auto& f : part.mesh.mat) {
        if (f.cacDinh.size() < 3) continue;

        // 1. Back-face Culling trên mặt phẳng chiếu 2D (Mục 1.3 Đề cương)
        Diem2D p0 = vScreen[f.cacDinh[0]];
        Diem2D p1 = vScreen[f.cacDinh[1]];
        Diem2D p2 = vScreen[f.cacDinh[2]];
        float cross2D = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);

        if (cross2D <= 0 && !g_ui.cheDoKhungDay) continue;

        // 2. Tính Lambertian Flat Shading từ pháp tuyến thế giới (Phần B Mục 3)
        // I = ka + kd * max(0, n . L) với ka = 0.45, kd = 0.55
        Diem3D pw0 = vWorld[f.cacDinh[0]];
        Diem3D pw1 = vWorld[f.cacDinh[1]];
        Diem3D pw2 = vWorld[f.cacDinh[2]];
        Diem3D vA = pw1 - pw0;
        Diem3D vB = pw2 - pw0;
        Diem3D norm = (vA * vB).donVi();

        float dotNL = norm.x * huongSang.x + norm.y * huongSang.y + norm.z * huongSang.z;
        if (dotNL < 0.0f) dotNL = 0.0f;
        float intensity = 0.45f + 0.55f * dotNL;

        MauSac mauShaded = MauSac(
            (uint8_t)std::clamp(f.mauSac.r * intensity, 0.0f, 255.0f),
            (uint8_t)std::clamp(f.mauSac.g * intensity, 0.0f, 255.0f),
            (uint8_t)std::clamp(f.mauSac.b * intensity, 0.0f, 255.0f)
        );

        if (idLinhKien > 0 && idLinhKien == g_ui.linhKienDangChon) {
            mauShaded = mauShaded.lamSang(1.30f);
        }

        // 3. Tính độ sâu trung bình Z phục vụ Painter's Algorithm
        float sumZ = 0.0f;
        std::vector<Diem2D> poly2D;
        for (int idx : f.cacDinh) {
            sumZ += vView[idx].z;
            poly2D.push_back(vScreen[idx]);
        }
        float zAvg = sumZ / (float)f.cacDinh.size();

        // 4. Xén đa giác theo hình chữ nhật CenterViewport (Sutherland-Hodgman)
        poly2D = xenDaGiacSutherlandHodgman(poly2D, wmin, wmax);

        if (poly2D.size() >= 3) {
            PhanTuCanVe3D pt;
            pt.kieu = PHAN_TU_MAT;
            pt.poly2D = poly2D;
            pt.doSauZ = zAvg;
            pt.mauSac = mauShaded;
            pt.doDay = 1;
            pt.laKhungDay = g_ui.cheDoKhungDay;
            danhSach.push_back(pt);
        }
    }
}

void renderCanh3D(float dt) {
    datVungXen(VIEWPORT_X_MIN, VIEWPORT_X_MAX, VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);

    // Cập nhật động học xe
    g_xeRobot.capNhat(dt);

    // Ma trận thế giới gốc của xe
    float radCar = g_xeRobot.gocHuong * (float)M_PI / 180.0f;
    MaTran4x4 matWorldCar = taoMaTranQuayY(radCar) * taoMaTranTinhTien(g_xeRobot.x, 0.0f, g_xeRobot.z);

    // Cập nhật phân cấp cánh tay robot
    g_tayGap.capNhat(dt, matWorldCar);

    // Cập nhật khối vật thể tương tác
    g_vatThe.capNhat(g_tayGap);

    // Tính tâm quan sát camera và ma trận chiếu
    Diem3D camTarget = g_ui.camTargetHienTai;
    if (g_ui.cheDoHienTai == CHE_DO_TONG_THE) {
        camTarget = Diem3D(g_xeRobot.x, 3.5f, g_xeRobot.z);
    }

    float thetaRad = g_ui.camTheta * (float)M_PI / 180.0f;
    float phiRad   = g_ui.camPhi   * (float)M_PI / 180.0f;
    float R_cam    = g_ui.camDistHienTai;
    float D_proj   = 36.0f;
    float cx = (VIEWPORT_X_MIN + VIEWPORT_X_MAX) * 0.5f;
    float cy = (VIEWPORT_Y_MIN + VIEWPORT_Y_MAX) * 0.5f;

    Diem2D wmin(VIEWPORT_X_MIN + 2, VIEWPORT_Y_MIN + 2);
    Diem2D wmax(VIEWPORT_X_MAX - 2, VIEWPORT_Y_MAX - 2);

    // Vector hướng sáng định hướng cố định L = normalize(0.4, 0.8, 0.5)
    Diem3D huongSang = Diem3D(0.4f, 0.8f, 0.5f).donVi();

    std::vector<PhanTuCanVe3D> danhSachPhanTu;

    if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
        // =====================================================================
        // CHẾ ĐỘ 2: MÔI TRƯỜNG BÀN GIA CÔNG & KIỂM THỬ 3D ĐỘC LẬP (CAD WORKBENCH)
        // (Phần B Đặc tả: Cô lập 100%, không vẽ xe, không vẽ dây, chỉ vẽ linh kiện tại (0,0,0))
        // =====================================================================

        // 1. Vẽ bàn lưới gia công 16x16 cm tại y = 0
        MauSac mauLuoiPhu = MauSac(35, 45, 58);
        MauSac mauTrucSan  = MauSac(65, 85, 115);
        MauSac mauVienSan  = MauSac(75, 105, 140);

        for (int i = -8; i <= 8; ++i) {
            MauSac cX = (i == 0) ? mauTrucSan : mauLuoiPhu;
            Diem3D pA1((float)i, 0.0f, -8.0f), pA2((float)i, 0.0f, 8.0f);
            Diem3D vA1 = chuyenHeToaDoQuanSat(pA1, camTarget, thetaRad, phiRad, R_cam);
            Diem3D vA2 = chuyenHeToaDoQuanSat(pA2, camTarget, thetaRad, phiRad, R_cam);
            Diem2D sA1 = chieuLenManHinh(vA1, D_proj, true, cx, cy);
            Diem2D sA2 = chieuLenManHinh(vA2, D_proj, true, cx, cy);
            Diem2D cA = sA1, cB = sA2;
            if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, cX);
            }

            MauSac cZ = (i == 0) ? mauTrucSan : mauLuoiPhu;
            Diem3D pB1(-8.0f, 0.0f, (float)i), pB2(8.0f, 0.0f, (float)i);
            Diem3D vB1 = chuyenHeToaDoQuanSat(pB1, camTarget, thetaRad, phiRad, R_cam);
            Diem3D vB2 = chuyenHeToaDoQuanSat(pB2, camTarget, thetaRad, phiRad, R_cam);
            Diem2D sB1 = chieuLenManHinh(vB1, D_proj, true, cx, cy);
            Diem2D sB2 = chieuLenManHinh(vB2, D_proj, true, cx, cy);
            cA = sB1; cB = sB2;
            if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, cZ);
            }
        }

        // 2. Vẽ 3 Trục tọa độ chuẩn Ox (Đỏ), Oy (Xanh lá), Oz (Xanh dương)
        struct Truc3D { Diem3D goc, ngon; MauSac mau; };
        std::vector<Truc3D> cacTruc = {
            {Diem3D(0.0f, 0.02f, 0.0f), Diem3D(6.0f, 0.02f, 0.0f), MauSac(235, 55, 55)},  // Ox
            {Diem3D(0.0f, 0.02f, 0.0f), Diem3D(0.0f, 6.0f, 0.0f),  MauSac(45, 215, 75)},  // Oy
            {Diem3D(0.0f, 0.02f, 0.0f), Diem3D(0.0f, 0.02f, 6.0f), MauSac(50, 135, 245)}  // Oz
        };
        for (const auto& tr : cacTruc) {
            Diem3D v1 = chuyenHeToaDoQuanSat(tr.goc,  camTarget, thetaRad, phiRad, R_cam);
            Diem3D v2 = chuyenHeToaDoQuanSat(tr.ngon, camTarget, thetaRad, phiRad, R_cam);
            Diem2D s1 = chieuLenManHinh(v1, D_proj, true, cx, cy);
            Diem2D s2 = chieuLenManHinh(v2, D_proj, true, cx, cy);
            Diem2D cA = s1, cB = s2;
            if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                veDoanThangDay((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, tr.mau, 2);
            }
        }

        // 3. Đường gióng kích thước bao (Dimension Lines)
        const auto& lkHienTai = g_dsLinhKien[g_ui.linhKienDangChon];
        float hw = lkHienTai.kichThuocBao.x * 0.5f;
        float hd = lkHienTai.kichThuocBao.z * 0.5f;
        Diem3D b0(-hw, 0.02f, -hd), b1(hw, 0.02f, -hd), b2(hw, 0.02f, hd), b3(-hw, 0.02f, hd);
        std::vector<std::pair<Diem3D, Diem3D>> vienBao = {{b0, b1}, {b1, b2}, {b2, b3}, {b3, b0}};
        MauSac mauVienBao(0, 168, 204);
        for (const auto& e : vienBao) {
            Diem3D v1 = chuyenHeToaDoQuanSat(e.first,  camTarget, thetaRad, phiRad, R_cam);
            Diem3D v2 = chuyenHeToaDoQuanSat(e.second, camTarget, thetaRad, phiRad, R_cam);
            Diem2D s1 = chieuLenManHinh(v1, D_proj, true, cx, cy);
            Diem2D s2 = chieuLenManHinh(v2, D_proj, true, cx, cy);
            Diem2D cA = s1, cB = s2;
            if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, mauVienBao);
            }
        }

        // 4. Thu thập các mặt của linh kiện được chọn theo phân loại tương tác
        if (lkHienTai.category == InspectCategory::ARM_MANIPULATOR) {
            // Hiển thị nguyên Cụm tay gắp 3-DOF tại tâm bàn (0, 0, 0)
            MaTran4x4 matBase, matElbow, matWrist;
            g_tayGap.tinhMaTranBanThucNghiem(matBase, matElbow, matWrist, g_ui.tyLeTachRoi);

            // Cập nhật khối vật mẫu thử gắp trên bàn thử
            g_vatThe.capNhatTrenBan(matWrist);

            for (const auto& part : g_dsLinhKien[LK_SERVO_DE].parts) {
                themPartVaoDanhSachRender(part, matBase, LK_SERVO_DE, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }
            for (const auto& part : g_dsLinhKien[LK_SERVO_KHUU].parts) {
                themPartVaoDanhSachRender(part, matElbow, LK_SERVO_KHUU, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }
            for (const auto& part : g_dsLinhKien[LK_SERVO_KEP].parts) {
                themPartVaoDanhSachRender(part, matWrist, LK_SERVO_KEP, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }

            // Vật mẫu thử gắp trên bàn
            Part partVatMau;
            partVatMau.ten = "VatMau";
            partVatMau.mesh = taoKhoiHop(g_vatThe.kichThuoc, g_vatThe.kichThuoc, g_vatThe.kichThuoc, g_vatThe.mauSac);
            partVatMau.bienDoiCucBo = taoMaTranTinhTien(g_vatThe.viTriBanThucNghiem.x, g_vatThe.viTriBanThucNghiem.y, g_vatThe.viTriBanThucNghiem.z);
            themPartVaoDanhSachRender(partVatMau, MaTran4x4(), -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
        }
        else if (lkHienTai.category == InspectCategory::ROTARY_MOTOR_WHEEL) {
            // Hiển thị Mô-tơ DC + Trục quay + Bánh xe tại tâm bàn
            g_gocQuayMoTo += g_tocDoQuayMoTo * dt;
            float radMoTo = g_gocQuayMoTo * (float)M_PI / 180.0f;
            float huongTach = (lkHienTai.id == LK_DONG_CO_T) ? -4.5f : 4.5f;

            MaTran4x4 matMotor = taoMaTranTinhTien(0.0f, 2.5f, 0.0f);
            for (const auto& part : lkHienTai.parts) {
                MaTran4x4 mPart = taoMaTranTinhTien(part.partExplodeVec.x * g_ui.tyLeTachRoi,
                                                    part.partExplodeVec.y * g_ui.tyLeTachRoi,
                                                    part.partExplodeVec.z * g_ui.tyLeTachRoi)
                                * part.bienDoiCucBo * matMotor;
                themPartVaoDanhSachRender(part, mPart, lkHienTai.id, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }

            // Gắn Bánh xe và Mâm xe vào mô-tơ trên bàn thử, cho phép quay và tách rời
            MaTran4x4 matWheel = taoMaTranQuayX(radMoTo)
                               * taoMaTranTinhTien(huongTach * (0.6f + g_ui.tyLeTachRoi * 0.8f), 2.5f, 0.0f);
            Part wheelPart = (lkHienTai.id == LK_DONG_CO_T) ? g_khungXe.wheelL : g_khungXe.wheelR;
            Part rimPart   = (lkHienTai.id == LK_DONG_CO_T) ? g_khungXe.rimL   : g_khungXe.rimR;
            wheelPart.bienDoiCucBo = MaTran4x4();
            rimPart.bienDoiCucBo   = MaTran4x4();
            themPartVaoDanhSachRender(wheelPart, matWheel, lkHienTai.id, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            themPartVaoDanhSachRender(rimPart,   matWheel, lkHienTai.id, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
        }
        else if (lkHienTai.category == InspectCategory::TOGGLE_SWITCH) {
            // Hiển thị Hộp pin + 2 Cell pin + Công tắc bập bênh
            MaTran4x4 matBat = taoMaTranTinhTien(0.0f, 0.8f, 0.0f);
            for (const auto& part : lkHienTai.parts) {
                MaTran4x4 mPart = matBat;
                if (part.ten == "Rocker_Button") {
                    float gocNghiengRad = (g_congTacPinBat ? 15.0f : -15.0f) * (float)M_PI / 180.0f;
                    mPart = taoMaTranQuayZ(gocNghiengRad)
                          * taoMaTranTinhTien(part.partExplodeVec.x * g_ui.tyLeTachRoi,
                                              part.partExplodeVec.y * g_ui.tyLeTachRoi,
                                              part.partExplodeVec.z * g_ui.tyLeTachRoi)
                          * part.bienDoiCucBo * matBat;
                } else {
                    mPart = taoMaTranTinhTien(part.partExplodeVec.x * g_ui.tyLeTachRoi,
                                              part.partExplodeVec.y * g_ui.tyLeTachRoi,
                                              part.partExplodeVec.z * g_ui.tyLeTachRoi)
                          * part.bienDoiCucBo * matBat;
                }
                themPartVaoDanhSachRender(part, mPart, lkHienTai.id, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }
        }
        else {
            // Linh kiện tĩnh: ARD, L298N, HCSR04, BUCK, DOMINO
            MaTran4x4 matBench = taoMaTranTinhTien(0.0f, 0.2f, 0.0f);
            for (const auto& part : lkHienTai.parts) {
                MaTran4x4 mPart = taoMaTranTinhTien(part.partExplodeVec.x * g_ui.tyLeTachRoi,
                                                    part.partExplodeVec.y * g_ui.tyLeTachRoi,
                                                    part.partExplodeVec.z * g_ui.tyLeTachRoi)
                                * part.bienDoiCucBo * matBench;
                themPartVaoDanhSachRender(part, mPart, lkHienTai.id, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }
        }

        // 5. Đường gióng lắp ráp nét đứt (Explode Assembly Guide Lines) - Mục B.2
        if (g_ui.tyLeTachRoi > 0.05f) {
            MaTran4x4 matBaseAnchor;
            if (lkHienTai.category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                matBaseAnchor = taoMaTranTinhTien(0.0f, 2.5f, 0.0f);
            } else if (lkHienTai.category == InspectCategory::TOGGLE_SWITCH) {
                matBaseAnchor = taoMaTranTinhTien(0.0f, 0.8f, 0.0f);
            } else if (lkHienTai.category != InspectCategory::ARM_MANIPULATOR) {
                matBaseAnchor = taoMaTranTinhTien(0.0f, 0.2f, 0.0f);
            }

            for (const auto& part : lkHienTai.parts) {
                if (part.isLeadAnchor && part.partExplodeVec.doDai() > 0.01f) {
                    Diem3D pBase = matBaseAnchor.bienDoi(part.anchorOnPCB);
                    Diem3D pExp  = matBaseAnchor.bienDoi(part.anchorOnPCB + part.partExplodeVec * g_ui.tyLeTachRoi);

                    Diem3D v1 = chuyenHeToaDoQuanSat(pBase, camTarget, thetaRad, phiRad, R_cam);
                    Diem3D v2 = chuyenHeToaDoQuanSat(pExp,  camTarget, thetaRad, phiRad, R_cam);

                    Diem2D s1 = chieuLenManHinh(v1, D_proj, true, cx, cy);
                    Diem2D s2 = chieuLenManHinh(v2, D_proj, true, cx, cy);

                    Diem2D cA = s1, cB = s2;
                    if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                        PhanTuCanVe3D pt;
                        pt.kieu = PHAN_TU_DOAN_DAY;
                        pt.pA = cA;
                        pt.pB = cB;
                        pt.doSauZ = (v1.z + v2.z) * 0.5f;
                        pt.mauSac = MauSac(220, 190, 60); // Màu vàng hổ phách kỹ thuật
                        pt.doDay = 2;
                        pt.laKhungDay = false;
                        danhSachPhanTu.push_back(pt);
                    }
                }
            }
        }
    } else {
        // =====================================================================
        // CHẾ ĐỘ 1 (TỔNG THỂ) & CHẾ ĐỘ 3 (BÊN TRONG XE): RENDER TOÀN BỘ XE
        // =====================================================================

        // --- 1. VẼ LƯỚI MẶT SÀN ---
        MauSac mauLuoi = BangMau::VIEN_PANEL.lamToi(0.55f);
        for (int i = -16; i <= 16; i += 2) {
            Diem3D pA1((float)i, 0.0f, -16.0f), pA2((float)i, 0.0f, 16.0f);
            Diem3D vA1 = chuyenHeToaDoQuanSat(pA1, camTarget, thetaRad, phiRad, R_cam);
            Diem3D vA2 = chuyenHeToaDoQuanSat(pA2, camTarget, thetaRad, phiRad, R_cam);
            Diem2D sA1 = chieuLenManHinh(vA1, D_proj, true, cx, cy);
            Diem2D sA2 = chieuLenManHinh(vA2, D_proj, true, cx, cy);

            Diem2D cA = sA1, cB = sA2;
            if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, mauLuoi);
            }

            Diem3D pB1(-16.0f, 0.0f, (float)i), pB2(16.0f, 0.0f, (float)i);
            Diem3D vB1 = chuyenHeToaDoQuanSat(pB1, camTarget, thetaRad, phiRad, R_cam);
            Diem3D vB2 = chuyenHeToaDoQuanSat(pB2, camTarget, thetaRad, phiRad, R_cam);
            Diem2D sB1 = chieuLenManHinh(vB1, D_proj, true, cx, cy);
            Diem2D sB2 = chieuLenManHinh(vB2, D_proj, true, cx, cy);

            cA = sB1; cB = sB2;
            if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, mauLuoi);
            }
        }

        // --- 2. VẬT THỂ TƯƠNG TÁC (KHỐI HÀNG HÓA MÀU XANH NGỌC) ---
        Part partVatThe;
        partVatThe.ten = "VatThe";
        partVatThe.mesh = taoKhoiHop(g_vatThe.kichThuoc, g_vatThe.kichThuoc, g_vatThe.kichThuoc, g_vatThe.mauSac);
        partVatThe.bienDoiCucBo = taoMaTranTinhTien(g_vatThe.viTriTheGioi.x, g_vatThe.viTriTheGioi.y, g_vatThe.viTriTheGioi.z);
        themPartVaoDanhSachRender(partVatThe, MaTran4x4(), -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);

        // --- 3. KHUNG XE 2 TẦNG ACRYLIC (PHẦN B MỤC 1) ---
        if (g_ui.cheDoHienTai != CHE_DO_BEN_TRONG) {
            // Tầng 1 (LowerDeck): y = 2.2 cm
            themPartVaoDanhSachRender(g_khungXe.lowerDeck, matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);

            // Tầng 2 (UpperDeck): y = 6.5 cm (Tách rời theo trục Y trong Exploded View)
            MaTran4x4 matUpper = matWorldCar;
            if (g_ui.tyLeTachRoi > 1e-4f) {
                matUpper = taoMaTranTinhTien(0.0f, g_ui.tyLeTachRoi * 5.5f, 0.0f) * matWorldCar;
            }
            themPartVaoDanhSachRender(g_khungXe.upperDeck, matUpper, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);

            // 4 Trụ đồng lục giác Standoff
            for (const auto& st : g_khungXe.standoffs) {
                themPartVaoDanhSachRender(st, matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }
        }

        // Máng cáp trung tâm (Central Cable Duct)
        themPartVaoDanhSachRender(g_khungXe.cableDuct, matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);

        // 2 Bánh xe và mâm xe
        themPartVaoDanhSachRender(g_khungXe.wheelL, matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
        themPartVaoDanhSachRender(g_khungXe.rimL,   matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
        themPartVaoDanhSachRender(g_khungXe.wheelR, matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
        themPartVaoDanhSachRender(g_khungXe.rimR,   matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);

        // Bánh mắt trâu caster
        themPartVaoDanhSachRender(g_khungXe.caster, matWorldCar, -1, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);

        // --- 4. THU THẬP CÁC MẶT CỦA 11 LINH KIỆN VÀO DANH SÁCH RENDER ---
        for (int lkId = 1; lkId <= TONG_SO_LINH_KIEN; ++lkId) {
            const auto& lk = g_dsLinhKien[lkId];
            if (!lk.hienThi) continue;
            if (g_ui.coLapLinhKien && lkId != g_ui.linhKienDangChon) continue;

            MaTran4x4 matWorldLK = tinhMaTranTheGioiLinhKien(lk, matWorldCar, g_ui.tyLeTachRoi);

            for (const auto& part : lk.parts) {
                themPartVaoDanhSachRender(part, matWorldLK, lkId, camTarget, thetaRad, phiRad, R_cam, D_proj, cx, cy, wmin, wmax, huongSang, danhSachPhanTu);
            }
        }

        // --- 5. ĐỊNH TUYẾN & THU THẬP 34 ĐƯỜNG DÂY DẪN 3D (PHẦN B MỤC 4) ---
        if (g_ui.hienThiDayDien) {
            for (size_t i = 0; i < g_bangDay.size(); ++i) {
                const auto& day = g_bangDay[i];

                // Bỏ qua dây nếu một trong hai linh kiện bị ẩn
                if (!g_dsLinhKien[day.idLKA].hienThi || !g_dsLinhKien[day.idLKB].hienThi) continue;
                if (g_ui.coLapLinhKien && (day.idLKA != g_ui.linhKienDangChon && day.idLKB != g_ui.linhKienDangChon)) continue;

                Diem3D localA, localB;
                if (!g_dsLinhKien[day.idLKA].timChan(day.chanA, localA)) continue;
                if (!g_dsLinhKien[day.idLKB].timChan(day.chanB, localB)) continue;

                MaTran4x4 matA = tinhMaTranTheGioiLinhKien(g_dsLinhKien[day.idLKA], matWorldCar, g_ui.tyLeTachRoi);
                MaTran4x4 matB = tinhMaTranTheGioiLinhKien(g_dsLinhKien[day.idLKB], matWorldCar, g_ui.tyLeTachRoi);

                Diem3D posWorldA = matA.bienDoi(localA);
                Diem3D posWorldB = matB.bienDoi(localB);

                // Sinh chuỗi điểm định tuyến theo máng cáp / bám xương cánh tay & Bézier bậc 2
                std::vector<Diem3D> waypoints = taoDuongDiDay3DNangCap(
                    posWorldA, posWorldB, day.idLKA, day.idLKB, (int)i + 1,
                    g_tayGap.matWorldWrist, g_tayGap.matWorldElbow, g_tayGap.matWorldBase, matWorldCar
                );

                // Xác định màu sắc và độ dày hiển thị
                bool lienQuanLK = (g_ui.linhKienDangChon > 0 &&
                                  (day.idLKA == g_ui.linhKienDangChon || day.idLKB == g_ui.linhKienDangChon));
                MauSac mauDayRender = day.mauDay;
                int doDayRender = 2;

                if (g_ui.linhKienDangChon > 0) {
                    if (lienQuanLK) {
                        mauDayRender = day.mauDay.lamSang(1.25f);
                        doDayRender = 3;
                    } else {
                        mauDayRender = day.mauDay.lamToi(0.35f);
                        doDayRender = 1;
                    }
                } else {
                    mauDayRender = day.mauDay.lamToi(0.85f);
                }

                // Chiếu từng đoạn nhỏ và đưa vào danh sách Painter's Algorithm
                for (size_t seg = 0; seg + 1 < waypoints.size(); ++seg) {
                    Diem3D p1 = waypoints[seg];
                    Diem3D p2 = waypoints[seg + 1];

                    Diem3D v1 = chuyenHeToaDoQuanSat(p1, camTarget, thetaRad, phiRad, R_cam);
                    Diem3D v2 = chuyenHeToaDoQuanSat(p2, camTarget, thetaRad, phiRad, R_cam);

                    Diem2D s1 = chieuLenManHinh(v1, D_proj, true, cx, cy);
                    Diem2D s2 = chieuLenManHinh(v2, D_proj, true, cx, cy);

                    Diem2D cA = s1, cB = s2;
                    if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                        PhanTuCanVe3D pt;
                        pt.kieu = PHAN_TU_DOAN_DAY;
                        pt.pA = cA;
                        pt.pB = cB;
                        pt.doSauZ = (v1.z + v2.z) * 0.5f;
                        pt.mauSac = mauDayRender;
                        pt.doDay = doDayRender;
                        pt.laKhungDay = false;
                        danhSachPhanTu.push_back(pt);
                    }
                }

                // Vẽ chân cắm chấm tròn nếu được bật
                if (g_ui.hienThiNhanChan) {
                    Diem3D vA = chuyenHeToaDoQuanSat(posWorldA, camTarget, thetaRad, phiRad, R_cam);
                    Diem2D sA = chieuLenManHinh(vA, D_proj, true, cx, cy);
                    if (sA.x >= wmin.x && sA.x <= wmax.x && sA.y >= wmin.y && sA.y <= wmax.y) {
                        veHinhTronDac((int)sA.x, (int)sA.y, lienQuanLK ? 4 : 2, lienQuanLK ? MauSac(255, 230, 80) : day.mauDay);
                    }
                }
            }
        }
    }

    // --- 6. GLOBAL PAINTER'S ALGORITHM: SẮP XẾP TẤT CẢ MẶT & ĐOẠN DÂY THEO ĐỘ SÂU Z GIẢM DẦN ---
    std::sort(danhSachPhanTu.begin(), danhSachPhanTu.end(), [](const PhanTuCanVe3D& a, const PhanTuCanVe3D& b) {
        return a.doSauZ > b.doSauZ; // Phần tử xa vẽ trước, phần tử gần vẽ sau đè lên
    });

    // Vẽ toàn bộ các phần tử đã sắp xếp
    for (const auto& pt : danhSachPhanTu) {
        if (pt.kieu == PHAN_TU_MAT) {
            if (!pt.laKhungDay) {
                toMauScanline(pt.poly2D, pt.mauSac);
            }
            // Vẽ đường viền cạnh mảnh phân tách khối
            MauSac mauVien = pt.mauSac.lamToi(0.40f);
            int n = (int)pt.poly2D.size();
            for (int k = 0; k < n; ++k) {
                Diem2D pA = pt.poly2D[k], pB = pt.poly2D[(k + 1) % n];
                veDoanThangBresenham((int)pA.x, (int)pA.y, (int)pB.x, (int)pB.y, mauVien);
            }
        } else if (pt.kieu == PHAN_TU_DOAN_DAY) {
            veDoanThangDay((int)pt.pA.x, (int)pt.pA.y, (int)pt.pB.x, (int)pt.pB.y, pt.mauSac, pt.doDay);
        }
    }
}

// =========================================================================
// RENDER SƠ ĐỒ NGUYÊN LÝ 2D (CHẾ ĐỘ 4)
// Áp dụng Ma trận 3x3 (Pan/Zoom/Xoay/Lật), Cohen-Sutherland, Sutherland-Hodgman
// và Tô sáng linh kiện được chọn bằng Boundary Fill dùng Stack (Mục 6)
// =========================================================================

void renderSoDo2D() {
    datVungXen(VIEWPORT_X_MIN, VIEWPORT_X_MAX, VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);

    float cx = (VIEWPORT_X_MIN + VIEWPORT_X_MAX) * 0.5f;
    float cy = (VIEWPORT_Y_MIN + VIEWPORT_Y_MAX) * 0.5f;

    Diem2D wmin(VIEWPORT_X_MIN + 2, VIEWPORT_Y_MIN + 2);
    Diem2D wmax(VIEWPORT_X_MAX - 2, VIEWPORT_Y_MAX - 2);

    // Tính ma trận tổng hợp 2D 3x3:
    // M_diagram2D = Pan * Center * Rotate * Scale * Flip * Uncenter
    MaTran3x3 mat2D = taoMaTranTinhTien2D(g_ui.soDo2DPanX, g_ui.soDo2DPanY)
                    * taoMaTranQuayQuanhTam2D(cx, cy, g_ui.soDo2DGocRad)
                    * taoMaTranTiLeQuanhTam2D(cx, cy, g_ui.soDo2DZoom, g_ui.soDo2DZoom)
                    * taoMaTranDoiXung2D(cx, cy, g_ui.soDo2DLatNgang, g_ui.soDo2DLatDoc);

    // 1. Vẽ 34 sợi dây nối trên sơ đồ 2D (xén bằng Cohen-Sutherland)
    for (size_t i = 0; i < g_bangDay.size(); ++i) {
        const auto& day = g_bangDay[i];
        if (!g_dsLinhKien[day.idLKA].hienThi || !g_dsLinhKien[day.idLKB].hienThi) continue;

        Diem2D pA = g_dsLinhKien[day.idLKA].toaDo2D;
        Diem2D pB = g_dsLinhKien[day.idLKB].toaDo2D;

        // Biến đổi qua ma trận 3x3
        Diem2D sA = mat2D.bienDoi(pA);
        Diem2D sB = mat2D.bienDoi(pB);

        Diem2D cA = sA, cB = sB;
        if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
            veDoanThangDay((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, day.mauDay, 2);
        }
    }

    // 2. Vẽ 11 khối hộp linh kiện 2D (xén bằng Sutherland-Hodgman)
    for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
        const auto& lk = g_dsLinhKien[i];
        if (!lk.hienThi) continue;

        float hw = lk.rong2D * 0.5f;
        float hh = lk.cao2D  * 0.5f;

        // 4 Đỉnh hình chữ nhật ban đầu
        std::vector<Diem2D> poly = {
            mat2D.bienDoi(Diem2D(lk.toaDo2D.x - hw, lk.toaDo2D.y - hh)),
            mat2D.bienDoi(Diem2D(lk.toaDo2D.x + hw, lk.toaDo2D.y - hh)),
            mat2D.bienDoi(Diem2D(lk.toaDo2D.x + hw, lk.toaDo2D.y + hh)),
            mat2D.bienDoi(Diem2D(lk.toaDo2D.x - hw, lk.toaDo2D.y + hh))
        };

        // Xén đa giác bằng Sutherland-Hodgman
        std::vector<Diem2D> polyXen = xenDaGiacSutherlandHodgman(poly, wmin, wmax);

        if (polyXen.size() >= 3) {
            // Tô màu khối linh kiện 2D
            MauSac mauNen = BangMau::NEN_PANEL.lamSang(1.2f);
            toMauScanline(polyXen, mauNen);

            // Vẽ viền bao quanh
            MauSac mauVien = (i == g_ui.linhKienDangChon) ? BangMau::VIEN_SANG : BangMau::VIEN_PANEL;
            int n = (int)polyXen.size();
            for (int k = 0; k < n; ++k) {
                Diem2D p1 = polyXen[k], p2 = polyXen[(k + 1) % n];
                veDoanThangBresenham((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, mauVien);
            }

            // NẾU LÀ LINH KIỆN ĐANG ĐƯỢC CHỌN:
            // Áp dụng THUẬT TOÁN TÔ LOANG BOUNDARY FILL DÙNG STACK (Mục 6.3 Đề cương)
            if (i == g_ui.linhKienDangChon) {
                // Tính trọng tâm (centroid) của đa giác đã xén làm điểm hạt giống seed
                float sumX = 0.0f, sumY = 0.0f;
                for (const auto& pt : polyXen) {
                    sumX += pt.x;
                    sumY += pt.y;
                }
                int seedX = (int)std::round(sumX / (float)polyXen.size());
                int seedY = (int)std::round(sumY / (float)polyXen.size());

                // Tô sáng bằng Boundary Fill Stack không đệ quy
                MauSac mauToLoang = MauSac(40, 110, 185); // Xanh neon highlight
                toLoangBoundaryFillStack(seedX, seedY, mauToLoang, mauVien);
            }

        }
    }
}

// =========================================================================
// HÀM VẼ NHÃN VĂN BẢN TRÊN CẢNH 3D VÀ SƠ ĐỒ 2D
// (Thực hiện ở Pass 2 sau khi đẩy FrameBuffer để đảm bảo chữ không bị đè)
// =========================================================================

// =========================================================================
// HÀM VẼ NHÃN VĂN BẢN TRÊN CẢNH 3D VÀ SƠ ĐỒ 2D
// Áp dụng kỹ thuật Callout Labels (Điểm mỏ neo + Đường dẫn + Hộp nhãn + Viền)
// (Thực hiện ở Pass 2 sau khi đẩy FrameBuffer để đảm bảo chữ không bị đè)
// =========================================================================

void veNhan3D() {
    float thetaRad = g_ui.camTheta * (float)M_PI / 180.0f;
    float phiRad   = g_ui.camPhi   * (float)M_PI / 180.0f;
    float R_cam    = g_ui.camDistHienTai;
    float D_proj   = 36.0f;
    float cx = (VIEWPORT_X_MIN + VIEWPORT_X_MAX) * 0.5f;
    float cy = (VIEWPORT_Y_MIN + VIEWPORT_Y_MAX) * 0.5f;

    Diem2D wmin(VIEWPORT_X_MIN + 2, VIEWPORT_Y_MIN + 2);
    Diem2D wmax(VIEWPORT_X_MAX - 2, VIEWPORT_Y_MAX - 2);

    Diem3D camTarget = g_ui.camTargetHienTai;
    if (g_ui.cheDoHienTai == CHE_DO_TONG_THE) {
        camTarget = Diem3D(g_xeRobot.x, 1.2f, g_xeRobot.z);
    }

    if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 1. Nhãn 3 trục tọa độ chuẩn Ox (Đỏ), Oy (Xanh lá), Oz (Xanh dương)
        struct TrucLabel { Diem3D p; const char* text; MauSac col; };
        std::vector<TrucLabel> labels = {
            {Diem3D(6.4f, 0.02f, 0.0f), "+X", MauSac(235, 55, 55)},
            {Diem3D(0.0f, 6.4f, 0.0f),  "+Y", MauSac(45, 215, 75)},
            {Diem3D(0.0f, 0.02f, 6.4f), "+Z", MauSac(50, 135, 245)}
        };
        for (const auto& lb : labels) {
            Diem3D v = chuyenHeToaDoQuanSat(lb.p, camTarget, thetaRad, phiRad, R_cam);
            Diem2D s = chieuLenManHinh(v, D_proj, true, cx, cy);
            if (s.x >= wmin.x + 10 && s.x <= wmax.x - 10 && s.y >= wmin.y + 10 && s.y <= wmax.y - 10) {
                veChu((int)s.x - 6, (int)s.y - 4, lb.text, lb.col, GLUT_BITMAP_HELVETICA_12);
            }
        }

        // 2. Nhãn Callout cho linh kiện đang chọn
        const auto& lk = g_dsLinhKien[g_ui.linhKienDangChon];
        float hComp = lk.kichThuocBao.y;
        Diem3D anchor(0.0f, hComp * 0.5f, 0.0f);
        Diem3D vC = chuyenHeToaDoQuanSat(anchor, camTarget, thetaRad, phiRad, R_cam);
        Diem2D sC = chieuLenManHinh(vC, D_proj, true, cx, cy);

        if (sC.x >= wmin.x + 10 && sC.x <= wmax.x - 10 && sC.y >= wmin.y + 10 && sC.y <= wmax.y - 10) {
            float lx = sC.x + 55.0f;
            float ly = sC.y + 35.0f;
            float boxW = 85.0f;
            float boxH = 22.0f;

            if (lx + boxW * 0.5f > wmax.x - 5) lx = wmax.x - 5 - boxW * 0.5f;
            if (ly + boxH * 0.5f > wmax.y - 5) ly = wmax.y - 5 - boxH * 0.5f;

            // Điểm mỏ neo
            glColor4ub(255, 210, 60, 230);
            glPointSize(5.0f);
            glBegin(GL_POINTS);
            glVertex2f(sC.x, sC.y);
            glEnd();

            // Đường chỉ dẫn leader line
            glLineWidth(1.6f);
            glColor4ub(255, 200, 50, 220);
            glBegin(GL_LINES);
            glVertex2f(sC.x, sC.y);
            glVertex2f(lx - boxW * 0.5f, ly);
            glEnd();

            // Hộp nền
            float x0 = lx - boxW * 0.5f, y0 = ly - boxH * 0.5f;
            float x1 = lx + boxW * 0.5f, y1 = ly + boxH * 0.5f;
            glColor4ub(16, 24, 38, 235);
            glBegin(GL_QUADS);
            glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
            glEnd();

            // Viền hộp
            glColor4ub(255, 204, 0, 255);
            glLineWidth(1.8f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
            glEnd();

            int textOffset = (int)(lk.ma.length() * 3.5f);
            veChu((int)lx - textOffset, (int)ly - 4, lk.ma.c_str(), BangMau::CANH_BAO_VANG, GLUT_BITMAP_HELVETICA_12);
        }

        // 3. Điểm neo và hiệu ứng cho các cụm tách rời khi Explode > 0.05f
        if (g_ui.tyLeTachRoi > 0.05f) {
            MaTran4x4 matBaseAnchor;
            if (lk.category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                matBaseAnchor = taoMaTranTinhTien(0.0f, 2.5f, 0.0f);
            } else if (lk.category == InspectCategory::TOGGLE_SWITCH) {
                matBaseAnchor = taoMaTranTinhTien(0.0f, 0.8f, 0.0f);
            } else if (lk.category != InspectCategory::ARM_MANIPULATOR) {
                matBaseAnchor = taoMaTranTinhTien(0.0f, 0.2f, 0.0f);
            }

            for (const auto& part : lk.parts) {
                if (part.isLeadAnchor && part.partExplodeVec.doDai() > 0.01f) {
                    Diem3D pBase = matBaseAnchor.bienDoi(part.anchorOnPCB);
                    Diem3D pExp  = matBaseAnchor.bienDoi(part.anchorOnPCB + part.partExplodeVec * g_ui.tyLeTachRoi);

                    Diem3D v1 = chuyenHeToaDoQuanSat(pBase, camTarget, thetaRad, phiRad, R_cam);
                    Diem3D v2 = chuyenHeToaDoQuanSat(pExp,  camTarget, thetaRad, phiRad, R_cam);

                    Diem2D s1 = chieuLenManHinh(v1, D_proj, true, cx, cy);
                    Diem2D s2 = chieuLenManHinh(v2, D_proj, true, cx, cy);

                    // Điểm neo trên PCB (vàng đồng)
                    if (s1.x >= wmin.x + 5 && s1.x <= wmax.x - 5 && s1.y >= wmin.y + 5 && s1.y <= wmax.y - 5) {
                        glColor4ub(220, 180, 50, 240);
                        glPointSize(5.0f);
                        glBegin(GL_POINTS);
                        glVertex2f(s1.x, s1.y);
                        glEnd();
                    }
                    // Điểm chân cụm nhấc lên (vàng hổ phách sáng)
                    if (s2.x >= wmin.x + 5 && s2.x <= wmax.x - 5 && s2.y >= wmin.y + 5 && s2.y <= wmax.y - 5) {
                        glColor4ub(255, 230, 80, 255);
                        glPointSize(6.0f);
                        glBegin(GL_POINTS);
                        glVertex2f(s2.x, s2.y);
                        glEnd();
                    }
                }
            }
        }

        glDisable(GL_BLEND);
        glPointSize(1.0f);
        glLineWidth(1.0f);
        return;
    }

    float radCar = g_xeRobot.gocHuong * (float)M_PI / 180.0f;
    MaTran4x4 matWorldCar = taoMaTranQuayY(radCar) * taoMaTranTinhTien(g_xeRobot.x, 0.0f, g_xeRobot.z);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int lkId = 1; lkId <= TONG_SO_LINH_KIEN; ++lkId) {
        const auto& lk = g_dsLinhKien[lkId];
        if (!lk.hienThi) continue;
        if (g_ui.coLapLinhKien && lkId != g_ui.linhKienDangChon) continue;

        MaTran4x4 matWorldLK = tinhMaTranTheGioiLinhKien(lk, matWorldCar, g_ui.tyLeTachRoi);
        Diem3D posCenter = matWorldLK.bienDoi(Diem3D(0, 0, 0));
        Diem3D vC = chuyenHeToaDoQuanSat(posCenter, camTarget, thetaRad, phiRad, R_cam);
        Diem2D sC = chieuLenManHinh(vC, D_proj, true, cx, cy);

        // Bỏ qua nếu mỏ neo nằm ngoài Center Viewport
        if (sC.x < wmin.x + 10 || sC.x > wmax.x - 10 || sC.y < wmin.y + 10 || sC.y > wmax.y - 10) {
            continue;
        }

        bool dangChon = (lkId == g_ui.linhKienDangChon);

        // Tọa độ hộp nhãn Callout dịch theo lk.labelOffset
        float lx = sC.x + lk.labelOffset.x;
        float ly = sC.y + lk.labelOffset.y;

        float boxW = 76.0f;
        float boxH = 20.0f;

        // Giữ hộp nhãn trong viewport
        if (lx - boxW * 0.5f < wmin.x + 5) lx = wmin.x + 5 + boxW * 0.5f;
        if (lx + boxW * 0.5f > wmax.x - 5) lx = wmax.x - 5 - boxW * 0.5f;
        if (ly - boxH * 0.5f < wmin.y + 5) ly = wmin.y + 5 + boxH * 0.5f;
        if (ly + boxH * 0.5f > wmax.y - 5) ly = wmax.y - 5 - boxH * 0.5f;

        // 1. Điểm mỏ neo (Anchor dot) tại tâm linh kiện
        glColor4ub(dangChon ? 255 : 0, dangChon ? 210 : 200, dangChon ? 60 : 255, 230);
        glPointSize(dangChon ? 5.0f : 3.5f);
        glBegin(GL_POINTS);
        glVertex2f(sC.x, sC.y);
        glEnd();

        // 2. Đường chỉ dẫn (Leader line) từ sC đến mép hộp nhãn
        float targetEdgeX = (lx > sC.x) ? (lx - boxW * 0.5f) : (lx + boxW * 0.5f);
        float targetEdgeY = ly;

        glLineWidth(dangChon ? 1.8f : 1.0f);
        glColor4ub(dangChon ? 255 : 70, dangChon ? 200 : 130, dangChon ? 50 : 180, dangChon ? 240 : 150);
        glBegin(GL_LINES);
        glVertex2f(sC.x, sC.y);
        glVertex2f(targetEdgeX, targetEdgeY);
        glEnd();

        // 3. Hộp nền Callout (Nền tối hơi mờ)
        float x0 = lx - boxW * 0.5f;
        float y0 = ly - boxH * 0.5f;
        float x1 = lx + boxW * 0.5f;
        float y1 = ly + boxH * 0.5f;

        glColor4ub(16, 24, 38, dangChon ? 240 : 205);
        glBegin(GL_QUADS);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();

        // 4. Viền hộp nhãn
        if (dangChon) {
            glColor4ub(255, 204, 0, 255);
            glLineWidth(1.8f);
        } else {
            glColor4ub(0, 168, 204, 180);
            glLineWidth(1.0f);
        }
        glBegin(GL_LINE_LOOP);
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
        glEnd();

        // 5. In chữ mã linh kiện căn giữa trong hộp
        MauSac textCol = dangChon ? BangMau::CANH_BAO_VANG : BangMau::CHU_CHINH;
        int textOffset = (int)(lk.ma.length() * 3.5f);
        veChu((int)lx - textOffset, (int)ly - 4, lk.ma.c_str(), textCol, GLUT_BITMAP_HELVETICA_12);
    }

    glDisable(GL_BLEND);
    glPointSize(1.0f);
    glLineWidth(1.0f);
}

void veNhan2D() {
    float cx = (VIEWPORT_X_MIN + VIEWPORT_X_MAX) * 0.5f;
    float cy = (VIEWPORT_Y_MIN + VIEWPORT_Y_MAX) * 0.5f;
    Diem2D wmin(VIEWPORT_X_MIN + 2, VIEWPORT_Y_MIN + 2);
    Diem2D wmax(VIEWPORT_X_MAX - 2, VIEWPORT_Y_MAX - 2);

    MaTran3x3 mat2D = taoMaTranTinhTien2D(g_ui.soDo2DPanX, g_ui.soDo2DPanY)
                    * taoMaTranQuayQuanhTam2D(cx, cy, g_ui.soDo2DGocRad)
                    * taoMaTranTiLeQuanhTam2D(cx, cy, g_ui.soDo2DZoom, g_ui.soDo2DZoom)
                    * taoMaTranDoiXung2D(cx, cy, g_ui.soDo2DLatNgang, g_ui.soDo2DLatDoc);

    for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
        const auto& lk = g_dsLinhKien[i];
        if (!lk.hienThi) continue;

        Diem2D tam = mat2D.bienDoi(lk.toaDo2D);
        if (tam.x >= wmin.x && tam.x <= wmax.x && tam.y >= wmin.y && tam.y <= wmax.y) {
            MauSac col = (i == g_ui.linhKienDangChon) ? BangMau::CANH_BAO_VANG : BangMau::CHU_CHINH;
            int offset = (int)(lk.ma.length() * 3.5f);
            veChu((int)tam.x - offset, (int)tam.y - 4, lk.ma.c_str(), col, GLUT_BITMAP_HELVETICA_12);
        }
    }
}

// =========================================================================
// HÀM HIỂN THỊ DISPLAY CALLBACK CHÍNH
// =========================================================================

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // BƯỚC 1: XÓA VÀ RENDER TẤT CẢ VÀO FRAMEBUFFER TRONG RAM
    xoaFrameBuffer(BangMau::NEN_CHINH);

    // Tính delta time
    auto thoiGianHienTai = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(thoiGianHienTai - g_thoiGianTruoc).count();
    g_thoiGianTruoc = thoiGianHienTai;
    if (dt > 0.1f) dt = 0.1f;

    // Cập nhật hoạt họa camera nội suy mượt (Camera Fly-to)
    g_ui.capNhatNoiSuyCamera(dt);

    // Render cảnh chính ở CenterViewport
    if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
        renderSoDo2D();
    } else {
        renderCanh3D(dt);
    }

    // Render các khối hình học của giao diện 5 vùng (Giai đoạn 4.2)
    datVungXen(0, CUA_SO_RONG - 1, 0, CUA_SO_CAO - 1);
    Diem3D posCar(g_xeRobot.x, 0.0f, g_xeRobot.z);
    float kcHCSR04 = (posCar - g_vatThe.viTriTheGioi).doDai();
    g_ui.veKhungGiaoDien(g_dsLinhKien, g_tayGap, kcHCSR04);

    // BƯỚC 2: ĐẨY TOÀN BỘ FRAMEBUFFER RA MÀN HÌNH TẠI GỐC (0, 0)
    renderFrameBuffer();

    // BƯỚC 3: VẼ TOÀN BỘ CHỮ (TEXT BITMAP) LÊN TRÊN CÙNG
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, CUA_SO_RONG, 0, CUA_SO_CAO);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Vẽ nhãn chữ các Panel, Nút bấm, Slider, Checkbox
    g_ui.veChuGiaoDien(g_dsLinhKien, g_tayGap, kcHCSR04);

    // Vẽ tên linh kiện trên khung nhìn
    if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
        veNhan2D();
    } else if (g_ui.hienThiNhanChan) {
        veNhan3D();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

// =========================================================================
// XỬ LÝ SỰ KIỆN CHUỘT VÀ BÀN PHÍM
// =========================================================================

void mouseButton(int button, int state, int x, int y) {
    // OpenGL tính tọa độ Y từ dưới lên, GLUT từ trên xuống
    int glY = CUA_SO_CAO - 1 - y;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            g_dangKeoChuotTrai = true;
            g_chuotCuX = x;
            g_chuotCuY = glY;

            // 1. Kiểm tra bấm vào 4 Tab trên TopBar
            for (size_t i = 0; i < g_ui.cacTabCheDo.size(); ++i) {
                if (g_ui.cacTabCheDo[i].kiemTraChuot(x, glY)) {
                    g_ui.cheDoHienTai = (CheDoHienThi)i;
                    if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
                        g_ui.chonLinhKien(g_ui.linhKienDangChon, g_dsLinhKien);
                    } else if (g_ui.cheDoHienTai == CHE_DO_TONG_THE) {
                        g_ui.camTargetMucTieu = Diem3D(g_xeRobot.x, 3.5f, g_xeRobot.z);
                        g_ui.camDistMucTieu = 34.0f;
                        g_ui.capNhatQuyenTuongTac(g_dsLinhKien);
                    } else if (g_ui.cheDoHienTai == CHE_DO_BEN_TRONG) {
                        g_ui.camTargetMucTieu = Diem3D(g_xeRobot.x, 2.5f, g_xeRobot.z);
                        g_ui.camDistMucTieu = 26.0f;
                        g_ui.capNhatQuyenTuongTac(g_dsLinhKien);
                    } else {
                        g_ui.capNhatQuyenTuongTac(g_dsLinhKien);
                    }
                    glutPostRedisplay();
                    return;
                }
            }

            // 2. Kiểm tra bấm chọn linh kiện trong LeftPanel
            if (x >= LEFT_PANEL_X_MIN && x <= LEFT_PANEL_X_MAX && glY >= LEFT_PANEL_Y_MIN && glY <= LEFT_PANEL_Y_MAX) {
                for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
                    int yPos = LEFT_PANEL_Y_MAX - 45 - (i - 1) * 52;
                    if (glY >= yPos - 12 && glY <= yPos + 28) {
                        g_ui.chonLinhKien(i, g_dsLinhKien);
                        glutPostRedisplay();
                        return;
                    }
                }
            }

            // 3. Kiểm tra bấm vào các nút điều khiển ở BottomBar
            for (size_t i = 0; i < g_ui.cacNutDieuKhienXe.size(); ++i) {
                if (g_ui.cacNutDieuKhienXe[i].kiemTraChuot(x, glY)) {
                    if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
                        const auto& lk = g_dsLinhKien[g_ui.linhKienDangChon];
                        if (lk.category == InspectCategory::ARM_MANIPULATOR) {
                            if (i == 0) { // Nút GẮP / THẢ
                                if (g_vatThe.dangBiGapTrenBan) g_vatThe.thaVatTrenBan(g_tayGap);
                                else g_vatThe.thuGapVatTrenBan(g_tayGap);
                            }
                        } else if (lk.category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                            if (i == 1) { // QUAY THUẬN
                                g_tocDoQuayMoTo = 360.0f;
                            } else if (i == 2) { // QUAY NGHỊCH
                                g_tocDoQuayMoTo = -360.0f;
                            } else if (i == 5) { // DỪNG MÔ-TƠ
                                g_tocDoQuayMoTo = 0.0f;
                            }
                        } else if (lk.category == InspectCategory::TOGGLE_SWITCH) {
                            if (i == 0) { // Nút CÔNG TẮC (Nút 0)
                                g_congTacPinBat = !g_congTacPinBat;
                            }
                        }
                        if (i == 6) { // Nút XOAY 45 (Nút 6) ở Chế độ 2
                            g_ui.camTheta += 45.0f;
                        } else if (i == 8) { // Nút RESET CAM (Nút 8) ở Chế độ 2
                            g_ui.camTheta = 45.0f;
                            g_ui.camPhi = 25.0f;
                            g_ui.tyLeTachRoi = 0.0f;
                            g_ui.chonLinhKien(g_ui.linhKienDangChon, g_dsLinhKien);
                        }
                    } else if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
                        if (i == 6) { // Xoay 2D 45 độ
                            g_ui.soDo2DGocRad += (float)M_PI * 0.25f;
                        } else if (i == 7) { // Lật ngang 2D
                            g_ui.soDo2DLatNgang = !g_ui.soDo2DLatNgang;
                        } else if (i == 8) { // Reset 2D
                            g_ui.soDo2DPanX = 0; g_ui.soDo2DPanY = 0;
                            g_ui.soDo2DZoom = 1.0f; g_ui.soDo2DGocRad = 0;
                            g_ui.soDo2DLatNgang = false;
                        }
                    } else { // Chế độ 1 (Tổng thể) & Chế độ 3 (Bên trong xe)
                        if (i == 0) { // Nút GẮP / THẢ
                            if (g_vatThe.dangBiGap) g_vatThe.thaVat(g_tayGap);
                            else g_vatThe.thuGapVat(g_tayGap);
                        } else if (i == 1) { // Tiến
                            g_xeRobot.vL += 18.0f; g_xeRobot.vR += 18.0f;
                        } else if (i == 2) { // Lùi
                            g_xeRobot.vL -= 18.0f; g_xeRobot.vR -= 18.0f;
                        } else if (i == 3) { // Trái
                            g_xeRobot.vL -= 14.0f; g_xeRobot.vR += 14.0f;
                        } else if (i == 4) { // Phải
                            g_xeRobot.vL += 14.0f; g_xeRobot.vR -= 14.0f;
                        } else if (i == 5) { // Dừng
                            g_xeRobot.vL = g_xeRobot.vR = 0.0f;
                        } else if (i == 8) { // Reset góc nhìn xe
                            g_ui.camTheta = 45.0f;
                            g_ui.camPhi = 25.0f;
                            g_ui.camTargetMucTieu = Diem3D(g_xeRobot.x, (g_ui.cheDoHienTai == CHE_DO_BEN_TRONG ? 2.5f : 3.5f), g_xeRobot.z);
                            g_ui.camDistMucTieu = (g_ui.cheDoHienTai == CHE_DO_BEN_TRONG ? 26.0f : 34.0f);
                        }
                    }
                    glutPostRedisplay();
                    return;
                }
            }

            // 4. Kiểm tra bấm vào Checkbox
            for (auto& hk : g_ui.cacHopKiem) {
                if (hk.kiemTraChuot(x, glY)) {
                    hk.toggle();
                    glutPostRedisplay();
                    return;
                }
            }

            // 5. Kiểm tra kéo thanh trượt Slider
            for (size_t i = 0; i < g_ui.cacThanhTruot.size(); ++i) {
                if (g_ui.cacThanhTruot[i].kiemTraChuot(x, glY)) {
                    g_sliderDangKeo = (int)i;
                    g_ui.cacThanhTruot[i].dangKeo = true;
                    g_ui.cacThanhTruot[i].capNhatTuChuot(x);
                    glutPostRedisplay();
                    return;
                }
            }

            // 6. Kiểm tra Click trong CenterViewport ở chế độ Sơ đồ 2D để chọn linh kiện (Hit-test)
            if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D &&
                x >= VIEWPORT_X_MIN && x <= VIEWPORT_X_MAX && glY >= VIEWPORT_Y_MIN && glY <= VIEWPORT_Y_MAX) {

                float cx = (VIEWPORT_X_MIN + VIEWPORT_X_MAX) * 0.5f;
                float cy = (VIEWPORT_Y_MIN + VIEWPORT_Y_MAX) * 0.5f;

                MaTran3x3 mat2D = taoMaTranTinhTien2D(g_ui.soDo2DPanX, g_ui.soDo2DPanY)
                                * taoMaTranQuayQuanhTam2D(cx, cy, g_ui.soDo2DGocRad)
                                * taoMaTranTiLeQuanhTam2D(cx, cy, g_ui.soDo2DZoom, g_ui.soDo2DZoom)
                                * taoMaTranDoiXung2D(cx, cy, g_ui.soDo2DLatNgang, g_ui.soDo2DLatDoc);
                MaTran3x3 invMat = mat2D.nghichDao();

                // Đưa tọa độ click chuột về không gian gốc của sơ đồ 2D (Mục 6.2 Đề cương)
                Diem2D pGoc = invMat.bienDoi(Diem2D((float)x, (float)glY));

                for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
                    const auto& lk = g_dsLinhKien[i];
                    float hw = lk.rong2D * 0.5f;
                    float hh = lk.cao2D  * 0.5f;
                    if (pGoc.x >= lk.toaDo2D.x - hw && pGoc.x <= lk.toaDo2D.x + hw &&
                        pGoc.y >= lk.toaDo2D.y - hh && pGoc.y <= lk.toaDo2D.y + hh) {
                        g_ui.chonLinhKien(i, g_dsLinhKien);
                        glutPostRedisplay();
                        return;
                    }
                }
            }
        } else {
            g_dangKeoChuotTrai = false;
            if (g_sliderDangKeo >= 0) {
                g_ui.cacThanhTruot[g_sliderDangKeo].dangKeo = false;
                g_sliderDangKeo = -1;
            }
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            g_dangKeoChuotPhai = true;
            g_chuotCuX = x;
            g_chuotCuY = glY;
        } else {
            g_dangKeoChuotPhai = false;
        }
    }
    // Con lăn chuột (Scroll) để Zoom
    else if (button == 3 && state == GLUT_DOWN) { // Lăn lên -> Zoom In
        if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
            g_ui.soDo2DZoom = std::min(3.5f, g_ui.soDo2DZoom + 0.12f);
        } else {
            g_ui.camDistHienTai = std::max(12.0f, g_ui.camDistHienTai - 1.8f);
        }
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) { // Lăn xuống -> Zoom Out
        if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
            g_ui.soDo2DZoom = std::max(0.4f, g_ui.soDo2DZoom - 0.12f);
        } else {
            g_ui.camDistHienTai = std::min(60.0f, g_ui.camDistHienTai + 1.8f);
        }
        glutPostRedisplay();
    }
}

void mouseMotion(int x, int y) {
    int glY = CUA_SO_CAO - 1 - y;
    int dx = x - g_chuotCuX;
    int dy = glY - g_chuotCuY;

    if (g_sliderDangKeo >= 0) {
        g_ui.cacThanhTruot[g_sliderDangKeo].capNhatTuChuot(x);
        glutPostRedisplay();
    } else if (g_dangKeoChuotPhai || (g_dangKeoChuotTrai && g_ui.cheDoHienTai != CHE_DO_SO_DO_2D && x >= VIEWPORT_X_MIN && x <= VIEWPORT_X_MAX)) {
        // Xoay Camera Orbit 360 độ quanh tâm
        g_ui.camTheta += dx * 0.45f;
        g_ui.camPhi   += dy * 0.35f;
        if (g_ui.camPhi >  85.0f) g_ui.camPhi =  85.0f;
        if (g_ui.camPhi < -15.0f) g_ui.camPhi = -15.0f;
        glutPostRedisplay();
    } else if (g_dangKeoChuotTrai && g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
        // Kéo Pan sơ đồ 2D
        g_ui.soDo2DPanX += dx;
        g_ui.soDo2DPanY += dy;
        glutPostRedisplay();
    }

    g_chuotCuX = x;
    g_chuotCuY = glY;
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        // Điều khiển xe hoặc mô-tơ độc lập bằng phím
        case 'w': case 'W':
            if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
                if (g_dsLinhKien[g_ui.linhKienDangChon].category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                    g_tocDoQuayMoTo = 360.0f;
                }
            } else {
                g_xeRobot.vL += 18.0f; g_xeRobot.vR += 18.0f;
            }
            break;
        case 's': case 'S':
            if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
                if (g_dsLinhKien[g_ui.linhKienDangChon].category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                    g_tocDoQuayMoTo = -360.0f;
                }
            } else {
                g_xeRobot.vL -= 18.0f; g_xeRobot.vR -= 18.0f;
            }
            break;
        case 'a': case 'A':
            if (g_ui.cheDoHienTai != CHE_DO_LINH_KIEN) {
                g_xeRobot.vL -= 14.0f; g_xeRobot.vR += 14.0f;
            }
            break;
        case 'd': case 'D':
            if (g_ui.cheDoHienTai != CHE_DO_LINH_KIEN) {
                g_xeRobot.vL += 14.0f; g_xeRobot.vR -= 14.0f;
            }
            break;
        case 'x': case 'X':
            if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
                if (g_dsLinhKien[g_ui.linhKienDangChon].category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                    g_tocDoQuayMoTo = 0.0f;
                }
            } else {
                g_xeRobot.vL = g_xeRobot.vR = 0.0f;
            }
            break;

        // Phím tắt đổi 4 chế độ 1..4
        case '1':
            g_ui.cheDoHienTai = CHE_DO_TONG_THE;
            g_ui.camTargetMucTieu = Diem3D(g_xeRobot.x, 3.5f, g_xeRobot.z);
            g_ui.camDistMucTieu = 34.0f;
            g_ui.capNhatQuyenTuongTac(g_dsLinhKien);
            break;
        case '2':
            g_ui.cheDoHienTai = CHE_DO_LINH_KIEN;
            g_ui.chonLinhKien(g_ui.linhKienDangChon, g_dsLinhKien);
            break;
        case '3':
            g_ui.cheDoHienTai = CHE_DO_BEN_TRONG;
            g_ui.camTargetMucTieu = Diem3D(g_xeRobot.x, 2.5f, g_xeRobot.z);
            g_ui.camDistMucTieu = 26.0f;
            g_ui.capNhatQuyenTuongTac(g_dsLinhKien);
            break;
        case '4':
            g_ui.cheDoHienTai = CHE_DO_SO_DO_2D;
            g_ui.capNhatQuyenTuongTac(g_dsLinhKien);
            break;

        // Phím E: Bật/Tắt bóc tách không gian (Exploded View)
        case 'e': case 'E':
            g_ui.tyLeTachRoi = (g_ui.tyLeTachRoi > 0.5f) ? 0.0f : 1.0f;
            break;

        // Phím G: Gắp / Thả vật thể
        case 'g': case 'G':
            if (g_ui.cheDoHienTai == CHE_DO_LINH_KIEN) {
                const auto& lk = g_dsLinhKien[g_ui.linhKienDangChon];
                if (lk.category == InspectCategory::ARM_MANIPULATOR) {
                    if (g_vatThe.dangBiGapTrenBan) g_vatThe.thaVatTrenBan(g_tayGap);
                    else g_vatThe.thuGapVatTrenBan(g_tayGap);
                }
            } else {
                if (g_vatThe.dangBiGap) g_vatThe.thaVat(g_tayGap);
                else g_vatThe.thuGapVat(g_tayGap);
            }
            break;

        case 27: // ESC
            exit(0);
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    if (w != CUA_SO_RONG || h != CUA_SO_CAO) {
        glutReshapeWindow(CUA_SO_RONG, CUA_SO_CAO);
    }
    glViewport(0, 0, CUA_SO_RONG, CUA_SO_CAO);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, CUA_SO_RONG, 0, CUA_SO_CAO);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// =========================================================================
// HÀM MAIN KHỞI ĐỘNG CHƯƠNG TRÌNH
// =========================================================================

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(CUA_SO_RONG, CUA_SO_CAO);
    glutInitWindowPosition(50, 25);
    glutCreateWindow("Mo Phong 3D & So Do Dau Noi 2D Xe Robot Tay Gap Arduino (11 Linh Kien - 34 Day Noi) - VKU");

    // Khởi tạo các hệ thống
    khoiTao11LinhKien(g_dsLinhKien);
    g_khungXe = taoKhungXe2Tang();
    g_bangDay = khoiTao34DayDien();
    g_tayGap.khoiTao();
    g_ui.khoiTao(g_tayGap);

    // Kiểm tra tính toàn vẹn 34 dây nối (Mục 5.3)
    kiemTraToanVenDay(g_bangDay, g_dsLinhKien);

    // Đăng ký callback GLUT
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
