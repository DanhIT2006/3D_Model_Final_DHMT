#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "config.h"
#include "math_3d.h"
#include "rasterizer.h"
#include "clipping_fill.h"
#include "components_mesh.h"
#include "wiring_table.h"
#include "robot_kinematics.h"
#include <string>
#include <vector>
#include <algorithm>

// =========================================================================
// 1. CÁC THÀNH PHẦN ĐIỀU KHIỂN UI (BUTTON, SLIDER, CHECKBOX)
// =========================================================================

struct NutBamUI {
    int x0, y0, x1, y1;
    std::string nhan;
    bool dangHover = false;
    bool dangChon  = false;
    bool coTheTuongTac = true; // Phân quyền tương tác thông minh (Phần B Mục 2)

    bool kiemTraChuot(int mx, int my) const {
        return coTheTuongTac && (mx >= x0 && mx <= x1 && my >= y0 && my <= y1);
    }

    void veKhung(MauSac mauChinh = BangMau::NUT_BINH_THUONG) const {
        if (!coTheTuongTac) {
            // Nút bị khóa / vô hiệu hóa: màu xám chìm
            veHCN(x0, y0, x1, y1, MauSac(32, 36, 44));
            veVienHCN(x0, y0, x1, y1, MauSac(48, 54, 64));
            return;
        }
        MauSac mauNen = dangChon ? BangMau::NUT_ACTIVE : (dangHover ? BangMau::NUT_HOVER : mauChinh);
        veHCN(x0, y0, x1, y1, mauNen);
        veVienHCN(x0, y0, x1, y1, dangChon ? BangMau::VIEN_SANG : BangMau::VIEN_PANEL);
    }

    void veChuNhan() const {
        // Vẽ nhãn chữ ở giữa nút
        int textX = x0 + (x1 - x0) / 2 - (int)(nhan.length() * 3.5f);
        int textY = y0 + (y1 - y0) / 2 - 4;
        MauSac mauChu = coTheTuongTac ? BangMau::CHU_CHINH : MauSac(85, 92, 105);
        veChu(textX, textY, nhan.c_str(), mauChu, GLUT_BITMAP_HELVETICA_12);
    }
};

struct ThanhTruotUI {
    int x0, y0, x1, y1;
    float giaTriMin, giaTriMax;
    float* pGiaTri;
    std::string nhan;
    bool dangKeo = false;
    bool coTheTuongTac = true; // Phân quyền tương tác thông minh

    bool kiemTraChuot(int mx, int my) const {
        return coTheTuongTac && (mx >= x0 - 4 && mx <= x1 + 4 && my >= y0 - 6 && my <= y1 + 6);
    }

    void capNhatTuChuot(int mx) {
        if (!coTheTuongTac) return;
        float tyLe = (float)(mx - x0) / (float)(x1 - x0);
        tyLe = std::clamp(tyLe, 0.0f, 1.0f);
        if (pGiaTri) {
            *pGiaTri = giaTriMin + tyLe * (giaTriMax - giaTriMin);
        }
    }

    void veKhung() const {
        if (!pGiaTri) return;
        if (!coTheTuongTac) {
            // Rãnh trượt mờ khi bị khóa
            veHCN(x0, y0 + 3, x1, y0 + 7, MauSac(28, 32, 40));
            veVienHCN(x0, y0 + 3, x1, y0 + 7, MauSac(42, 48, 58));
            return;
        }
        // Rãnh trượt bình thường
        veHCN(x0, y0 + 3, x1, y0 + 7, MauSac(35, 42, 52));
        veVienHCN(x0, y0 + 3, x1, y0 + 7, BangMau::VIEN_PANEL);

        // Nút kéo (Thumb)
        float tyLe = (*pGiaTri - giaTriMin) / (giaTriMax - giaTriMin);
        int thumbX = x0 + (int)(tyLe * (x1 - x0));
        veHCN(thumbX - 6, y0 - 3, thumbX + 6, y1 + 3, dangKeo ? BangMau::VIEN_SANG : BangMau::NUT_ACTIVE);
        veVienHCN(thumbX - 6, y0 - 3, thumbX + 6, y1 + 3, BangMau::CHU_CHINH);
    }

    void veChuNhan() const {
        if (!pGiaTri) return;
        char buf[64];
        if (!coTheTuongTac) {
            std::snprintf(buf, sizeof(buf), "%s: [KHOA]", nhan.c_str());
            veChu(x0, y1 + 5, buf, MauSac(85, 92, 105), GLUT_BITMAP_HELVETICA_10);
            return;
        }
        std::snprintf(buf, sizeof(buf), "%s: %.1f", nhan.c_str(), *pGiaTri);
        veChu(x0, y1 + 5, buf, BangMau::CHU_PHU, GLUT_BITMAP_HELVETICA_10);
    }
};

struct HopKiemUI {
    int x, y;
    int kichThuoc = 16;
    bool* pGiaTri;
    std::string nhan;

    bool kiemTraChuot(int mx, int my) const {
        return (mx >= x && mx <= x + 160 && my >= y && my <= y + kichThuoc);
    }

    void toggle() {
        if (pGiaTri) *pGiaTri = !(*pGiaTri);
    }

    void veKhung() const {
        if (!pGiaTri) return;
        veHCN(x, y, x + kichThuoc, y + kichThuoc, MauSac(25, 30, 38));
        veVienHCN(x, y, x + kichThuoc, y + kichThuoc, BangMau::VIEN_PANEL);

        if (*pGiaTri) {
            veHCN(x + 3, y + 3, x + kichThuoc - 3, y + kichThuoc - 3, BangMau::VIEN_SANG);
        }
    }

    void veChuNhan() const {
        if (!pGiaTri) return;
        veChu(x + kichThuoc + 8, y + 3, nhan.c_str(), BangMau::CHU_CHINH, GLUT_BITMAP_HELVETICA_12);
    }
};

// =========================================================================
// 2. BỐ CỤC TOÀN BỘ GIAO DIỆN (UI MANAGER)
// =========================================================================

struct TrinhQuanLyGiaoDien {
    CheDoHienThi cheDoHienTai = CHE_DO_TONG_THE;
    int linhKienDangChon = LK_ARDUINO;

    // Các cờ tùy chọn hiển thị
    bool hienThiDayDien  = true;
    bool hienThiNhanChan = true;
    bool cheDoKhungDay   = false;
    bool coLapLinhKien   = false;

    // Thanh trượt tách rời (Exploded View)
    float tyLeTachRoi = 0.0f; // 0.0 (Gắn kết) -> 1.0 (Tách rời 100%)

    // Camera nội suy bay tới linh kiện (Camera Fly-to - Mục 4.3)
    Diem3D camTargetHienTai = Diem3D(0, 0, 0);
    Diem3D camTargetMucTieu = Diem3D(0, 0, 0);
    float camDistHienTai    = 34.0f;
    float camDistMucTieu    = 34.0f;
    float camTheta          = -35.0f;
    float camPhi            = 28.0f;

    // Sơ đồ 2D (Chế độ 4) - Biến đổi ma trận 3x3
    float soDo2DPanX   = 0.0f;
    float soDo2DPanY   = 0.0f;
    float soDo2DZoom   = 1.0f;
    float soDo2DGocRad = 0.0f;
    bool soDo2DLatNgang= false;
    bool soDo2DLatDoc  = false;

    // Các thành phần UI
    std::vector<NutBamUI> cacTabCheDo;
    std::vector<NutBamUI> cacNutDieuKhienXe;
    std::vector<NutBamUI> cacNutSoDo2D;
    std::vector<ThanhTruotUI> cacThanhTruot;
    std::vector<HopKiemUI> cacHopKiem;

    void khoiTao(CungCapTayGap& arm) {
        // 1. Tab Bar trên cùng (TopBar)
        cacTabCheDo.clear();
        const char* tabNames[] = {"1. TONG THE", "2. LINH KIEN", "3. BEN TRONG XE", "4. SO DO 2D"};
        for (int i = 0; i < 4; ++i) {
            NutBamUI btn;
            btn.x0 = 15 + i * 220;
            btn.y0 = TOP_BAR_Y_MIN + 6;
            btn.x1 = btn.x0 + 210;
            btn.y1 = TOP_BAR_Y_MAX - 6;
            btn.nhan = tabNames[i];
            btn.dangChon = (i == 0);
            cacTabCheDo.push_back(btn);
        }

        // 2. Thanh trượt ở BottomBar
        cacThanhTruot.clear();
        // Thanh trượt Tách rời (Explode)
        ThanhTruotUI slExp;
        slExp.x0 = 240; slExp.y0 = 65; slExp.x1 = 430; slExp.y1 = 77;
        slExp.giaTriMin = 0.0f; slExp.giaTriMax = 1.0f;
        slExp.pGiaTri = &tyLeTachRoi;
        slExp.nhan = "TACH ROI (EXPLODE)";
        cacThanhTruot.push_back(slExp);

        // 3 Slider Servo
        ThanhTruotUI slDe;
        slDe.x0 = 460; slDe.y0 = 65; slDe.x1 = 630; slDe.y1 = 77;
        slDe.giaTriMin = 0.0f; slDe.giaTriMax = 180.0f;
        slDe.pGiaTri = &arm.servoDe.gocMucTieu;
        slDe.nhan = "SERVO DE (YAW)";
        cacThanhTruot.push_back(slDe);

        ThanhTruotUI slKhuu;
        slKhuu.x0 = 655; slKhuu.y0 = 65; slKhuu.x1 = 825; slKhuu.y1 = 77;
        slKhuu.giaTriMin = 0.0f; slKhuu.giaTriMax = 180.0f;
        slKhuu.pGiaTri = &arm.servoKhuu.gocMucTieu;
        slKhuu.nhan = "SERVO KHUU (PITCH)";
        cacThanhTruot.push_back(slKhuu);

        ThanhTruotUI slKep;
        slKep.x0 = 850; slKep.y0 = 65; slKep.x1 = 1010; slKep.y1 = 77;
        slKep.giaTriMin = 0.0f; slKep.giaTriMax = 180.0f;
        slKep.pGiaTri = &arm.servoKep.gocMucTieu;
        slKep.nhan = "SERVO KEP (GRIP)";
        cacThanhTruot.push_back(slKep);

        // 3. Nút bấm điều khiển xe & Gắp/Thả
        cacNutDieuKhienXe.clear();
        NutBamUI btnGap;
        btnGap.x0 = 240; btnGap.y0 = 15; btnGap.x1 = 340; btnGap.y1 = 48;
        btnGap.nhan = "GAP / THA";
        cacNutDieuKhienXe.push_back(btnGap);

        NutBamUI btnTien;
        btnTien.x0 = 355; btnTien.y0 = 15; btnTien.x1 = 415; btnTien.y1 = 48;
        btnTien.nhan = "TIEN";
        cacNutDieuKhienXe.push_back(btnTien);

        NutBamUI btnLui;
        btnLui.x0 = 425; btnLui.y0 = 15; btnLui.x1 = 485; btnLui.y1 = 48;
        btnLui.nhan = "LUI";
        cacNutDieuKhienXe.push_back(btnLui);

        NutBamUI btnTrai;
        btnTrai.x0 = 495; btnTrai.y0 = 15; btnTrai.x1 = 555; btnTrai.y1 = 48;
        btnTrai.nhan = "TRAI";
        cacNutDieuKhienXe.push_back(btnTrai);

        NutBamUI btnPhai;
        btnPhai.x0 = 565; btnPhai.y0 = 15; btnPhai.x1 = 625; btnPhai.y1 = 48;
        btnPhai.nhan = "PHAI";
        cacNutDieuKhienXe.push_back(btnPhai);

        NutBamUI btnDung;
        btnDung.x0 = 635; btnDung.y0 = 15; btnDung.x1 = 695; btnDung.y1 = 48;
        btnDung.nhan = "DUNG";
        cacNutDieuKhienXe.push_back(btnDung);

        // Nút xoay và lật cho sơ đồ 2D
        NutBamUI btnXoay2D;
        btnXoay2D.x0 = 715; btnXoay2D.y0 = 15; btnXoay2D.x1 = 805; btnXoay2D.y1 = 48;
        btnXoay2D.nhan = "XOAY 45";
        cacNutDieuKhienXe.push_back(btnXoay2D);

        NutBamUI btnLatNgang;
        btnLatNgang.x0 = 815; btnLatNgang.y0 = 15; btnLatNgang.x1 = 905; btnLatNgang.y1 = 48;
        btnLatNgang.nhan = "LAT NGANG";
        cacNutDieuKhienXe.push_back(btnLatNgang);

        NutBamUI btnReset2D;
        btnReset2D.x0 = 915; btnReset2D.y0 = 15; btnReset2D.x1 = 1005; btnReset2D.y1 = 48;
        btnReset2D.nhan = "RESET 2D";
        cacNutDieuKhienXe.push_back(btnReset2D);

        // 4. Hộp kiểm Checkbox ở BottomBar
        cacHopKiem.clear();
        cacHopKiem.push_back({1030, 80, 15, &hienThiDayDien,  "Hien day noi"});
        cacHopKiem.push_back({1030, 56, 15, &hienThiNhanChan, "Nhan ten chan"});
        cacHopKiem.push_back({1030, 32, 15, &cheDoKhungDay,   "Khung day (Wire)"});
        cacHopKiem.push_back({1030,  8, 15, &coLapLinhKien,   "Co lap linh kien"});
    }

    void capNhatQuyenTuongTac(const std::vector<LinhKien>& ds) {
        if (cacThanhTruot.size() < 4 || cacNutDieuKhienXe.size() < 9) return;

        if (cheDoHienTai == CHE_DO_TONG_THE || cheDoHienTai == CHE_DO_BEN_TRONG) {
            // Chế độ 1 & Chế độ 3: Cho phép lái xe và điều khiển cánh tay
            cacThanhTruot[0].nhan = "TACH ROI (EXPLODE)";
            cacThanhTruot[0].coTheTuongTac = true;
            cacThanhTruot[1].nhan = "SERVO DE (YAW)";
            cacThanhTruot[1].coTheTuongTac = true;
            cacThanhTruot[2].nhan = "SERVO KHUU (PITCH)";
            cacThanhTruot[2].coTheTuongTac = true;
            cacThanhTruot[3].nhan = "SERVO KEP (GRIP)";
            cacThanhTruot[3].coTheTuongTac = true;

            cacNutDieuKhienXe[0].nhan = "GAP / THA";
            cacNutDieuKhienXe[0].coTheTuongTac = true;
            cacNutDieuKhienXe[1].nhan = "TIEN";
            cacNutDieuKhienXe[1].coTheTuongTac = true;
            cacNutDieuKhienXe[2].nhan = "LUI";
            cacNutDieuKhienXe[2].coTheTuongTac = true;
            cacNutDieuKhienXe[3].nhan = "TRAI";
            cacNutDieuKhienXe[3].coTheTuongTac = true;
            cacNutDieuKhienXe[4].nhan = "PHAI";
            cacNutDieuKhienXe[4].coTheTuongTac = true;
            cacNutDieuKhienXe[5].nhan = "DUNG";
            cacNutDieuKhienXe[5].coTheTuongTac = true;

            cacNutDieuKhienXe[6].coTheTuongTac = false; // Xoay 2D
            cacNutDieuKhienXe[7].coTheTuongTac = false; // Lật ngang 2D
            cacNutDieuKhienXe[8].coTheTuongTac = false; // Reset 2D
        }
        else if (cheDoHienTai == CHE_DO_SO_DO_2D) {
            // Chế độ 4: Chỉ tương tác sơ đồ 2D (Pan/Zoom/Rotate/Flip)
            for (auto& sl : cacThanhTruot) sl.coTheTuongTac = false;
            for (int i = 0; i <= 5; ++i) cacNutDieuKhienXe[i].coTheTuongTac = false;
            cacNutDieuKhienXe[6].coTheTuongTac = true;
            cacNutDieuKhienXe[7].coTheTuongTac = true;
            cacNutDieuKhienXe[8].coTheTuongTac = true;
        }
        else if (cheDoHienTai == CHE_DO_LINH_KIEN) {
            // Chế độ 2: BÀN GIA CÔNG & KIỂM THỬ 3D ĐỘC LẬP (PHẦN B MỤC 2)
            cacThanhTruot[0].nhan = "TACH LOP CON (PARTS)";
            cacThanhTruot[0].coTheTuongTac = true;

            cacNutDieuKhienXe[6].coTheTuongTac = false;
            cacNutDieuKhienXe[7].coTheTuongTac = false;
            cacNutDieuKhienXe[8].coTheTuongTac = false;

            if (linhKienDangChon >= 1 && linhKienDangChon < (int)ds.size()) {
                InspectCategory cat = ds[linhKienDangChon].category;

                if (cat == InspectCategory::ARM_MANIPULATOR) {
                    // Cụm tay gắp: Mở khóa 3 Slider Servo và nút Gắp/Thả
                    cacThanhTruot[1].nhan = "SERVO DE (YAW)";
                    cacThanhTruot[1].coTheTuongTac = true;
                    cacThanhTruot[2].nhan = "SERVO KHUU (PITCH)";
                    cacThanhTruot[2].coTheTuongTac = true;
                    cacThanhTruot[3].nhan = "SERVO KEP (GRIP)";
                    cacThanhTruot[3].coTheTuongTac = true;

                    cacNutDieuKhienXe[0].nhan = "GAP / THA";
                    cacNutDieuKhienXe[0].coTheTuongTac = true;

                    // Khóa toàn bộ nút lái xe
                    for (int i = 1; i <= 5; ++i) cacNutDieuKhienXe[i].coTheTuongTac = false;
                }
                else if (cat == InspectCategory::ROTARY_MOTOR_WHEEL) {
                    // Động cơ & Bánh xe: Khóa Slider Servo, Mở Quay thuận / nghịch / dừng
                    cacThanhTruot[1].coTheTuongTac = false;
                    cacThanhTruot[2].coTheTuongTac = false;
                    cacThanhTruot[3].coTheTuongTac = false;

                    cacNutDieuKhienXe[0].coTheTuongTac = false;

                    cacNutDieuKhienXe[1].nhan = "QUAY THUAN";
                    cacNutDieuKhienXe[1].coTheTuongTac = true;
                    cacNutDieuKhienXe[2].nhan = "QUAY NGHICH";
                    cacNutDieuKhienXe[2].coTheTuongTac = true;
                    cacNutDieuKhienXe[3].coTheTuongTac = false;
                    cacNutDieuKhienXe[4].coTheTuongTac = false;
                    cacNutDieuKhienXe[5].nhan = "DUNG MO-TO";
                    cacNutDieuKhienXe[5].coTheTuongTac = true;
                }
                else if (cat == InspectCategory::TOGGLE_SWITCH) {
                    // Hộp pin & Công tắc: Mở nút Bật/Tắt công tắc nguồn
                    cacThanhTruot[1].coTheTuongTac = false;
                    cacThanhTruot[2].coTheTuongTac = false;
                    cacThanhTruot[3].coTheTuongTac = false;

                    cacNutDieuKhienXe[0].coTheTuongTac = false;
                    cacNutDieuKhienXe[1].nhan = "BAT/TAT NGUON";
                    cacNutDieuKhienXe[1].coTheTuongTac = true;
                    for (int i = 2; i <= 5; ++i) cacNutDieuKhienXe[i].coTheTuongTac = false;
                }
                else {
                    // STATIC_VIEW_ONLY: Khóa toàn bộ các nút cơ học! Chỉ xem CAD
                    cacThanhTruot[1].coTheTuongTac = false;
                    cacThanhTruot[2].coTheTuongTac = false;
                    cacThanhTruot[3].coTheTuongTac = false;
                    for (int i = 0; i <= 5; ++i) cacNutDieuKhienXe[i].coTheTuongTac = false;
                }
            }
        }
    }

    void chonLinhKien(int id, const std::vector<LinhKien>& ds) {
        if (id <= 0 || id >= (int)ds.size()) return;
        linhKienDangChon = id;

        // Cập nhật đích Camera Fly-to (Phần B Mục 1.2: Auto-Fit Camera)
        if (cheDoHienTai == CHE_DO_LINH_KIEN) {
            camTargetMucTieu = Diem3D(0.0f, ds[id].kichThuocBao.y * 0.5f, 0.0f);
            camDistMucTieu = ds[id].inspectRadius;
        } else {
            Diem3D posLK = ds[id].viTriLapDat.bienDoi(Diem3D(0, 0, 0));
            camTargetMucTieu = posLK;
            camDistMucTieu = 34.0f;
        }
    }

    void capNhatNoiSuyCamera(float dt) {
        // Công thức mượt: alpha = 1 - exp(-8 * dt) (Mục 4.3 Đề cương)
        float alpha = 1.0f - std::exp(-8.0f * dt);
        camTargetHienTai = camTargetHienTai + (camTargetMucTieu - camTargetHienTai) * alpha;
        camDistHienTai   = camDistHienTai + (camDistMucTieu - camDistHienTai) * alpha;
    }

    // 1. Vẽ các khối hình học nền và viền (ghi vào FrameBuffer trong RAM)
    void veKhungGiaoDien(const std::vector<LinhKien>& dsLinhKien, const CungCapTayGap& arm, float kcHCSR04) {
        capNhatQuyenTuongTac(dsLinhKien);
        // --- 1. TOP BAR ---
        veHCN(0, TOP_BAR_Y_MIN, CUA_SO_RONG, TOP_BAR_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(0, TOP_BAR_Y_MIN, CUA_SO_RONG, TOP_BAR_Y_MIN, BangMau::VIEN_PANEL);

        for (size_t i = 0; i < cacTabCheDo.size(); ++i) {
            cacTabCheDo[i].dangChon = ((int)i == (int)cheDoHienTai);
            cacTabCheDo[i].veKhung();
        }

        // --- 2. LEFT PANEL (DANH SÁCH 11 LINH KIỆN) ---
        veHCN(LEFT_PANEL_X_MIN, LEFT_PANEL_Y_MIN, LEFT_PANEL_X_MAX, LEFT_PANEL_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(LEFT_PANEL_X_MAX, LEFT_PANEL_Y_MIN, LEFT_PANEL_X_MAX, LEFT_PANEL_Y_MAX, BangMau::VIEN_PANEL);
        veDoanThangBresenham(12, LEFT_PANEL_Y_MAX - 30, LEFT_PANEL_X_MAX - 12, LEFT_PANEL_Y_MAX - 30, BangMau::VIEN_PANEL);

        for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
            int yPos = LEFT_PANEL_Y_MAX - 45 - (i - 1) * 52;
            bool dangChon = (i == linhKienDangChon);

            if (dangChon) {
                veHCN(8, yPos - 12, LEFT_PANEL_X_MAX - 8, yPos + 28, BangMau::NUT_ACTIVE.lamToi(0.45f));
                veVienHCN(8, yPos - 12, LEFT_PANEL_X_MAX - 8, yPos + 28, BangMau::VIEN_SANG);
            }

            HopKiemUI hk;
            hk.x = 14; hk.y = yPos + 4; hk.kichThuoc = 14;
            bool hien = dsLinhKien[i].hienThi;
            hk.pGiaTri = &hien;
            hk.nhan = "";
            hk.veKhung();
        }

        // --- 3. RIGHT PANEL (BẢNG THÔNG TIN CHI TIẾT) ---
        veHCN(RIGHT_PANEL_X_MIN, RIGHT_PANEL_Y_MIN, RIGHT_PANEL_X_MAX, RIGHT_PANEL_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(RIGHT_PANEL_X_MIN, RIGHT_PANEL_Y_MIN, RIGHT_PANEL_X_MIN, RIGHT_PANEL_Y_MAX, BangMau::VIEN_PANEL);
        veDoanThangBresenham(RIGHT_PANEL_X_MIN + 12, RIGHT_PANEL_Y_MAX - 30, RIGHT_PANEL_X_MAX - 12, RIGHT_PANEL_Y_MAX - 30, BangMau::VIEN_PANEL);

        if (linhKienDangChon >= 1 && linhKienDangChon <= TONG_SO_LINH_KIEN) {
            const auto& lk = dsLinhKien[linhKienDangChon];
            int curY = RIGHT_PANEL_Y_MAX - 78;
            size_t start = 0;
            while (start < lk.vaiTro.length()) {
                size_t len = std::min((size_t)32, lk.vaiTro.length() - start);
                if (start + len < lk.vaiTro.length() && lk.vaiTro[start + len] != ' ') {
                    size_t sp = lk.vaiTro.rfind(' ', start + len);
                    if (sp != std::string::npos && sp > start) len = sp - start;
                }
                curY -= 16;
                start += len;
                if (start < lk.vaiTro.length() && lk.vaiTro[start] == ' ') ++start;
            }

            curY -= 8;
            veHCN(RIGHT_PANEL_X_MIN + 10, curY - 60, RIGHT_PANEL_X_MAX - 10, curY + 6, MauSac(18, 22, 28));
            veVienHCN(RIGHT_PANEL_X_MIN + 10, curY - 60, RIGHT_PANEL_X_MAX - 10, curY + 6, BangMau::VIEN_PANEL);
        }

        // --- 4. BOTTOM BAR ---
        veHCN(BOTTOM_BAR_X_MIN, BOTTOM_BAR_Y_MIN, BOTTOM_BAR_X_MAX, BOTTOM_BAR_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(BOTTOM_BAR_X_MIN, BOTTOM_BAR_Y_MAX, BOTTOM_BAR_X_MAX, BOTTOM_BAR_Y_MAX, BangMau::VIEN_PANEL);

        for (const auto& sl : cacThanhTruot) {
            sl.veKhung();
        }
        for (const auto& btn : cacNutDieuKhienXe) {
            btn.veKhung();
        }
        for (const auto& hk : cacHopKiem) {
            hk.veKhung();
        }
    }

    // 2. Vẽ toàn bộ các nhãn chữ (Text Bitmap) qua OpenGL sau khi đẩy FrameBuffer
    void veChuGiaoDien(const std::vector<LinhKien>& dsLinhKien, const CungCapTayGap& arm, float kcHCSR04) {
        capNhatQuyenTuongTac(dsLinhKien);
        // --- 1. TOP BAR CHỮ ---
        veChu(CUA_SO_RONG - 330, TOP_BAR_Y_MIN + 16, "VKU - DO HOA MAY TINH (CG.3 & CG.4)", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);

        for (const auto& tab : cacTabCheDo) {
            tab.veChuNhan();
        }

        // --- 2. LEFT PANEL CHỮ ---
        veChu(15, LEFT_PANEL_Y_MAX - 24, "DANH MUC LINH KIEN", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);

        for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
            int yPos = LEFT_PANEL_Y_MAX - 45 - (i - 1) * 52;
            bool dangChon = (i == linhKienDangChon);

            char buf[64];
            std::snprintf(buf, sizeof(buf), "[%02d] %s", i, dsLinhKien[i].ma.c_str());
            veChu(36, yPos + 6, buf, dangChon ? BangMau::CHU_CHINH : BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);

            std::string tenRutGon = dsLinhKien[i].ten;
            if (tenRutGon.length() > 22) tenRutGon = tenRutGon.substr(0, 20) + "..";
            veChu(36, yPos - 8, tenRutGon.c_str(), BangMau::CHU_PHU, GLUT_BITMAP_HELVETICA_10);
        }

        // --- 3. RIGHT PANEL CHỮ ---
        veChu(RIGHT_PANEL_X_MIN + 15, RIGHT_PANEL_Y_MAX - 24, "THONG SO LINH KIEN", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);

        if (linhKienDangChon >= 1 && linhKienDangChon <= TONG_SO_LINH_KIEN) {
            const auto& lk = dsLinhKien[linhKienDangChon];

            // Tên linh kiện
            veChu(RIGHT_PANEL_X_MIN + 15, RIGHT_PANEL_Y_MAX - 55, lk.ten.c_str(), BangMau::CANH_BAO_VANG, GLUT_BITMAP_HELVETICA_12);

            // Vai trò (xuống dòng tự động)
            std::string vt = lk.vaiTro;
            int curY = RIGHT_PANEL_Y_MAX - 78;
            size_t start = 0;
            while (start < vt.length()) {
                size_t len = std::min((size_t)32, vt.length() - start);
                if (start + len < vt.length() && vt[start + len] != ' ') {
                    size_t sp = vt.rfind(' ', start + len);
                    if (sp != std::string::npos && sp > start) len = sp - start;
                }
                veChu(RIGHT_PANEL_X_MIN + 15, curY, vt.substr(start, len).c_str(), BangMau::CHU_PHU, GLUT_BITMAP_HELVETICA_10);
                curY -= 16;
                start += len;
                if (start < vt.length() && vt[start] == ' ') ++start;
            }

            // Thông số động theo thời gian thực (Phân quyền thông minh)
            curY -= 8;
            veChu(RIGHT_PANEL_X_MIN + 16, curY - 14, "THONG SO HOAT DONG:", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_10);
            char b1[64], b2[64];
            if (cheDoHienTai == CHE_DO_LINH_KIEN && lk.category == InspectCategory::STATIC_VIEW_ONLY) {
                std::snprintf(b1, sizeof(b1), "Trang thai: [CHI XEM - LINH KIEN TINH]");
                std::snprintf(b2, sizeof(b2), "Kich thuoc CAD: %.1fx%.1fx%.1f cm", lk.kichThuocBao.x, lk.kichThuocBao.y, lk.kichThuocBao.z);
            } else if (lk.category == InspectCategory::ARM_MANIPULATOR) {
                if (linhKienDangChon == LK_SERVO_DE) {
                    std::snprintf(b1, sizeof(b1), "Goc: %.1f deg (Yaw)", arm.servoDe.gocHienTai);
                    std::snprintf(b2, sizeof(b2), "Xung PWM: %d us", arm.servoDe.layDoRongXungUs());
                } else if (linhKienDangChon == LK_SERVO_KHUU) {
                    std::snprintf(b1, sizeof(b1), "Goc: %.1f deg (Pitch)", arm.servoKhuu.gocHienTai);
                    std::snprintf(b2, sizeof(b2), "Xung PWM: %d us", arm.servoKhuu.layDoRongXungUs());
                } else {
                    std::snprintf(b1, sizeof(b1), "Goc: %.1f deg (Grip)", arm.servoKep.gocHienTai);
                    std::snprintf(b2, sizeof(b2), "Trang thai: %s", (arm.servoKep.gocHienTai < 60.0f) ? "[DANG DONG KEP]" : "[DANG MO NGAM]");
                }
            } else if (lk.category == InspectCategory::ROTARY_MOTOR_WHEEL) {
                std::snprintf(b1, sizeof(b1), "Dien ap PWM: 5.0 V DC (Ti so 1:48)");
                std::snprintf(b2, sizeof(b2), "Co cau: [BANH XE & HOP SO DANG KET NOI]");
            } else if (lk.category == InspectCategory::TOGGLE_SWITCH) {
                std::snprintf(b1, sizeof(b1), "Nguon: 7.4 V (2S Li-ion 2600mAh)");
                std::snprintf(b2, sizeof(b2), "Trang thai: [CAP NGUON HE THONG]");
            } else if (linhKienDangChon == LK_HCSR04) {
                std::snprintf(b1, sizeof(b1), "Khoang cach: %.1f cm", kcHCSR04);
                std::snprintf(b2, sizeof(b2), "Trang thai: %s", (kcHCSR04 < 18.0f) ? "[CO VAT CAN]" : "[AN TOAN]");
            } else {
                std::snprintf(b1, sizeof(b1), "Dien ap cap: 5.0 V DC");
                std::snprintf(b2, sizeof(b2), "Trang thai: [BINH THUONG]");
            }
            veChu(RIGHT_PANEL_X_MIN + 16, curY - 32, b1, BangMau::CHU_CHINH, GLUT_BITMAP_HELVETICA_10);
            veChu(RIGHT_PANEL_X_MIN + 16, curY - 48, b2, BangMau::HOAT_DONG_XANH, GLUT_BITMAP_HELVETICA_10);

            // Bảng danh sách chân Pin của linh kiện
            curY -= 82;
            veChu(RIGHT_PANEL_X_MIN + 15, curY, "DANH SACH CHAN NOI (PINS):", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_10);
            curY -= 14;

            int countPins = 0;
            for (const auto& pin : lk.pins) {
                if (countPins++ > 14) break; // Giới hạn hiển thị nếu quá dài
                char pinStr[64];
                std::snprintf(pinStr, sizeof(pinStr), "- Chan: %-8s  (%.1f, %.1f, %.1f)",
                              pin.ten.c_str(), pin.viTriCucBo.x, pin.viTriCucBo.y, pin.viTriCucBo.z);
                veChu(RIGHT_PANEL_X_MIN + 15, curY, pinStr, BangMau::CHU_PHU, GLUT_BITMAP_HELVETICA_10);
                curY -= 15;
            }
        }

        // --- 4. BOTTOM BAR CHỮ ---
        for (const auto& sl : cacThanhTruot) {
            sl.veChuNhan();
        }
        for (const auto& btn : cacNutDieuKhienXe) {
            btn.veChuNhan();
        }
        for (const auto& hk : cacHopKiem) {
            hk.veChuNhan();
        }
    }
};

// Standard Type Aliases
using UIManager = TrinhQuanLyGiaoDien;
using UIButton = NutBamUI;
using UISlider = ThanhTruotUI;
using UICheckBox = HopKiemUI;

#endif // UI_MANAGER_H
