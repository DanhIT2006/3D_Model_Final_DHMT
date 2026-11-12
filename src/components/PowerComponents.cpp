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

    onElectricalUpdate();
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

    // Vẽ cuộn cảm hình xuyến và tản nhiệt LM2596
    glPushMatrix();
    glTranslatef(-0.5f, 0.6f, 0.0f);
    glColor3f(0.2f, 0.2f, 0.22f); // Tản nhiệt đen
    glutSolidCube(1.1f);
    glPopMatrix();

    // Cuộn cảm tròn màu xám
    glPushMatrix();
    glTranslatef(0.8f, 0.6f, -0.3f);
    glColor3f(0.35f, 0.35f, 0.38f);
    glutSolidCylinder(0.45f, 0.7f, 12, 1);
    glPopMatrix();

    // Mô phỏng LED 7 đoạn hiển thị điện áp ngõ ra (màu đỏ rực)
    glPushMatrix();
    glTranslatef(0.5f, cadSize.y * 0.52f, 0.4f);
    glColor3f(0.05f, 0.05f, 0.05f); // Nền LED đen
    glScalef(1.4f, 0.2f, 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();

    if (isPowered) {
        // Ánh sáng LED đỏ số 5.0V
        glPushMatrix();
        glTranslatef(0.5f, cadSize.y * 0.55f + 0.12f, 0.4f);
        glColor3f(1.0f, 0.1f, 0.1f);
        glutSolidSphere(0.12f, 6, 6);
        glPopMatrix();
    }
}
