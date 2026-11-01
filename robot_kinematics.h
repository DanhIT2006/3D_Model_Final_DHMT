#ifndef ROBOT_KINEMATICS_H
#define ROBOT_KINEMATICS_H

#include "config.h"
#include "math_3d.h"
#include <cmath>
#include <algorithm>

// =========================================================================
// 1. MÔ HÌNH TOÁN HỌC SERVO & ÁNH XẠ XUNG PWM (Mục 3.2 Đề cương)
// 0 độ -> 544 µs, 180 độ -> 2400 µs (Chuẩn thư viện Arduino Servo.h)
// =========================================================================

struct MoHinhServo {
    float gocHienTai  = 90.0f;
    float gocMucTieu  = 90.0f;
    float gocMin      = 0.0f;
    float gocMax      = 180.0f;
    float tocDoToiDa  = 120.0f; // Độ/giây

    void capNhat(float dt) {
        gocMucTieu = std::clamp(gocMucTieu, gocMin, gocMax);
        float diff = gocMucTieu - gocHienTai;
        float buocNhay = tocDoToiDa * dt;
        if (std::abs(diff) <= buocNhay) {
            gocHienTai = gocMucTieu;
        } else {
            gocHienTai += (diff > 0 ? buocNhay : -buocNhay);
        }
    }

    int layDoRongXungUs() const {
        return (int)std::round(544.0f + (gocHienTai / 180.0f) * (2400.0f - 544.0f));
    }
};

// =========================================================================
// 2. ĐỘNG HỌC VI SAI XE 2 BÁNH CHỦ ĐỘNG (DIFFERENTIAL DRIVE - Mục 3.3)
// =========================================================================

struct DongHocXe {
    float x = 0.0f, z = 0.0f;
    float gocHuong = 0.0f; // Yaw quanh trục Y (độ)
    float vL = 0.0f, vR = 0.0f; // Vận tốc dài hai bánh (cm/s)
    float khoangCachHaiBanh = 12.0f; // B (cm)
    float banKinhBanhXe = 3.2f;      // rw (cm)
    float gocLanBanhL = 0.0f, gocLanBanhR = 0.0f;

    void capNhat(float dt) {
        float v = (vL + vR) * 0.5f;
        float omegaRad = (vR - vL) / khoangCachHaiBanh; // rad/s
        float omegaDeg = omegaRad * 180.0f / (float)M_PI;

        gocHuong += omegaDeg * dt;
        float rad = gocHuong * (float)M_PI / 180.0f;

        x += v * std::sin(rad) * dt;
        z += v * std::cos(rad) * dt;

        gocLanBanhL += (vL * dt / banKinhBanhXe) * (180.0f / (float)M_PI);
        gocLanBanhR += (vR * dt / banKinhBanhXe) * (180.0f / (float)M_PI);

        // Giảm dần vận tốc (ma sát chậm dần)
        vL *= 0.88f;
        vR *= 0.88f;
        if (std::abs(vL) < 0.01f) vL = 0.0f;
        if (std::abs(vR) < 0.01f) vR = 0.0f;
    }
};

// =========================================================================
// 3. MÔ HÌNH PHÂN CẤP CÁNH TAY ROBOT 3 BẬC TỰ DO (Mục 3.1)
// Cây phân cấp: Node_Car -> Node_Base -> Node_Elbow -> Node_Wrist -> Fingers
// =========================================================================

struct CungCapTayGap {
    MoHinhServo servoDe;   // Servo 1: Đế xoay (0 -> 180 độ, 90 độ là chính diện)
    MoHinhServo servoKhuu; // Servo 2: Khớp khuỷu vai
    MoHinhServo servoKep;  // Servo 3: Ngàm kẹp

    // Kích thước các đốt tay máy (cm) theo Phần B Mục 1
    float hChassis = 6.8f; // Gắn trên sàn Tầng 2
    float L1 = 3.7f;       // Khoảng cách từ Đế lên trục quay Khuỷu
    float L2 = 8.5f;       // Chiều dài cánh tay trên vươn tới ngàm kẹp

    // Ma trận thế giới tính được của từng khớp
    MaTran4x4 matWorldBase;
    MaTran4x4 matWorldElbow;
    MaTran4x4 matWorldWrist;

    void khoiTao() {
        servoDe.gocHienTai   = servoDe.gocMucTieu   = 90.0f;
        servoKhuu.gocHienTai = servoKhuu.gocMucTieu = 65.0f;
        servoKep.gocHienTai  = servoKep.gocMucTieu  = 80.0f; // Mở ngàm
    }

    void capNhat(float dt, const MaTran4x4& matWorldCar) {
        servoDe.capNhat(dt);
        servoKhuu.capNhat(dt);
        servoKep.capNhat(dt);

        // 1. Đế xoay (Node_Base): Đặt tại (0.0, 6.8, +6.5) trên UpperDeck
        float gocDeRad = (servoDe.gocHienTai - 90.0f) * (float)M_PI / 180.0f;
        MaTran4x4 matLocalBase = taoMaTranQuayY(gocDeRad) * taoMaTranTinhTien(0.0f, hChassis, 6.5f);
        matWorldBase = matLocalBase * matWorldCar;

        // 2. Khớp khuỷu (Node_Elbow): Đặt tại cục bộ (0.0, 3.7, 0.0) so với Đế
        float gocKhuuRad = (servoKhuu.gocHienTai - 90.0f) * (float)M_PI / 180.0f;
        MaTran4x4 matLocalElbow = taoMaTranQuayX(gocKhuuRad) * taoMaTranTinhTien(0.0f, L1, 0.0f);
        matWorldElbow = matLocalElbow * matWorldBase;

        // 3. Cổ tay & Ngàm gắp (Node_Wrist): Đặt tại cục bộ (0.0, 0.0, 8.5) so với Khuỷu
        MaTran4x4 matLocalWrist = taoMaTranTinhTien(0.0f, 0.0f, L2);
        matWorldWrist = matLocalWrist * matWorldElbow;
    }

    // Tính ma trận các khớp khi đưa vào Bàn gia công / Kiểm thử 3D Độc lập (Chế độ 2)
    void tinhMaTranBanThucNghiem(MaTran4x4& outBase, MaTran4x4& outElbow, MaTran4x4& outWrist, float tyLeTachRoi = 0.0f) const {
        float gocDeRad = (servoDe.gocHienTai - 90.0f) * (float)M_PI / 180.0f;
        outBase = taoMaTranQuayY(gocDeRad) * taoMaTranTinhTien(0.0f, 0.4f + tyLeTachRoi * 1.5f, 0.0f);

        float gocKhuuRad = (servoKhuu.gocHienTai - 90.0f) * (float)M_PI / 180.0f;
        MaTran4x4 matLocalElbow = taoMaTranQuayX(gocKhuuRad) * taoMaTranTinhTien(0.0f, L1 + tyLeTachRoi * 2.0f, 0.0f);
        outElbow = matLocalElbow * outBase;

        MaTran4x4 matLocalWrist = taoMaTranTinhTien(0.0f, 0.0f, L2 + tyLeTachRoi * 2.5f);
        outWrist = matLocalWrist * outElbow;
    }
};

// =========================================================================
// 4. VẬT THỂ TƯƠNG TÁC (KHỐI HỘP GẮP / THẢ - Mục 3.4 Đề cương)
// Chuyển đổi không gian ma trận không nhảy vị trí (Attach / Detach)
// =========================================================================

struct VatTheTuongTac {
    Diem3D viTriTheGioi = Diem3D(0.0f, 1.2f, 18.0f); // Vị trí khối hộp trên sàn phía trước xe
    Diem3D viTriBanThucNghiem = Diem3D(0.0f, 1.2f, 9.0f); // Vị trí khối hộp trên bàn thử Chế độ 2
    float kichThuoc = 2.4f; // Cạnh 2.4 cm
    MauSac mauSac = MauSac(46, 204, 113); // Màu xanh ngọc bắt mắt
    bool dangBiGap = false;
    bool dangBiGapTrenBan = false;
    MaTran4x4 matLocalObj; // Tọa độ tương đối khi gắn vào Wrist (trên xe)
    MaTran4x4 matLocalObjBan; // Tọa độ tương đối khi gắn vào Wrist (trên bàn thử)

    // Kiểm tra khoảng cách ngàm tới vật thể và kẹp
    bool thuGapVat(const CungCapTayGap& arm) {
        Diem3D posWrist = arm.matWorldWrist.bienDoi(Diem3D(0, 0, 0));
        float kc = (posWrist - viTriTheGioi).doDai();

        // Nếu ngàm ở gần vật thể (d <= 3.8 cm) và ngàm đang đóng góc kẹp
        if (kc <= 3.8f && arm.servoKep.gocHienTai < 60.0f) {
            dangBiGap = true;
            // Tính M_local = M_world_obj * (M_world_wrist)^-1 (Trang 15, CG.4)
            MaTran4x4 invWrist = arm.matWorldWrist.nghichDaoBienDoiCung();
            MaTran4x4 matWorldObj = taoMaTranTinhTien(viTriTheGioi.x, viTriTheGioi.y, viTriTheGioi.z);
            matLocalObj = matWorldObj * invWrist;
            return true;
        }
        return false;
    }

    void thaVat(const CungCapTayGap& arm) {
        if (!dangBiGap) return;
        // Chuyển về tọa độ thế giới hiện tại
        MaTran4x4 matWorldObj = matLocalObj * arm.matWorldWrist;
        viTriTheGioi = matWorldObj.bienDoi(Diem3D(0, 0, 0));
        viTriTheGioi.y = kichThuoc * 0.5f; // Hạ sát mặt sàn
        dangBiGap = false;
    }

    void capNhat(const CungCapTayGap& arm) {
        if (dangBiGap) {
            // Cập nhật vị trí thế giới theo chuyển động cánh tay
            MaTran4x4 matWorldObj = matLocalObj * arm.matWorldWrist;
            viTriTheGioi = matWorldObj.bienDoi(Diem3D(0, 0, 0));
        }
    }

    bool thuGapVatTrenBan(const MaTran4x4& matWrist, float gocKep) {
        Diem3D posWrist = matWrist.bienDoi(Diem3D(0, 0, 0));
        float kc = (posWrist - viTriBanThucNghiem).doDai();
        if (kc <= 3.8f && gocKep < 60.0f) {
            dangBiGapTrenBan = true;
            MaTran4x4 invWrist = matWrist.nghichDaoBienDoiCung();
            MaTran4x4 matWorldObj = taoMaTranTinhTien(viTriBanThucNghiem.x, viTriBanThucNghiem.y, viTriBanThucNghiem.z);
            matLocalObjBan = matWorldObj * invWrist;
            return true;
        }
        return false;
    }

    void thaVatTrenBan(const MaTran4x4& matWrist) {
        if (!dangBiGapTrenBan) return;
        MaTran4x4 matWorldObj = matLocalObjBan * matWrist;
        viTriBanThucNghiem = matWorldObj.bienDoi(Diem3D(0, 0, 0));
        viTriBanThucNghiem.y = kichThuoc * 0.5f;
        dangBiGapTrenBan = false;
    }

    bool thuGapVatTrenBan(const CungCapTayGap& arm) {
        MaTran4x4 mBase, mElbow, mWrist;
        arm.tinhMaTranBanThucNghiem(mBase, mElbow, mWrist, 0.0f);
        return thuGapVatTrenBan(mWrist, arm.servoKep.gocHienTai);
    }

    void thaVatTrenBan(const CungCapTayGap& arm) {
        MaTran4x4 mBase, mElbow, mWrist;
        arm.tinhMaTranBanThucNghiem(mBase, mElbow, mWrist, 0.0f);
        thaVatTrenBan(mWrist);
    }

    void capNhatTrenBan(const MaTran4x4& matWrist) {
        if (dangBiGapTrenBan) {
            MaTran4x4 matWorldObj = matLocalObjBan * matWrist;
            viTriBanThucNghiem = matWorldObj.bienDoi(Diem3D(0, 0, 0));
        }
    }
};

// Standard Type Aliases
using RobotChassis = DongHocXe;
using RobotArm = CungCapTayGap;
using InteractiveObject = VatTheTuongTac;

#endif // ROBOT_KINEMATICS_H
