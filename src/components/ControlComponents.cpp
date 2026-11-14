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

    // 1. Vi dieu khien ATmega328P DIP-28
    SubComponent atmega;
    atmega.id = "ARD_MCU_328P";
    atmega.name = "Vi Dieu Khien Chinh ATmega328P-PU (DIP-28)";
    atmega.category = "Dieu khien trung tam";
    atmega.rating = "CPU 8-bit AVR RISC 16MHz / 32KB Flash / 2KB SRAM / Socket DIP-28";
    atmega.roleDesc = "Bo nao trung tam cua he thong xe robot: Nap va thuc thi firmware C++, xu ly thuat toan tu hanh do duong, tinh toan dong hoc 3 servo tay gap, phat xung PWM dieu toc 4 dong co va tinh toan khoang cach tu cam bien sieu am.";
    atmega.liveStatusText = "Tan so Clock: 16MHz | Chu ky lenh: 62.5ns | Flash: 32KB";
    atmega.localPos = Vec3(0.95f, 0.10f + 0.21f, 0.85f);
    atmega.size = Vec3(3.50f, 0.42f, 0.75f);
    atmega.color = Vec3(0.15f, 0.16f, 0.18f);
    atmega.shape = SubShapeType::BOX_SMD;
    atmega.mesh = Mesh3D::createBox(3.50f, 0.42f, 0.75f, atmega.color);
    addSubComponent(atmega);

    // 2. Nut nhan Reset
    SubComponent btnRst;
    btnRst.id = "ARD_RESET_BTN";
    btnRst.name = "Nut Nhan Reset Phan Cung (Tactile Reset Button)";
    btnRst.category = "Dieu khien khoi dong";
    btnRst.rating = "Push Button 6x6mm 4-pin tiep diem dong / Active LOW";
    btnRst.roleDesc = "Khi nhan, chan RESET cua ATmega328P bi keo xuong muc LOW (0V), dua CPU tro ve dia chi 0x0000 cua bo nho Flash, khoi dong lai toan bo firmware xe robot, dung dong co va dua 3 Servo ve vi tri chuan.";
    btnRst.liveStatusText = "Chan RESET: 5.0V (HIGH - Dang hoat dong binh thuong)";
    btnRst.localPos = Vec3(-2.85f, 0.10f + 0.22f, -2.10f);
    btnRst.size = Vec3(0.60f, 0.45f, 0.60f);
    btnRst.color = Vec3(0.85f, 0.2f, 0.1f);
    btnRst.shape = SubShapeType::PUSH_BUTTON;
    btnRst.isInteractive = true;
    btnRst.actionName = "NHAN RESET ARDUINO";
    btnRst.mesh = Mesh3D::createBox(0.60f, 0.45f, 0.60f, btnRst.color);
    addSubComponent(btnRst);

    // 3. Cong USB Type-B & ATmega16U2
    SubComponent usbPort;
    usbPort.id = "ARD_USB_16U2";
    usbPort.name = "Cong USB Type-B & Chip Cau Noi ATmega16U2";
    usbPort.category = "Giao tiep & Nap code";
    usbPort.rating = "USB 2.0 Full-Speed 12Mbps / Vo kim loai Niken / Chip ATmega16U2";
    usbPort.roleDesc = "Ket noi voi may tinh de nap firmware vao bo nho Flash qua Bootloader Optiboot, dong thoi giao tiep UART Serial truyen nhan du lieu dieu khien giua PC va Arduino.";
    usbPort.liveStatusText = "Giao tiep: UART 115200 baud | Nguon USB: 5.0V VBUS";
    usbPort.localPos = Vec3(-2.85f, 0.10f + 0.55f, -0.95f);
    usbPort.size = Vec3(1.60f, 1.10f, 1.20f);
    usbPort.color = Vec3(0.78f, 0.80f, 0.84f);
    usbPort.shape = SubShapeType::BOX_SMD;
    usbPort.mesh = Mesh3D::createBox(1.60f, 1.10f, 1.20f, usbPort.color);
    addSubComponent(usbPort);

    // 4. Jack nguon ngoai DC Barrel
    SubComponent dcJack;
    dcJack.id = "ARD_DC_JACK";
    dcJack.name = "Jack Nguon Ngoai DC Barrel (2.1mm Center Positive)";
    dcJack.category = "Cap nguon ngoai";
    dcJack.rating = "Duong kinh 5.5mm ngoai / 2.1mm trong / Dien ap khuyen nghi 7-12V";
    dcJack.roleDesc = "Dau tiep nhan nguon dien DC ngoai tu pack pin 7.4V tren xe robot, dan dien vao mach on ap tuyen tinh LDO tren bo de tao nguon 5V nuoi he thong.";
    dcJack.liveStatusText = "Nguon vao VIN: 7.4V tu Pin Li-ion 18650";
    dcJack.localPos = Vec3(-2.80f, 0.10f + 0.55f, 1.55f);
    dcJack.size = Vec3(1.45f, 1.10f, 0.90f);
    dcJack.color = Vec3(0.12f, 0.12f, 0.12f);
    dcJack.shape = SubShapeType::BOX_SMD;
    dcJack.mesh = Mesh3D::createBox(1.45f, 1.10f, 0.90f, dcJack.color);
    addSubComponent(dcJack);

    // 5. IC On ap 5V LDO AMS1117 & Cap tu loc 47uF
    SubComponent regLdo;
    regLdo.id = "ARD_VREG_5V";
    regLdo.name = "IC On Ap Tuyen Tinh 5V LDO & Cap Tu Loc 47uF";
    regLdo.category = "On ap nguon 5V";
    regLdo.rating = "AMS1117-5.0 / Vin 6.5-12V / Vout 5.0V / 2x Tu 47uF 16V tach roi";
    regLdo.roleDesc = "Ha ap tu 7.4V cua pack pin (qua jack DC hoac Vin) xuong muc 5.0V sieu phang on dinh de nuoi vi dieu khien ATmega328P va cac cam bien ngoai vi.";
    regLdo.liveStatusText = "Dien ap ra: 5.00V (On dinh tuyet doi, do gon < 15mV)";
    regLdo.localPos = Vec3(-2.45f, 0.10f + 0.09f, 0.35f);
    regLdo.size = Vec3(0.65f, 0.18f, 0.40f);
    regLdo.color = Vec3(0.18f, 0.18f, 0.20f);
    regLdo.shape = SubShapeType::BOX_SMD;
    regLdo.mesh = Mesh3D::createBox(0.65f, 0.18f, 0.40f, regLdo.color);
    addSubComponent(regLdo);

    // 6. Thach anh dao dong 16MHz
    SubComponent crystal;
    crystal.id = "ARD_CRYSTAL";
    crystal.name = "Thach Anh Dao Dong 16MHz (16.000 MHz Crystal)";
    crystal.category = "Dinh thoi xung nhip";
    crystal.rating = "Tan so 16.000 MHz / Do lech +/- 20ppm / Vo kim loai elip HC-49S";
    crystal.roleDesc = "Cung cap xung nhip clock ngoai cuc ky on dinh cho CPU ATmega328P (moi chu ky may = 62.5ns), dam bao do chinh xac tuyet doi cho bo dinh thoi Timer, xung PWM dieu toc va toc do truyen thong Baud rate.";
    crystal.liveStatusText = "Tan so dao dong: 16.000.000 Hz | Chu ky Clock: 62.5 ns";
    crystal.localPos = Vec3(-1.15f, 0.10f + 0.16f, 0.20f);
    crystal.size = Vec3(1.05f, 0.32f, 0.45f);
    crystal.color = Vec3(0.85f, 0.85f, 0.88f);
    crystal.shape = SubShapeType::CRYSTAL_OVAL;
    crystal.mesh = Mesh3D::createBox(1.05f, 0.32f, 0.45f, crystal.color);
    addSubComponent(crystal);

    // 7. Bo 4 den LED SMD chi bao (ON, L D13, TX, RX)
    SubComponent ledArd;
    ledArd.id = "ARD_LEDS";
    ledArd.name = "Cum 4 Den LED SMD Chi Bao (ON, L - Pin 13, TX, RX)";
    ledArd.category = "Chi bao trang thai";
    ledArd.rating = "4x SMD LED 0805 mau xanh la, vang, cam";
    ledArd.roleDesc = "LED ON sang xanh khi co nguon. LED L noi chan D13 test logic. LED TX/RX nhap nhay khi truyen nhan du lieu qua cong USB.";
    ledArd.liveStatusText = "LED ON: Sang xanh | LED D13: San sang test toggle";
    ledArd.localPos = Vec3(-0.65f, 0.10f + 0.08f, -1.15f);
    ledArd.size = Vec3(1.20f, 0.25f, 1.20f);
    ledArd.color = Vec3(0.95f, 0.85f, 0.1f);
    ledArd.shape = SubShapeType::LED_INDICATOR;
    ledArd.isInteractive = true;
    ledArd.actionName = "TEST BAT / TAT LED D13";
    ledArd.mesh = Mesh3D::createBox(1.20f, 0.25f, 1.20f, ledArd.color);
    addSubComponent(ledArd);

    // 8. Hang chan Digital PWM D0-D13 & Analog/Power
    SubComponent hdrDig;
    hdrDig.id = "ARD_HEADERS";
    hdrDig.name = "Hang Chan Cam Digital (0-13) & Power/Analog";
    hdrDig.category = "Dau noi chan I/O";
    hdrDig.rating = "Header nu 2.54mm mau den / 14 Digital + 6 Analog + 8 Power";
    hdrDig.roleDesc = "Noi day toi cam bien sieu am HC-SR04 (D2/D3), module L298N (PWM D5/D6, IN1-IN4) va 3 Servo MG996R (D9/D10/D11).";
    hdrDig.liveStatusText = "PWM D5/D6 -> L298N; D9/D10/D11 -> 3x Servo; D2/D3 -> HC-SR04";
    hdrDig.localPos = Vec3(0.0f, 0.10f + 0.425f, 0.0f);
    hdrDig.size = Vec3(6.0f, 0.85f, 5.0f);
    hdrDig.color = Vec3(0.12f, 0.12f, 0.12f);
    hdrDig.shape = SubShapeType::BOX_SMD;
    hdrDig.mesh = Mesh3D::createBox(6.0f, 0.85f, 5.0f, hdrDig.color);
    addSubComponent(hdrDig);

    selectSubComponentByIndex(0);
}
void ArduinoUnoComp::onSubComponentAction(const std::string& subId) {
    if (subId == "ARD_RESET_BTN" || subId == "BTN_RESET") {
        resetAutoSequence();
        motorSpeedL = motorSpeedR = 0;
        motorDirL = motorDirR = 0;
        baseAngleDeg = 90.0f;
        elbowAngleDeg = 90.0f;
        gripAngleDeg = 45.0f;
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->liveStatusText = "DA RESET VE 0x0000! (3 Servo ve vi tri goc, dong co dung)";
            sub->rating = "DA RESET VE 0x0000 (Active LOW 0V)";
        }
    } else if (subId == "ARD_LEDS" || subId == "LED_ARD") {
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->isActiveState = !sub->isActiveState;
            if (sub->isActiveState) {
                sub->liveStatusText = "LED D13: DANG BAT (Logic HIGH - 5.0V)";
                sub->color = Vec3(1.0f, 0.85f, 0.1f);
            } else {
                sub->liveStatusText = "LED D13: DANG TAT (Logic LOW - 0.0V)";
                sub->color = Vec3(0.2f, 0.2f, 0.25f);
            }
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

    // 0. 4 Lo bat oc vien vang o 4 goc PCB
    glColor3f(0.85f, 0.75f, 0.2f);
    for (float hx : {-2.85f, 3.05f}) {
        for (float hz : {-2.10f, 2.10f}) {
            float rx = hx;
            if (hx < 0 && hz > 0) rx = -1.25f; // Lo bat oc gan jack DC lui vao trong
            glPushMatrix();
            glTranslatef(rx, 0.10f, hz);
            glRotatef(-90.0f, 1, 0, 0);
            glutSolidCylinder(0.18f, 0.02f, 12, 1);
            glPopMatrix();
        }
    }

    // 1. Vi dieu khien ATmega328P DIP-28 tai (+0.95f, 0.10f, +0.85f)
    glPushMatrix();
    glTranslatef(0.95f, 0.10f, 0.85f);
    // De cam socket IC DIP-28 mau den
    glColor3f(0.08f, 0.08f, 0.08f);
    glPushMatrix();
    glTranslatef(0.0f, 0.12f, 0.0f);
    glScalef(3.50f, 0.24f, 0.75f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Than chip ATmega328P
    glColor3f(0.15f, 0.16f, 0.18f);
    glPushMatrix();
    glTranslatef(0.0f, 0.30f, 0.0f);
    glScalef(3.40f, 0.20f, 0.70f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Khe khuyet ban nguyet dinh huong Pin 1 o dau chip (-X)
    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix();
    glTranslatef(-1.70f, 0.35f, 0.0f);
    glScalef(0.12f, 0.12f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 28 Chan kim loai ma bac hai ben suon
    glColor3f(0.85f, 0.88f, 0.92f);
    for (int p = 0; p < 14; ++p) {
        float px = -1.45f + p * 0.223f;
        glPushMatrix();
        glTranslatef(px, 0.15f, -0.42f);
        glScalef(0.06f, 0.30f, 0.10f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(px, 0.15f, 0.42f);
        glScalef(0.06f, 0.30f, 0.10f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 2. Cong USB Type-B kim loai tai mep trai-tren (-2.85f, 0.10f, -0.95f)
    glPushMatrix();
    glTranslatef(-2.85f, 0.10f + 0.55f, -0.95f);
    glColor3f(0.78f, 0.80f, 0.84f);
    glPushMatrix();
    glScalef(1.60f, 1.10f, 1.20f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Hoc cam USB mau den ben trong quay sang mep trai (-X)
    glColor3f(0.10f, 0.10f, 0.12f);
    glPushMatrix();
    glTranslatef(-0.76f, 0.0f, 0.0f);
    glScalef(0.15f, 0.65f, 0.75f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Luoi chan tiep xuc ma vang ben trong
    glColor3f(0.95f, 0.82f, 0.2f);
    glPushMatrix();
    glTranslatef(-0.72f, -0.10f, 0.0f);
    glScalef(0.08f, 0.08f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // Chip cau noi USB ATmega16U2 tai (-1.45f, 0.10f, -0.75f)
    glPushMatrix();
    glTranslatef(-1.45f, 0.10f + 0.06f, -0.75f);
    glColor3f(0.15f, 0.15f, 0.18f);
    glScalef(0.55f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 3. Jack nguon tron DC Barrel tai mep trai-duoi (-2.80f, 0.10f, +1.55f)
    glPushMatrix();
    glTranslatef(-2.80f, 0.10f + 0.55f, 1.55f);
    glColor3f(0.12f, 0.12f, 0.14f);
    glPushMatrix();
    glScalef(1.45f, 1.10f, 0.90f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Lo cam nguon tron quay sang trai (-X)
    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix();
    glTranslatef(-0.73f, 0.0f, 0.0f);
    glRotatef(90.0f, 0, 1, 0);
    glutSolidCylinder(0.35f, 0.15f, 14, 1);
    // Chan kim cot dong o tam
    glColor3f(0.92f, 0.78f, 0.25f);
    glutSolidCylinder(0.09f, 0.25f, 8, 1);
    glPopMatrix();
    glPopMatrix();

    // 4. IC On ap AMS1117-5.0 tai (-2.45f, 0.10f, +0.35f)
    glPushMatrix();
    glTranslatef(-2.45f, 0.10f + 0.09f, 0.35f);
    glColor3f(0.18f, 0.18f, 0.20f);
    glPushMatrix();
    glScalef(0.65f, 0.18f, 0.40f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // La tan nhiet SOT-223
    glColor3f(0.85f, 0.88f, 0.92f);
    glPushMatrix();
    glTranslatef(-0.35f, 0.0f, 0.0f);
    glScalef(0.12f, 0.05f, 0.28f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // 2 Tu hoa SMD loc nguon TACH ROI NHAU tai (-1.45f, +1.55f) va (-0.75f, +1.55f)
    for (float cx : {-1.45f, -0.75f}) {
        glPushMatrix();
        glTranslatef(cx, 0.10f, 1.55f);
        glRotatef(-90.0f, 1, 0, 0);
        // Than tru nhom bac
        glColor3f(0.82f, 0.85f, 0.88f);
        glutSolidCylinder(0.28f, 0.50f, 14, 2);
        // Nap tren
        glTranslatef(0.0f, 0.0f, 0.50f);
        glColor3f(0.88f, 0.90f, 0.94f);
        glutSolidCylinder(0.275f, 0.05f, 14, 1);
        glPopMatrix();
        // De vuong den duoi chan
        glPushMatrix();
        glTranslatef(cx, 0.10f + 0.03f, 1.55f);
        glColor3f(0.12f, 0.12f, 0.14f);
        glScalef(0.60f, 0.06f, 0.60f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // 5. Thach anh dao dong 16MHz HC-49S kim loai noi bat tai (-1.15f, 0.10f, +0.20f)
    glPushMatrix();
    glTranslatef(-1.15f, 0.10f + 0.16f, 0.20f);
    glColor3f(0.86f, 0.88f, 0.92f);
    glPushMatrix();
    glScalef(1.05f, 0.32f, 0.45f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 2 Dau bo cong HC-49S
    for (float ex : {-0.52f, 0.52f}) {
        glPushMatrix();
        glTranslatef(ex, 0.0f, 0.0f);
        glRotatef(90.0f, 0, 1, 0);
        glutSolidCylinder(0.22f, 0.05f, 12, 1);
        glPopMatrix();
    }
    glPopMatrix();

    // 6. Nut nhan Reset phan cung tai (-2.85f, 0.10f, -2.10f)
    glPushMatrix();
    glTranslatef(-2.85f, 0.10f, -2.10f);
    // De vuong kim loai
    glColor3f(0.75f, 0.78f, 0.82f);
    glPushMatrix();
    glTranslatef(0.0f, 0.12f, 0.0f);
    glScalef(0.60f, 0.24f, 0.60f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Nut tron mau do nhan o tam
    glColor3f(0.88f, 0.20f, 0.15f);
    glPushMatrix();
    glTranslatef(0.0f, 0.24f, 0.0f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCylinder(0.18f, 0.18f, 12, 1);
    glPopMatrix();
    glPopMatrix();

    // 7. Cum 4 den LED SMD phat sang (ON, L, TX, RX) - KHONG DUNG QUA CAU XANH TO!
    // LED ON (Nguon - Xanh la): (+1.85f, -0.55f)
    glPushMatrix();
    glTranslatef(1.85f, 0.10f + 0.06f, -0.55f);
    glColor3f(isPowered ? 0.2f : 0.05f, isPowered ? 1.0f : 0.2f, isPowered ? 0.3f : 0.05f);
    glScalef(0.25f, 0.12f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // LED L (Pin 13 - Vang): (-0.65f, -1.55f)
    bool ledLOn = (autoSeqStep % 2 == 1);
    glPushMatrix();
    glTranslatef(-0.65f, 0.10f + 0.06f, -1.55f);
    glColor3f(ledLOn ? 1.0f : 0.2f, ledLOn ? 0.85f : 0.15f, 0.05f);
    glScalef(0.25f, 0.12f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // LED TX (Cam): (-0.65f, -1.05f)
    glPushMatrix();
    glTranslatef(-0.65f, 0.10f + 0.06f, -1.05f);
    glColor3f(0.85f, 0.45f, 0.05f);
    glScalef(0.25f, 0.12f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // LED RX (Cam): (-0.65f, -0.75f)
    glPushMatrix();
    glTranslatef(-0.65f, 0.10f + 0.06f, -0.75f);
    glColor3f(0.85f, 0.45f, 0.05f);
    glScalef(0.25f, 0.12f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 8. Hang header nu mau den (Digital High, Digital Low, Power, Analog)
    glColor3f(0.12f, 0.12f, 0.14f);
    // Day Digital canh tren (z = -2.40f)
    glPushMatrix();
    glTranslatef(0.85f, 0.10f + 0.425f, -2.40f);
    glScalef(4.80f, 0.85f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Day Power & Analog canh duoi (z = +2.40f)
    glPushMatrix();
    glTranslatef(0.65f, 0.10f + 0.425f, +2.40f);
    glScalef(4.40f, 0.85f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();
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
