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

    // -------------------------------------------------------------
    // TEST 5: KIỂM THỬ KHUNG XE 2 TẦNG & 11 CỤM LINH KIỆN MULTI-PART
    // -------------------------------------------------------------
    std::cout << "[TEST 5] Kiem thu Khung xe 2 tang & 11 Linh kien Multi-part:\n";
    KhungXe2Tang khungXe = taoKhungXe2Tang();
    std::cout << "  - So tru dong standoffs: " << khungXe.standoffs.size() << " (Mong doi: 4)\n";
    assert(khungXe.standoffs.size() == 4);

    size_t tongSoPartLK = 0;
    for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
        assert(!dsLK[i].parts.empty());
        tongSoPartLK += dsLK[i].parts.size();
    }
    std::cout << "  - Tong so part tren 11 linh kien: " << tongSoPartLK << " parts (Mong doi >= 30)\n";
    assert(tongSoPartLK >= 30);
    std::cout << "  => TEST 5: PASS!\n\n";

    // -------------------------------------------------------------
    // TEST 6: KIỂM THỬ NỘI SUY BÉZIER BẬC 2 VÀ ĐỊNH TUYẾN DÂY NÂNG CẤP
    // -------------------------------------------------------------
    std::cout << "[TEST 6] Kiem thu Noi suy Bezier bac 2 (CG.3 Trang 99):\n";
    Diem3D p0(0, 0, 0), p1(10, 20, 0), p2(20, 0, 0);
    Diem3D pMid = tinhBezierBac2(p0, p1, p2, 0.5f);
    // B(0.5) = 0.25*p0 + 0.5*p1 + 0.25*p2 = (10, 10, 0)
    std::cout << "  - Diem giua Bezier t=0.5: (" << pMid.x << ", " << pMid.y << ", " << pMid.z << ") -> Mong doi: (10, 10, 0)\n";
    assert(std::abs(pMid.x - 10.0f) < 1e-4f && std::abs(pMid.y - 10.0f) < 1e-4f);

    // Kiểm thử định tuyến 34 dây không bị văng lên trời
    CungCapTayGap tayGap;
    tayGap.khoiTao();
    MaTran4x4 matId;
    for (size_t i = 0; i < bangDay.size(); ++i) {
        const auto& d = bangDay[i];
        Diem3D locA, locB;
        dsLK[d.idLKA].timChan(d.chanA, locA);
        dsLK[d.idLKB].timChan(d.chanB, locB);
        Diem3D pA = dsLK[d.idLKA].viTriLapDat.bienDoi(locA);
        Diem3D pB = dsLK[d.idLKB].viTriLapDat.bienDoi(locB);
        std::vector<Diem3D> pts = taoDuongDiDay3DNangCap(pA, pB, d.idLKA, d.idLKB, (int)i + 1,
            tayGap.matWorldWrist, tayGap.matWorldElbow, tayGap.matWorldBase, matId);
        assert(pts.size() >= 2);
        for (const auto& pt : pts) {
            // Chiều cao y không được vượt quá độ cao sải tay cực đại (y <= 25 cm)
            assert(pt.y >= 0.0f && pt.y <= 25.0f);
        }
    }
    std::cout << "  - 34 day noi deu sinh duong di hop le, khong vuot qua gioi han khong gian xe.\n";
    std::cout << "  => TEST 6: PASS!\n\n";

    // -------------------------------------------------------------
    // TEST 7: KIỂM THỬ CHẾ ĐỘ 2 - PHÂN QUYỀN TƯƠNG TÁC & SUB-PART EXPLODE
    // -------------------------------------------------------------
    std::cout << "[TEST 7] Kiem thu Che do 2 (CAD Workbench & Phan quyen tuong tac):\n";
    assert(dsLK[LK_ARDUINO].category == InspectCategory::STATIC_VIEW_ONLY);
    assert(dsLK[LK_L298N].category == InspectCategory::STATIC_VIEW_ONLY);
    assert(dsLK[LK_DONG_CO_T].category == InspectCategory::ROTARY_MOTOR_WHEEL);
    assert(dsLK[LK_DONG_CO_P].category == InspectCategory::ROTARY_MOTOR_WHEEL);
    assert(dsLK[LK_SERVO_DE].category == InspectCategory::ARM_MANIPULATOR);
    assert(dsLK[LK_SERVO_KHUU].category == InspectCategory::ARM_MANIPULATOR);
    assert(dsLK[LK_SERVO_KEP].category == InspectCategory::ARM_MANIPULATOR);
    assert(dsLK[LK_HCSR04].category == InspectCategory::STATIC_VIEW_ONLY);
    assert(dsLK[LK_BUCK_5V].category == InspectCategory::STATIC_VIEW_ONLY);
    assert(dsLK[LK_PIN_18650].category == InspectCategory::TOGGLE_SWITCH);
    assert(dsLK[LK_DOMINO].category == InspectCategory::STATIC_VIEW_ONLY);

    for (int i = 1; i <= TONG_SO_LINH_KIEN; ++i) {
        assert(dsLK[i].inspectRadius >= 8.0f && dsLK[i].inspectRadius <= 30.0f);
        assert(dsLK[i].kichThuocBao.x > 0.0f && dsLK[i].kichThuocBao.y > 0.0f && dsLK[i].kichThuocBao.z > 0.0f);
    }
    std::cout << "  - 11 linh kien deu co InspectCategory, ban kinh Auto-Fit va kich thuoc bao CAD hop le.\n";

    // Kiểm thử bàn thực nghiệm tay gắp 3-DOF
    MaTran4x4 mBase, mElbow, mWrist;
    tayGap.tinhMaTranBanThucNghiem(mBase, mElbow, mWrist, 0.0f);
    Diem3D wristPos = mWrist.bienDoi(Diem3D(0, 0, 0));
    std::cout << "  - Toa do co tay gap tren ban thu: (" << wristPos.x << ", " << wristPos.y << ", " << wristPos.z << ")\n";
    assert(wristPos.y >= 0.0f);

    VatTheTuongTac vatTest;
    vatTest.viTriBanThucNghiem = Diem3D(wristPos.x, wristPos.y, wristPos.z);
    tayGap.servoKep.gocHienTai = 20.0f; // Dong ngam de gap
    bool gapOk = vatTest.thuGapVatTrenBan(tayGap);
    std::cout << "  - Thu nghiem gap vat tren ban thu: " << (gapOk ? "THANH CONG" : "KHONG GAP DUOC") << "\n";
    assert(gapOk && vatTest.dangBiGapTrenBan);
    vatTest.thaVatTrenBan(tayGap);
    assert(!vatTest.dangBiGapTrenBan);
    std::cout << "  - Thu nghiem tha vat tren ban thu: THANH CONG\n";
    std::cout << "  => TEST 7: PASS!\n\n";

    std::cout << "=======================================================\n";
    std::cout << "   TAT CA 7 CA KIEM THU DEU DAT CHUAN 100% (ALL PASSED)\n";
    std::cout << "=======================================================\n";
    return 0;
}
