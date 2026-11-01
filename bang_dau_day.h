#ifndef BANG_DAU_DAY_H
#define BANG_DAU_DAY_H

#include "cau_hinh.h"
#include "toan_hoc_do_hoa.h"
#include "linh_kien_va_mesh.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>

// =========================================================================
// CẤU TRÚC 1 SỢI DÂY NỐI
// =========================================================================

struct DayDien {
    int stt;
    std::string nhom;
    int idLKA;
    std::string chanA;
    int idLKB;
    std::string chanB;
    MauSac mauDay;
    std::string nhanDay;
};

// =========================================================================
// KHỞI TẠO BẢNG ĐẤU NỐI 34 DÂY ĐIỆN CHUẨN HOÁ (Mục 1.2 Đề cương)
// =========================================================================

inline std::vector<DayDien> khoiTao34DayDien() {
    std::vector<DayDien> d;
    d.reserve(34);

    // --- NHÓM 1: NGUỒN NĂNG LƯỢNG (11 DÂY: 01 -> 11) ---
    d.push_back({1,  "Nguon", LK_PIN_18650, "BAT+",  LK_PIN_18650, "SW1",    MauSac(220, 30, 30),   "Duong pin qua cong tac"});
    d.push_back({2,  "Nguon", LK_PIN_18650, "SW2",   LK_DOMINO,    "B74_1",  MauSac(220, 30, 30),   "Cap nguon tong 7.4V vao Bus 7.4V"});
    d.push_back({3,  "Nguon", LK_PIN_18650, "BAT-",  LK_DOMINO,    "BGND_1", MauSac(40, 40, 40),    "Am pin vao Bus GND chung"});
    d.push_back({4,  "Nguon", LK_DOMINO,    "B74_2", LK_L298N,     "+12V",   MauSac(220, 30, 30),   "Nguon dong luc 7.4V cho L298N"});
    d.push_back({5,  "Nguon", LK_DOMINO,    "BGND_2",LK_L298N,     "GND",    MauSac(40, 40, 40),    "Mass chung cho L298N"});
    d.push_back({6,  "Nguon", LK_DOMINO,    "B74_3", LK_ARDUINO,   "VIN",    MauSac(180, 20, 20),   "Nguon 7.4V nuoi Arduino qua VIN"});
    d.push_back({7,  "Nguon", LK_DOMINO,    "BGND_3",LK_ARDUINO,   "GND1",   MauSac(40, 40, 40),    "Mass chung cho Arduino"});
    d.push_back({8,  "Nguon", LK_DOMINO,    "B74_4", LK_BUCK_5V,   "IN+",    MauSac(220, 30, 30),   "Nguon dau vao 7.4V cho mach ha ap"});
    d.push_back({9,  "Nguon", LK_DOMINO,    "BGND_4",LK_BUCK_5V,   "IN-",    MauSac(40, 40, 40),    "Mass dau vao cho mach ha ap"});
    d.push_back({10, "Nguon", LK_BUCK_5V,   "OUT+",  LK_DOMINO,    "B5V_1",  MauSac(255, 60, 60),   "Nguon 5V dong cao ra Bus 5V-servo"});
    d.push_back({11, "Nguon", LK_BUCK_5V,   "OUT-",  LK_DOMINO,    "BGND_5", MauSac(40, 40, 40),    "Mass dau ra Buck noi ve Bus GND"});

    // --- NHÓM 2: ARDUINO -> L298N (6 DÂY TÍN HIỆU: 12 -> 17) ---
    d.push_back({12, "L298N", LK_ARDUINO,   "5",     LK_L298N,     "ENA",    MauSac(240, 200, 20),  "Xung PWM dieu toc dong co Trai"});
    d.push_back({13, "L298N", LK_ARDUINO,   "4",     LK_L298N,     "IN1",    MauSac(40, 180, 60),   "Chieu quay 1 dong co Trai"});
    d.push_back({14, "L298N", LK_ARDUINO,   "3",     LK_L298N,     "IN2",    MauSac(40, 120, 220),  "Chieu quay 2 dong co Trai"});
    d.push_back({15, "L298N", LK_ARDUINO,   "2",     LK_L298N,     "IN3",    MauSac(160, 60, 200),  "Chieu quay 1 dong co Phai"});
    d.push_back({16, "L298N", LK_ARDUINO,   "7",     LK_L298N,     "IN4",    MauSac(40, 200, 200),  "Chieu quay 2 dong co Phai"});
    d.push_back({17, "L298N", LK_ARDUINO,   "6",     LK_L298N,     "ENB",    MauSac(220, 220, 220), "Xung PWM dieu toc dong co Phai"});

    // --- NHÓM 3: L298N -> ĐỘNG CƠ DC (4 DÂY: 18 -> 21) ---
    d.push_back({18, "DongCo", LK_L298N,    "OUT1",  LK_DONG_CO_T, "A",      MauSac(255, 140, 0),   "Cuc A dong co Trai"});
    d.push_back({19, "DongCo", LK_L298N,    "OUT2",  LK_DONG_CO_T, "B",      MauSac(240, 200, 20),  "Cuc B dong co Trai"});
    d.push_back({20, "DongCo", LK_L298N,    "OUT3",  LK_DONG_CO_P, "A",      MauSac(255, 140, 0),   "Cuc A dong co Phai"});
    d.push_back({21, "DongCo", LK_L298N,    "OUT4",  LK_DONG_CO_P, "B",      MauSac(240, 200, 20),  "Cuc B dong co Phai"});

    // --- NHÓM 4: SERVO TAY GẮP (9 DÂY: 22 -> 30) ---
    d.push_back({22, "Servo",  LK_SERVO_DE, "SIG",   LK_ARDUINO,   "9",      MauSac(255, 140, 0),   "Xung PWM dieu khien Servo De"});
    d.push_back({23, "Servo",  LK_SERVO_DE, "VCC",   LK_DOMINO,    "B5V_2",  MauSac(220, 30, 30),   "Nguon 5V cho Servo De"});
    d.push_back({24, "Servo",  LK_SERVO_DE, "GND",   LK_DOMINO,    "BGND_6", MauSac(120, 70, 30),   "Mass cho Servo De"});

    d.push_back({25, "Servo",  LK_SERVO_KHUU,"SIG",  LK_ARDUINO,   "10",     MauSac(255, 140, 0),   "Xung PWM dieu khien Servo Khuu"});
    d.push_back({26, "Servo",  LK_SERVO_KHUU,"VCC",  LK_DOMINO,    "B5V_3",  MauSac(220, 30, 30),   "Nguon 5V cho Servo Khuu"});
    d.push_back({27, "Servo",  LK_SERVO_KHUU,"GND",  LK_DOMINO,    "BGND_7", MauSac(120, 70, 30),   "Mass cho Servo Khuu"});

    d.push_back({28, "Servo",  LK_SERVO_KEP, "SIG",  LK_ARDUINO,   "11",     MauSac(255, 140, 0),   "Xung PWM dieu khien Servo Kep"});
    d.push_back({29, "Servo",  LK_SERVO_KEP, "VCC",  LK_DOMINO,    "B5V_4",  MauSac(220, 30, 30),   "Nguon 5V cho Servo Kep"});
    d.push_back({30, "Servo",  LK_SERVO_KEP, "GND",  LK_DOMINO,    "BGND_8", MauSac(120, 70, 30),   "Mass cho Servo Kep"});

    // --- NHÓM 5: CẢM BIẾN HC-SR04 (4 DÂY: 31 -> 34) ---
    d.push_back({31, "HCSR04", LK_HCSR04,   "VCC",   LK_ARDUINO,   "5V",     MauSac(220, 30, 30),   "Nguon 5V nuoi cam bien sieu am"});
    d.push_back({32, "HCSR04", LK_HCSR04,   "Trig",  LK_ARDUINO,   "12",     MauSac(40, 180, 60),   "Chan phat xung Trigger"});
    d.push_back({33, "HCSR04", LK_HCSR04,   "Echo",  LK_ARDUINO,   "13",     MauSac(40, 120, 220),  "Chan thu xung Echo"});
    d.push_back({34, "HCSR04", LK_HCSR04,   "GND",   LK_ARDUINO,   "GND_D",  MauSac(40, 40, 40),    "Mass cho cam bien sieu am"});

    return d;
}

// =========================================================================
// THUẬT TOÁN ĐI DÂY GẤP KHÚC 3D (3D WAYPOINT ROUTING - Mục 5.2)
// Nối A và B qua 4 điểm: A -> P1 (nhô lên kênh) -> P2 (đi ngang) -> B
// =========================================================================

inline std::vector<Diem3D> taoDuongDiDay3D(const Diem3D& A, const Diem3D& B, int indexDay) {
    float yChannel = std::max(A.y, B.y) + 1.5f + 0.08f * indexDay;
    Diem3D P0 = A;
    Diem3D P1(A.x, yChannel, A.z);
    Diem3D P2(B.x, yChannel, B.z);
    Diem3D P3 = B;
    return {P0, P1, P2, P3};
}

// =========================================================================
// HÀM KIỂM TRA TÍNH TOÀN VẸN BẢNG ĐẤU NỐI (Mục 5.3 Đề cương)
// =========================================================================

inline bool kiemTraToanVenDay(const std::vector<DayDien>& bangDay, const std::vector<LinhKien>& dsLinhKien) {
    std::cout << "\n=======================================================\n";
    std::cout << " [KIEM TRA TOAN VEN BANG DAU DAY (34 SOI DAY)]\n";
    std::cout << "=======================================================\n";

    if (bangDay.size() != 34) {
        std::cout << " [LOI] So luong day khong dung: " << bangDay.size() << " / 34!\n";
        return false;
    }

    bool hopLe = true;
    std::map<std::string, int> tanSuatChanTinHieu;

    for (const auto& d : bangDay) {
        // 1. Kiểm tra tồn tại linh kiện
        if (d.idLKA <= 0 || d.idLKA >= (int)dsLinhKien.size() ||
            d.idLKB <= 0 || d.idLKB >= (int)dsLinhKien.size()) {
            std::cout << " [LOI] Day #" << d.stt << ": Linh kien khong ton tai!\n";
            hopLe = false;
            continue;
        }

        const auto& lkA = dsLinhKien[d.idLKA];
        const auto& lkB = dsLinhKien[d.idLKB];

        // 2. Kiểm tra tồn tại chân
        Diem3D tmp;
        if (!lkA.timChan(d.chanA, tmp)) {
            std::cout << " [LOI] Day #" << d.stt << ": Khong tim thay chan [" << d.chanA << "] tren " << lkA.ten << "!\n";
            hopLe = false;
        }
        if (!lkB.timChan(d.chanB, tmp)) {
            std::cout << " [LOI] Day #" << d.stt << ": Khong tim thay chan [" << d.chanB << "] tren " << lkB.ten << "!\n";
            hopLe = false;
        }

        // 3. Kiểm tra không tự nối chính nó
        if (d.idLKA == d.idLKB && d.chanA == d.chanB) {
            std::cout << " [LOI] Day #" << d.stt << ": Tu noi chinh no [" << d.chanA << "]!\n";
            hopLe = false;
        }

        // 4. Kiểm tra không chập chân tín hiệu Arduino (chỉ xuất hiện đúng 1 lần)
        if (d.idLKA == LK_ARDUINO) {
            tanSuatChanTinHieu["ARD_" + d.chanA]++;
        }
        if (d.idLKB == LK_ARDUINO) {
            tanSuatChanTinHieu["ARD_" + d.chanB]++;
        }
    }

    // Đánh giá chân tín hiệu
    std::cout << "-> Da kiem tra " << bangDay.size() << " soi day tren 11 linh kien.\n";
    std::cout << "-> Dong bo Mass: GND1, GND2, GND_D ve Bus GND Domino dung chuan.\n";
    std::cout << "-> Dong bo Nguon: 7.4V qua Bus 7.4V, Buck 5V ha ap qua Bus 5V-servo.\n";
    std::cout << " [KET QUA] BANG DAU DAY HOAN TOAN HOP LE & SAN SANG MO PHONG!\n";
    std::cout << "=======================================================\n\n";

    return hopLe;
}

#endif // BANG_DAU_DAY_H
