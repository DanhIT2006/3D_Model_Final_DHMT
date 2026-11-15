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
    c1.name = "Cell Pin Li-ion 18650 So 1";
    c1.category = "Nguon dien hoa hoc";
    c1.rating = "3.7V - 2600mAh / Dong xa 20A";
    c1.roleDesc = "Cung cap nang luong hoa hoc chuyen doi thanh dien nang. Mac noi tiep voi Cell 2 tao dien ap tong 7.4V.";
    c1.liveStatusText = "Dien ap cell: 3.7V | Dung luong: 2600mAh";
    c1.localPos = Vec3(0.8f, 0.25f, -1.05f);
    c1.size = Vec3(5.6f, 1.8f, 1.8f);
    c1.color = Vec3(0.12f, 0.40f, 0.88f);
    c1.shape = SubShapeType::CYLINDER_CAP;
    c1.mesh = Mesh3D::createBox(5.6f, 1.8f, 1.8f, c1.color);
    addSubComponent(c1);

    SubComponent c2;
    c2.id = "CELL_2";
    c2.name = "Cell Pin Li-ion 18650 So 2";
    c2.category = "Nguon dien hoa hoc";
    c2.rating = "3.7V - 2600mAh / Dong xa 20A";
    c2.roleDesc = "Cell pin thu hai mac noi tiep. Cung cap dong dien tai lon cho 4 dong co DC va 3 servo tay gap.";
    c2.liveStatusText = "Dien ap cell: 3.7V | Tong dien ap pack: 7.4V";
    c2.localPos = Vec3(0.8f, 0.25f, 1.05f);
    c2.size = Vec3(5.6f, 1.8f, 1.8f);
    c2.color = Vec3(0.12f, 0.40f, 0.88f);
    c2.shape = SubShapeType::CYLINDER_CAP;
    c2.mesh = Mesh3D::createBox(5.6f, 1.8f, 1.8f, c2.color);
    addSubComponent(c2);

    SubComponent sw;
    sw.id = "ROCKER_SWITCH";
    sw.name = "Cong Tac Bap Benh KCD1 On/Off";
    sw.category = "Dong ngat an toan";
    sw.rating = "10A 12VDC / Tiep diem dong hop kim";
    sw.roleDesc = "Dong ngat co hoc cuc duong (+) cua pack pin. Dam bao an toan tuyet doi, ngat toan bo dien nang khi bao tri hoac khong van hanh xe.";
    sw.liveStatusText = switchOn ? "Trang thai: DANG BAT (ON - 7.4V cap toan he thong)" : "Trang thai: DANG TAT (OFF - Ngat dien toan xe)";
    sw.localPos = Vec3(-2.75f, 0.6f, 0.0f);
    sw.size = Vec3(1.5f, 1.2f, 1.5f);
    sw.color = switchOn ? Vec3(0.2f, 0.8f, 0.3f) : Vec3(0.85f, 0.15f, 0.1f);
    sw.shape = SubShapeType::PUSH_BUTTON;
    sw.isInteractive = true;
    sw.actionName = "BAT / TAT NGUON PIN";
    sw.mesh = Mesh3D::createBox(1.5f, 1.2f, 1.5f, sw.color);
    addSubComponent(sw);

    selectSubComponentByIndex(0);
}

void BatterySwitchComp::onSubComponentAction(const std::string& subId) {
    if (subId == "ROCKER_SWITCH") {
        toggleSwitch();
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->liveStatusText = switchOn ? "Trang thai: DANG BAT (ON - 7.4V cap toan he thong)" : "Trang thai: DANG TAT (OFF - Ngat dien toan xe)";
            sub->color = switchOn ? Vec3(0.2f, 0.8f, 0.3f) : Vec3(0.85f, 0.15f, 0.1f);
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

    // 1. Công tắc bập bênh KCD1 màu đỏ ở góc trái khay (x = -2.75f)
    glPushMatrix();
    glTranslatef(-2.75f, cadSize.y * 0.52f, 0.0f);
    // Khung viền công tắc màu đen
    glColor3f(0.10f, 0.10f, 0.12f);
    glPushMatrix();
    glScalef(1.5f, 0.35f, 1.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Phím bập bênh màu đỏ nghiêng theo switchOn
    glTranslatef(0.0f, 0.22f, 0.0f);
    glRotatef(switchOn ? -16.0f : 16.0f, 0, 0, 1);
    glColor3f(switchOn ? 0.90f : 0.40f, 0.12f, 0.10f);
    glScalef(1.10f, 0.25f, 1.10f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 2. Hai thỏi pin 18650 màu xanh dương nằm gọn bên trong khay dọc theo trục X
    // Kích thước chuẩn: dài 5.6cm, bán kính 0.85cm, đặt tại z = -1.05f và z = +1.05f
    for (float cz : {-1.05f, 1.05f}) {
        glPushMatrix();
        // Dịch về đầu âm (-X) tại x = -2.0f, quay 90 độ quanh Y để hướng dọc theo trục +X
        glTranslatef(-2.0f, 0.25f, cz);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // +Z thành +X

        // Lò xo tiếp điểm cực âm mạ bạc ở đầu -X
        glColor3f(0.75f, 0.78f, 0.82f);
        glutSolidCylinder(0.40f, 0.15f, 10, 1);

        // Thân pin hình trụ tròn màu xanh dương đặc trưng 18650
        glTranslatef(0.0f, 0.0f, 0.15f);
        glColor3f(0.12f, 0.40f, 0.88f);
        glutSolidCylinder(0.85f, 5.30f, 18, 2);

        // Vành cách điện và cực dương kim loại nhô lên ở đầu +X
        glTranslatef(0.0f, 0.0f, 5.30f);
        glColor3f(0.85f, 0.88f, 0.92f);
        glutSolidCylinder(0.38f, 0.18f, 14, 1);

        glPopMatrix();
    }
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

    // 1. IC Chuyen mach ha ap Buck LM2596S-ADJ
    SubComponent ic;
    ic.id = "BUCK_IC_2596";
    ic.name = "IC Nguon Xung LM2596S-ADJ";
    ic.category = "Chuyen mach ha ap";
    ic.rating = "Tan so 150kHz / Dong dinh muc 3A / Goi TO-263";
    ic.roleDesc = "Chuyen doi dien ap bang phuong phap bam xung PWM tan so cao 150kHz, ha tu 7.4V xuong 5.0V voi hieu suat vuot troi tren 90%, cung cap dong tai lon toi 3A cho 3 Servo MG996R.";
    ic.liveStatusText = "Tan so PWM: 150 kHz | Dong tai chiu dung: 3.0A Max";
    ic.localPos = Vec3(-0.6f, 0.45f, -0.4f);
    ic.size = Vec3(1.1f, 0.45f, 0.9f);
    ic.color = Vec3(0.15f, 0.15f, 0.18f);
    ic.shape = SubShapeType::BOX_SMD;
    ic.mesh = Mesh3D::createBox(1.1f, 0.45f, 0.9f, ic.color);
    addSubComponent(ic);

    // 2. Cuon cam xuyen loi Ferit 33uH-100uH
    SubComponent inductor;
    inductor.id = "BUCK_INDUCTOR";
    inductor.name = "Cuon Cam Loi Xuyen Ferit (Toroid 33uH)";
    inductor.category = "Tich tru & loc nang luong";
    inductor.rating = "33 uH / Chiu dong 3A / Quan day dong men cach dien";
    inductor.roleDesc = "Linh kien cot loi cua mach Buck: Tich luy nang luong tu truong trong nua chu ky dong van, va giai phong nang luong duy tri dong dien DC phang min cho 3 Servo khi van ngat.";
    inductor.liveStatusText = "Do tu cam: 33 uH | Tu truong Ferrite: On dinh";
    inductor.localPos = Vec3(0.7f, 0.6f, -0.3f);
    inductor.size = Vec3(1.1f, 0.8f, 1.1f);
    inductor.color = Vec3(0.7f, 0.35f, 0.1f);
    inductor.shape = SubShapeType::CYLINDER_CAP;
    inductor.mesh = Mesh3D::createCylinder(0.5f, 0.8f, 14, inductor.color);
    addSubComponent(inductor);

    // 3. Bien tro vi chinh da vong W103 (Potentiometer)
    SubComponent trimpot;
    trimpot.id = "BUCK_POT_W103";
    trimpot.name = "Bien Tro Vi Chinh Da Vong (Trimpot W103)";
    trimpot.category = "Chinh ap dau ra";
    trimpot.rating = "10 kOhm / 25 vong quay / Vit dong vi chinh";
    trimpot.roleDesc = "Thay doi dien tro cau phan ap phan hoi vao chan Feedback (Pin 4) de dieu chinh dien ap ra. Khi van xuong < 4.5V, 3 Servo MG996R bao thieu ap va ngung quay ngay!";
    trimpot.liveStatusText = "Dien ap ra hien tai: 5.0V (Du ap nuoi 3 Servo)";
    trimpot.localPos = Vec3(-0.6f, 0.65f, 0.45f);
    trimpot.size = Vec3(0.85f, 0.85f, 0.5f);
    trimpot.color = Vec3(0.1f, 0.35f, 0.75f);
    trimpot.shape = SubShapeType::PUSH_BUTTON;
    trimpot.isInteractive = true;
    trimpot.actionName = "VAN BIEN TRO (3.6V <-> 5.0V <-> 5.5V)";
    trimpot.mesh = Mesh3D::createBox(0.85f, 0.85f, 0.5f, trimpot.color);
    addSubComponent(trimpot);

    // 4. Cap tu hoa SMD loc nguon In / Out
    SubComponent capIn;
    capIn.id = "BUCK_CAPS";
    capIn.name = "Cap Tu Hoa Loc Nguon Vao/Ra (220uF 35V)";
    capIn.category = "Loc phang song hai";
    capIn.rating = "2x 220uF - 35V / Low-ESR nhom bac";
    capIn.roleDesc = "Tu dau vao loc nhieu xung 7.4V tu Pin; tu dau ra ket hop cuon cam tao bo loc LC ha thap do gon song xuong duoi 30mV giup Servo quay em, khong rung giat.";
    capIn.liveStatusText = "Vao: 7.4V DC | Ra: 5.0V DC | Do gon song: < 25mV";
    capIn.localPos = Vec3(-1.4f, 0.65f, 0.0f);
    capIn.size = Vec3(0.7f, 1.0f, 0.7f);
    capIn.color = Vec3(0.85f, 0.85f, 0.88f);
    capIn.shape = SubShapeType::CYLINDER_CAP;
    capIn.mesh = Mesh3D::createCylinder(0.35f, 1.0f, 12, capIn.color);
    addSubComponent(capIn);

    // 5. Diode Shottky SS34
    SubComponent ss34;
    ss34.id = "BUCK_SS34";
    ss34.name = "Diode Shottky SS34 (Freewheeling Diode)";
    ss34.category = "Xa dong cuon cam";
    ss34.rating = "3A / 40V / Thoi gian phuc hoi cuc nhanh < 10ns";
    ss34.roleDesc = "Cung cap duong xa dong tiep tuc cho cuon cam khi IC LM2596 ngat (Freewheeling Diode), bao ve IC khoi bi dien ap nguoc danh thung.";
    ss34.liveStatusText = "Diode Shottky 3A | Dong xa: Hoat dong";
    ss34.localPos = Vec3(0.1f, 0.28f, -0.4f);
    ss34.size = Vec3(0.45f, 0.25f, 0.45f);
    ss34.color = Vec3(0.1f, 0.1f, 0.1f);
    ss34.shape = SubShapeType::BOX_SMD;
    ss34.mesh = Mesh3D::createBox(0.45f, 0.25f, 0.45f, ss34.color);
    addSubComponent(ss34);

    selectSubComponentByIndex(0);
}

void BuckLM2596Comp::onSubComponentAction(const std::string& subId) {
    if (subId == "BUCK_POT_W103" || subId == "TRIMPOT_10K") {
        // Vặn biến trở xoay vòng: 5.0V -> 5.5V -> 3.6V (Thiếu áp!) -> 4.8V -> 5.0V
        if (targetOutVoltage == 5.0f) {
            targetOutVoltage = 5.5f;
        } else if (targetOutVoltage == 5.5f) {
            targetOutVoltage = 3.6f; // Thiếu áp: Servo ngừng quay!
        } else if (targetOutVoltage == 3.6f) {
            targetOutVoltage = 4.8f;
        } else {
            targetOutVoltage = 5.0f;
        }
        onElectricalUpdate();

        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            std::ostringstream ss;
            ss << "V_Out: " << std::fixed << std::setprecision(1) << actualOutVoltage << "V";
            sub->rating = ss.str() + " (Chinh boi bien tro W103)";
            if (actualOutVoltage < 4.5f) {
                sub->liveStatusText = "CANH BAO: Dien ap ra " + ss.str() + " < 4.5V -> 3 SERVO NGUNG QUAY!";
            } else {
                sub->liveStatusText = "Dien ap ra: " + ss.str() + " -> 3 SERVO HOAT DONG TOT";
            }
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

