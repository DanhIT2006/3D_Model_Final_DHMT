#ifndef DONG_HOC_XE_TAY_GAP_H
#define DONG_HOC_XE_TAY_GAP_H

#include "cau_hinh.h"
#include "toan_hoc_do_hoa.h"
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

    // Kích thước các đốt tay máy (cm)
    float hChassis = 1.6f;
    float L1 = 4.5f; // Chiều dài đốt dưới
    float L2 = 5.0f; // Chiều dài đốt trên

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

        // 1. Đế xoay (Node_Base): Quay quanh trục Y góc (theta1 - 90)
        float gocDeRad = (servoDe.gocHienTai - 90.0f) * (float)M_PI / 180.0f;
        MaTran4x4 matLocalBase = taoMaTranQuayY(gocDeRad) * taoMaTranTinhTien(0.0f, hChassis, 4.5f);
        matWorldBase = matLocalBase * matWorldCar;

        // 2. Khớp khuỷu (Node_Elbow): Quay quanh trục X góc (theta2 - 90)
        float gocKhuuRad = (servoKhuu.gocHienTai - 90.0f) * (float)M_PI / 180.0f;
        MaTran4x4 matLocalElbow = taoMaTranQuayX(gocKhuuRad) * taoMaTranTinhTien(0.0f, L1, 0.0f);
        matWorldElbow = matLocalElbow * matWorldBase;

        // 3. Cổ tay & Ngàm gắp (Node_Wrist)
        MaTran4x4 matLocalWrist = taoMaTranTinhTien(0.0f, 0.0f, L2);
        matWorldWrist = matLocalWrist * matWorldElbow;
    }
};

// =========================================================================
// 4. VẬT THỂ TƯƠNG TÁC (KHỐI HỘP GẮP / THẢ - Mục 3.4 Đề cương)
// Chuyển đổi không gian ma trận không nhảy vị trí (Attach / Detach)
// =========================================================================

struct VatTheTuongTac {
    Diem3D viTriTheGioi = Diem3D(0.0f, 1.2f, 16.0f); // Vị trí khối hộp trên sàn
    float kichThuoc = 2.4f; // Cạnh 2.4 cm
    MauSac mauSac = MauSac(46, 204, 113); // Màu xanh ngọc bắt mắt
    bool dangBiGap = false;
    MaTran4x4 matLocalObj; // Tọa độ tương đối khi gắn vào Wrist

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
};

#endif // DONG_HOC_XE_TAY_GAP_H
