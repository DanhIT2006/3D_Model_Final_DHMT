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

    bool kiemTraChuot(int mx, int my) const {
        return (mx >= x0 && mx <= x1 && my >= y0 && my <= y1);
    }

    void ve(MauSac mauChinh = BangMau::NUT_BINH_THUONG) const {
        MauSac mauNen = dangChon ? BangMau::NUT_ACTIVE : (dangHover ? BangMau::NUT_HOVER : mauChinh);
        veHCN(x0, y0, x1, y1, mauNen);
        veVienHCN(x0, y0, x1, y1, dangChon ? BangMau::VIEN_SANG : BangMau::VIEN_PANEL);

        // Vẽ nhãn chữ ở giữa nút
        int textX = x0 + (x1 - x0) / 2 - (int)nhan.length() * 4;
        int textY = y0 + (y1 - y0) / 2 - 4;
        veChu(textX, textY, nhan.c_str(), BangMau::CHU_CHINH, GLUT_BITMAP_HELVETICA_12);
    }
};

struct ThanhTruotUI {
    int x0, y0, x1, y1;
    float giaTriMin, giaTriMax;
    float* pGiaTri;
    std::string nhan;
    bool dangKeo = false;

    bool kiemTraChuot(int mx, int my) const {
        return (mx >= x0 - 4 && mx <= x1 + 4 && my >= y0 - 6 && my <= y1 + 6);
    }

    void capNhatTuChuot(int mx) {
        float tyLe = (float)(mx - x0) / (float)(x1 - x0);
        tyLe = std::clamp(tyLe, 0.0f, 1.0f);
        if (pGiaTri) {
            *pGiaTri = giaTriMin + tyLe * (giaTriMax - giaTriMin);
        }
    }

    void ve() const {
        if (!pGiaTri) return;

        // Vẽ nhãn thanh trượt phía trên
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%s: %.1f", nhan.c_str(), *pGiaTri);
        veChu(x0, y1 + 5, buf, BangMau::CHU_PHU, GLUT_BITMAP_HELVETICA_10);

        // Rãnh trượt
        veHCN(x0, y0 + 3, x1, y0 + 7, MauSac(35, 42, 52));
        veVienHCN(x0, y0 + 3, x1, y0 + 7, BangMau::VIEN_PANEL);

        // Nút kéo (Thumb)
        float tyLe = (*pGiaTri - giaTriMin) / (giaTriMax - giaTriMin);
        int thumbX = x0 + (int)(tyLe * (x1 - x0));
        veHCN(thumbX - 6, y0 - 3, thumbX + 6, y1 + 3, dangKeo ? BangMau::VIEN_SANG : BangMau::NUT_ACTIVE);
        veVienHCN(thumbX - 6, y0 - 3, thumbX + 6, y1 + 3, BangMau::CHU_CHINH);
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

    void ve() const {
        if (!pGiaTri) return;
        veHCN(x, y, x + kichThuoc, y + kichThuoc, MauSac(25, 30, 38));
        veVienHCN(x, y, x + kichThuoc, y + kichThuoc, BangMau::VIEN_PANEL);

        if (*pGiaTri) {
            veHCN(x + 3, y + 3, x + kichThuoc - 3, y + kichThuoc - 3, BangMau::VIEN_SANG);
        }
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

    void chonLinhKien(int id, const std::vector<LinhKien>& ds) {
        if (id <= 0 || id >= (int)ds.size()) return;
        linhKienDangChon = id;

        // Cập nhật đích Camera Fly-to
        Diem3D posLK = ds[id].viTriLapDat.bienDoi(Diem3D(0, 0, 0));
        camTargetMucTieu = posLK;
        camDistMucTieu = (cheDoHienTai == CHE_DO_LINH_KIEN) ? 18.0f : 34.0f;
    }

    void capNhatNoiSuyCamera(float dt) {
        // Công thức mượt: alpha = 1 - exp(-8 * dt) (Mục 4.3 Đề cương)
        float alpha = 1.0f - std::exp(-8.0f * dt);
        camTargetHienTai = camTargetHienTai + (camTargetMucTieu - camTargetHienTai) * alpha;
        camDistHienTai   = camDistHienTai + (camDistMucTieu - camDistHienTai) * alpha;
    }

    // Vẽ toàn bộ các thanh panel giao diện (TopBar, LeftPanel, RightPanel, BottomBar)
    void veGiaoDien(const std::vector<LinhKien>& dsLinhKien, const CungCapTayGap& arm, float kcHCSR04) {
        // --- 1. TOP BAR ---
        veHCN(0, TOP_BAR_Y_MIN, CUA_SO_RONG, TOP_BAR_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(0, TOP_BAR_Y_MIN, CUA_SO_RONG, TOP_BAR_Y_MIN, BangMau::VIEN_PANEL);

        // Logo & Tiêu đề
        veChu(CUA_SO_RONG - 330, TOP_BAR_Y_MIN + 16, "VKU - DO HOA MAY TINH (CG.3 & CG.4)", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);

        // 4 Nút Tab chế độ
        for (size_t i = 0; i < cacTabCheDo.size(); ++i) {
            cacTabCheDo[i].dangChon = ((int)i == (int)cheDoHienTai);
            cacTabCheDo[i].ve();
        }

        // --- 2. LEFT PANEL (DANH SÁCH 11 LINH KIỆN) ---
        veHCN(LEFT_PANEL_X_MIN, LEFT_PANEL_Y_MIN, LEFT_PANEL_X_MAX, LEFT_PANEL_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(LEFT_PANEL_X_MAX, LEFT_PANEL_Y_MIN, LEFT_PANEL_X_MAX, LEFT_PANEL_Y_MAX, BangMau::VIEN_PANEL);

        veChu(15, LEFT_PANEL_Y_MAX - 24, "DANH MUC LINH KIEN", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);
        veDoanThangBresenham(12, LEFT_PANEL_Y_MAX - 30, LEFT_PANEL_X_MAX - 12, LEFT_PANEL_Y_MAX - 30, BangMau::VIEN_PANEL);

        // Danh sách 11 linh kiện
        for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
            int yPos = LEFT_PANEL_Y_MAX - 45 - (i - 1) * 52;
            bool dangChon = (i == linhKienDangChon);

            if (dangChon) {
                veHCN(8, yPos - 12, LEFT_PANEL_X_MAX - 8, yPos + 28, BangMau::NUT_ACTIVE.lamToi(0.45f));
                veVienHCN(8, yPos - 12, LEFT_PANEL_X_MAX - 8, yPos + 28, BangMau::VIEN_SANG);
            }

            // Hộp kiểm ẩn/hiện linh kiện
            HopKiemUI hk;
            hk.x = 14; hk.y = yPos + 4; hk.kichThuoc = 14;
            bool hien = dsLinhKien[i].hienThi;
            hk.pGiaTri = &hien;
            hk.nhan = "";
            hk.ve();

            char buf[64];
            std::snprintf(buf, sizeof(buf), "[%02d] %s", i, dsLinhKien[i].ma.c_str());
            veChu(36, yPos + 6, buf, dangChon ? BangMau::CHU_CHINH : BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);

            std::string tenRutGon = dsLinhKien[i].ten;
            if (tenRutGon.length() > 22) tenRutGon = tenRutGon.substr(0, 20) + "..";
            veChu(36, yPos - 8, tenRutGon.c_str(), BangMau::CHU_PHU, GLUT_BITMAP_HELVETICA_10);
        }

        // --- 3. RIGHT PANEL (BẢNG THÔNG TIN CHI TIẾT & TRA CỨU CHÂN) ---
        veHCN(RIGHT_PANEL_X_MIN, RIGHT_PANEL_Y_MIN, RIGHT_PANEL_X_MAX, RIGHT_PANEL_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(RIGHT_PANEL_X_MIN, RIGHT_PANEL_Y_MIN, RIGHT_PANEL_X_MIN, RIGHT_PANEL_Y_MAX, BangMau::VIEN_PANEL);

        veChu(RIGHT_PANEL_X_MIN + 15, RIGHT_PANEL_Y_MAX - 24, "THONG SO LINH KIEN", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_12);
        veDoanThangBresenham(RIGHT_PANEL_X_MIN + 12, RIGHT_PANEL_Y_MAX - 30, RIGHT_PANEL_X_MAX - 12, RIGHT_PANEL_Y_MAX - 30, BangMau::VIEN_PANEL);

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

            // Thông số động theo thời gian thực
            curY -= 8;
            veHCN(RIGHT_PANEL_X_MIN + 10, curY - 60, RIGHT_PANEL_X_MAX - 10, curY + 6, MauSac(18, 22, 28));
            veVienHCN(RIGHT_PANEL_X_MIN + 10, curY - 60, RIGHT_PANEL_X_MAX - 10, curY + 6, BangMau::VIEN_PANEL);

            veChu(RIGHT_PANEL_X_MIN + 16, curY - 14, "THONG SO HOAT DONG:", BangMau::VIEN_SANG, GLUT_BITMAP_HELVETICA_10);
            char b1[64], b2[64];
            if (linhKienDangChon == LK_SERVO_DE) {
                std::snprintf(b1, sizeof(b1), "Goc: %.1f deg", arm.servoDe.gocHienTai);
                std::snprintf(b2, sizeof(b2), "Xung PWM: %d us", arm.servoDe.layDoRongXungUs());
            } else if (linhKienDangChon == LK_SERVO_KHUU) {
                std::snprintf(b1, sizeof(b1), "Goc: %.1f deg", arm.servoKhuu.gocHienTai);
                std::snprintf(b2, sizeof(b2), "Xung PWM: %d us", arm.servoKhuu.layDoRongXungUs());
            } else if (linhKienDangChon == LK_SERVO_KEP) {
                std::snprintf(b1, sizeof(b1), "Goc: %.1f deg", arm.servoKep.gocHienTai);
                std::snprintf(b2, sizeof(b2), "Xung PWM: %d us", arm.servoKep.layDoRongXungUs());
            } else if (linhKienDangChon == LK_HCSR04) {
                std::snprintf(b1, sizeof(b1), "Khoang cach: %.1f cm", kcHCSR04);
                std::snprintf(b2, sizeof(b2), "Trang thai: %s", (kcHCSR04 < 18.0f) ? "[CO VAT CAN]" : "[AN TOAN]");
            } else if (linhKienDangChon == LK_PIN_18650) {
                std::snprintf(b1, sizeof(b1), "Dien ap: 7.65 V (2S Li-ion)");
                std::snprintf(b2, sizeof(b2), "Dung luong: 85%% (2600 mAh)");
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

        // --- 4. BOTTOM BAR (THANH ĐIỀU KHIỂN DƯỚI) ---
        veHCN(BOTTOM_BAR_X_MIN, BOTTOM_BAR_Y_MIN, BOTTOM_BAR_X_MAX, BOTTOM_BAR_Y_MAX, BangMau::NEN_PANEL);
        veDoanThangBresenham(BOTTOM_BAR_X_MIN, BOTTOM_BAR_Y_MAX, BOTTOM_BAR_X_MAX, BOTTOM_BAR_Y_MAX, BangMau::VIEN_PANEL);

        // Vẽ các thanh trượt
        for (const auto& sl : cacThanhTruot) {
            sl.ve();
        }

        // Vẽ các nút điều khiển
        for (const auto& btn : cacNutDieuKhienXe) {
            btn.ve();
        }

        // Vẽ các hộp kiểm
        for (const auto& hk : cacHopKiem) {
            hk.ve();
        }
    }
};

// Standard Type Aliases
using UIManager = TrinhQuanLyGiaoDien;
using UIButton = NutBamUI;
using UISlider = ThanhTruotUI;
using UICheckBox = HopKiemUI;

#endif // UI_MANAGER_H
