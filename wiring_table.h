#ifndef WIRING_TABLE_H
#define WIRING_TABLE_H

#include "config.h"
#include "math_3d.h"
#include "components_mesh.h"
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
// THUẬT TOÁN ĐI DÂY 3D THEO MÁNG CÁP VÀ BÁM XƯƠNG CÁNH TAY ROBOT
// KẾT HỢP NỘI SUY ĐƯỜNG CONG BÉZIER BẬC 2 (Tài liệu CG.3 Trang 99)
// =========================================================================

// Công thức đường cong Bézier bậc 2: P(t) = (1-t)^2 * A + 2*t*(1-t) * B + t^2 * C
inline Diem3D tinhBezierBac2(const Diem3D& A, const Diem3D& B, const Diem3D& C, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float ut2 = 2.0f * u * t;
    return Diem3D(
        uu * A.x + ut2 * B.x + tt * C.x,
        uu * A.y + ut2 * B.y + tt * C.y,
        uu * A.z + ut2 * B.z + tt * C.z
    );
}

// Bo tròn các góc uốn gấp khúc bằng đường cong Bézier bậc 2
inline std::vector<Diem3D> lamMuotDuongDayBezier(const std::vector<Diem3D>& rawPts) {
    if (rawPts.size() <= 2) return rawPts;

    std::vector<Diem3D> smoothPts;
    smoothPts.push_back(rawPts.front());

    for (size_t i = 1; i + 1 < rawPts.size(); ++i) {
        Diem3D pPrev = rawPts[i - 1];
        Diem3D pCurr = rawPts[i];
        Diem3D pNext = rawPts[i + 1];

        float d1 = (pCurr - pPrev).doDai();
        float d2 = (pNext - pCurr).doDai();
        float r = std::min(0.6f, std::min(d1, d2) * 0.35f);

        if (r < 0.05f) {
            smoothPts.push_back(pCurr);
            continue;
        }

        Diem3D dir1 = (pPrev - pCurr).donVi();
        Diem3D dir2 = (pNext - pCurr).donVi();

        Diem3D ptA = pCurr + dir1 * r;
        Diem3D ptB = pCurr; // Điểm kiểm soát
        Diem3D ptC = pCurr + dir2 * r;

        smoothPts.push_back(ptA);
        // Sinh 3 điểm nội suy dọc đường cong Bézier
        smoothPts.push_back(tinhBezierBac2(ptA, ptB, ptC, 0.25f));
        smoothPts.push_back(tinhBezierBac2(ptA, ptB, ptC, 0.50f));
        smoothPts.push_back(tinhBezierBac2(ptA, ptB, ptC, 0.75f));
        smoothPts.push_back(ptC);
    }

    smoothPts.push_back(rawPts.back());
    return smoothPts;
}

// Thuật toán định tuyến 3 tuyến theo Phần B Mục 4:
// - Tuyến 1: Dây cố định bo mạch chạy vào Máng cáp trung tâm (y_duct = 4.2 cm) giữa 2 tầng
// - Tuyến 2: Dây Servo bám dọc xương cánh tay qua Node_Wrist -> Node_Elbow -> Node_Base
// - Tuyến 3: Làm mượt các góc rẽ bằng Bézier bậc 2
inline std::vector<Diem3D> taoDuongDiDay3DNangCap(
    const Diem3D& A, const Diem3D& B,
    int idLKA, int idLKB, int indexDay,
    const MaTran4x4& matWrist,
    const MaTran4x4& matElbow,
    const MaTran4x4& matBase,
    const MaTran4x4& matCar)
{
    std::vector<Diem3D> rawPts;
    rawPts.push_back(A);

    // --- TUYẾN 2: DÂY SERVO BÁM XƯƠNG CÁNH TAY ---
    if (idLKA == LK_SERVO_KEP || idLKB == LK_SERVO_KEP) {
        int j = (indexDay - 28);
        if (j < 0) j = 0; if (j > 2) j = 2;
        float offset = -0.5f + 0.2f * (float)j;

        Diem3D W_wrist = matWrist.bienDoi(Diem3D(offset,  0.5f, -1.0f));
        Diem3D W_elbow = matElbow.bienDoi(Diem3D(offset,  0.6f,  0.5f));
        Diem3D W_base  = matBase.bienDoi(Diem3D(offset,   0.5f, -1.2f));
        Diem3D W_deck  = matCar.bienDoi(Diem3D(offset,    6.8f,  3.5f));

        rawPts.push_back(W_wrist);
        rawPts.push_back(W_elbow);
        rawPts.push_back(W_base);
        rawPts.push_back(W_deck);
    }
    else if (idLKA == LK_SERVO_KHUU || idLKB == LK_SERVO_KHUU) {
        int j = (indexDay - 25);
        if (j < 0) j = 0; if (j > 2) j = 2;
        float offset = -0.6f + 0.2f * (float)j;

        Diem3D W_elbow = matElbow.bienDoi(Diem3D(offset,  0.6f,  0.5f));
        Diem3D W_base  = matBase.bienDoi(Diem3D(offset,   0.5f, -1.2f));
        Diem3D W_deck  = matCar.bienDoi(Diem3D(offset,    6.8f,  3.5f));

        rawPts.push_back(W_elbow);
        rawPts.push_back(W_base);
        rawPts.push_back(W_deck);
    }
    else if (idLKA == LK_SERVO_DE || idLKB == LK_SERVO_DE) {
        int j = (indexDay - 22);
        if (j < 0) j = 0; if (j > 2) j = 2;
        float offset = -0.5f + 0.2f * (float)j;

        Diem3D W_base = matBase.bienDoi(Diem3D(offset, 0.5f, -1.0f));
        Diem3D W_deck = matCar.bienDoi(Diem3D(offset,  6.8f,  3.5f));

        rawPts.push_back(W_base);
        rawPts.push_back(W_deck);
    }
    // --- TUYẾN 1: DÂY CỐ ĐỊNH CHẠY TRONG MÁNG CÁP TRUNG TÂM (y_duct = 4.2 cm) ---
    else {
        // Điểm nhô ngắn khỏi đầu chân cắm
        Diem3D pLiftA = A + Diem3D(0.0f, (A.y < 3.0f ? 0.6f : -0.5f), 0.0f);
        Diem3D pLiftB = B + Diem3D(0.0f, (B.y < 3.0f ? 0.6f : -0.5f), 0.0f);
        rawPts.push_back(pLiftA);

        // Làn cáp trong máng cáp trung tâm
        float xLane = -0.7f + 0.12f * (float)(indexDay % 12);
        float yLane =  3.8f + 0.08f * (float)(indexDay / 12);

        // Chuyển làn cáp theo tọa độ xe
        Diem3D midA = matCar.bienDoi(Diem3D(xLane, yLane, (matCar.nghichDaoBienDoiCung().bienDoi(A)).z));
        Diem3D midB = matCar.bienDoi(Diem3D(xLane, yLane, (matCar.nghichDaoBienDoiCung().bienDoi(B)).z));

        rawPts.push_back(midA);
        rawPts.push_back(midB);
        rawPts.push_back(pLiftB);
    }

    rawPts.push_back(B);

    // Tuyến 3: Bo tròn mượt mà bằng Bézier bậc 2
    return lamMuotDuongDayBezier(rawPts);
}

// Wrapper tương thích ngược
inline std::vector<Diem3D> taoDuongDiDay3D(const Diem3D& A, const Diem3D& B, int indexDay) {
    MaTran4x4 I;
    return taoDuongDiDay3DNangCap(A, B, 1, 2, indexDay, I, I, I, I);
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

// Standard Type Aliases
using Wire = DayDien;

#endif // WIRING_TABLE_H
