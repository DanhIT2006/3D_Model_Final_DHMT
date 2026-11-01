#ifndef COMPONENTS_MESH_H
#define COMPONENTS_MESH_H

#include "config.h"
#include "math_3d.h"
#include <vector>
#include <string>

// =========================================================================
// 1. CẤU TRÚC HÌNH HỌC MESH 3D
// =========================================================================

struct MatDaGiac {
    std::vector<int> cacDinh; // Chỉ số các đỉnh theo chiều ngược kim đồng hồ (CCW)
    MauSac mauSac;
    Diem3D phapTuyen;
    float doSauZ;             // Độ sâu trung bình dùng sắp xếp Painter's Algorithm
};

struct DoanCanh {
    int dau, cuoi;
};

struct Mesh3D {
    std::vector<Diem3D> dinh;
    std::vector<DoanCanh> canh;
    std::vector<MatDaGiac> mat;
};

// =========================================================================
// 2. KHỞI TẠO CÁC PRIMITIVE CƠ BẢN: makeBox & makeCylinder (Giai đoạn 2.1)
// =========================================================================

// Tạo khối hộp chữ nhật (w x h x d) tại gốc tọa độ cục bộ
inline Mesh3D taoKhoiHop(float w, float h, float d, MauSac mau) {
    Mesh3D m;
    float x = w * 0.5f, y = h * 0.5f, z = d * 0.5f;

    // 8 Đỉnh
    m.dinh = {
        {-x, -y,  z}, { x, -y,  z}, { x,  y,  z}, {-x,  y,  z}, // Mặt trước: 0,1,2,3
        {-x, -y, -z}, { x, -y, -z}, { x,  y, -z}, {-x,  y, -z}  // Mặt sau:   4,5,6,7
    };

    // 12 Đoạn cạnh
    m.canh = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    // 6 Mặt tứ giác theo chiều CCW khi nhìn từ ngoài vào
    m.mat.push_back({{0, 1, 2, 3}, mau.lamSang(1.05f), { 0,  0,  1}, 0}); // Trước (+Z)
    m.mat.push_back({{5, 4, 7, 6}, mau.lamToi(0.85f),  { 0,  0, -1}, 0}); // Sau (-Z)
    m.mat.push_back({{1, 5, 6, 2}, mau.lamToi(0.92f),  { 1,  0,  0}, 0}); // Phải (+X)
    m.mat.push_back({{4, 0, 3, 7}, mau.lamToi(0.80f),  {-1,  0,  0}, 0}); // Trái (-X)
    m.mat.push_back({{3, 2, 6, 7}, mau.lamSang(1.15f), { 0,  1,  0}, 0}); // Trên (+Y)
    m.mat.push_back({{4, 5, 1, 0}, mau.lamToi(0.70f),  { 0, -1,  0}, 0}); // Dưới (-Y)

    return m;
}

// Tạo khối trụ dọc theo trục Y (bán kính r, chiều cao h)
inline Mesh3D taoKhoiTru(float r, float h, int segments, MauSac mau) {
    Mesh3D m;
    float halfH = h * 0.5f;

    // Tạo 2 vòng đỉnh trên và dưới
    for (int k = 0; k < 2; ++k) {
        float y = (k == 0) ? -halfH : halfH;
        for (int i = 0; i < segments; ++i) {
            float ang = 2.0f * M_PI * i / segments;
            m.dinh.push_back({r * std::cos(ang), y, r * std::sin(ang)});
        }
    }

    // Các mặt bên và nắp
    std::vector<int> napDuoi, napTren;
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int d0 = i;
        int d1 = next;
        int d2 = segments + next;
        int d3 = segments + i;

        m.canh.push_back({d0, d1});
        m.canh.push_back({d3, d2});
        m.canh.push_back({d0, d3});

        float angMid = 2.0f * M_PI * (i + 0.5f) / segments;
        Diem3D norm(std::cos(angMid), 0.0f, std::sin(angMid));
        float shade = 0.85f + 0.20f * std::cos(angMid);

        m.mat.push_back({{d0, d1, d2, d3}, mau.lamSang(shade), norm, 0});
        napDuoi.push_back(segments - 1 - i);
        napTren.push_back(segments + i);
    }

    // Nắp dưới và nắp trên
    m.mat.push_back({napDuoi, mau.lamToi(0.75f), {0, -1, 0}, 0});
    m.mat.push_back({napTren, mau.lamSang(1.15f), {0,  1, 0}, 0});

    return m;
}

// =========================================================================
// 3. CẤU TRÚC DỮ LIỆU PART, PIN, COMPONENT (Giai đoạn 2.2)
// =========================================================================

struct Part {
    std::string ten;
    Mesh3D mesh;
    MaTran4x4 bienDoiCucBo;
};

struct Pin {
    std::string ten;
    Diem3D viTriCucBo; // Tọa độ cục bộ trên linh kiện (cm)
    int parentNodeId;  // -1 nếu cố định trên linh kiện, hoặc ID khớp phân cấp
};

struct LinhKien {
    int id;
    std::string ma;
    std::string ten;
    std::string vaiTro;
    std::vector<Part> parts;
    std::vector<Pin> pins;
    MaTran4x4 viTriLapDat; // Vị trí cơ sở trên khung xe
    Diem3D vectorTachRoi;  // Vector hướng nổ khi kéo Slider Explode (cm)
    bool hienThi = true;
    bool coLap   = false;
    Diem2D toaDo2D;        // Tọa độ trên sơ đồ 2D (cm / px)
    float rong2D = 60.0f;
    float cao2D  = 40.0f;

    // Tìm tọa độ cục bộ của một chân
    bool timChan(const std::string& tenChan, Diem3D& outPos) const {
        for (const auto& p : pins) {
            if (p.ten == tenChan) {
                outPos = p.viTriCucBo;
                return true;
            }
        }
        return false;
    }
};

// =========================================================================
// 4. KHỞI TẠO ĐẦY ĐỦ 11 LINH KIỆN THEO ĐẶC TẢ BẢNG 1.1
// =========================================================================

inline void khoiTao11LinhKien(std::vector<LinhKien>& ds) {
    ds.clear();
    ds.resize(TONG_SO_LINH_KIEN + 1);

    // ---------------------------------------------------------------------
    // 1. ARDUINO UNO R3 (ARD) - Kích thước: 6.8 x 1.2 x 5.3 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_ARDUINO;
        lk.ma = "ARD";
        lk.ten = "Arduino Uno R3";
        lk.vaiTro = "Bo dieu khien trung tam: Giao tiep cam bien, xu ly dong hoc xe va phat xung PWM dieu khien 3 Servo tay gap.";
        lk.vectorTachRoi = Diem3D(0.0f, 8.0f, 0.0f);
        lk.viTriLapDat = taoMaTranTinhTien(0.0f, 1.2f, -1.5f);
        lk.toaDo2D = Diem2D(460.0f, 380.0f);
        lk.rong2D = 130.0f; lk.cao2D = 90.0f;

        // PCB Xanh dương
        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(6.8f, 0.20f, 5.3f, MauSac(15, 85, 155));
        lk.parts.push_back(pcb);

        // Cổng USB kim loại bạc
        Part usb;
        usb.ten = "USB";
        usb.mesh = taoKhoiHop(1.2f, 1.0f, 1.6f, MauSac(200, 205, 215));
        usb.bienDoiCucBo = taoMaTranTinhTien(-2.8f, 0.6f, -1.8f);
        lk.parts.push_back(usb);

        // Jack nguồn DC đen
        Part dc;
        dc.ten = "JackDC";
        dc.mesh = taoKhoiHop(1.4f, 1.1f, 0.9f, MauSac(35, 35, 38));
        dc.bienDoiCucBo = taoMaTranTinhTien(-2.7f, 0.65f, 1.8f);
        lk.parts.push_back(dc);

        // Chip ATmega328P
        Part chip;
        chip.ten = "ATmega328P";
        chip.mesh = taoKhoiHop(3.2f, 0.25f, 0.85f, MauSac(25, 25, 28));
        chip.bienDoiCucBo = taoMaTranTinhTien(0.6f, 0.22f, 0.2f);
        lk.parts.push_back(chip);

        // Các hàng chân Header (Pins)
        // Hàng số 13 chân: D0-D13 ở mép z = -2.4 cm
        const char* pinSo[] = {"GND_D", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2"};
        for (int i = 0; i < 13; ++i) {
            float xPos = -1.5f + i * 0.35f;
            lk.pins.push_back({pinSo[i], Diem3D(xPos, 0.6f, -2.4f), -1});
        }
        // Hàng Nguồn (Power) ở mép z = +2.4 cm phía trái
        lk.pins.push_back({"5V",   Diem3D(-1.8f, 0.6f, 2.4f), -1});
        lk.pins.push_back({"GND1", Diem3D(-1.4f, 0.6f, 2.4f), -1});
        lk.pins.push_back({"GND2", Diem3D(-1.0f, 0.6f, 2.4f), -1});
        lk.pins.push_back({"VIN",  Diem3D(-0.6f, 0.6f, 2.4f), -1});

        // Hàng Analog A0-A5 ở mép z = +2.4 cm phía phải
        const char* pinAnalog[] = {"A0", "A1", "A2", "A3", "A4", "A5"};
        for (int i = 0; i < 6; ++i) {
            lk.pins.push_back({pinAnalog[i], Diem3D(0.4f + i * 0.35f, 0.6f, 2.4f), -1});
        }

        ds[LK_ARDUINO] = lk;
    }

    // ---------------------------------------------------------------------
    // 2. MODULE L298N (L298N) - Kích thước: 4.3 x 2.7 x 4.3 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_L298N;
        lk.ma = "L298N";
        lk.ten = "Module Cau H L298N";
        lk.vaiTro = "Khuech dai cong suat dieu khien 2 dong co DC banh xe, nhan PWM (ENA, ENB) va tin hieu huong tu Arduino.";
        lk.vectorTachRoi = Diem3D(-6.0f, 6.0f, 0.0f);
        lk.viTriLapDat = taoMaTranTinhTien(-3.8f, 1.2f, -1.8f);
        lk.toaDo2D = Diem2D(250.0f, 420.0f);
        lk.rong2D = 100.0f; lk.cao2D = 90.0f;

        // PCB Đỏ
        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(4.3f, 0.20f, 4.3f, MauSac(175, 25, 25));
        lk.parts.push_back(pcb);

        // Tản nhiệt nhôm đen đa rãnh
        Part heatsink;
        heatsink.ten = "TanNhiet";
        heatsink.mesh = taoKhoiHop(2.0f, 2.5f, 1.5f, MauSac(35, 38, 42));
        heatsink.bienDoiCucBo = taoMaTranTinhTien(0.0f, 1.35f, -0.4f);
        lk.parts.push_back(heatsink);

        // 2 Tụ lọc hình trụ
        Part c1; c1.ten = "Tu1"; c1.mesh = taoKhoiTru(0.45f, 1.2f, 10, MauSac(25, 30, 80));
        c1.bienDoiCucBo = taoMaTranTinhTien(-1.3f, 0.7f, 1.2f);
        lk.parts.push_back(c1);

        Part c2; c2.ten = "Tu2"; c2.mesh = taoKhoiTru(0.45f, 1.2f, 10, MauSac(25, 30, 80));
        c2.bienDoiCucBo = taoMaTranTinhTien(1.3f, 0.7f, 1.2f);
        lk.parts.push_back(c2);

        // Hàng tín hiệu 6 chân ở mép trước: ENA, IN1, IN2, IN3, IN4, ENB
        const char* l298Sig[] = {"ENA", "IN1", "IN2", "IN3", "IN4", "ENB"};
        for (int i = 0; i < 6; ++i) {
            lk.pins.push_back({l298Sig[i], Diem3D(-1.25f + i * 0.5f, 0.35f, 1.9f), -1});
        }
        // Cổng vít động cơ OUT1, OUT2 (trái) và OUT3, OUT4 (phải)
        lk.pins.push_back({"OUT1", Diem3D(-1.9f, 0.5f, -0.7f), -1});
        lk.pins.push_back({"OUT2", Diem3D(-1.9f, 0.5f,  0.7f), -1});
        lk.pins.push_back({"OUT3", Diem3D( 1.9f, 0.5f, -0.7f), -1});
        lk.pins.push_back({"OUT4", Diem3D( 1.9f, 0.5f,  0.7f), -1});

        // Cổng nguồn: +12V, GND, +5V ở mép trước
        lk.pins.push_back({"+12V", Diem3D(-1.2f, 0.5f, 1.3f), -1});
        lk.pins.push_back({"GND",  Diem3D(-0.4f, 0.5f, 1.3f), -1});
        lk.pins.push_back({"+5V",  Diem3D( 0.4f, 0.5f, 1.3f), -1});

        ds[LK_L298N] = lk;
    }

    // ---------------------------------------------------------------------
    // 3. ĐỘNG CƠ DC TRÁI (MOT_L) - Kích thước: 2.2 x 2.2 x 6.5 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_DONG_CO_T;
        lk.ma = "MOT_L";
        lk.ten = "Dong co DC Trai";
        lk.vaiTro = "Truyen dong truc tiep cho banh xe trai thong qua hop so giam toc ti so 1:48.";
        lk.vectorTachRoi = Diem3D(-8.0f, -3.0f, 0.0f);
        lk.viTriLapDat = taoMaTranTinhTien(-5.2f, 0.0f, -1.0f);
        lk.toaDo2D = Diem2D(80.0f, 480.0f);
        lk.rong2D = 80.0f; lk.cao2D = 50.0f;

        Part hopSo;
        hopSo.ten = "HopSo";
        hopSo.mesh = taoKhoiHop(2.2f, 1.9f, 3.7f, MauSac(235, 190, 30));
        lk.parts.push_back(hopSo);

        Part moto;
        moto.ten = "MoTo";
        moto.mesh = taoKhoiTru(1.0f, 2.8f, 12, MauSac(225, 230, 235));
        moto.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.0f, -2.5f) * taoMaTranQuayX(M_PI * 0.5f);
        lk.parts.push_back(moto);

        lk.pins.push_back({"A", Diem3D(-0.5f, 0.0f, -3.8f), -1});
        lk.pins.push_back({"B", Diem3D( 0.5f, 0.0f, -3.8f), -1});

        ds[LK_DONG_CO_T] = lk;
    }

    // ---------------------------------------------------------------------
    // 4. ĐỘNG CƠ DC PHẢI (MOT_R) - Kích thước: 2.2 x 2.2 x 6.5 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_DONG_CO_P;
        lk.ma = "MOT_R";
        lk.ten = "Dong co DC Phai";
        lk.vaiTro = "Truyen dong truc tiep cho banh xe phai thong qua hop so giam toc ti so 1:48.";
        lk.vectorTachRoi = Diem3D(8.0f, -3.0f, 0.0f);
        lk.viTriLapDat = taoMaTranTinhTien(5.2f, 0.0f, -1.0f);
        lk.toaDo2D = Diem2D(80.0f, 350.0f);
        lk.rong2D = 80.0f; lk.cao2D = 50.0f;

        Part hopSo;
        hopSo.ten = "HopSo";
        hopSo.mesh = taoKhoiHop(2.2f, 1.9f, 3.7f, MauSac(235, 190, 30));
        lk.parts.push_back(hopSo);

        Part moto;
        moto.ten = "MoTo";
        moto.mesh = taoKhoiTru(1.0f, 2.8f, 12, MauSac(225, 230, 235));
        moto.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.0f, -2.5f) * taoMaTranQuayX(M_PI * 0.5f);
        lk.parts.push_back(moto);

        lk.pins.push_back({"A", Diem3D(-0.5f, 0.0f, -3.8f), -1});
        lk.pins.push_back({"B", Diem3D( 0.5f, 0.0f, -3.8f), -1});

        ds[LK_DONG_CO_P] = lk;
    }

    // ---------------------------------------------------------------------
    // 5. SERVO ĐẾ (SRV_BASE) - Kích thước: 4.0 x 3.8 x 2.0 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_SERVO_DE;
        lk.ma = "SRV_BASE";
        lk.ten = "Servo De (Khop 1)";
        lk.vaiTro = "Xoay toan bo canh tay robot theo phuong ngang (Yaw quanh truc Y tu 0 den 180 do).";
        lk.vectorTachRoi = Diem3D(0.0f, 5.0f, 6.0f);
        lk.viTriLapDat = taoMaTranTinhTien(0.0f, 1.2f, 4.5f);
        lk.toaDo2D = Diem2D(670.0f, 520.0f);
        lk.rong2D = 90.0f; lk.cao2D = 55.0f;

        Part than;
        than.ten = "Than";
        than.mesh = taoKhoiHop(4.0f, 3.8f, 2.0f, MauSac(35, 55, 95));
        lk.parts.push_back(than);

        Part horn;
        horn.ten = "CanhServo";
        horn.mesh = taoKhoiTru(1.0f, 0.35f, 12, MauSac(230, 230, 235));
        horn.bienDoiCucBo = taoMaTranTinhTien(0.0f, 2.1f, 0.0f);
        lk.parts.push_back(horn);

        lk.pins.push_back({"SIG", Diem3D(-0.5f, -1.6f, 0.9f), 0});
        lk.pins.push_back({"VCC", Diem3D( 0.0f, -1.6f, 0.9f), 0});
        lk.pins.push_back({"GND", Diem3D( 0.5f, -1.6f, 0.9f), 0});

        ds[LK_SERVO_DE] = lk;
    }

    // ---------------------------------------------------------------------
    // 6. SERVO KHUỶU (SRV_ELBOW) - Kích thước: 2.3 x 2.4 x 1.2 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_SERVO_KHUU;
        lk.ma = "SRV_ELBOW";
        lk.ten = "Servo Khuu (Khop 2)";
        lk.vaiTro = "Nang ha dot canh tay tren (Pitch quanh truc X cuc bo tu 0 den 180 do).";
        lk.vectorTachRoi = Diem3D(0.0f, 9.0f, 8.0f);
        lk.viTriLapDat = taoMaTranTinhTien(0.0f, 5.0f, 4.5f);
        lk.toaDo2D = Diem2D(800.0f, 520.0f);
        lk.rong2D = 90.0f; lk.cao2D = 55.0f;

        Part than;
        than.ten = "Than";
        than.mesh = taoKhoiHop(2.3f, 2.4f, 1.2f, MauSac(20, 110, 200));
        lk.parts.push_back(than);

        lk.pins.push_back({"SIG", Diem3D(-0.35f, -1.0f, 0.5f), 1});
        lk.pins.push_back({"VCC", Diem3D( 0.0f,  -1.0f, 0.5f), 1});
        lk.pins.push_back({"GND", Diem3D( 0.35f, -1.0f, 0.5f), 1});

        ds[LK_SERVO_KHUU] = lk;
    }

    // ---------------------------------------------------------------------
    // 7. SERVO KẸP (SRV_GRIP) - Kích thước: 2.3 x 2.4 x 1.2 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_SERVO_KEP;
        lk.ma = "SRV_GRIP";
        lk.ten = "Servo Kep & Ngam Gap";
        lk.vaiTro = "Dieu khien 2 ngon kep dong mo kep chat vat the de van chuyen.";
        lk.vectorTachRoi = Diem3D(0.0f, 12.0f, 10.0f);
        lk.viTriLapDat = taoMaTranTinhTien(0.0f, 8.5f, 4.5f);
        lk.toaDo2D = Diem2D(930.0f, 520.0f);
        lk.rong2D = 90.0f; lk.cao2D = 55.0f;

        Part than;
        than.ten = "Than";
        than.mesh = taoKhoiHop(2.3f, 2.4f, 1.2f, MauSac(20, 110, 200));
        lk.parts.push_back(than);

        // 2 Ngón kẹp
        Part ngonT;
        ngonT.ten = "NgonTrai";
        ngonT.mesh = taoKhoiHop(0.4f, 1.8f, 0.25f, MauSac(220, 50, 40));
        ngonT.bienDoiCucBo = taoMaTranTinhTien(-0.8f, 1.6f, 0.0f);
        lk.parts.push_back(ngonT);

        Part ngonP;
        ngonP.ten = "NgonPhai";
        ngonP.mesh = taoKhoiHop(0.4f, 1.8f, 0.25f, MauSac(220, 50, 40));
        ngonP.bienDoiCucBo = taoMaTranTinhTien(0.8f, 1.6f, 0.0f);
        lk.parts.push_back(ngonP);

        lk.pins.push_back({"SIG", Diem3D(-0.35f, -1.0f, 0.5f), 2});
        lk.pins.push_back({"VCC", Diem3D( 0.0f,  -1.0f, 0.5f), 2});
        lk.pins.push_back({"GND", Diem3D( 0.35f, -1.0f, 0.5f), 2});

        ds[LK_SERVO_KEP] = lk;
    }

    // ---------------------------------------------------------------------
    // 8. CẢM BIẾN HC-SR04 (HCSR04) - Kích thước: 4.5 x 2.0 x 1.5 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_HCSR04;
        lk.ma = "HCSR04";
        lk.ten = "Cam bien Sieu am HC-SR04";
        lk.vaiTro = "Phat hien vat can phia truoc dau xe va do khoang cach tiep can vat the can gap.";
        lk.vectorTachRoi = Diem3D(0.0f, 4.0f, 10.0f);
        lk.viTriLapDat = taoMaTranTinhTien(0.0f, 1.8f, 7.8f);
        lk.toaDo2D = Diem2D(460.0f, 210.0f);
        lk.rong2D = 95.0f; lk.cao2D = 50.0f;

        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(4.5f, 2.0f, 0.2f, MauSac(15, 95, 175));
        lk.parts.push_back(pcb);

        // 2 Mắt phát/thu siêu âm kim loại
        Part mat1; mat1.ten = "MatPhat"; mat1.mesh = taoKhoiTru(0.75f, 1.2f, 12, MauSac(205, 210, 220));
        mat1.bienDoiCucBo = taoMaTranTinhTien(-1.3f, 0.0f, 0.65f) * taoMaTranQuayX(M_PI * 0.5f);
        lk.parts.push_back(mat1);

        Part mat2; mat2.ten = "MatThu"; mat2.mesh = taoKhoiTru(0.75f, 1.2f, 12, MauSac(205, 210, 220));
        mat2.bienDoiCucBo = taoMaTranTinhTien(1.3f, 0.0f, 0.65f) * taoMaTranQuayX(M_PI * 0.5f);
        lk.parts.push_back(mat2);

        // 4 Chân kim hướng xuống: VCC, Trig, Echo, GND
        lk.pins.push_back({"VCC",  Diem3D(-0.75f, -0.9f, 0.0f), -1});
        lk.pins.push_back({"Trig", Diem3D(-0.25f, -0.9f, 0.0f), -1});
        lk.pins.push_back({"Echo", Diem3D( 0.25f, -0.9f, 0.0f), -1});
        lk.pins.push_back({"GND",  Diem3D( 0.75f, -0.9f, 0.0f), -1});

        ds[LK_HCSR04] = lk;
    }

    // ---------------------------------------------------------------------
    // 9. MẠCH GIẢM ÁP BUCK LM2596 (BUCK) - Kích thước: 4.3 x 1.4 x 2.1 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_BUCK_5V;
        lk.ma = "BUCK";
        lk.ten = "Mach Ha Ap Buck 5V LM2596";
        lk.vaiTro = "Ha ap tu 7.4V/11.1V xuong 5.0V on dinh cap rieng cho 3 Servo tay gap, tranh sut ap vi dieu khien.";
        lk.vectorTachRoi = Diem3D(6.0f, 5.0f, -2.0f);
        lk.viTriLapDat = taoMaTranTinhTien(3.6f, 1.2f, 1.5f);
        lk.toaDo2D = Diem2D(670.0f, 240.0f);
        lk.rong2D = 90.0f; lk.cao2D = 55.0f;

        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(4.3f, 0.2f, 2.1f, MauSac(15, 115, 165));
        lk.parts.push_back(pcb);

        Part cuonCam;
        cuonCam.ten = "CuonCam";
        cuonCam.mesh = taoKhoiHop(1.2f, 1.0f, 1.2f, MauSac(40, 40, 45));
        cuonCam.bienDoiCucBo = taoMaTranTinhTien(-0.3f, 0.55f, 0.0f);
        lk.parts.push_back(cuonCam);

        Part bienTro;
        bienTro.ten = "BienTro";
        bienTro.mesh = taoKhoiHop(0.9f, 0.9f, 0.5f, MauSac(30, 95, 210));
        bienTro.bienDoiCucBo = taoMaTranTinhTien(1.2f, 0.5f, -0.3f);
        lk.parts.push_back(bienTro);

        lk.pins.push_back({"IN+",  Diem3D(-1.9f, 0.2f,  0.8f), -1});
        lk.pins.push_back({"IN-",  Diem3D(-1.9f, 0.2f, -0.8f), -1});
        lk.pins.push_back({"OUT+", Diem3D( 1.9f, 0.2f,  0.8f), -1});
        lk.pins.push_back({"OUT-", Diem3D( 1.9f, 0.2f, -0.8f), -1});

        ds[LK_BUCK_5V] = lk;
    }

    // ---------------------------------------------------------------------
    // 10. HỘP PIN 18650 & CÔNG TẮC (BAT_SW) - Kích thước: 7.5 x 2.1 x 4.0 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_PIN_18650;
        lk.ma = "BAT_SW";
        lk.ten = "Hop Pin 18650 & Cong Tac";
        lk.vaiTro = "Nguon nang luong chinh: 2 cell Li-ion 18650 (7.4V danh dinh, 8.4V max) qua cong tac bap benh.";
        lk.vectorTachRoi = Diem3D(0.0f, 6.0f, -8.0f);
        lk.viTriLapDat = taoMaTranTinhTien(0.0f, 1.2f, -6.5f);
        lk.toaDo2D = Diem2D(880.0f, 350.0f);
        lk.rong2D = 110.0f; lk.cao2D = 65.0f;

        Part khay;
        khay.ten = "KhayPin";
        khay.mesh = taoKhoiHop(7.5f, 2.1f, 4.0f, MauSac(25, 26, 28));
        lk.parts.push_back(khay);

        // 2 Viên pin 18650 xanh lam
        Part p1; p1.ten = "Pin1"; p1.mesh = taoKhoiTru(0.85f, 6.2f, 12, MauSac(35, 140, 220));
        p1.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.4f, -1.0f) * taoMaTranQuayZ(M_PI * 0.5f);
        lk.parts.push_back(p1);

        Part p2; p2.ten = "Pin2"; p2.mesh = taoKhoiTru(0.85f, 6.2f, 12, MauSac(35, 140, 220));
        p2.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.4f,  1.0f) * taoMaTranQuayZ(M_PI * 0.5f);
        lk.parts.push_back(p2);

        // Công tắc bập bênh
        Part sw; sw.ten = "CongTac"; sw.mesh = taoKhoiHop(1.5f, 1.2f, 1.0f, MauSac(180, 30, 30));
        sw.bienDoiCucBo = taoMaTranTinhTien(2.6f, 1.3f, 0.0f);
        lk.parts.push_back(sw);

        lk.pins.push_back({"BAT+", Diem3D(-3.4f, 0.6f, -1.0f), -1});
        lk.pins.push_back({"BAT-", Diem3D(-3.4f, 0.6f,  1.0f), -1});
        lk.pins.push_back({"SW1",  Diem3D( 2.2f, 0.2f,  0.0f), -1});
        lk.pins.push_back({"SW2",  Diem3D( 3.0f, 0.2f,  0.0f), -1});

        ds[LK_PIN_18650] = lk;
    }

    // ---------------------------------------------------------------------
    // 11. DOMINO ĐẤU NỐI 3 BUS (DOMINO) - Kích thước: 6.0 x 1.5 x 4.5 cm
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_DOMINO;
        lk.ma = "DOMINO";
        lk.ten = "Domino Dau Noi (3 Bus)";
        lk.vaiTro = "Tram phan phoi nguon tap trung gom 3 Bus doc lap: Bus 7.4V, Bus GND dung chung va Bus 5V ha ap.";
        lk.vectorTachRoi = Diem3D(6.0f, 8.0f, 2.0f);
        lk.viTriLapDat = taoMaTranTinhTien(3.5f, 1.2f, -1.5f);
        lk.toaDo2D = Diem2D(880.0f, 200.0f);
        lk.rong2D = 110.0f; lk.cao2D = 80.0f;

        Part de;
        de.ten = "DeDomino";
        de.mesh = taoKhoiHop(6.0f, 1.5f, 4.5f, MauSac(235, 235, 240));
        lk.parts.push_back(de);

        // Bus 7.4V (4 cổng)
        for (int i = 1; i <= 4; ++i) {
            std::string pName = "B74_" + std::to_string(i);
            lk.pins.push_back({pName, Diem3D(-2.0f + (i - 1) * 1.3f, 0.8f, -1.4f), -1});
        }
        // Bus GND (9 cổng)
        for (int i = 1; i <= 9; ++i) {
            std::string pName = "BGND_" + std::to_string(i);
            lk.pins.push_back({pName, Diem3D(-2.4f + (i - 1) * 0.6f, 0.8f, 0.0f), -1});
        }
        // Bus 5V-servo (4 cổng)
        for (int i = 1; i <= 4; ++i) {
            std::string pName = "B5V_" + std::to_string(i);
            lk.pins.push_back({pName, Diem3D(-2.0f + (i - 1) * 1.3f, 0.8f, 1.4f), -1});
        }

        ds[LK_DOMINO] = lk;
    }
}

// Standard Type Aliases
using Component = LinhKien;

#endif // COMPONENTS_MESH_H
