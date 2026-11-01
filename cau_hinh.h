#ifndef CAU_HINH_H
#define CAU_HINH_H

#include <cstdint>
#include <string>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =========================================================================
// CẤU HÌNH KÍCH THƯỚC CỬA SỔ & BỐ CỤC 5 VÙNG GIAO DIỆN
// =========================================================================
const int CUA_SO_RONG = 1280;
const int CUA_SO_CAO  = 800;

// Tọa độ 5 vùng giao diện chuẩn theo Đề cương (Giai đoạn 4.2)
// 1. Thanh tiêu đề & Chế độ (TopBar)
const int TOP_BAR_Y_MIN = CUA_SO_CAO - 45;
const int TOP_BAR_Y_MAX = CUA_SO_CAO;

// 2. Danh sách 11 linh kiện (LeftPanel)
const int LEFT_PANEL_X_MIN = 0;
const int LEFT_PANEL_X_MAX = 230;
const int LEFT_PANEL_Y_MIN = 110;
const int LEFT_PANEL_Y_MAX = TOP_BAR_Y_MIN;

// 3. Bảng thông số kỹ thuật & Tra cứu chân (RightPanel)
const int RIGHT_PANEL_X_MIN = CUA_SO_RONG - 300;
const int RIGHT_PANEL_X_MAX = CUA_SO_RONG;
const int RIGHT_PANEL_Y_MIN = 110;
const int RIGHT_PANEL_Y_MAX = TOP_BAR_Y_MIN;

// 4. Thanh điều khiển dưới (BottomBar)
const int BOTTOM_BAR_X_MIN = 0;
const int BOTTOM_BAR_X_MAX = CUA_SO_RONG;
const int BOTTOM_BAR_Y_MIN = 0;
const int BOTTOM_BAR_Y_MAX = 110;

// 5. Khung nhìn trung tâm (CenterViewport - vùng xén hình 3D & sơ đồ 2D)
const int VIEWPORT_X_MIN = LEFT_PANEL_X_MAX;
const int VIEWPORT_X_MAX = RIGHT_PANEL_X_MIN;
const int VIEWPORT_Y_MIN = BOTTOM_BAR_Y_MAX;
const int VIEWPORT_Y_MAX = TOP_BAR_Y_MIN;

// =========================================================================
// MÃ CHẾ ĐỘ HIỂN THỊ (4 CHẾ ĐỘ TRÊN TAB TOPBAR)
// =========================================================================
enum CheDoHienThi {
    CHE_DO_TONG_THE    = 0, // Chế độ 1: Mô phỏng 3D tổng thể & Động học xe, tay gắp
    CHE_DO_LINH_KIEN   = 1, // Chế độ 2: Kiểm tra linh kiện chi tiết & Camera Fly-to
    CHE_DO_BEN_TRONG   = 2, // Chế độ 3: Ẩn vỏ xe để nhìn xuyên thấu linh kiện & dây dẫn
    CHE_DO_SO_DO_2D    = 3  // Chế độ 4: Sơ đồ nguyên lý đấu nối 2D (Pan/Zoom/Rotate/BoundaryFill)
};

// =========================================================================
// MÃ ĐỊNH DANH 11 LINH KIỆN TRÊN XE ROBOT
// =========================================================================
enum MaLinhKien {
    LK_KHONG      = 0,
    LK_ARDUINO    = 1,  // Arduino Uno R3
    LK_L298N      = 2,  // Module L298N
    LK_DONG_CO_T  = 3,  // Động cơ DC Trái
    LK_DONG_CO_P  = 4,  // Động cơ DC Phải
    LK_SERVO_DE   = 5,  // Servo Đế (SG90/MG996R)
    LK_SERVO_KHUU = 6,  // Servo Khuỷu
    LK_SERVO_KEP  = 7,  // Servo Kẹp
    LK_HCSR04     = 8,  // Cảm biến siêu âm HC-SR04
    LK_BUCK_5V    = 9,  // Mạch hạ áp Buck LM2596
    LK_PIN_18650  = 10, // Hộp pin 18650 & Công tắc
    LK_DOMINO     = 11  // Domino đấu nối 3 Bus
};

const int TONG_SO_LINH_KIEN = 11;
const int TONG_SO_DAY_NOI   = 34;

// =========================================================================
// CẤU TRÚC MÀU SẮC (RGB & RGBA)
// =========================================================================
struct MauSac {
    uint8_t r, g, b, a;

    MauSac() : r(0), g(0), b(0), a(255) {}
    MauSac(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
        : r(_r), g(_g), b(_b), a(_a) {}

    bool operator==(const MauSac& o) const {
        return r == o.r && g == o.g && b == o.b;
    }
    bool operator!=(const MauSac& o) const {
        return !(*this == o);
    }

    MauSac lamSang(float heSo) const {
        uint8_t nr = (uint8_t)std::min(255.0f, r * heSo);
        uint8_t ng = (uint8_t)std::min(255.0f, g * heSo);
        uint8_t nb = (uint8_t)std::min(255.0f, b * heSo);
        return MauSac(nr, ng, nb, a);
    }

    MauSac lamToi(float heSo) const {
        uint8_t nr = (uint8_t)(r * heSo);
        uint8_t ng = (uint8_t)(g * heSo);
        uint8_t nb = (uint8_t)(b * heSo);
        return MauSac(nr, ng, nb, a);
    }
};

// Bảng màu giao diện chuẩn hiện đại (Cyberpunk / Modern CAD Dark Theme)
namespace BangMau {
    const MauSac NEN_CHINH         (13,  17,  23);  // Nền xám đen GitHub/Dark
    const MauSac NEN_PANEL         (22,  27,  34);  // Nền các Panel
    const MauSac VIEN_PANEL        (48,  54,  61);  // Viền ngăn cách
    const MauSac VIEN_SANG         (88,  166, 255); // Viền nổi bật Electric Cyan
    const MauSac CHU_CHINH         (240, 246, 252); // Chữ trắng sáng
    const MauSac CHU_PHU           (139, 148, 158); // Chữ xám nhạt
    const MauSac NUT_BINH_THUONG   (33,  38,  45);  // Nút bấm bình thường
    const MauSac NUT_HOVER         (45,  54,  66);  // Nút khi rê chuột
    const MauSac NUT_ACTIVE        (31,  111, 235); // Nút được chọn / kích hoạt
    const MauSac CANH_BAO_VANG     (210, 153, 34);  // Cảnh báo vàng
    const MauSac HOAT_DONG_XANH    (46,  160, 67);  // Trạng thái xanh lá
    const MauSac DONG_CO_CAM       (240, 136, 62);  // Màu cam động lực
}

#endif // CAU_HINH_H
