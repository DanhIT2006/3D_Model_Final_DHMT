#include "components/ControlComponents.h"
#include <GL/freeglut.h>
#include <cmath>
#include <sstream>
#include <iomanip>

// ===================== ARDUINO UNO R3 =====================
ArduinoUnoComp::ArduinoUnoComp()
    : HardwareComponent("ARD", "Arduino Uno R3", "Vi Dieu Khien") {
    cadSize = Vec3(6.8f, 1.5f, 5.3f);
    nominalVoltage = 7.4f;

    // PCB màu xanh lục lam đặc trưng Arduino Uno
    mesh = Mesh3D::createBox(cadSize.x, cadSize.y, cadSize.z, Vec3(0.0f, 0.42f, 0.62f));

    // Khai báo các chân nguồn
    addPin("VIN", PinType::POWER_7V4, Vec3(-2.0f, 0.8f, 2.2f));
    addPin("5V",  PinType::POWER_5V,  Vec3(-1.5f, 0.8f, 2.2f), true); // Ngõ ra 5V LDO cấp cho HCSR04
    addPin("GND", PinType::POWER_GND, Vec3(-1.0f, 0.8f, 2.2f));

    // Khai báo các chân Digital / PWM
    addPin("D5",  PinType::PWM_IO,    Vec3( 1.0f, 0.8f, -2.2f), true); // ENA (L298N)
    addPin("D2",  PinType::DIGITAL_IO,Vec3(-0.5f, 0.8f, -2.2f), true); // IN1
    addPin("D4",  PinType::DIGITAL_IO,Vec3( 0.5f, 0.8f, -2.2f), true); // IN2

    addPin("D6",  PinType::PWM_IO,    Vec3( 1.5f, 0.8f, -2.2f), true); // ENB (L298N)
    addPin("D12", PinType::DIGITAL_IO,Vec3(-2.5f, 0.8f, -2.2f), true); // IN3
    addPin("D13", PinType::DIGITAL_IO,Vec3(-2.8f, 0.8f, -2.2f), true); // IN4

    addPin("D7",  PinType::DIGITAL_IO,Vec3( 2.0f, 0.8f, -2.2f), true); // TRIG (HCSR04)
    addPin("D8",  PinType::DIGITAL_IO,Vec3( 2.4f, 0.8f, -2.2f));       // ECHO (HCSR04)

    addPin("D9",  PinType::PWM_IO,    Vec3( 0.0f, 0.8f, -2.2f), true); // SRV_BASE (Yaw)
    addPin("D10", PinType::PWM_IO,    Vec3(-0.4f, 0.8f, -2.2f), true); // SRV_ELBOW (Pitch)
    addPin("D11", PinType::PWM_IO,    Vec3(-0.8f, 0.8f, -2.2f), true); // SRV_GRIP (Kẹp)

    initSubComponents();
}

void ArduinoUnoComp::initSubComponents() {
    subComponents.clear();

    // 1. Vi dieu khien ATmega328P
    SubComponent atmega;
    atmega.id = "ATMEGA328P";
    atmega.name = "Vi dieu khien ATmega328P (Microcontroller IC DIP-28)";
    atmega.rating = "CPU 8-bit AVR RISC 16MHz / 32KB Flash / 2KB SRAM / DIP-28";
    atmega.roleDesc = "Bo nao trung tam cua he thong xe robot: Nap va thuc thi firmware C++, xu ly thuat toan tu hanh do duong, tinh toan dong hoc 3 servo tay gap, phat xung PWM dieu toc 2 dong co va tinh toan khoang cach tu cam bien sieu am.";
    atmega.localPos = Vec3(0.9f, 0.42f, 0.5f);
    atmega.size = Vec3(3.6f, 0.55f, 0.95f);
    atmega.color = Vec3(0.08f, 0.08f, 0.08f);
    atmega.mesh = Mesh3D::createBox(3.6f, 0.55f, 0.95f, atmega.color);
    addSubComponent(atmega);

    // 2. Nut nhan Reset
    SubComponent btnRst;
    btnRst.id = "BTN_RESET";
    btnRst.name = "Nut nhan Reset phan cung (Tactile Reset Button)";
    btnRst.rating = "Push Button 6x6mm 4-pin tiep diem dong / Active LOW";
    btnRst.roleDesc = "Khi nhan, chan RESET cua ATmega328P bi keo xuong muc LOW (0V), dua CPU tro ve dia chi 0x0000 cua bo nho Flash, khoi dong lai toan bo firmware xe robot ma khong can tat nguon.";
    btnRst.localPos = Vec3(-2.8f, 0.45f, -1.8f);
    btnRst.size = Vec3(0.7f, 0.55f, 0.7f);
    btnRst.color = Vec3(0.85f, 0.2f, 0.1f);
    btnRst.isInteractive = true;
    btnRst.actionName = "NHAN RESET ARDUINO";
    btnRst.mesh = Mesh3D::createBox(0.7f, 0.55f, 0.7f, btnRst.color);
    addSubComponent(btnRst);

    // 3. Cong USB Type-B
    SubComponent usbPort;
    usbPort.id = "USB_PORT";
    usbPort.name = "Cong giao tiep USB Type-B (USB Programming & Serial Port)";
    usbPort.rating = "Chuan USB 2.0 Full-Speed 12Mbps / Vo kim loai ma Niken";
    usbPort.roleDesc = "Ket noi voi may tinh de nap chuong trinh firmware vao bo nho Flash cua vi dieu khien qua Bootloader Optiboot, truyen du lieu Serial Monitor va co the cap nguon 5VDC thu nghiem.";
    usbPort.localPos = Vec3(-2.6f, 0.7f, -0.9f);
    usbPort.size = Vec3(1.4f, 1.15f, 1.25f);
    usbPort.color = Vec3(0.75f, 0.78f, 0.82f);
    usbPort.mesh = Mesh3D::createBox(1.4f, 1.15f, 1.25f, usbPort.color);
    addSubComponent(usbPort);

    // 4. Jack nguon ngoai DC Barrel
    SubComponent dcJack;
    dcJack.id = "DC_JACK";
    dcJack.name = "Jack nguon ngoai DC Barrel Jack (2.1mm Center Positive)";
    dcJack.rating = "Duong kinh 5.5mm ngoai / 2.1mm trong / Dien ap khuyen nghi 7-12V";
    dcJack.roleDesc = "Dau tiep nhan nguon dien DC ngoai tu pack pin 7.4V tren xe robot, dan dien vao mach on ap tuyen tinh LDO tren bo de tao nguon 5V nuoi he thong.";
    dcJack.localPos = Vec3(-2.6f, 0.65f, 1.6f);
    dcJack.size = Vec3(1.5f, 1.05f, 1.0f);
    dcJack.color = Vec3(0.12f, 0.12f, 0.12f);
    dcJack.mesh = Mesh3D::createBox(1.5f, 1.05f, 1.0f, dcJack.color);
    addSubComponent(dcJack);

    // 5. IC On ap 5V LDO AMS1117
    SubComponent regLdo;
    regLdo.id = "REG_LDO_5V";
    regLdo.name = "IC On ap tuyen tinh 5V LDO (AMS1117-5.0)";
    regLdo.rating = "Dien ap vao 6.5V - 12V / Ra chuan 5.0V / Dong 1A / SOT-223";
    regLdo.roleDesc = "Ha ap tu 7.4V cua pack pin (qua jack DC hoac Vin) xuong muc 5.0V sieu phang on dinh de nuoi vi dieu khien ATmega328P va cac cam bien ngoai vi.";
    regLdo.localPos = Vec3(-1.2f, 0.32f, 0.9f);
    regLdo.size = Vec3(0.75f, 0.35f, 0.55f);
    regLdo.color = Vec3(0.2f, 0.2f, 0.22f);
    regLdo.mesh = Mesh3D::createBox(0.75f, 0.35f, 0.55f, regLdo.color);
    addSubComponent(regLdo);

    // 6. Cap tu hoa SMD loc nguon
    SubComponent capArd;
    capArd.id = "CAP_ARD";
    capArd.name = "Cap tu hoa SMD loc nguon (Input 47uF & Output 100uF 16V)";
    capArd.rating = "47 uF & 100 uF / 16V / Tieu chuan SMD nhom bac";
    capArd.roleDesc = "Loc nhieu gơn song o hai dau vao va ra cua IC on ap AMS1117, ngan chan hien tuong dao dong tu kich va sut ap gay reset vi dieu khien.";
    capArd.localPos = Vec3(-0.35f, 0.5f, 1.5f);
    capArd.size = Vec3(0.65f, 0.75f, 0.65f);
    capArd.color = Vec3(0.82f, 0.82f, 0.86f);
    capArd.mesh = Mesh3D::createCylinder(0.32f, 0.75f, 12, capArd.color);
    addSubComponent(capArd);

    // 7. Thach anh dao dong 16MHz
    SubComponent crystal;
    crystal.id = "CRYSTAL_16MHZ";
    crystal.name = "Thach anh dao dong 16MHz (16.000 MHz Crystal Oscillator)";
    crystal.rating = "Tan so 16.000 MHz / Do lech +/- 20ppm / Vo kim loai HC-49S";
    crystal.roleDesc = "Cung cap xung nhip clock ngoai cuc ky on dinh cho CPU ATmega328P (moi chu ky may = 62.5ns), dam bao do chinh xac tuyet doi cho bo dinh thoi Timer, xung PWM dieu toc va toc do truyen thong Baud rate.";
    crystal.localPos = Vec3(-0.4f, 0.35f, -0.2f);
    crystal.size = Vec3(1.1f, 0.4f, 0.45f);
    crystal.color = Vec3(0.85f, 0.85f, 0.88f);
    crystal.mesh = Mesh3D::createBox(1.1f, 0.4f, 0.45f, crystal.color);
    addSubComponent(crystal);

    // 8. Hang socket Digital Pins D0-D13
    SubComponent hdrDig;
    hdrDig.id = "HEADER_DIGITAL";
    hdrDig.name = "Hang socket chan xuat nhap so (Digital Pins D0 - D13, GND, AREF)";
    hdrDig.rating = "Socket nu 10-pin & 8-pin 2.54mm / 6 kenh PWM phan giai 8-bit";
    hdrDig.roleDesc = "Giao tiep voi cac module ngoai vi: D5, D6 xuat xung PWM dieu toc L298N; D4, D7 xuat tin hieu chieu quay; D9, D10, D11 xuat xung PWM goc cho 3 Servo; D2, D3 giao tiep cam bien sieu am.";
    hdrDig.localPos = Vec3(0.8f, 0.48f, -2.25f);
    hdrDig.size = Vec3(4.8f, 0.75f, 0.45f);
    hdrDig.color = Vec3(0.12f, 0.12f, 0.12f);
    hdrDig.mesh = Mesh3D::createBox(4.8f, 0.75f, 0.45f, hdrDig.color);
    addSubComponent(hdrDig);

    // 9. Hang socket Analog Pins A0-A5
    SubComponent hdrAna;
    hdrAna.id = "HEADER_ANALOG";
    hdrAna.name = "Hang socket chan ngo vao tuong tu (Analog Pins A0 - A5)";
    hdrAna.rating = "Socket nu 6-pin 2.54mm / Bo bien doi ADC 10-bit (0-1023)";
    hdrAna.roleDesc = "Doc dien ap tuong tu tu 0V den 5V tu cac cam bien mo rong, bien tro hoac do dien ap giam sat nguon pin cua xe.";
    hdrAna.localPos = Vec3(1.85f, 0.48f, 2.25f);
    hdrAna.size = Vec3(1.8f, 0.75f, 0.45f);
    hdrAna.color = Vec3(0.12f, 0.12f, 0.12f);
    hdrAna.mesh = Mesh3D::createBox(1.8f, 0.75f, 0.45f, hdrAna.color);
    addSubComponent(hdrAna);

    // 10. Hang socket Power Header
    SubComponent hdrPwr;
    hdrPwr.id = "HEADER_POWER";
    hdrPwr.name = "Hang socket chan cap nguon he thong (Power Header - Vin, 5V, 3.3V, GND)";
    hdrPwr.rating = "Socket nu 8-pin 2.54mm";
    hdrPwr.roleDesc = "Xuat nguon 5V va 3.3V on dinh cap cho cam bien sieu am HC-SR04; chan GND noi dat chung; chan Vin nhan dien ap truc tiep tu pin.";
    hdrPwr.localPos = Vec3(-0.45f, 0.48f, 2.25f);
    hdrPwr.size = Vec3(2.2f, 0.75f, 0.45f);
    hdrPwr.color = Vec3(0.12f, 0.12f, 0.12f);
    hdrPwr.mesh = Mesh3D::createBox(2.2f, 0.75f, 0.45f, hdrPwr.color);
    addSubComponent(hdrPwr);

    // 11. IC Giao tiep ATmega16U2
    SubComponent atmega16u2;
    atmega16u2.id = "ATMEGA16U2";
    atmega16u2.name = "IC Giao tiep USB-UART (ATmega16U2 Microcontroller)";
    atmega16u2.rating = "CPU 8-bit AVR / Goi QFN-32 / Firmware USB-CDC";
    atmega16u2.roleDesc = "Cau noi truyen thong: Nhan goi tin USB tu cong COM cua may tinh, giai ma va chuyen thanh tin hieu UART (TX/RX) truyen vao ATmega328P de nap chuong trinh.";
    atmega16u2.localPos = Vec3(-1.25f, 0.28f, -0.6f);
    atmega16u2.size = Vec3(0.65f, 0.25f, 0.65f);
    atmega16u2.color = Vec3(0.1f, 0.1f, 0.12f);
    atmega16u2.mesh = Mesh3D::createBox(0.65f, 0.25f, 0.65f, atmega16u2.color);
    addSubComponent(atmega16u2);

    // 12. Bo den LED chi bao
    SubComponent ledArd;
    ledArd.id = "LED_ARD";
    ledArd.name = "Bo den LED chi bao (Power LED 'ON' & Status LED 'L' D13)";
    ledArd.rating = "2x SMD LED 0805 mau xanh la va vang cam";
    ledArd.roleDesc = "LED 'ON' bao nguon 5V dang cap vao board. LED 'L' noi voi chan D13, dung de kiem tra hoat dong nhap nhay logic hoac canh bao trang thai tu hanh.";
    ledArd.localPos = Vec3(0.2f, 0.28f, -1.25f);
    ledArd.size = Vec3(0.45f, 0.22f, 0.45f);
    ledArd.color = Vec3(0.2f, 1.0f, 0.3f);
    ledArd.mesh = Mesh3D::createBox(0.45f, 0.22f, 0.45f, ledArd.color);
    addSubComponent(ledArd);

    // Mac dinh chon chip vi dieu khien
    selectSubComponentByIndex(0);
}

void ArduinoUnoComp::onSubComponentAction(const std::string& subId) {
    if (subId == "BTN_RESET") {
        resetAutoSequence();
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->rating = "DA RESET THANH CONG! (Reload 0x0000 Flash)";
        }
    }
}

void ArduinoUnoComp::setControlMode(RobotControlMode mode) {
    controlMode = mode;
    resetAutoSequence();
}

void ArduinoUnoComp::toggleControlMode() {
    controlMode = (controlMode == RobotControlMode::MANUAL) ? RobotControlMode::AUTO_GRAB : RobotControlMode::MANUAL;
    resetAutoSequence();
}

void ArduinoUnoComp::resetAutoSequence() {
    autoSeqStep = 0;
    autoSeqTimer = 0.0f;
    motorSpeedL = motorSpeedR = 0;
    motorDirL = motorDirR = 0;
    baseAngleDeg = 90.0f;
    elbowAngleDeg = 90.0f;
    gripAngleDeg = 45.0f;
}

void ArduinoUnoComp::updateFirmware(float dt, float measuredDistanceCm, bool& shouldGraspObject, bool& objectGrasped) {
    if (!isPowered) {
        motorSpeedL = motorSpeedR = 0;
        motorDirL = motorDirR = 0;
        return;
    }

    obstacleDistCm = measuredDistanceCm;

    if (controlMode == RobotControlMode::AUTO_GRAB) {
        autoSeqTimer += dt;

        switch (autoSeqStep) {
            case 0: // Bước 0: Xe tiến lại gần vật thể
                if (obstacleDistCm > 11.5f) {
                    motorSpeedL = motorSpeedR = 150;
                    motorDirL = motorDirR = 1; // Chạy tiến
                    baseAngleDeg = 90.0f;
                    elbowAngleDeg = 140.0f; // Bàn nâng ở vị trí cao khi chạy
                    gripAngleDeg = 85.0f;  // Mở rộng ngàm sẵn
                } else {
                    // Đã tới cự ly chuẩn (~10-11 cm), phanh dừng xe
                    motorSpeedL = motorSpeedR = 0;
                    motorDirL = motorDirR = 0;
                    autoSeqStep = 1;
                    autoSeqTimer = 0.0f;
                }
                break;

            case 1: // Bước 1: Hạ bàn nâng thẳng đứng xuống sát sàn
                elbowAngleDeg = std::max(0.0f, elbowAngleDeg - 80.0f * dt);
                gripAngleDeg = 85.0f; // Ngàm mở
                if (autoSeqTimer > 1.8f) {
                    autoSeqStep = 2;
                    autoSeqTimer = 0.0f;
                }
                break;

            case 2: // Bước 2: Kẹp ngàm lại để ôm chặt vật thể
                gripAngleDeg = std::max(10.0f, gripAngleDeg - 60.0f * dt);
                if (gripAngleDeg <= 25.0f) {
                    shouldGraspObject = true;
                }
                if (autoSeqTimer > 1.5f) {
                    autoSeqStep = 3;
                    autoSeqTimer = 0.0f;
                }
                break;

            case 3: // Bước 3: Nâng bàn nâng thẳng đứng lên cao mang theo vật thể
                elbowAngleDeg = std::min(180.0f, elbowAngleDeg + 70.0f * dt);
                if (autoSeqTimer > 2.5f) {
                    autoSeqStep = 4;
                    autoSeqTimer = 0.0f;
                }
                break;

            case 4: // Bước 4: Hoàn thành, giữ nguyên vật thể trên cao
                break;
        }
    }

    // Cập nhật chân điều khiển hướng động cơ L298N (IN1, IN2, IN3, IN4)
    if (auto* p = getPin("D2"))  p->voltage = (motorDirL > 0) ? 5.0f : 0.0f; // IN1
    if (auto* p = getPin("D4"))  p->voltage = (motorDirL < 0) ? 5.0f : 0.0f; // IN2
    if (auto* p = getPin("D12")) p->voltage = (motorDirR > 0) ? 5.0f : 0.0f; // IN3
    if (auto* p = getPin("D13")) p->voltage = (motorDirR < 0) ? 5.0f : 0.0f; // IN4

    // Cập nhật giá trị ra các chân Pin PWM
    if (auto* p = getPin("D5"))  { p->pwmDuty = (float)motorSpeedL / 255.0f; p->voltage = p->pwmDuty * 5.0f; }
    if (auto* p = getPin("D6"))  { p->pwmDuty = (float)motorSpeedR / 255.0f; p->voltage = p->pwmDuty * 5.0f; }
    if (auto* p = getPin("D9"))  { p->pwmDuty = baseAngleDeg / 180.0f; p->voltage = 5.0f; }
    if (auto* p = getPin("D10")) { p->pwmDuty = elbowAngleDeg / 180.0f; p->voltage = 5.0f; }
    if (auto* p = getPin("D11")) { p->pwmDuty = gripAngleDeg / 180.0f; p->voltage = 5.0f; }
}

void ArduinoUnoComp::onElectricalUpdate() {
    Pin* vin = getPin("VIN");
    float vIn = vin ? vin->voltage : 0.0f;

    // Arduino cần từ 6.5V trở lên để LDO 5V hoạt động
    isPowered = (vIn >= 6.5f);

    Pin* out5v = getPin("5V");
    if (out5v) {
        out5v->voltage = isPowered ? 5.0f : 0.0f;
    }

    std::ostringstream ss;
    ss << (controlMode == RobotControlMode::MANUAL ? "THU CONG (Manual)" : "TU HANH GAP (Auto Grab)");
    if (controlMode == RobotControlMode::AUTO_GRAB) {
        ss << " [B" << autoSeqStep << "]";
    }
    ss << " | Cu ly: " << std::fixed << std::setprecision(1) << obstacleDistCm << "cm";
    statusDesc = ss.str();
}

void ArduinoUnoComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // 1. Vi dieu khien ATmega328P DIP-28 kem 28 chan kim loai va khe dinh huong
    glPushMatrix();
    glTranslatef(0.9f, 0.42f, 0.5f);
    // De cam socket IC DIP-28 mau den
    glColor3f(0.08f, 0.08f, 0.08f);
    glPushMatrix();
    glScalef(3.6f, 0.55f, 0.95f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Vach khuyet ban nguyet dinh huong Pin 1 o dau chip (-X)
    glColor3f(0.35f, 0.35f, 0.35f);
    glPushMatrix();
    glTranslatef(-1.75f, 0.28f, 0.0f);
    glScalef(0.12f, 0.05f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 28 Chan kim loai ma bac hai ben suon
    glColor3f(0.85f, 0.88f, 0.92f);
    for (int p = 0; p < 14; ++p) {
        float px = -1.5f + p * 0.23f;
        glPushMatrix();
        glTranslatef(px, -0.1f, -0.52f);
        glScalef(0.08f, 0.45f, 0.12f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(px, -0.1f, 0.52f);
        glScalef(0.08f, 0.45f, 0.12f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 2. Cong USB Type-B kim loai ma Niken chong nhieu
    glPushMatrix();
    glTranslatef(-2.6f, 0.7f, -0.9f);
    glColor3f(0.78f, 0.80f, 0.84f);
    glPushMatrix();
    glScalef(1.4f, 1.15f, 1.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Khe cam USB mau den ben trong
    glColor3f(0.12f, 0.12f, 0.15f);
    glTranslatef(-0.71f, 0.0f, 0.0f);
    glScalef(0.05f, 0.75f, 0.85f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 3. Jack nguon ngoai DC Barrel Jack 2.1mm
    glPushMatrix();
    glTranslatef(-2.6f, 0.65f, 1.6f);
    glColor3f(0.12f, 0.12f, 0.14f);
    glPushMatrix();
    glScalef(1.5f, 1.05f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Lo cam tron + Kim tiep xuc giua ma vang
    glColor3f(0.05f, 0.05f, 0.05f);
    glTranslatef(-0.76f, 0.0f, 0.0f);
    glRotatef(90.0f, 0, 1, 0);
    glutSolidCylinder(0.35f, 0.05f, 12, 1);
    glColor3f(0.85f, 0.75f, 0.2f);
    glutSolidCylinder(0.1f, 0.35f, 8, 1);
    glPopMatrix();

    // 4. IC On ap 5V LDO AMS1117-5.0 SOT-223
    glPushMatrix();
    glTranslatef(-1.2f, 0.32f, 0.9f);
    glColor3f(0.18f, 0.18f, 0.20f);
    glPushMatrix();
    glScalef(0.75f, 0.3f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Tab dong tan nhiet ma thiec
    glColor3f(0.75f, 0.78f, 0.82f);
    glTranslatef(0.0f, 0.05f, 0.32f);
    glScalef(0.55f, 0.1f, 0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 5. Cap tu hoa SMD nhom bac
    for (float z : {1.2f, 1.7f}) {
        glPushMatrix();
        glTranslatef(-0.35f, 0.45f, z);
        glColor3f(0.82f, 0.85f, 0.88f);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.32f, 0.65f, 12, 2);
        glPopMatrix();
    }

    // 6. Thach anh dao dong 16.000 MHz vo kim loai HC-49
    glPushMatrix();
    glTranslatef(-0.4f, 0.35f, -0.2f);
    glColor3f(0.85f, 0.88f, 0.92f);
    glScalef(1.1f, 0.4f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 7. Nut nhan Reset phan cung vien bac nut do
    glPushMatrix();
    glTranslatef(-2.8f, 0.42f, -1.8f);
    glColor3f(0.75f, 0.78f, 0.82f);
    glPushMatrix();
    glScalef(0.7f, 0.35f, 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glColor3f(0.85f, 0.2f, 0.1f);
    glTranslatef(0.0f, 0.22f, 0.0f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCylinder(0.22f, 0.2f, 10, 1);
    glPopMatrix();

    // 8. Hang socket nu cam chan (Female Header Strips) 2 mep bo mach
    glColor3f(0.12f, 0.12f, 0.14f);
    // Mep tren: Digital Pins D0-D13
    glPushMatrix();
    glTranslatef(0.8f, 0.48f, -2.25f);
    glScalef(4.8f, 0.75f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Mep duoi: Power Header & Analog Header
    glPushMatrix();
    glTranslatef(-0.45f, 0.48f, 2.25f);
    glScalef(2.2f, 0.75f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.85f, 0.48f, 2.25f);
    glScalef(1.8f, 0.75f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 9. Bo den LED chi thi (ON mau xanh, D13 mau cam)
    glPushMatrix();
    glTranslatef(0.2f, 0.28f, -1.25f);
    glColor3f(isPowered ? 0.2f : 0.05f, isPowered ? 1.0f : 0.2f, isPowered ? 0.3f : 0.05f);
    glutSolidCube(0.25f);
    glTranslatef(0.35f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.6f, 0.1f);
    glutSolidCube(0.25f);
    glPopMatrix();
}

// ===================== HC-SR04 =====================
UltrasonicComp::UltrasonicComp()
    : HardwareComponent("HCSR04", "Cam Bien Sieu Am HC-SR04", "Cam Bien") {
    cadSize = Vec3(4.5f, 2.0f, 1.8f);
    nominalVoltage = 5.0f;

    // PCB xanh dương + 2 mắt trụ bạc
    mesh = Mesh3D::createSensorEyes(cadSize.x, cadSize.y, cadSize.z, Vec3(0.08f, 0.28f, 0.75f), Vec3(0.85f, 0.85f, 0.88f));

    addPin("VCC",  PinType::POWER_5V,  Vec3(-1.2f, -0.6f, -0.8f));
    addPin("TRIG", PinType::DIGITAL_IO,Vec3(-0.4f, -0.6f, -0.8f));
    addPin("ECHO", PinType::DIGITAL_IO,Vec3( 0.4f, -0.6f, -0.8f), true);
    addPin("GND",  PinType::POWER_GND, Vec3( 1.2f, -0.6f, -0.8f));

    initSubComponents();
}

void UltrasonicComp::initSubComponents() {
    subComponents.clear();

    SubComponent transT;
    transT.id = "TRANSMITTER_T";
    transT.name = "Loa phat song sieu am T (Ultrasonic Transmitter)";
    transT.rating = "Tan so 40.0 kHz / Luoi bao ve kim loai";
    transT.roleDesc = "Phat ra chuoi 8 xung song am tan so 40kHz lan truyen trong khong khi voi van toc am thanh ~340m/s khi nhan xung Trigger tu Arduino.";
    transT.localPos = Vec3(-1.3f, 0.0f, 0.4f);
    transT.size = Vec3(1.5f, 1.5f, 1.2f);
    transT.color = Vec3(0.85f, 0.85f, 0.88f);
    transT.mesh = Mesh3D::createCylinder(0.7f, 1.2f, 12, transT.color);
    addSubComponent(transT);

    SubComponent recvR;
    recvR.id = "RECEIVER_R";
    recvR.name = "Mang thu song sieu am R (Ultrasonic Receiver)";
    recvR.rating = "Dau thu gom ap dien 40.0 kHz";
    recvR.roleDesc = "Tiep nhan song am phan xa nguoc lai tu be mat vat can, chuyen thanh tin hieu dien dua vao IC khuếch dai LM324 de kich hoat chan Echo.";
    recvR.localPos = Vec3(1.3f, 0.0f, 0.4f);
    recvR.size = Vec3(1.5f, 1.5f, 1.2f);
    recvR.color = Vec3(0.85f, 0.85f, 0.88f);
    recvR.mesh = Mesh3D::createCylinder(0.7f, 1.2f, 12, recvR.color);
    addSubComponent(recvR);

    SubComponent crystal4;
    crystal4.id = "CRYSTAL_4MHZ";
    crystal4.name = "Thach anh dao dong 4MHz";
    crystal4.rating = "4.000 MHz / Dinh chuan nhip phat xung";
    crystal4.roleDesc = "On dinh nhip xung 40kHz phat ra, tranh bi sai lech cu ly do nhiet do moi truong.";
    crystal4.localPos = Vec3(0.0f, 0.25f, -0.4f);
    crystal4.size = Vec3(0.8f, 0.35f, 0.4f);
    crystal4.color = Vec3(0.8f, 0.8f, 0.82f);
    crystal4.mesh = Mesh3D::createBox(0.8f, 0.35f, 0.4f, crystal4.color);
    addSubComponent(crystal4);

    SubComponent hdr4;
    hdr4.id = "HEADER_4PIN";
    hdr4.name = "Hang 4 chan cam tin hieu (VCC, Trig, Echo, GND)";
    hdr4.rating = "Header 4-pin goc vuong 2.54mm";
    hdr4.roleDesc = "Giao tiep voi Arduino Uno: Chan Trig nhan tin hieu phat; chan Echo xuat do rong xung ty le thuan voi thoi gian bay cua song am.";
    hdr4.localPos = Vec3(0.0f, -0.6f, -0.7f);
    hdr4.size = Vec3(1.2f, 0.5f, 0.4f);
    hdr4.color = Vec3(0.85f, 0.75f, 0.2f);
    hdr4.mesh = Mesh3D::createBox(1.2f, 0.5f, 0.4f, hdr4.color);
    addSubComponent(hdr4);

    selectSubComponentByIndex(0);
}

void UltrasonicComp::updateRaycast(const Vec3& sensorWorldPos, const Vec3& forwardDir, const Vec3& targetObjPos, float targetObjRadius) {
    if (!isPowered) {
        measuredDistanceCm = maxDetectionRange;
        obstacleDetected = false;
        return;
    }

    Vec3 toTarget = targetObjPos - sensorWorldPos;
    // Chiếu lên hướng mũi xe (+Z)
    float projDist = toTarget.dot(forwardDir);

    if (projDist > 0.0f && projDist < maxDetectionRange) {
        Vec3 closestPtOnRay = sensorWorldPos + forwardDir * projDist;
        float lateralDist = closestPtOnRay.distanceTo(targetObjPos);

        // Bán kính góc quét siêu âm ~ 15 độ
        float beamRadiusAtDist = projDist * std::tan(0.26f) + targetObjRadius;
        if (lateralDist <= beamRadiusAtDist) {
            measuredDistanceCm = projDist - targetObjRadius;
            obstacleDetected = (measuredDistanceCm < 30.0f);
            return;
        }
    }

    measuredDistanceCm = maxDetectionRange;
    obstacleDetected = false;
}

void UltrasonicComp::onElectricalUpdate() {
    Pin* vcc = getPin("VCC");
    isPowered = (vcc && vcc->voltage >= 4.5f);

    std::ostringstream ss;
    ss << (obstacleDetected ? "PHAT HIEN VAT CAN: " : "Quet tu do: ") 
       << std::fixed << std::setprecision(1) << measuredDistanceCm << " cm";
    statusDesc = ss.str();
}

void UltrasonicComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // Thạch anh dao động 4.0MHz nhỏ ở giữa 2 mắt
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, cadSize.z * 0.52f);
    glColor3f(0.85f, 0.75f, 0.4f);
    glScalef(0.6f, 0.3f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Nếu phát hiện vật cản gần (< 15cm), nháy tia đỏ cảnh báo
    if (isPowered && obstacleDetected) {
        glPushMatrix();
        glTranslatef(0.0f, 0.7f, 0.0f);
        glColor3f(1.0f, 0.1f, 0.1f);
        glutSolidSphere(0.18f, 6, 6);
        glPopMatrix();
    }
}
