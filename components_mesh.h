#ifndef COMPONENTS_MESH_H
#define COMPONENTS_MESH_H

#include "config.h"
#include "math_3d.h"
#include <vector>
#include <string>
#include <cmath>

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
// 2. KHỞI TẠO CÁC PRIMITIVE CƠ BẢN: makeBox, makeCylinder (Y, X, Z)
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
    m.mat.push_back({{0, 1, 2, 3}, mau, { 0,  0,  1}, 0}); // Trước (+Z)
    m.mat.push_back({{5, 4, 7, 6}, mau, { 0,  0, -1}, 0}); // Sau (-Z)
    m.mat.push_back({{1, 5, 6, 2}, mau, { 1,  0,  0}, 0}); // Phải (+X)
    m.mat.push_back({{4, 0, 3, 7}, mau, {-1,  0,  0}, 0}); // Trái (-X)
    m.mat.push_back({{3, 2, 6, 7}, mau, { 0,  1,  0}, 0}); // Trên (+Y)
    m.mat.push_back({{4, 5, 1, 0}, mau, { 0, -1,  0}, 0}); // Dưới (-Y)

    return m;
}

// Tạo khối trụ dọc theo trục Y (bán kính r, chiều cao h)
inline Mesh3D taoKhoiTru(float r, float h, int segments, MauSac mau) {
    Mesh3D m;
    float halfH = h * 0.5f;

    for (int k = 0; k < 2; ++k) {
        float y = (k == 0) ? -halfH : halfH;
        for (int i = 0; i < segments; ++i) {
            float ang = 2.0f * (float)M_PI * i / segments;
            m.dinh.push_back({r * std::cos(ang), y, r * std::sin(ang)});
        }
    }

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

        float angMid = 2.0f * (float)M_PI * (i + 0.5f) / segments;
        Diem3D norm(std::cos(angMid), 0.0f, std::sin(angMid));

        m.mat.push_back({{d0, d1, d2, d3}, mau, norm, 0});
        napDuoi.push_back(segments - 1 - i);
        napTren.push_back(segments + i);
    }

    m.mat.push_back({napDuoi, mau, {0, -1, 0}, 0});
    m.mat.push_back({napTren, mau, {0,  1, 0}, 0});

    return m;
}

// Tạo khối trụ dọc theo trục X (bánh xe hoặc can motor nằm ngang)
inline Mesh3D taoKhoiTruX(float r, float h, int segments, MauSac mau) {
    Mesh3D m;
    float halfH = h * 0.5f;

    for (int k = 0; k < 2; ++k) {
        float x = (k == 0) ? -halfH : halfH;
        for (int i = 0; i < segments; ++i) {
            float ang = 2.0f * (float)M_PI * i / segments;
            m.dinh.push_back({x, r * std::cos(ang), r * std::sin(ang)});
        }
    }

    std::vector<int> napTrai, napPhai;
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int d0 = i;
        int d1 = next;
        int d2 = segments + next;
        int d3 = segments + i;

        m.canh.push_back({d0, d1});
        m.canh.push_back({d3, d2});
        m.canh.push_back({d0, d3});

        float angMid = 2.0f * (float)M_PI * (i + 0.5f) / segments;
        Diem3D norm(0.0f, std::cos(angMid), std::sin(angMid));

        m.mat.push_back({{d0, d1, d2, d3}, mau, norm, 0});
        napTrai.push_back(segments - 1 - i);
        napPhai.push_back(segments + i);
    }

    m.mat.push_back({napTrai, mau, {-1, 0, 0}, 0});
    m.mat.push_back({napPhai, mau, { 1, 0, 0}, 0});

    return m;
}

// Tạo khối trụ dọc theo trục Z (pin nằm ngang hoặc mắt siêu âm)
inline Mesh3D taoKhoiTruZ(float r, float h, int segments, MauSac mau) {
    Mesh3D m;
    float halfH = h * 0.5f;

    for (int k = 0; k < 2; ++k) {
        float z = (k == 0) ? -halfH : halfH;
        for (int i = 0; i < segments; ++i) {
            float ang = 2.0f * (float)M_PI * i / segments;
            m.dinh.push_back({r * std::cos(ang), r * std::sin(ang), z});
        }
    }

    std::vector<int> napSau, napTruoc;
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int d0 = i;
        int d1 = next;
        int d2 = segments + next;
        int d3 = segments + i;

        m.canh.push_back({d0, d1});
        m.canh.push_back({d3, d2});
        m.canh.push_back({d0, d3});

        float angMid = 2.0f * (float)M_PI * (i + 0.5f) / segments;
        Diem3D norm(std::cos(angMid), std::sin(angMid), 0.0f);

        m.mat.push_back({{d0, d1, d2, d3}, mau, norm, 0});
        napSau.push_back(segments - 1 - i);
        napTruoc.push_back(segments + i);
    }

    m.mat.push_back({napSau, mau, {0, 0, -1}, 0});
    m.mat.push_back({napTruoc, mau, {0, 0,  1}, 0});

    return m;
}

// =========================================================================
// 3. CẤU TRÚC DỮ LIỆU PART, PIN, COMPONENT & KHUNG XE 2 TẦNG
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
    Diem3D labelOffset;    // Độ lệch đặt nhãn 3D không chồng lấn
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

// Cấu trúc khung xe 2 tầng Acrylic theo đặc tả Phần B
struct KhungXe2Tang {
    Part lowerDeck;
    Part upperDeck;
    std::vector<Part> standoffs; // 4 trụ đồng lục giác
    Part wheelL;
    Part wheelR;
    Part rimL;
    Part rimR;
    Part caster;
    Part cableDuct; // Máng cáp trung tâm giữa 2 tầng
};

inline KhungXe2Tang taoKhungXe2Tang() {
    KhungXe2Tang kx;

    // 1. Tầng 1 (LowerDeck): 15.0 x 0.4 x 22.0 cm tại y = 2.2 cm
    kx.lowerDeck.ten = "LowerDeck";
    kx.lowerDeck.mesh = taoKhoiHop(15.0f, 0.4f, 22.0f, MauSac(45, 50, 58));
    kx.lowerDeck.bienDoiCucBo = taoMaTranTinhTien(0.0f, 2.2f, 0.0f);

    // 2. Tầng 2 (UpperDeck): 15.0 x 0.4 x 16.0 cm tại y = 6.5 cm
    kx.upperDeck.ten = "UpperDeck";
    kx.upperDeck.mesh = taoKhoiHop(15.0f, 0.4f, 16.0f, MauSac(50, 56, 66));
    kx.upperDeck.bienDoiCucBo = taoMaTranTinhTien(0.0f, 6.5f, 0.0f);

    // 3. 4 Trụ đồng lục giác Standoff cao 4.3 cm tại (±6.5, 4.35, ±7.0)
    float sx[2] = {-6.5f, 6.5f};
    float sz[2] = {-7.0f, 7.0f};
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            Part p;
            p.ten = "Standoff";
            p.mesh = taoKhoiTru(0.35f, 4.3f, 6, MauSac(210, 165, 40));
            p.bienDoiCucBo = taoMaTranTinhTien(sx[i], 4.35f, sz[j]);
            kx.standoffs.push_back(p);
        }
    }

    // 4. Bánh xe chủ động (Wheel_L, Wheel_R) tại (±8.2, 3.2, -3.5)
    // Lốp cao su đen r = 3.2, h = 1.6
    kx.wheelL.ten = "Wheel_L";
    kx.wheelL.mesh = taoKhoiTruX(3.2f, 1.6f, 16, MauSac(25, 27, 30));
    kx.wheelL.bienDoiCucBo = taoMaTranTinhTien(-8.2f, 3.2f, -3.5f);

    kx.wheelR.ten = "Wheel_R";
    kx.wheelR.mesh = taoKhoiTruX(3.2f, 1.6f, 16, MauSac(25, 27, 30));
    kx.wheelR.bienDoiCucBo = taoMaTranTinhTien(8.2f, 3.2f, -3.5f);

    // Mâm xe vàng r = 2.2, h = 1.7
    kx.rimL.ten = "Rim_L";
    kx.rimL.mesh = taoKhoiTruX(2.2f, 1.7f, 8, MauSac(235, 195, 30));
    kx.rimL.bienDoiCucBo = taoMaTranTinhTien(-8.2f, 3.2f, -3.5f);

    kx.rimR.ten = "Rim_R";
    kx.rimR.mesh = taoKhoiTruX(2.2f, 1.7f, 8, MauSac(235, 195, 30));
    kx.rimR.bienDoiCucBo = taoMaTranTinhTien(8.2f, 3.2f, -3.5f);

    // 5. Bánh mắt trâu Caster tại (0, 1.1, +8.0)
    kx.caster.ten = "Caster";
    kx.caster.mesh = taoKhoiTru(1.1f, 1.1f, 12, MauSac(180, 185, 195));
    kx.caster.bienDoiCucBo = taoMaTranTinhTien(0.0f, 1.1f, 8.0f);

    // 6. Máng cáp trung tâm (Central Cable Duct) chạy dọc trục Z ở y = 4.2
    kx.cableDuct.ten = "CableDuct";
    kx.cableDuct.mesh = taoKhoiHop(2.0f, 0.5f, 16.0f, MauSac(35, 40, 48));
    kx.cableDuct.bienDoiCucBo = taoMaTranTinhTien(0.0f, 4.2f, 0.0f);

    return kx;
}

// =========================================================================
// 4. KHỞI TẠO CHI TIẾT 11 LINH KIỆN (REDESIGN NÂNG CẤP THEO PHẦN B)
// =========================================================================

inline void khoiTao11LinhKien(std::vector<LinhKien>& ds) {
    ds.clear();
    ds.resize(TONG_SO_LINH_KIEN + 1);

    // ---------------------------------------------------------------------
    // 1. ARDUINO UNO R3 (ARD) - Tầng 2 (Giữa): (-1.5, 6.8, -0.5)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_ARDUINO;
        lk.ma = "ARD";
        lk.ten = "Arduino Uno R3";
        lk.vaiTro = "Bo dieu khien trung tam: Giao tiep cam bien, xu ly dong hoc xe va phat xung PWM dieu khien 3 Servo.";
        lk.vectorTachRoi = Diem3D(-4.0f, 7.5f, 0.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(-1.5f, 6.8f, -0.5f);
        lk.labelOffset   = Diem3D(-4.5f, 2.5f, 0.0f);
        lk.toaDo2D       = Diem2D(460.0f, 380.0f);
        lk.rong2D        = 130.0f; lk.cao2D = 90.0f;

        // PCB Xanh cổ vịt đặc trưng Arduino (0, 130, 140)
        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(6.8f, 0.20f, 5.3f, MauSac(0, 130, 140));
        lk.parts.push_back(pcb);

        // Cổng USB kim loại bạc tại (-2.8, 0.65, -1.2)
        Part usb;
        usb.ten = "USB_Port";
        usb.mesh = taoKhoiHop(1.6f, 1.1f, 1.2f, MauSac(200, 205, 210));
        usb.bienDoiCucBo = taoMaTranTinhTien(-2.8f, 0.65f, -1.2f);
        lk.parts.push_back(usb);

        // Jack nguồn DC đen tại (-2.8, 0.65, 1.5)
        Part dc;
        dc.ten = "DC_Jack";
        dc.mesh = taoKhoiHop(1.4f, 1.1f, 0.9f, MauSac(35, 35, 40));
        dc.bienDoiCucBo = taoMaTranTinhTien(-2.8f, 0.65f, 1.5f);
        lk.parts.push_back(dc);

        // Đế cắm chip MCU DIP-28 đen tại (0.8, 0.3, 0.6)
        Part mcu;
        mcu.ten = "MCU_Socket";
        mcu.mesh = taoKhoiHop(2.8f, 0.4f, 0.9f, MauSac(25, 25, 30));
        mcu.bienDoiCucBo = taoMaTranTinhTien(0.8f, 0.3f, 0.6f);
        lk.parts.push_back(mcu);

        // Nút bấm Reset tròn đỏ tại (-2.9, 0.35, -2.2)
        Part btn;
        btn.ten = "Reset_Button";
        btn.mesh = taoKhoiHop(0.4f, 0.4f, 0.4f, MauSac(220, 50, 50));
        btn.bienDoiCucBo = taoMaTranTinhTien(-2.9f, 0.35f, -2.2f);
        lk.parts.push_back(btn);

        // Hàng chân cắm Pin Headers màu đen với đỉnh tiếp xúc màu vàng đồng
        Part headerDig;
        headerDig.ten = "Header_Digital";
        headerDig.mesh = taoKhoiHop(4.8f, 0.85f, 0.28f, MauSac(20, 20, 25));
        headerDig.bienDoiCucBo = taoMaTranTinhTien(0.5f, 0.5f, -2.4f);
        lk.parts.push_back(headerDig);

        Part headerPwr;
        headerPwr.ten = "Header_Power";
        headerPwr.mesh = taoKhoiHop(4.2f, 0.85f, 0.28f, MauSac(20, 20, 25));
        headerPwr.bienDoiCucBo = taoMaTranTinhTien(0.5f, 0.5f, 2.4f);
        lk.parts.push_back(headerPwr);

        // Pins: Digital (z = -2.4)
        const char* pinSo[] = {"GND_D", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2"};
        for (int i = 0; i < 13; ++i) {
            float xPos = -1.8f + i * 0.38f;
            lk.pins.push_back({pinSo[i], Diem3D(xPos, 0.95f, -2.4f), -1});
        }
        // Pins: Power (z = +2.4)
        lk.pins.push_back({"VIN",  Diem3D(-1.4f, 0.95f, 2.4f), -1});
        lk.pins.push_back({"GND1", Diem3D(-1.0f, 0.95f, 2.4f), -1});
        lk.pins.push_back({"GND2", Diem3D(-0.6f, 0.95f, 2.4f), -1});
        lk.pins.push_back({"5V",   Diem3D(-0.2f, 0.95f, 2.4f), -1});
        lk.pins.push_back({"3V3",  Diem3D( 0.2f, 0.95f, 2.4f), -1});

        ds[LK_ARDUINO] = lk;
    }

    // ---------------------------------------------------------------------
    // 2. MODULE CẦU H L298N (L298N) - Tầng 1 (Giữa-Sau): (0.0, 2.5, -3.5)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_L298N;
        lk.ma = "L298N";
        lk.ten = "Module Cau H L298N";
        lk.vaiTro = "Khuech dai cong suat: Dieu khien 2 dong co DC banh xe, nhan PWM (ENA, ENB) va tin hieu huong tu Arduino.";
        lk.vectorTachRoi = Diem3D(0.0f, 4.5f, -6.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(0.0f, 2.5f, -3.5f);
        lk.labelOffset   = Diem3D(-4.0f, 2.0f, -2.0f);
        lk.toaDo2D       = Diem2D(280.0f, 260.0f);
        lk.rong2D        = 90.0f; lk.cao2D = 80.0f;

        // PCB Đỏ sẫm (185, 25, 25)
        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(4.3f, 0.20f, 4.3f, MauSac(185, 25, 25));
        lk.parts.push_back(pcb);

        // Khối tản nhiệt nhôm đen gồm 5 LÁ TẢN NHIỆT ĐỨNG đặc trưng
        for (int f = -2; f <= 2; ++f) {
            Part fin;
            fin.ten = "Heatsink_Fin";
            fin.mesh = taoKhoiHop(0.18f, 2.4f, 1.8f, MauSac(35, 38, 42));
            fin.bienDoiCucBo = taoMaTranTinhTien(f * 0.35f, 1.3f, -0.4f);
            lk.parts.push_back(fin);
        }

        // 3 Khối Domino Terminal Xanh dương
        Part termOutL; // OUT1, OUT2
        termOutL.ten = "Term_OUT_L";
        termOutL.mesh = taoKhoiHop(1.0f, 1.2f, 1.6f, MauSac(30, 100, 220));
        termOutL.bienDoiCucBo = taoMaTranTinhTien(-1.6f, 0.7f, 0.6f);
        lk.parts.push_back(termOutL);

        Part termOutR; // OUT3, OUT4
        termOutR.ten = "Term_OUT_R";
        termOutR.mesh = taoKhoiHop(1.0f, 1.2f, 1.6f, MauSac(30, 100, 220));
        termOutR.bienDoiCucBo = taoMaTranTinhTien(1.6f, 0.7f, 0.6f);
        lk.parts.push_back(termOutR);

        Part termPwr; // +12V, GND, +5V
        termPwr.ten = "Term_Power";
        termPwr.mesh = taoKhoiHop(2.2f, 1.2f, 1.0f, MauSac(30, 100, 220));
        termPwr.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.7f, 1.6f);
        lk.parts.push_back(termPwr);

        // 2 Tụ lọc đứng màu đen bạc
        Part cap1;
        cap1.ten = "Cap1";
        cap1.mesh = taoKhoiTru(0.42f, 1.4f, 10, MauSac(30, 32, 36));
        cap1.bienDoiCucBo = taoMaTranTinhTien(-0.6f, 0.8f, 0.8f);
        lk.parts.push_back(cap1);

        Part cap2;
        cap2.ten = "Cap2";
        cap2.mesh = taoKhoiTru(0.42f, 1.4f, 10, MauSac(30, 32, 36));
        cap2.bienDoiCucBo = taoMaTranTinhTien(0.6f, 0.8f, 0.8f);
        lk.parts.push_back(cap2);

        // Pins:
        lk.pins.push_back({"ENA",  Diem3D(-1.2f, 0.4f, -1.9f), -1});
        lk.pins.push_back({"IN1",  Diem3D(-0.8f, 0.4f, -1.9f), -1});
        lk.pins.push_back({"IN2",  Diem3D(-0.2f, 0.4f, -1.9f), -1});
        lk.pins.push_back({"IN3",  Diem3D( 0.2f, 0.4f, -1.9f), -1});
        lk.pins.push_back({"IN4",  Diem3D( 0.8f, 0.4f, -1.9f), -1});
        lk.pins.push_back({"ENB",  Diem3D( 1.2f, 0.4f, -1.9f), -1});

        lk.pins.push_back({"OUT1", Diem3D(-1.9f, 0.8f,  0.2f), -1});
        lk.pins.push_back({"OUT2", Diem3D(-1.9f, 0.8f,  1.0f), -1});
        lk.pins.push_back({"OUT3", Diem3D( 1.9f, 0.8f,  0.2f), -1});
        lk.pins.push_back({"OUT4", Diem3D( 1.9f, 0.8f,  1.0f), -1});

        lk.pins.push_back({"+12V", Diem3D(-0.8f, 0.8f,  1.9f), -1});
        lk.pins.push_back({"GND",  Diem3D( 0.0f, 0.8f,  1.9f), -1});
        lk.pins.push_back({"+5V",  Diem3D( 0.8f, 0.8f,  1.9f), -1});

        ds[LK_L298N] = lk;
    }

    // ---------------------------------------------------------------------
    // 3. ĐỘNG CƠ DC TRÁI (MOT_L) - Dưới Tầng 1 (Trái): (-5.5, 2.0, -3.5)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_DONG_CO_T;
        lk.ma = "MOT_L";
        lk.ten = "Dong co DC Trai (Kem hop so)";
        lk.vaiTro = "Dan dong banh xe trai: Dong co DC giam toc TT ti so truyen 1:48, kem encoder do toc do.";
        lk.vectorTachRoi = Diem3D(-9.0f, -2.5f, -3.5f);
        lk.viTriLapDat   = taoMaTranTinhTien(-5.5f, 2.0f, -3.5f);
        lk.labelOffset   = Diem3D(-3.5f, -1.5f, 0.0f);
        lk.toaDo2D       = Diem2D(150.0f, 260.0f);
        lk.rong2D        = 80.0f; lk.cao2D = 50.0f;

        // Hộp số nhựa vàng (235, 190, 20)
        Part gb;
        gb.ten = "Gearbox";
        gb.mesh = taoKhoiHop(3.7f, 2.2f, 1.9f, MauSac(235, 190, 20));
        lk.parts.push_back(gb);

        // Can motor kim loại bạc nằm ngang
        Part can;
        can.ten = "Motor_Can";
        can.mesh = taoKhoiTruZ(0.9f, 2.8f, 12, MauSac(210, 215, 220));
        can.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.0f, -1.5f);
        lk.parts.push_back(can);

        lk.pins.push_back({"A", Diem3D(-0.4f, 0.0f, -2.9f), -1});
        lk.pins.push_back({"B", Diem3D( 0.4f, 0.0f, -2.9f), -1});

        ds[LK_DONG_CO_T] = lk;
    }

    // ---------------------------------------------------------------------
    // 4. ĐỘNG CƠ DC PHẢI (MOT_R) - Dưới Tầng 1 (Phải): (+5.5, 2.0, -3.5)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_DONG_CO_P;
        lk.ma = "MOT_R";
        lk.ten = "Dong co DC Phai (Kem hop so)";
        lk.vaiTro = "Dan dong banh xe phai: Dong co DC giam toc TT ti so truyen 1:48, kem encoder do toc do.";
        lk.vectorTachRoi = Diem3D(9.0f, -2.5f, -3.5f);
        lk.viTriLapDat   = taoMaTranTinhTien(5.5f, 2.0f, -3.5f);
        lk.labelOffset   = Diem3D(3.5f, -1.5f, 0.0f);
        lk.toaDo2D       = Diem2D(150.0f, 180.0f);
        lk.rong2D        = 80.0f; lk.cao2D = 50.0f;

        Part gb;
        gb.ten = "Gearbox";
        gb.mesh = taoKhoiHop(3.7f, 2.2f, 1.9f, MauSac(235, 190, 20));
        lk.parts.push_back(gb);

        Part can;
        can.ten = "Motor_Can";
        can.mesh = taoKhoiTruZ(0.9f, 2.8f, 12, MauSac(210, 215, 220));
        can.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.0f, -1.5f);
        lk.parts.push_back(can);

        lk.pins.push_back({"A", Diem3D(-0.4f, 0.0f, -2.9f), -1});
        lk.pins.push_back({"B", Diem3D( 0.4f, 0.0f, -2.9f), -1});

        ds[LK_DONG_CO_P] = lk;
    }

    // ---------------------------------------------------------------------
    // 5. SERVO ĐẾ (SRV_BASE) - Mũi Tầng 2: (0.0, 6.8, +6.5)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_SERVO_DE;
        lk.ma = "SRV_BASE";
        lk.ten = "Servo De Xoay (Khop 1)";
        lk.vaiTro = "Dieu khien xoay toan bo truc canh tay robot theo phuong ngang (Yaw) tu 0 - 180 do.";
        lk.vectorTachRoi = Diem3D(0.0f, 5.0f, 6.5f);
        lk.viTriLapDat   = taoMaTranTinhTien(0.0f, 6.8f, 6.5f);
        lk.labelOffset   = Diem3D(-3.5f, 2.5f, 0.0f);
        lk.toaDo2D       = Diem2D(650.0f, 480.0f);
        lk.rong2D        = 85.0f; lk.cao2D = 55.0f;

        // Đế xoay tròn mâm nhôm (r = 2.8, h = 0.8)
        Part tt;
        tt.ten = "Turntable_Plate";
        tt.mesh = taoKhoiTru(2.8f, 0.8f, 16, MauSac(180, 190, 205));
        tt.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.4f, 0.0f);
        lk.parts.push_back(tt);

        // Vỏ Servo MG996R màu xanh đen
        Part body;
        body.ten = "Servo_Body";
        body.mesh = taoKhoiHop(4.0f, 3.6f, 2.0f, MauSac(35, 42, 52));
        body.bienDoiCucBo = taoMaTranTinhTien(0.0f, 2.2f, 0.0f);
        lk.parts.push_back(body);

        // Giá đỡ chữ U vươn lên đỡ khớp khuỷu
        Part uBracket;
        uBracket.ten = "U_Bracket";
        uBracket.mesh = taoKhoiHop(3.2f, 1.5f, 2.4f, MauSac(170, 180, 195));
        uBracket.bienDoiCucBo = taoMaTranTinhTien(0.0f, 3.2f, 0.0f);
        lk.parts.push_back(uBracket);

        lk.pins.push_back({"SIG", Diem3D(-0.4f, 0.8f, -1.0f), -1});
        lk.pins.push_back({"VCC", Diem3D( 0.0f, 0.8f, -1.0f), -1});
        lk.pins.push_back({"GND", Diem3D( 0.4f, 0.8f, -1.0f), -1});

        ds[LK_SERVO_DE] = lk;
    }

    // ---------------------------------------------------------------------
    // 6. SERVO KHUỶU (SRV_ELBOW) - Cục bộ (0, 3.7, 0) so với Đế
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_SERVO_KHUU;
        lk.ma = "SRV_ELBOW";
        lk.ten = "Servo Khuu Nang (Khop 2)";
        lk.vaiTro = "Dieu khien nang ha canh tay theo phuong doc (Pitch) tu 0 - 135 do, chiu tai trong chinh.";
        lk.vectorTachRoi = Diem3D(0.0f, 9.5f, 9.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(0.0f, 10.5f, 6.5f);
        lk.labelOffset   = Diem3D(3.5f, 2.5f, 0.0f);
        lk.toaDo2D       = Diem2D(780.0f, 480.0f);
        lk.rong2D        = 85.0f; lk.cao2D = 55.0f;

        // 2 Thanh giằng nhôm song song cánh tay dưới (L = 4.5 cm)
        Part armLower1;
        armLower1.ten = "Lower_Arm_Link_L";
        armLower1.mesh = taoKhoiHop(0.35f, 3.8f, 1.2f, MauSac(180, 190, 205));
        armLower1.bienDoiCucBo = taoMaTranTinhTien(-1.5f, -1.8f, 0.0f);
        lk.parts.push_back(armLower1);

        Part armLower2;
        armLower2.ten = "Lower_Arm_Link_R";
        armLower2.mesh = taoKhoiHop(0.35f, 3.8f, 1.2f, MauSac(180, 190, 205));
        armLower2.bienDoiCucBo = taoMaTranTinhTien(1.5f, -1.8f, 0.0f);
        lk.parts.push_back(armLower2);

        // Vỏ Servo MG996R nằm ngang
        Part body;
        body.ten = "Servo_Body";
        body.mesh = taoKhoiHop(3.8f, 2.0f, 3.6f, MauSac(35, 42, 52));
        lk.parts.push_back(body);

        // Thanh giằng cánh tay trên vươn về trước (dài 8.0 cm)
        Part forearm;
        forearm.ten = "Upper_Forearm";
        forearm.mesh = taoKhoiHop(1.4f, 0.8f, 7.8f, MauSac(175, 185, 200));
        forearm.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.0f, 4.2f);
        lk.parts.push_back(forearm);

        lk.pins.push_back({"SIG", Diem3D(-0.4f, 0.8f, -1.8f), -1});
        lk.pins.push_back({"VCC", Diem3D( 0.0f, 0.8f, -1.8f), -1});
        lk.pins.push_back({"GND", Diem3D( 0.4f, 0.8f, -1.8f), -1});

        ds[LK_SERVO_KHUU] = lk;
    }

    // ---------------------------------------------------------------------
    // 7. SERVO KẸP (SRV_GRIP) - Cổ tay & Ngàm gắp: (0, 0, +8.5) so với Khuỷu
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_SERVO_KEP;
        lk.ma = "SRV_GRIP";
        lk.ten = "Servo Kep (Khop 3 & Ngam)";
        lk.vaiTro = "Dieu khien dong mo 2 ngam gap vat the, ho tro kep giu hang hoa khi di chuyen.";
        lk.vectorTachRoi = Diem3D(0.0f, 13.0f, 13.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(0.0f, 10.5f, 15.0f);
        lk.labelOffset   = Diem3D(3.5f, 2.5f, 0.0f);
        lk.toaDo2D       = Diem2D(910.0f, 480.0f);
        lk.rong2D        = 85.0f; lk.cao2D = 55.0f;

        // Vỏ Micro Servo SG90 xanh dương
        Part body;
        body.ten = "Micro_Servo";
        body.mesh = taoKhoiHop(2.3f, 2.2f, 1.2f, MauSac(30, 90, 200));
        lk.parts.push_back(body);

        // Khung đế ngàm
        Part baseGrip;
        baseGrip.ten = "Gripper_Base";
        baseGrip.mesh = taoKhoiHop(4.8f, 0.5f, 1.4f, MauSac(200, 100, 25));
        baseGrip.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.0f, 1.2f);
        lk.parts.push_back(baseGrip);

        // 2 Ngón kẹp chữ L đối xứng
        Part fingerL;
        fingerL.ten = "Finger_Left";
        fingerL.mesh = taoKhoiHop(0.4f, 1.8f, 3.2f, MauSac(230, 120, 30));
        fingerL.bienDoiCucBo = taoMaTranTinhTien(-1.6f, 0.0f, 2.8f);
        lk.parts.push_back(fingerL);

        Part fingerR;
        fingerR.ten = "Finger_Right";
        fingerR.mesh = taoKhoiHop(0.4f, 1.8f, 3.2f, MauSac(230, 120, 30));
        fingerR.bienDoiCucBo = taoMaTranTinhTien(1.6f, 0.0f, 2.8f);
        lk.parts.push_back(fingerR);

        lk.pins.push_back({"SIG", Diem3D(-0.3f, 1.0f, -0.5f), -1});
        lk.pins.push_back({"VCC", Diem3D( 0.0f, 1.0f, -0.5f), -1});
        lk.pins.push_back({"GND", Diem3D( 0.3f, 1.0f, -0.5f), -1});

        ds[LK_SERVO_KEP] = lk;
    }

    // ---------------------------------------------------------------------
    // 8. CẢM BIẾN SIÊU ÂM (HCSR04) - Mũi Tầng 1: (0.0, 3.2, +10.2)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_HCSR04;
        lk.ma = "HCSR04";
        lk.ten = "Cam bien Sieu am HC-SR04";
        lk.vaiTro = "Phat hien vat can: Phat song am 40kHz qua chan Trig va thu tin hieu qua Echo de do khoang cach.";
        lk.vectorTachRoi = Diem3D(0.0f, 1.0f, 10.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(0.0f, 3.2f, 10.2f);
        lk.labelOffset   = Diem3D(0.0f, 2.5f, 2.5f);
        lk.toaDo2D       = Diem2D(780.0f, 380.0f);
        lk.rong2D        = 95.0f; lk.cao2D = 55.0f;

        // Mạch PCB xanh dương dựng đứng
        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(4.5f, 2.0f, 0.20f, MauSac(20, 95, 190));
        lk.parts.push_back(pcb);

        // 2 Mắt phát/thu siêu âm nhôm bạc
        Part tx;
        tx.ten = "Transducer_T";
        tx.mesh = taoKhoiTruZ(0.8f, 1.2f, 12, MauSac(200, 205, 215));
        tx.bienDoiCucBo = taoMaTranTinhTien(-1.3f, 0.0f, 0.65f);
        lk.parts.push_back(tx);

        Part rx;
        rx.ten = "Transducer_R";
        rx.mesh = taoKhoiTruZ(0.8f, 1.2f, 12, MauSac(200, 205, 215));
        rx.bienDoiCucBo = taoMaTranTinhTien(1.3f, 0.0f, 0.65f);
        lk.parts.push_back(rx);

        // 2 Lưới đen chữ T / R
        Part meshT;
        meshT.ten = "Mesh_T";
        meshT.mesh = taoKhoiTruZ(0.65f, 0.08f, 12, MauSac(35, 40, 48));
        meshT.bienDoiCucBo = taoMaTranTinhTien(-1.3f, 0.0f, 1.28f);
        lk.parts.push_back(meshT);

        Part meshR;
        meshR.ten = "Mesh_R";
        meshR.mesh = taoKhoiTruZ(0.65f, 0.08f, 12, MauSac(35, 40, 48));
        meshR.bienDoiCucBo = taoMaTranTinhTien(1.3f, 0.0f, 1.28f);
        lk.parts.push_back(meshR);

        // Thạch anh dao động bạc
        Part crystal;
        crystal.ten = "Crystal";
        crystal.mesh = taoKhoiHop(0.9f, 0.4f, 0.3f, MauSac(210, 210, 215));
        crystal.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.6f, 0.2f);
        lk.parts.push_back(crystal);

        // 4 Chân cắm ở mặt sau
        lk.pins.push_back({"VCC",  Diem3D(-0.6f, -0.9f, -0.2f), -1});
        lk.pins.push_back({"Trig", Diem3D(-0.2f, -0.9f, -0.2f), -1});
        lk.pins.push_back({"Echo", Diem3D( 0.2f, -0.9f, -0.2f), -1});
        lk.pins.push_back({"GND",  Diem3D( 0.6f, -0.9f, -0.2f), -1});

        ds[LK_HCSR04] = lk;
    }

    // ---------------------------------------------------------------------
    // 9. MẠCH HẠ ÁP BUCK LM2596 (BUCK) - Tầng 1 (Trước-Trái): (-4.2, 2.5, +3.0)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_BUCK_5V;
        lk.ma = "BUCK";
        lk.ten = "Mach Ha Ap Buck LM2596";
        lk.vaiTro = "On ap nguon: Ha dien ap tu 7.4V/11.1V xuong 5V/3A on dinh, cap rieng cho 3 Servo de tranh sung dien.";
        lk.vectorTachRoi = Diem3D(-8.5f, 3.5f, 4.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(-4.2f, 2.5f, 3.0f);
        lk.labelOffset   = Diem3D(-3.5f, 2.0f, 0.0f);
        lk.toaDo2D       = Diem2D(280.0f, 380.0f);
        lk.rong2D        = 85.0f; lk.cao2D = 60.0f;

        // PCB Xanh dương đậm (15, 70, 160)
        Part pcb;
        pcb.ten = "PCB";
        pcb.mesh = taoKhoiHop(4.3f, 0.20f, 2.1f, MauSac(15, 70, 160));
        lk.parts.push_back(pcb);

        // 2 Tụ nhôm trụ bạc ở hai đầu
        Part capIn;
        capIn.ten = "Cap_In";
        capIn.mesh = taoKhoiTru(0.42f, 1.0f, 10, MauSac(190, 195, 200));
        capIn.bienDoiCucBo = taoMaTranTinhTien(-1.5f, 0.6f, 0.0f);
        lk.parts.push_back(capIn);

        Part capOut;
        capOut.ten = "Cap_Out";
        capOut.mesh = taoKhoiTru(0.42f, 1.0f, 10, MauSac(190, 195, 200));
        capOut.bienDoiCucBo = taoMaTranTinhTien(1.5f, 0.6f, 0.0f);
        lk.parts.push_back(capOut);

        // Cuộn cảm vuông đen ở giữa
        Part ind;
        ind.ten = "Inductor";
        ind.mesh = taoKhoiHop(1.2f, 0.8f, 1.2f, MauSac(35, 35, 38));
        ind.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.5f, -0.2f);
        lk.parts.push_back(ind);

        // Biến trở tinh chỉnh màu xanh lam có ốc đồng vàng
        Part pot;
        pot.ten = "Potentiometer";
        pot.mesh = taoKhoiHop(0.9f, 1.0f, 0.5f, MauSac(40, 130, 230));
        pot.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.6f, 0.6f);
        lk.parts.push_back(pot);

        lk.pins.push_back({"IN+",  Diem3D(-1.8f, 0.2f, -0.7f), -1});
        lk.pins.push_back({"IN-",  Diem3D(-1.8f, 0.2f,  0.7f), -1});
        lk.pins.push_back({"OUT+", Diem3D( 1.8f, 0.2f, -0.7f), -1});
        lk.pins.push_back({"OUT-", Diem3D( 1.8f, 0.2f,  0.7f), -1});

        ds[LK_BUCK_5V] = lk;
    }

    // ---------------------------------------------------------------------
    // 10. HỘP PIN 18650 & CÔNG TẮC (BAT_SW) - Tầng 2 (Sau): (0.0, 6.8, -6.5)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_PIN_18650;
        lk.ma = "BAT_SW";
        lk.ten = "Hop Pin 18650 & Cong Tac";
        lk.vaiTro = "Nguon dien tong: 2 cell 18650 mac noi tiep (7.4V, 2600mAh), kem cong tac gat bat/tat he thong.";
        lk.vectorTachRoi = Diem3D(0.0f, 8.5f, -9.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(0.0f, 6.8f, -6.5f);
        lk.labelOffset   = Diem3D(0.0f, 2.5f, -3.5f);
        lk.toaDo2D       = Diem2D(120.0f, 480.0f);
        lk.rong2D        = 95.0f; lk.cao2D = 60.0f;

        // Khay nhựa đen có thành bên
        Part tray;
        tray.ten = "Battery_Tray";
        tray.mesh = taoKhoiHop(7.5f, 1.4f, 4.2f, MauSac(28, 30, 35));
        lk.parts.push_back(tray);

        // 2 Viên pin 18650 trụ màu hồng tím đặc trưng (235, 80, 130)
        Part cell1;
        cell1.ten = "Cell_1";
        cell1.mesh = taoKhoiTruX(0.85f, 6.5f, 12, MauSac(235, 80, 130));
        cell1.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.65f, -1.0f);
        lk.parts.push_back(cell1);

        Part cell2;
        cell2.ten = "Cell_2";
        cell2.mesh = taoKhoiTruX(0.85f, 6.5f, 12, MauSac(235, 80, 130));
        cell2.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.65f, 1.0f);
        lk.parts.push_back(cell2);

        // Khối công tắc Rocker Switch đen kèm nút đỏ
        Part swBody;
        swBody.ten = "Switch_Body";
        swBody.mesh = taoKhoiHop(1.4f, 1.2f, 1.6f, MauSac(35, 35, 40));
        swBody.bienDoiCucBo = taoMaTranTinhTien(3.0f, 0.6f, 0.0f);
        lk.parts.push_back(swBody);

        Part swBtn;
        swBtn.ten = "Rocker_Button";
        swBtn.mesh = taoKhoiHop(1.0f, 0.4f, 1.2f, MauSac(230, 35, 35));
        swBtn.bienDoiCucBo = taoMaTranTinhTien(3.0f, 1.3f, 0.0f);
        lk.parts.push_back(swBtn);

        lk.pins.push_back({"BAT+", Diem3D(-3.4f, 0.5f, -1.0f), -1});
        lk.pins.push_back({"BAT-", Diem3D(-3.4f, 0.5f,  1.0f), -1});
        lk.pins.push_back({"SW1",  Diem3D( 3.5f, 0.5f, -0.4f), -1});
        lk.pins.push_back({"SW2",  Diem3D( 3.5f, 0.5f,  0.4f), -1});

        ds[LK_PIN_18650] = lk;
    }

    // ---------------------------------------------------------------------
    // 11. DOMINO ĐẤU NỐI 3 BUS (DOMINO) - Tầng 1 (Trước-Phải): (+4.2, 2.5, +3.0)
    // ---------------------------------------------------------------------
    {
        LinhKien lk;
        lk.id = LK_DOMINO;
        lk.ma = "DOMINO";
        lk.ten = "Domino Dau Noi (3 Bus)";
        lk.vaiTro = "Tram phan phoi nguon tap trung: Gom 3 Bus doc lap: Bus 7.4V (Do), Bus GND chung (Den) va Bus 5V ha ap (Cam).";
        lk.vectorTachRoi = Diem3D(8.5f, 3.5f, 4.0f);
        lk.viTriLapDat   = taoMaTranTinhTien(4.2f, 2.5f, 3.0f);
        lk.labelOffset   = Diem3D(3.5f, 2.0f, 0.0f);
        lk.toaDo2D       = Diem2D(280.0f, 480.0f);
        lk.rong2D        = 95.0f; lk.cao2D = 65.0f;

        // Thanh 1: Bus 7.4V (Vỏ trắng ngà, dải chỉ thị ĐỎ)
        Part b1;
        b1.ten = "Bus_7.4V";
        b1.mesh = taoKhoiHop(5.4f, 1.0f, 1.0f, MauSac(215, 220, 225));
        b1.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.5f, -1.3f);
        lk.parts.push_back(b1);

        Part s1;
        s1.ten = "Stripe_Red";
        s1.mesh = taoKhoiHop(5.4f, 0.15f, 0.35f, MauSac(220, 30, 30));
        s1.bienDoiCucBo = taoMaTranTinhTien(0.0f, 1.08f, -1.3f);
        lk.parts.push_back(s1);

        // Thanh 2: Bus GND (Vỏ trắng ngà, dải chỉ thị ĐEN)
        Part b2;
        b2.ten = "Bus_GND";
        b2.mesh = taoKhoiHop(5.4f, 1.0f, 1.0f, MauSac(215, 220, 225));
        b2.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.5f, 0.0f);
        lk.parts.push_back(b2);

        Part s2;
        s2.ten = "Stripe_Black";
        s2.mesh = taoKhoiHop(5.4f, 0.15f, 0.35f, MauSac(40, 40, 45));
        s2.bienDoiCucBo = taoMaTranTinhTien(0.0f, 1.08f, 0.0f);
        lk.parts.push_back(s2);

        // Thanh 3: Bus 5V-servo (Vỏ trắng ngà, dải chỉ thị CAM/VÀNG)
        Part b3;
        b3.ten = "Bus_5V";
        b3.mesh = taoKhoiHop(5.4f, 1.0f, 1.0f, MauSac(215, 220, 225));
        b3.bienDoiCucBo = taoMaTranTinhTien(0.0f, 0.5f, 1.3f);
        lk.parts.push_back(b3);

        Part s3;
        s3.ten = "Stripe_Orange";
        s3.mesh = taoKhoiHop(5.4f, 0.15f, 0.35f, MauSac(245, 140, 20));
        s3.bienDoiCucBo = taoMaTranTinhTien(0.0f, 1.08f, 1.3f);
        lk.parts.push_back(s3);

        // Bus 7.4V (4 cổng)
        for (int i = 1; i <= 4; ++i) {
            std::string pName = "B74_" + std::to_string(i);
            lk.pins.push_back({pName, Diem3D(-2.0f + (i - 1) * 1.3f, 1.1f, -1.3f), -1});
        }
        // Bus GND (9 cổng)
        for (int i = 1; i <= 9; ++i) {
            std::string pName = "BGND_" + std::to_string(i);
            lk.pins.push_back({pName, Diem3D(-2.4f + (i - 1) * 0.6f, 1.1f, 0.0f), -1});
        }
        // Bus 5V-servo (4 cổng)
        for (int i = 1; i <= 4; ++i) {
            std::string pName = "B5V_" + std::to_string(i);
            lk.pins.push_back({pName, Diem3D(-2.0f + (i - 1) * 1.3f, 1.1f, 1.3f), -1});
        }

        ds[LK_DOMINO] = lk;
    }
}

// Standard Type Aliases
using Component = LinhKien;

#endif // COMPONENTS_MESH_H
