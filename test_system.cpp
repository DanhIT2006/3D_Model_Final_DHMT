#include "config.h"
#include "math_3d.h"
#include "rasterizer.h"
#include "clipping_fill.h"
#include "components_mesh.h"
#include "wiring_table.h"
#include "robot_kinematics.h"
#include <iostream>
#include <cassert>

// Định nghĩa Framebuffer cho bài test
MauSac g_frameBuffer[CUA_SO_CAO][CUA_SO_RONG];
int g_clipMinX = 0, g_clipMaxX = CUA_SO_RONG - 1;
int g_clipMinY = 0, g_clipMaxY = CUA_SO_CAO - 1;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "   BAT DAU KIEM THU HE THONG (UNIT & INTEGRATION TESTS)\n";
    std::cout << "=======================================================\n\n";

    // -------------------------------------------------------------
    // TEST 1: KIỂM THỬ ÁNH XẠ XUNG PWM SERVO
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Kiem thu anh xa xung PWM Servo (0 -> 180 do):\n";
    MoHinhServo s;
    s.gocHienTai = 0.0f;
    int pulse0 = s.layDoRongXungUs();
    std::cout << "  - Goc 0 deg   -> " << pulse0 << " us (Mong doi: 544 us)\n";
    assert(pulse0 == 544);

    s.gocHienTai = 90.0f;
    int pulse90 = s.layDoRongXungUs();
    std::cout << "  - Goc 90 deg  -> " << pulse90 << " us (Mong doi: 1472 us)\n";
    assert(pulse90 == 1472);

    s.gocHienTai = 180.0f;
    int pulse180 = s.layDoRongXungUs();
    std::cout << "  - Goc 180 deg -> " << pulse180 << " us (Mong doi: 2400 us)\n";
    assert(pulse180 == 2400);
    std::cout << "  => TEST 1: PASS!\n\n";

    // -------------------------------------------------------------
    // TEST 2: KIỂM THỬ XÉN ĐOẠN THẲNG COHEN-SUTHERLAND
    // -------------------------------------------------------------
    std::cout << "[TEST 2] Kiem thu Xen doan thang Cohen-Sutherland:\n";
    Diem2D wmin(100, 100), wmax(200, 200);

    // Ca 1: Nằm hoàn toàn bên trong
    Diem2D A1(120, 120), B1(180, 180);
    bool r1 = xenDoanThangCohenSutherland(A1, B1, wmin, wmax);
    std::cout << "  - Doan trong hop: " << (r1 ? "GIU LAI (PASS)" : "FAIL") << "\n";
    assert(r1 && A1.x == 120 && B1.x == 180);

    // Ca 2: Nằm hoàn toàn bên ngoài cùng phía (LEFT)
    Diem2D A2(10, 120), B2(50, 180);
    bool r2 = xenDoanThangCohenSutherland(A2, B2, wmin, wmax);
    std::cout << "  - Doan ngoai cung ben: " << (!r2 ? "LOAI BO (PASS)" : "FAIL") << "\n";
    assert(!r2);

    // Ca 3: Cắt ngang cửa sổ (x từ 50 đến 250 tại y = 150)
    Diem2D A3(50, 150), B3(250, 150);
    bool r3 = xenDoanThangCohenSutherland(A3, B3, wmin, wmax);
    std::cout << "  - Doan cat ngang: " << (r3 ? "XEN HOP LE (PASS)" : "FAIL")
              << " -> (" << A3.x << "," << A3.y << ") den (" << B3.x << "," << B3.y << ")\n";
    float minX = std::min(A3.x, B3.x);
    float maxX = std::max(A3.x, B3.x);
    assert(r3 && std::abs(minX - 100.0f) < 1e-4f && std::abs(maxX - 200.0f) < 1e-4f);
    assert(std::abs(A3.y - 150.0f) < 1e-4f && std::abs(B3.y - 150.0f) < 1e-4f);
    std::cout << "  => TEST 2: PASS!\n\n";

    // -------------------------------------------------------------
    // TEST 3: KIỂM THỬ XÉN ĐA GIÁC SUTHERLAND-HODGMAN
    // -------------------------------------------------------------
    std::cout << "[TEST 3] Kiem thu Xen da giac Sutherland-Hodgman:\n";
    std::vector<Diem2D> poly = {
        {50, 150}, {150, 250}, {250, 150}, {150, 50} // Hình thoi cắt qua cả 4 cạnh
    };
    std::vector<Diem2D> polyXen = xenDaGiacSutherlandHodgman(poly, wmin, wmax);
    std::cout << "  - So dinh truoc xen: 4 -> Sau xen: " << polyXen.size() << " dinh\n";
    assert(polyXen.size() == 8); // Đa giác 8 đỉnh bị xén gọt ghẽ trong [100,200]
    for (const auto& p : polyXen) {
        assert(p.x >= wmin.x - 1e-4f && p.x <= wmax.x + 1e-4f);
        assert(p.y >= wmin.y - 1e-4f && p.y <= wmax.y + 1e-4f);
    }
    std::cout << "  => TEST 3: PASS!\n\n";

    // -------------------------------------------------------------
    // TEST 4: KIỂM THỬ TOÀN VẸN 34 DÂY NỐI TRÊN 11 LINH KIỆN
    // -------------------------------------------------------------
    std::cout << "[TEST 4] Kiem thu toan ven 34 day noi:\n";
    std::vector<LinhKien> dsLK;
    khoiTao11LinhKien(dsLK);
    std::vector<DayDien> bangDay = khoiTao34DayDien();

    bool okDay = kiemTraToanVenDay(bangDay, dsLK);
    assert(okDay);
    std::cout << "  => TEST 4: PASS!\n\n";

    std::cout << "=======================================================\n";
    std::cout << "   TAT CA 4 CA KIEM THU DEU DAT CHUAN 100% (ALL PASSED)\n";
    std::cout << "=======================================================\n";
    return 0;
}
