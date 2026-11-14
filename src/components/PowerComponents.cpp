#include "components/PowerComponents.h"
#include <GL/freeglut.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

// ===================== BAT_SW =====================
BatterySwitchComp::BatterySwitchComp()
    : HardwareComponent("BAT_SW", "Hop Pin 18650 & Cong Tac", "Nguon Dien") {
    cadSize = Vec3(7.8f, 2.2f, 4.2f);
    nominalVoltage = 7.4f;

    // Mesh hộp pin màu đen
    mesh = Mesh3D::createBox(cadSize.x, cadSize.y, cadSize.z, Vec3(0.12f, 0.12f, 0.14f));

    // Thêm các chân cắm
    addPin("BAT+", PinType::POWER_7V4, Vec3(cadSize.x * 0.45f, 0.0f, -cadSize.z * 0.25f), true);
    addPin("BAT-", PinType::POWER_GND, Vec3(cadSize.x * 0.45f, 0.0f,  cadSize.z * 0.25f), true);
    addPin("SW_OUT", PinType::POWER_7V4, Vec3(-cadSize.x * 0.45f, 0.0f, 0.0f), true);

    initSubComponents();
    onElectricalUpdate();
}

void BatterySwitchComp::initSubComponents() {
    subComponents.clear();

    SubComponent c1;
    c1.id = "CELL_1";
    c1.name = "Cell Pin Li-ion 18650 so 1";
    c1.rating = "3.7V - 2600mAh / Dong xa 20A";
    c1.roleDesc = "Cung cap nang luong hoa hoc chuyen doi thanh dien nang. Mac noi tiep voi Cell 2 tao dien ap tong 7.4V.";
    c1.localPos = Vec3(0.8f, 0.2f, -1.0f);
    c1.size = Vec3(5.6f, 1.8f, 1.8f);
    c1.color = Vec3(0.1f, 0.4f, 0.85f);
    c1.mesh = Mesh3D::createBox(5.6f, 1.8f, 1.8f, c1.color);
    addSubComponent(c1);

    SubComponent c2;
    c2.id = "CELL_2";
    c2.name = "Cell Pin Li-ion 18650 so 2";
    c2.rating = "3.7V - 2600mAh / Dong xa 20A";
    c2.roleDesc = "Cell pin thu hai mac noi tiep. Cung cap dong dien tai lon cho 2 dong co DC va 3 servo tay gap.";
    c2.localPos = Vec3(0.8f, 0.2f, 1.0f);
    c2.size = Vec3(5.6f, 1.8f, 1.8f);
    c2.color = Vec3(0.1f, 0.4f, 0.85f);
    c2.mesh = Mesh3D::createBox(5.6f, 1.8f, 1.8f, c2.color);
    addSubComponent(c2);

    SubComponent sw;
    sw.id = "ROCKER_SWITCH";
    sw.name = "Cong tac bap benh KCD1 On/Off";
    sw.rating = "6A 250VAC / 10A 12VDC / Mau do den";
    sw.roleDesc = "Dong ngat co hoc cuc duong (+) cua pack pin. Dam bao an toan tuyet doi, ngat toan bo dien nang khi bao tri hoac khong van hanh xe.";
    sw.localPos = Vec3(-2.2f, 0.6f, 0.0f);
    sw.size = Vec3(1.5f, 1.2f, 1.2f);
    sw.color = Vec3(0.85f, 0.15f, 0.1f);
    sw.isInteractive = true;
    sw.state = switchOn;
    sw.actionName = "BAT / TAT NGUON PIN";
    sw.mesh = Mesh3D::createBox(1.5f, 1.2f, 1.2f, sw.color);
    addSubComponent(sw);

    selectSubComponentByIndex(0);
}

void BatterySwitchComp::onSubComponentAction(const std::string& subId) {
    if (subId == "ROCKER_SWITCH") {
        toggleSwitch();
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->state = switchOn;
            sub->rating = switchOn ? "Trang thai: DANG BAT (ON - 7.4V)" : "Trang thai: DA TAT (OFF - 0V)";
            sub->color = switchOn ? Vec3(0.85f, 0.15f, 0.1f) : Vec3(0.35f, 0.35f, 0.35f);
        }
    }
}

void BatterySwitchComp::toggleSwitch() {
    switchOn = !switchOn;
    onElectricalUpdate();
}

void BatterySwitchComp::onElectricalUpdate() {
    isPowered = true; // Pin luôn có hóa năng tích lũy
    Pin* swPin = getPin("SW_OUT");
    Pin* batPos = getPin("BAT+");
    Pin* batNeg = getPin("BAT-");

    if (batPos) batPos->voltage = batteryVoltage;
    if (batNeg) batNeg->voltage = 0.0f;

    if (swPin) {
        swPin->voltage = switchOn ? batteryVoltage : 0.0f;
    }

    statusDesc = switchOn ? "BAT (7.4V Cap Nguon)" : "TAT (He Thong Nghi)";
}

void BatterySwitchComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // Vẽ công tắc gạt màu đỏ ở mặt sau/trên
    glPushMatrix();
    glTranslatef(-cadSize.x * 0.25f, cadSize.y * 0.55f, 0.0f);
    glColor3f(0.1f, 0.1f, 0.1f); // Khung công tắc
    glutSolidCube(1.0f);

    // Nút gập màu đỏ: nghiêng theo switchOn
    glTranslatef(0.0f, 0.2f, 0.0f);
    glRotatef(switchOn ? -18.0f : 18.0f, 0, 0, 1);
    glColor3f(switchOn ? 0.9f : 0.4f, 0.1f, 0.1f);
    glutSolidCube(0.7f);
    glPopMatrix();

    // Vẽ 2 thỏi pin 18650 xanh lam bên trong khay
    glPushMatrix();
    glTranslatef(cadSize.x * 0.12f, cadSize.y * 0.15f, -cadSize.z * 0.22f);
    glRotatef(90.0f, 0, 0, 1);
    glColor3f(0.1f, 0.35f, 0.85f); // Vỏ pin xanh lục/lam
    glutSolidCylinder(0.85f, 5.5f, 12, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(cadSize.x * 0.12f, cadSize.y * 0.15f, cadSize.z * 0.22f);
    glRotatef(90.0f, 0, 0, 1);
    glColor3f(0.1f, 0.35f, 0.85f);
    glutSolidCylinder(0.85f, 5.5f, 12, 1);
    glPopMatrix();
}

// ===================== DOMINO =====================
DominoBusComp::DominoBusComp()
    : HardwareComponent("DOMINO", "Tram Domino 3 Bus Nguon", "Phan Phoi Nguon") {
    cadSize = Vec3(6.0f, 1.5f, 2.5f);
    nominalVoltage = 7.4f;

    // Khối nhựa domino màu trắng đục cách điện
    mesh = Mesh3D::createBox(cadSize.x, cadSize.y, cadSize.z, Vec3(0.88f, 0.88f, 0.84f));

    // Chân Bus 7.4V
    addPin("IN_7V4",     PinType::POWER_7V4, Vec3(-2.2f, 0.6f, -0.8f));
    addPin("OUT_ARD",    PinType::POWER_7V4, Vec3(-0.8f, 0.6f, -0.8f), true);
    addPin("OUT_L298N",  PinType::POWER_7V4, Vec3( 0.6f, 0.6f, -0.8f), true);
    addPin("OUT_BUCK",   PinType::POWER_7V4, Vec3( 2.0f, 0.6f, -0.8f), true);

    // Chân Bus 5.0V
    addPin("IN_BUCK_5V", PinType::POWER_5V,  Vec3(-2.2f, 0.6f,  0.0f));
    addPin("OUT_SRV1",   PinType::POWER_5V,  Vec3(-0.8f, 0.6f,  0.0f), true);
    addPin("OUT_SRV2",   PinType::POWER_5V,  Vec3( 0.6f, 0.6f,  0.0f), true);
    addPin("OUT_SRV3",   PinType::POWER_5V,  Vec3( 2.0f, 0.6f,  0.0f), true);

    // Chân Bus GND
    addPin("IN_GND",     PinType::POWER_GND, Vec3(-2.2f, 0.6f,  0.8f));
    addPin("GND_ARD",    PinType::POWER_GND, Vec3(-0.8f, 0.6f,  0.8f), true);
    addPin("GND_L298N",  PinType::POWER_GND, Vec3( 0.6f, 0.6f,  0.8f), true);
    addPin("GND_BUCK",   PinType::POWER_GND, Vec3( 2.0f, 0.6f,  0.8f), true);
}

void DominoBusComp::onElectricalUpdate() {
    Pin* in7 = getPin("IN_7V4");
    Pin* in5 = getPin("IN_BUCK_5V");

    bus7V4 = in7 ? in7->voltage : 0.0f;
    bus5V  = in5 ? in5->voltage : 0.0f;
    busGND = 0.0f;

    isPowered = (bus7V4 > 3.0f || bus5V > 3.0f);

    // Phân phối áp ra các chân ngõ ra
    if (auto* p = getPin("OUT_ARD"))   p->voltage = bus7V4;
    if (auto* p = getPin("OUT_L298N")) p->voltage = bus7V4;
    if (auto* p = getPin("OUT_BUCK"))  p->voltage = bus7V4;

    if (auto* p = getPin("OUT_SRV1"))  p->voltage = bus5V;
    if (auto* p = getPin("OUT_SRV2"))  p->voltage = bus5V;
    if (auto* p = getPin("OUT_SRV3"))  p->voltage = bus5V;

    std::ostringstream ss;
    ss << "Bus 7.4V: " << std::fixed << std::setprecision(1) << bus7V4 << "V | Bus 5V: " << bus5V << "V";
    statusDesc = ss.str();
}

void DominoBusComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // Vẽ các vít kim loại trên đầu các mắt domino
    glDisable(GL_LIGHTING);
    glColor3f(0.7f, 0.7f, 0.75f);
    for (float z : {-0.8f, 0.0f, 0.8f}) {
        for (float x : {-2.2f, -0.8f, 0.6f, 2.0f}) {
            glPushMatrix();
            glTranslatef(x, 0.78f, z);
            glutSolidCube(0.25f);
            glPopMatrix();
        }
    }
}

// ===================== BUCK =====================
BuckLM2596Comp::BuckLM2596Comp()
    : HardwareComponent("BUCK", "Mach Ha Ap LM2596", "On Ap DC-DC") {
    cadSize = Vec3(4.3f, 1.4f, 2.1f);
    nominalVoltage = 7.4f;

    // Mạch PCB màu xanh dương/xanh lá
    mesh = Mesh3D::createBox(cadSize.x, cadSize.y, cadSize.z, Vec3(0.08f, 0.42f, 0.22f));

    addPin("IN+",  PinType::POWER_7V4, Vec3(-cadSize.x * 0.45f, 0.4f, -cadSize.z * 0.35f));
    addPin("IN-",  PinType::POWER_GND, Vec3(-cadSize.x * 0.45f, 0.4f,  cadSize.z * 0.35f));
    addPin("OUT+", PinType::POWER_5V,  Vec3( cadSize.x * 0.45f, 0.4f, -cadSize.z * 0.35f), true);
    addPin("OUT-", PinType::POWER_GND, Vec3( cadSize.x * 0.45f, 0.4f,  cadSize.z * 0.35f), true);

    initSubComponents();
}

void BuckLM2596Comp::initSubComponents() {
    subComponents.clear();

    SubComponent ic;
    ic.id = "IC_LM2596";
    ic.name = "IC Chuyen mach giam ap Buck LM2596S-ADJ";
    ic.rating = "Tan so 150kHz / Dong tai dinh muc 3A / TO-263";
    ic.roleDesc = "Chuyen doi dong dien bang co che bam xung PWM tan so cao 150kHz, ha tu 7.4V xuong 5.0V voi hieu suat vuot troi tren 90%, khong bi toa nhiet nhu on ap tuyen tinh.";
    ic.localPos = Vec3(-0.6f, 0.45f, -0.4f);
    ic.size = Vec3(1.1f, 0.45f, 0.9f);
    ic.color = Vec3(0.15f, 0.15f, 0.18f);
    ic.mesh = Mesh3D::createBox(1.1f, 0.45f, 0.9f, ic.color);
    addSubComponent(ic);

    SubComponent inductor;
    inductor.id = "TOROID_INDUCTOR";
    inductor.name = "Cuon cam xuyen loi Ferit 100uH";
    inductor.rating = "100 uH / 3A / Quan day dong boc men cach dien";
    inductor.roleDesc = "Linh kien cot loi cua mach Buck: Tich luy nang luong tu truong trong nua chu ky dong van, va xa nang luong duy tri dong dien phang min cho 3 dong co Servo khi van ngat.";
    inductor.localPos = Vec3(0.7f, 0.6f, -0.3f);
    inductor.size = Vec3(1.1f, 0.8f, 1.1f);
    inductor.color = Vec3(0.7f, 0.35f, 0.1f);
    inductor.mesh = Mesh3D::createCylinder(0.5f, 0.8f, 14, inductor.color);
    addSubComponent(inductor);

    SubComponent trimpot;
    trimpot.id = "TRIMPOT_10K";
    trimpot.name = "Chiet ap vi chinh da vong 10k (Trimpot 3296W)";
    trimpot.rating = "10 kOhm / 25 vong quay vi chinh do nhay cao";
    trimpot.roleDesc = "Tinh chinh cau phan ap phan hoi (Feedback Pin 4) de thay doi ti so chu ky xung PWM, giup dieu chinh chinh xac dien ap ra o muc 5.0V ly tuong nuoi Servo.";
    trimpot.localPos = Vec3(-0.6f, 0.65f, 0.45f);
    trimpot.size = Vec3(0.85f, 0.85f, 0.5f);
    trimpot.color = Vec3(0.1f, 0.35f, 0.75f);
    trimpot.isInteractive = true;
    trimpot.actionName = "CHINH TRIET AP (+0.2V)";
    trimpot.mesh = Mesh3D::createBox(0.85f, 0.85f, 0.5f, trimpot.color);
    addSubComponent(trimpot);

    SubComponent capIn;
    capIn.id = "CAP_IN";
    capIn.name = "Tu hoa dau vao 220uF 50V";
    capIn.rating = "220 uF - 50V / Loc song hai dau vao";
    capIn.roleDesc = "Loc nhieu dien ap nguon 7.4V cap vao mach Buck, triet tieu song hai tan so cao do van MOSFET dong ngat 150kHz sinh ra.";
    capIn.localPos = Vec3(-1.4f, 0.65f, 0.0f);
    capIn.size = Vec3(0.7f, 1.0f, 0.7f);
    capIn.color = Vec3(0.12f, 0.12f, 0.15f);
    capIn.mesh = Mesh3D::createCylinder(0.35f, 1.0f, 12, capIn.color);
    addSubComponent(capIn);

    SubComponent capOut;
    capOut.id = "CAP_OUT";
    capOut.name = "Tu hoa dau ra 220uF 35V";
    capOut.rating = "220 uF - 35V / San phang dien ap ra";
    capOut.roleDesc = "Ket hop voi cuon cam tao thanh mach loc thong thap LC, giu do gon song dien ap ra duoi 30mV de 3 servo MG996R hoat dong em ai, khong bi rung giat.";
    capOut.localPos = Vec3(1.5f, 0.65f, 0.0f);
    capOut.size = Vec3(0.7f, 1.0f, 0.7f);
    capOut.color = Vec3(0.12f, 0.12f, 0.15f);
    capOut.mesh = Mesh3D::createCylinder(0.35f, 1.0f, 12, capOut.color);
    addSubComponent(capOut);

    SubComponent ss34;
    ss34.id = "DIODE_SS34";
    ss34.name = "Diode Shottky phuc hoi nhanh SS34";
    ss34.rating = "3A / 40V / Thoi gian phuc hoi cuc nhanh < 10ns";
    ss34.roleDesc = "Cung cap duong xap dong tiep tuc cho cuon cam khi IC LM2596 ngat (Freewheeling Diode), bao ve IC khoi bi dien ap nguoc danh thung.";
    ss34.localPos = Vec3(0.1f, 0.28f, -0.4f);
    ss34.size = Vec3(0.45f, 0.25f, 0.45f);
    ss34.color = Vec3(0.1f, 0.1f, 0.1f);
    ss34.mesh = Mesh3D::createBox(0.45f, 0.25f, 0.45f, ss34.color);
    addSubComponent(ss34);

    selectSubComponentByIndex(0);
}

void BuckLM2596Comp::onSubComponentAction(const std::string& subId) {
    if (subId == "TRIMPOT_10K") {
        adjustVoltage(0.2f);
        if (targetOutVoltage > 5.8f) targetOutVoltage = 4.2f;
        onElectricalUpdate();
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            std::ostringstream ss;
            ss << "V_Out tinh chinh: " << std::fixed << std::setprecision(1) << actualOutVoltage << "V";
            sub->rating = ss.str();
        }
    }
}

void BuckLM2596Comp::adjustVoltage(float delta) {
    targetOutVoltage = std::clamp(targetOutVoltage + delta, 3.3f, 6.0f);
    onElectricalUpdate();
}

void BuckLM2596Comp::onElectricalUpdate() {
    Pin* inPos = getPin("IN+");
    float inV = inPos ? inPos->voltage : 0.0f;

    if (inV >= 6.0f) {
        isPowered = true;
        actualOutVoltage = targetOutVoltage;
    } else {
        isPowered = false;
        actualOutVoltage = 0.0f;
    }

    Pin* outPos = getPin("OUT+");
    if (outPos) outPos->voltage = actualOutVoltage;

    std::ostringstream ss;
    ss << "V_In: " << inV << "V -> V_Out: " << actualOutVoltage << "V";
    statusDesc = ss.str();
}

void BuckLM2596Comp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // 1. Cuon cam xuyen loi ferit quan day dong anh kim
    glPushMatrix();
    glTranslatef(0.7f, 0.6f, -0.3f);
    // Loi ferit tron mau xam den
    glColor3f(0.25f, 0.25f, 0.28f);
    glutSolidTorus(0.24f, 0.42f, 10, 16);
    // Cac vong day dong quan quanh xuyen
    glColor3f(0.85f, 0.45f, 0.12f);
    for (int d = 0; d < 8; ++d) {
        glPushMatrix();
        glRotatef(d * 45.0f, 0, 1, 0);
        glTranslatef(0.42f, 0.0f, 0.0f);
        glScalef(0.12f, 0.52f, 0.22f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 2. IC Chuyen mach Buck LM2596S kem tan nhiet han vao PCB
    glPushMatrix();
    glTranslatef(-0.6f, 0.45f, -0.4f);
    glColor3f(0.15f, 0.15f, 0.18f);
    glScalef(1.1f, 0.45f, 0.9f);
    glutSolidCube(1.0f);
    glColor3f(0.75f, 0.78f, 0.82f);
    glTranslatef(0.0f, 0.12f, -0.3f);
    glScalef(0.8f, 0.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 3. Chiet ap vi chinh da vong mau xanh duong 3296W num vit vang
    glPushMatrix();
    glTranslatef(-0.6f, 0.65f, 0.45f);
    glColor3f(0.1f, 0.35f, 0.75f); // Than xanh hop chiet ap
    glScalef(0.85f, 0.85f, 0.5f);
    glutSolidCube(1.0f);
    // Num vit vi chinh kim loai vang dong
    glColor3f(0.9f, 0.8f, 0.2f);
    glTranslatef(0.25f, 0.55f, 0.0f);
    glutSolidCylinder(0.12f, 0.15f, 8, 1);
    glPopMatrix();

    // 4. Cap tu hoa loc nguon vao va ra co nap nhom bac
    for (float cx : {-1.4f, 1.5f}) {
        glPushMatrix();
        glTranslatef(cx, 0.65f, 0.0f);
        glColor3f(0.12f, 0.12f, 0.15f);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.35f, 1.0f, 12, 2);
        glTranslatef(0.0f, 0.0f, 1.01f);
        glColor3f(0.82f, 0.85f, 0.88f);
        glutSolidCylinder(0.34f, 0.05f, 12, 1);
        glPopMatrix();
    }

    // 5. Man hinh LED 7 doan hien thi dien ap (Red LED Voltmeter)
    glPushMatrix();
    glTranslatef(0.35f, 0.52f, 0.4f);
    glColor3f(0.05f, 0.05f, 0.05f); // Nen led den
    glScalef(1.2f, 0.25f, 0.65f);
    glutSolidCube(1.0f);
    glPopMatrix();

    if (isPowered) {
        // Chu so LED do ruc sang 5.0V
        glPushMatrix();
        glTranslatef(0.35f, 0.66f, 0.4f);
        glColor3f(1.0f, 0.15f, 0.15f);
        glutSolidSphere(0.12f, 6, 6);
        glTranslatef(0.25f, 0.0f, 0.0f);
        glutSolidSphere(0.08f, 6, 6);
        glPopMatrix();
    }
}

