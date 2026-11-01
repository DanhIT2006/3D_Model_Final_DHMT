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

// Hệ thống mô phỏng động học
DongHocXe       g_xeRobot;
CungCapTayGap   g_tayGap;
VatTheTuongTac  g_vatThe;

// Quản lý giao diện UI
TrinhQuanLyGiaoDien g_ui;

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
// Áp dụng Back-face Culling, Painter's Algorithm, Sutherland-Hodgman & Scanline
// =========================================================================

struct MatCanVe3D {
    std::vector<Diem2D> poly2D;
    float doSauZ;
    MauSac mauSac;
    bool laKhungDay;
};

void renderCanh3D(float dt) {
    datVungXen(VIEWPORT_X_MIN, VIEWPORT_X_MAX, VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);

    // Cập nhật động học xe
    g_xeRobot.capNhat(dt);

    // Ma trận thế giới gốc của xe: M_world_car = R(y, theta_car) * Tr(x_car, 0, z_car)
    float radCar = g_xeRobot.gocHuong * (float)M_PI / 180.0f;
    MaTran4x4 matWorldCar = taoMaTranQuayY(radCar) * taoMaTranTinhTien(g_xeRobot.x, 0.0f, g_xeRobot.z);

    // Cập nhật phân cấp cánh tay robot
    g_tayGap.capNhat(dt, matWorldCar);

    // Cập nhật khối vật thể
    g_vatThe.capNhat(g_tayGap);

    // Tính tâm quan sát camera và ma trận chiếu
    Diem3D camTarget = g_ui.camTargetHienTai;
    if (g_ui.cheDoHienTai == CHE_DO_TONG_THE) {
        // Camera theo dõi trọng tâm xe
        camTarget = Diem3D(g_xeRobot.x, 1.2f, g_xeRobot.z);
    }

    float thetaRad = g_ui.camTheta * (float)M_PI / 180.0f;
    float phiRad   = g_ui.camPhi   * (float)M_PI / 180.0f;
    float R_cam    = g_ui.camDistHienTai;
    float D_proj   = 36.0f;
    float cx = (VIEWPORT_X_MIN + VIEWPORT_X_MAX) * 0.5f;
    float cy = (VIEWPORT_Y_MIN + VIEWPORT_Y_MAX) * 0.5f;

    Diem2D wmin(VIEWPORT_X_MIN + 2, VIEWPORT_Y_MIN + 2);
    Diem2D wmax(VIEWPORT_X_MAX - 2, VIEWPORT_Y_MAX - 2);

    std::vector<MatCanVe3D> danhSachMat;

    // --- 1. VẼ LƯỚI MẶT SÀN ---
    MauSac mauLuoi = BangMau::VIEN_PANEL.lamToi(0.6f);
    for (int i = -14; i <= 14; i += 2) {
        Diem3D pA1((float)i, 0.0f, -14.0f), pA2((float)i, 0.0f, 14.0f);
        Diem3D vA1 = chuyenHeToaDoQuanSat(pA1, camTarget, thetaRad, phiRad, R_cam);
        Diem3D vA2 = chuyenHeToaDoQuanSat(pA2, camTarget, thetaRad, phiRad, R_cam);
        Diem2D sA1 = chieuLenManHinh(vA1, D_proj, true, cx, cy);
        Diem2D sA2 = chieuLenManHinh(vA2, D_proj, true, cx, cy);

        Diem2D cA, cB;
        cA = sA1; cB = sA2;
        if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
            veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, mauLuoi);
        }

        Diem3D pB1(-14.0f, 0.0f, (float)i), pB2(14.0f, 0.0f, (float)i);
        Diem3D vB1 = chuyenHeToaDoQuanSat(pB1, camTarget, thetaRad, phiRad, R_cam);
        Diem3D vB2 = chuyenHeToaDoQuanSat(pB2, camTarget, thetaRad, phiRad, R_cam);
        Diem2D sB1 = chieuLenManHinh(vB1, D_proj, true, cx, cy);
        Diem2D sB2 = chieuLenManHinh(vB2, D_proj, true, cx, cy);

        cA = sB1; cB = sB2;
        if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
            veDoanThangBresenham((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, mauLuoi);
        }
    }

    // --- 2. VẼ KHỐI VẬT THỂ TƯƠNG TÁC (NẾU CHƯA BỊ GẮP HOẶC ĐANG GẮP) ---
    Mesh3D meshVatThe = taoKhoiHop(g_vatThe.kichThuoc, g_vatThe.kichThuoc, g_vatThe.kichThuoc, g_vatThe.mauSac);
    MaTran4x4 matWorldVatThe = taoMaTranTinhTien(g_vatThe.viTriTheGioi.x, g_vatThe.viTriTheGioi.y, g_vatThe.viTriTheGioi.z);

    // --- 3. THU THẬP CÁC MẶT CỦA 11 LINH KIỆN VÀ KHUNG XE VÀO DANH SÁCH RENDER ---
    for (int lkId = 1; lkId <= TONG_SO_LINH_KIEN; ++lkId) {
        const auto& lk = g_dsLinhKien[lkId];

        // Kiểm tra ẩn/hiện và cô lập
        if (!lk.hienThi) continue;
        if (g_ui.coLapLinhKien && lkId != g_ui.linhKienDangChon) continue;

        // Chế độ 3: Bên trong xe -> Ẩn khung gầm xe để thấy rõ dây dẫn và linh kiện bên trong
        if (g_ui.cheDoHienTai == CHE_DO_BEN_TRONG && lkId == LK_DOMINO) {
            // Vẫn giữ linh kiện bên trong
        }

        MaTran4x4 matWorldLK = tinhMaTranTheGioiLinhKien(lk, matWorldCar, g_ui.tyLeTachRoi);

        for (const auto& part : lk.parts) {
            MaTran4x4 matPart = part.bienDoiCucBo * matWorldLK;

            // Biến đổi đỉnh sang hệ tọa độ quan sát và chiếu sang màn hình
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

            // Xử lý từng mặt đa giác của Part
            for (const auto& f : part.mesh.mat) {
                if (f.cacDinh.size() < 3) continue;

                // Back-face Culling trên mặt phẳng chiếu 2D (Mục 1.3 Đề cương)
                Diem2D p0 = vScreen[f.cacDinh[0]];
                Diem2D p1 = vScreen[f.cacDinh[1]];
                Diem2D p2 = vScreen[f.cacDinh[2]];
                float cross2D = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);

                if (cross2D <= 0 && !g_ui.cheDoKhungDay) continue;

                // Tính độ sâu trung bình zAvg phục vụ Painter's Algorithm
                float sumZ = 0.0f;
                std::vector<Diem2D> poly2D;
                for (int idx : f.cacDinh) {
                    sumZ += vView[idx].z;
                    poly2D.push_back(vScreen[idx]);
                }
                float zAvg = sumZ / (float)f.cacDinh.size();

                // Xén đa giác theo hình chữ nhật CenterViewport (Sutherland-Hodgman)
                poly2D = xenDaGiacSutherlandHodgman(poly2D, wmin, wmax);

                if (poly2D.size() >= 3) {
                    MauSac mauMat = f.mauSac;
                    if (lkId == g_ui.linhKienDangChon) {
                        mauMat = mauMat.lamSang(1.35f); // Làm sáng linh kiện đang chọn
                    }
                    danhSachMat.push_back({poly2D, zAvg, mauMat, g_ui.cheDoKhungDay});
                }
            }
        }
    }

    // Thêm các mặt của vật thể tương tác
    {
        std::vector<Diem3D> vWorld, vView;
        std::vector<Diem2D> vScreen;
        for (const auto& d : meshVatThe.dinh) {
            Diem3D pw = matWorldVatThe.bienDoi(d);
            Diem3D pv = chuyenHeToaDoQuanSat(pw, camTarget, thetaRad, phiRad, R_cam);
            Diem2D ps = chieuLenManHinh(pv, D_proj, true, cx, cy);
            vWorld.push_back(pw);
            vView.push_back(pv);
            vScreen.push_back(ps);
        }
        for (const auto& f : meshVatThe.mat) {
            Diem2D p0 = vScreen[f.cacDinh[0]], p1 = vScreen[f.cacDinh[1]], p2 = vScreen[f.cacDinh[2]];
            float cross2D = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
            if (cross2D <= 0 && !g_ui.cheDoKhungDay) continue;

            float sumZ = 0.0f;
            std::vector<Diem2D> poly2D;
            for (int idx : f.cacDinh) {
                sumZ += vView[idx].z;
                poly2D.push_back(vScreen[idx]);
            }
            float zAvg = sumZ / (float)f.cacDinh.size();
            poly2D = xenDaGiacSutherlandHodgman(poly2D, wmin, wmax);
            if (poly2D.size() >= 3) {
                danhSachMat.push_back({poly2D, zAvg, f.mauSac, g_ui.cheDoKhungDay});
            }
        }
    }

    // --- 4. THUẬT TOÁN HỌA SĨ (PAINTER'S ALGORITHM): SẮP XẾP MẶT THEO ĐỘ SÂU Z GIẢM DẦN ---
    std::sort(danhSachMat.begin(), danhSachMat.end(), [](const MatCanVe3D& a, const MatCanVe3D& b) {
        return a.doSauZ > b.doSauZ; // Mặt xa vẽ trước, mặt gần vẽ sau đè lên
    });

    // Vẽ và tô màu các mặt
    for (const auto& m : danhSachMat) {
        if (!m.laKhungDay) {
            toMauScanline(m.poly2D, m.mauSac);
        }
        // Vẽ viền cạnh
        MauSac mauVien = m.mauSac.lamToi(0.55f);
        int n = (int)m.poly2D.size();
        for (int i = 0; i < n; ++i) {
            Diem2D pA = m.poly2D[i], pB = m.poly2D[(i + 1) % n];
            veDoanThangBresenham((int)pA.x, (int)pA.y, (int)pB.x, (int)pB.y, mauVien);
        }
    }

    // --- 5. VẼ HỆ THỐNG DÂY ĐIỆN 3D & KIỂM TRA TỌA ĐỘ CHÂN THẾ GIỚI (MỤC 5.1 & 5.2) ---
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

            // Sinh 4 điểm đường dẫn gấp khúc (3D Waypoint Routing)
            std::vector<Diem3D> waypoints = taoDuongDiDay3D(posWorldA, posWorldB, (int)i);

            // Chiếu từng đoạn và xén bằng Cohen-Sutherland
            for (size_t seg = 0; seg + 1 < waypoints.size(); ++seg) {
                Diem3D v1 = chuyenHeToaDoQuanSat(waypoints[seg],     camTarget, thetaRad, phiRad, R_cam);
                Diem3D v2 = chuyenHeToaDoQuanSat(waypoints[seg + 1], camTarget, thetaRad, phiRad, R_cam);

                Diem2D s1 = chieuLenManHinh(v1, D_proj, true, cx, cy);
                Diem2D s2 = chieuLenManHinh(v2, D_proj, true, cx, cy);

                Diem2D cA = s1, cB = s2;
                if (xenDoanThangCohenSutherland(cA, cB, wmin, wmax)) {
                    veDoanThangDay((int)cA.x, (int)cA.y, (int)cB.x, (int)cB.y, day.mauDay, 2);
                }
            }

            // Vẽ chân cắm và nhãn chân nếu được bật
            if (g_ui.hienThiNhanChan) {
                Diem3D vA = chuyenHeToaDoQuanSat(posWorldA, camTarget, thetaRad, phiRad, R_cam);
                Diem2D sA = chieuLenManHinh(vA, D_proj, true, cx, cy);
                if (sA.x >= wmin.x && sA.x <= wmax.x && sA.y >= wmin.y && sA.y <= wmax.y) {
                    veHinhTronDac((int)sA.x, (int)sA.y, 3, MauSac(255, 230, 80));
                    veChu((int)sA.x + 4, (int)sA.y + 4, day.chanA.c_str(), BangMau::CHU_CHINH, GLUT_BITMAP_HELVETICA_10);
                }
            }
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

            // Vẽ tên linh kiện
            Diem2D tam = mat2D.bienDoi(lk.toaDo2D);
            if (tam.x >= wmin.x && tam.x <= wmax.x && tam.y >= wmin.y && tam.y <= wmax.y) {
                veChu((int)tam.x - 20, (int)tam.y - 4, lk.ma.c_str(), BangMau::CHU_CHINH, GLUT_BITMAP_HELVETICA_12);
            }
        }
    }
}

// =========================================================================
// HÀM HIỂN THỊ DISPLAY CALLBACK CHÍNH
// =========================================================================

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Xóa bộ đệm màu RAM
    xoaFrameBuffer(BangMau::NEN_CHINH);

    // Tính delta time
    auto thoiGianHienTai = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(thoiGianHienTai - g_thoiGianTruoc).count();
    g_thoiGianTruoc = thoiGianHienTai;
    if (dt > 0.1f) dt = 0.1f;

    // Cập nhật hoạt họa camera nội suy mượt (Camera Fly-to)
    g_ui.capNhatNoiSuyCamera(dt);

    // 1. Render cảnh chính ở CenterViewport
    if (g_ui.cheDoHienTai == CHE_DO_SO_DO_2D) {
        renderSoDo2D();
    } else {
        renderCanh3D(dt);
    }

    // 2. Render giao diện 5 vùng (Giai đoạn 4.2)
    // Đặt lại vùng xén toàn màn hình để vẽ Panel UI
    datVungXen(0, CUA_SO_RONG - 1, 0, CUA_SO_CAO - 1);

    // Tính khoảng cách cảm biến siêu âm HC-SR04 tới khối vật thể
    Diem3D posCar(g_xeRobot.x, 0.0f, g_xeRobot.z);
    float kcHCSR04 = (posCar - g_vatThe.viTriTheGioi).doDai();

    g_ui.veGiaoDien(g_dsLinhKien, g_tayGap, kcHCSR04);

    // Đẩy toàn bộ FrameBuffer ra OpenGL
    renderFrameBuffer();

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
                    } else if (i == 6) { // Xoay 2D 45 độ
                        g_ui.soDo2DGocRad += (float)M_PI * 0.25f;
                    } else if (i == 7) { // Lật ngang 2D
                        g_ui.soDo2DLatNgang = !g_ui.soDo2DLatNgang;
                    } else if (i == 8) { // Reset 2D
                        g_ui.soDo2DPanX = 0; g_ui.soDo2DPanY = 0;
                        g_ui.soDo2DZoom = 1.0f; g_ui.soDo2DGocRad = 0;
                        g_ui.soDo2DLatNgang = false;
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
        // Điều khiển xe bằng phím
        case 'w': case 'W': g_xeRobot.vL += 18.0f; g_xeRobot.vR += 18.0f; break;
        case 's': case 'S': g_xeRobot.vL -= 18.0f; g_xeRobot.vR -= 18.0f; break;
        case 'a': case 'A': g_xeRobot.vL -= 14.0f; g_xeRobot.vR += 14.0f; break;
        case 'd': case 'D': g_xeRobot.vL += 14.0f; g_xeRobot.vR -= 14.0f; break;
        case 'x': case 'X': g_xeRobot.vL = g_xeRobot.vR = 0.0f; break;

        // Phím tắt đổi 4 chế độ 1..4
        case '1': g_ui.cheDoHienTai = CHE_DO_TONG_THE; break;
        case '2': g_ui.cheDoHienTai = CHE_DO_LINH_KIEN; g_ui.chonLinhKien(g_ui.linhKienDangChon, g_dsLinhKien); break;
        case '3': g_ui.cheDoHienTai = CHE_DO_BEN_TRONG; break;
        case '4': g_ui.cheDoHienTai = CHE_DO_SO_DO_2D;  break;

        // Phím E: Bật/Tắt bóc tách không gian (Exploded View)
        case 'e': case 'E':
            g_ui.tyLeTachRoi = (g_ui.tyLeTachRoi > 0.5f) ? 0.0f : 1.0f;
            break;

        // Phím G: Gắp / Thả vật thể
        case 'g': case 'G':
            if (g_vatThe.dangBiGap) g_vatThe.thaVat(g_tayGap);
            else g_vatThe.thuGapVat(g_tayGap);
            break;

        case 27: // ESC
            exit(0);
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
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
