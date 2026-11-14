#include "components/ActuatorComponents.h"
#include <GL/freeglut.h>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ===================== CAC HAM VE HINH HOC CO SO 3D CHUAN HOA =====================
namespace {
    const float PI = 3.14159265358979323846f;

    // Váº½ khá»‘i há»™p chá»¯ nháº­t Ä‘áº·c hoáº·c khung dÃ¢y vá»›i tÃ¢m táº¡i (cx, cy, cz)
    void drawSolidBox(float cx, float cy, float cz, float sx, float sy, float sz, const Vec3& color, bool wireframe) {
        float hx = sx * 0.5f;
        float hy = sy * 0.5f;
        float hz = sz * 0.5f;

        if (wireframe) {
            glDisable(GL_LIGHTING);
            glColor3f(color.x, color.y, color.z);
            glBegin(GL_LINES);
            // 4 Cáº¡nh Ä‘Ã¡y dÆ°á»›i
            glVertex3f(cx - hx, cy - hy, cz - hz); glVertex3f(cx + hx, cy - hy, cz - hz);
            glVertex3f(cx + hx, cy - hy, cz - hz); glVertex3f(cx + hx, cy - hy, cz + hz);
            glVertex3f(cx + hx, cy - hy, cz + hz); glVertex3f(cx - hx, cy - hy, cz + hz);
            glVertex3f(cx - hx, cy - hy, cz + hz); glVertex3f(cx - hx, cy - hy, cz - hz);
            // 4 Cáº¡nh náº¯p trÃªn
            glVertex3f(cx - hx, cy + hy, cz - hz); glVertex3f(cx + hx, cy + hy, cz - hz);
            glVertex3f(cx + hx, cy + hy, cz - hz); glVertex3f(cx + hx, cy + hy, cz + hz);
            glVertex3f(cx + hx, cy + hy, cz + hz); glVertex3f(cx - hx, cy + hy, cz + hz);
            glVertex3f(cx - hx, cy + hy, cz + hz); glVertex3f(cx - hx, cy + hy, cz - hz);
            // 4 Cá»™t Ä‘á»©ng
            glVertex3f(cx - hx, cy - hy, cz - hz); glVertex3f(cx - hx, cy + hy, cz - hz);
            glVertex3f(cx + hx, cy - hy, cz - hz); glVertex3f(cx + hx, cy + hy, cz - hz);
            glVertex3f(cx + hx, cy - hy, cz + hz); glVertex3f(cx + hx, cy + hy, cz + hz);
            glVertex3f(cx - hx, cy - hy, cz + hz); glVertex3f(cx - hx, cy + hy, cz + hz);
            glEnd();
        } else {
            glColor3f(color.x, color.y, color.z);
            glBegin(GL_QUADS);
            // Máº·t trÆ°á»›c (+Z)
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(cx - hx, cy - hy, cz + hz);
            glVertex3f(cx + hx, cy - hy, cz + hz);
            glVertex3f(cx + hx, cy + hy, cz + hz);
            glVertex3f(cx - hx, cy + hy, cz + hz);
            // Máº·t sau (-Z)
            glNormal3f(0.0f, 0.0f, -1.0f);
            glVertex3f(cx + hx, cy - hy, cz - hz);
            glVertex3f(cx - hx, cy - hy, cz - hz);
            glVertex3f(cx - hx, cy + hy, cz - hz);
            glVertex3f(cx + hx, cy + hy, cz - hz);
            // Máº·t trÃªn (+Y)
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(cx - hx, cy + hy, cz - hz);
            glVertex3f(cx - hx, cy + hy, cz + hz);
            glVertex3f(cx + hx, cy + hy, cz + hz);
            glVertex3f(cx + hx, cy + hy, cz - hz);
            // Máº·t dÆ°á»›i (-Y)
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(cx - hx, cy - hy, cz - hz);
            glVertex3f(cx + hx, cy - hy, cz - hz);
            glVertex3f(cx + hx, cy - hy, cz + hz);
            glVertex3f(cx - hx, cy - hy, cz + hz);
            // Máº·t pháº£i (+X)
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(cx + hx, cy - hy, cz - hz);
            glVertex3f(cx + hx, cy + hy, cz - hz);
            glVertex3f(cx + hx, cy + hy, cz + hz);
            glVertex3f(cx + hx, cy - hy, cz + hz);
            // Máº·t trÃ¡i (-X)
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(cx - hx, cy - hy, cz + hz);
            glVertex3f(cx - hx, cy + hy, cz + hz);
            glVertex3f(cx - hx, cy + hy, cz - hz);
            glVertex3f(cx - hx, cy - hy, cz - hz);
            glEnd();
        }
    }

    // Váº½ hÃ¬nh trá»¥ trÃ²n dá»c theo trá»¥c Y (tháº³ng Ä‘á»©ng), cÃ³ náº¯p 2 Ä‘áº§u vÃ  phÃ¡p tuyáº¿n chuáº©n
    void drawSolidCylinderY(float cx, float cy, float cz, float radius, float height, const Vec3& color, bool wireframe, int segments = 16) {
        float halfH = height * 0.5f;
        float yBot = cy - halfH;
        float yTop = cy + halfH;
        float dAngle = 2.0f * PI / segments;

        if (wireframe) {
            glDisable(GL_LIGHTING);
            glColor3f(color.x, color.y, color.z);
            // VÃ²ng trÃ²n Ä‘Ã¡y dÆ°á»›i & náº¯p trÃªn
            glBegin(GL_LINES);
            for (int i = 0; i < segments; ++i) {
                float a0 = i * dAngle;
                float a1 = (i + 1) * dAngle;
                float x0 = cx + radius * std::cos(a0);
                float z0 = cz + radius * std::sin(a0);
                float x1 = cx + radius * std::cos(a1);
                float z1 = cz + radius * std::sin(a1);

                // VÃ²ng Ä‘Ã¡y
                glVertex3f(x0, yBot, z0); glVertex3f(x1, yBot, z1);
                // VÃ²ng náº¯p
                glVertex3f(x0, yTop, z0); glVertex3f(x1, yTop, z1);
                // ÄÆ°á»ng sinh tháº³ng Ä‘á»©ng
                if (i % (segments / 4) == 0) {
                    glVertex3f(x0, yBot, z0); glVertex3f(x0, yTop, z0);
                }
            }
            glEnd();
        } else {
            glColor3f(color.x, color.y, color.z);
            // ThÃ¢n hÃ¬nh trá»¥
            glBegin(GL_QUADS);
            for (int i = 0; i < segments; ++i) {
                float a0 = i * dAngle;
                float a1 = (i + 1) * dAngle;
                float midA = (a0 + a1) * 0.5f;

                float x0 = cx + radius * std::cos(a0);
                float z0 = cz + radius * std::sin(a0);
                float x1 = cx + radius * std::cos(a1);
                float z1 = cz + radius * std::sin(a1);

                glNormal3f(std::cos(midA), 0.0f, std::sin(midA));
                glVertex3f(x0, yBot, z0);
                glVertex3f(x1, yBot, z1);
                glVertex3f(x1, yTop, z1);
                glVertex3f(x0, yTop, z0);
            }
            glEnd();

            // Náº¯p trÃªn (+Y)
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(cx, yTop, cz);
            for (int i = 0; i <= segments; ++i) {
                float a = i * dAngle;
                glVertex3f(cx + radius * std::cos(a), yTop, cz + radius * std::sin(a));
            }
            glEnd();

            // ÄÃ¡y dÆ°á»›i (-Y)
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(cx, yBot, cz);
            for (int i = segments; i >= 0; --i) {
                float a = i * dAngle;
                glVertex3f(cx + radius * std::cos(a), yBot, cz + radius * std::sin(a));
            }
            glEnd();
        }
    }

    // Váº½ hÃ¬nh trá»¥ trÃ²n náº±m ngang dá»c theo trá»¥c X
    void drawSolidCylinderX(float cx, float cy, float cz, float radius, float length, const Vec3& color, bool wireframe, int segments = 12) {
        float halfL = length * 0.5f;
        float xLeft = cx - halfL;
        float xRight = cx + halfL;
        float dAngle = 2.0f * PI / segments;

        if (wireframe) {
            glDisable(GL_LIGHTING);
            glColor3f(color.x, color.y, color.z);
            glBegin(GL_LINES);
            for (int i = 0; i < segments; ++i) {
                float a0 = i * dAngle;
                float a1 = (i + 1) * dAngle;
                float y0 = cy + radius * std::cos(a0);
                float z0 = cz + radius * std::sin(a0);
                float y1 = cy + radius * std::cos(a1);
                float z1 = cz + radius * std::sin(a1);

                glVertex3f(xLeft, y0, z0);  glVertex3f(xLeft, y1, z1);
                glVertex3f(xRight, y0, z0); glVertex3f(xRight, y1, z1);
                if (i % (segments / 4) == 0) {
                    glVertex3f(xLeft, y0, z0); glVertex3f(xRight, y0, z0);
                }
            }
            glEnd();
        } else {
            glColor3f(color.x, color.y, color.z);
            glBegin(GL_QUADS);
            for (int i = 0; i < segments; ++i) {
                float a0 = i * dAngle;
                float a1 = (i + 1) * dAngle;
                float midA = (a0 + a1) * 0.5f;

                float y0 = cy + radius * std::cos(a0);
                float z0 = cz + radius * std::sin(a0);
                float y1 = cy + radius * std::cos(a1);
                float z1 = cz + radius * std::sin(a1);

                glNormal3f(0.0f, std::cos(midA), std::sin(midA));
                glVertex3f(xLeft,  y0, z0);
                glVertex3f(xRight, y0, z0);
                glVertex3f(xRight, y1, z1);
                glVertex3f(xLeft,  y1, z1);
            }
            glEnd();

            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(xRight, cy, cz);
            for (int i = 0; i <= segments; ++i) {
                float a = i * dAngle;
                glVertex3f(xRight, cy + radius * std::cos(a), cz + radius * std::sin(a));
            }
            glEnd();

            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(xLeft, cy, cz);
            for (int i = segments; i >= 0; --i) {
                float a = i * dAngle;
                glVertex3f(xLeft, cy + radius * std::cos(a), cz + radius * std::sin(a));
            }
            glEnd();
        }
    }
}

// ===================== L298N =====================
L298NDriverComp::L298NDriverComp()
    : HardwareComponent("L298N", "Module Cau H Kep L298N", "Cong Suat Dong Co") {
    cadSize = Vec3(4.3f, 2.7f, 4.3f);
    nominalVoltage = 7.4f;

    // PCB mÃ u Ä‘á» sáº«m Ä‘áº·c trÆ°ng L298N
    mesh = Mesh3D::createBox(cadSize.x, 0.4f, cadSize.z, Vec3(0.72f, 0.12f, 0.12f));

    // Cá»c nguá»“n váº·n vÃ­t
    addPin("12V",  PinType::POWER_7V4, Vec3(-1.6f, 0.5f,  1.8f));
    addPin("GND",  PinType::POWER_GND, Vec3(-0.8f, 0.5f,  1.8f));
    addPin("5V",   PinType::POWER_5V,  Vec3( 0.0f, 0.5f,  1.8f));

    // ChÃ¢n tÃ­n hiá»‡u Ä‘iá»u khiá»ƒn tá»« ARD
    addPin("ENA",  PinType::PWM_IO,    Vec3(-1.8f, 0.4f, -1.8f));
    addPin("IN1",  PinType::DIGITAL_IO,Vec3(-1.2f, 0.4f, -1.8f));
    addPin("IN2",  PinType::DIGITAL_IO,Vec3(-0.6f, 0.4f, -1.8f));
    addPin("IN3",  PinType::DIGITAL_IO,Vec3( 0.6f, 0.4f, -1.8f));
    addPin("IN4",  PinType::DIGITAL_IO,Vec3( 1.2f, 0.4f, -1.8f));
    addPin("ENB",  PinType::PWM_IO,    Vec3( 1.8f, 0.4f, -1.8f));

    // NgÃµ ra Ä‘á»™ng cÆ¡
    addPin("OUT1", PinType::MOTOR_OUT, Vec3(-1.8f, 0.5f, 0.0f), true);
    addPin("OUT2", PinType::MOTOR_OUT, Vec3(-1.8f, 0.5f, 0.6f), true);
    addPin("OUT3", PinType::MOTOR_OUT, Vec3( 1.8f, 0.5f, 0.0f), true);
    addPin("OUT4", PinType::MOTOR_OUT, Vec3( 1.8f, 0.5f, 0.6f), true);

    initSubComponents();
}

void L298NDriverComp::initSubComponents() {
    subComponents.clear();

    // 1. IC Cau H L298N & Tan nhiet nhom xe ranh
    SubComponent heatsink;
    heatsink.id = "L298N_IC_HS";
    heatsink.name = "IC Cau H L298N & Tan Nhiet Nhom";
    heatsink.category = "Cong suat & Dao chieu";
    heatsink.rating = "Multiwatt15 / 46V max / 2A moi kenh / Nhom Anodized 6 ranh";
    heatsink.roleDesc = "Chua 2 mach cau H cong suat (Dual Full-Bridge) dieu khien toc do PWM va dao chieu 2 dong co DC. Tan nhiet nhom giup giai phong luong nhiet lon sinh ra tu cac transistor BJT cong suat khi dong co mang tai nang.";
    heatsink.liveStatusText = "Dien ap dong luc: 7.4V | Nhiet do: 42°C (Binh thuong)";
    heatsink.localPos = Vec3(0.0f, 1.25f, -1.05f);
    heatsink.size = Vec3(2.8f, 2.1f, 0.9f);
    heatsink.color = Vec3(0.18f, 0.18f, 0.20f);
    heatsink.shape = SubShapeType::HEATSINK_VERTICAL;
    heatsink.mesh = Mesh3D::createBox(2.8f, 2.1f, 0.9f, heatsink.color);
    addSubComponent(heatsink);

    // 2. Tu hoa loc nguon dong luc VCC C1
    SubComponent capVcc;
    capVcc.id = "L298N_CAP_HV";
    capVcc.name = "Tu Hoa Loc Nguon Dong Luc 220uF/35V";
    capVcc.category = "Loc nguon dau vao";
    capVcc.rating = "220 uF - 35V / Chiu nhiet 105°C / Nap nhom co ranh chong no";
    capVcc.roleDesc = "Nam tren duong nguon dong luc VCC (7.4V), tich luy va bu dong tuc thoi khi 2 dong co DC khoi dong (de-pa) hoac dao chieu dot ngot, ngan ngua sut ap nguon he thong.";
    capVcc.liveStatusText = "Dien ap nap: 7.4V DC | Trang thai: San phang tot";
    capVcc.localPos = Vec3(-0.45f, 0.65f, 0.15f);
    capVcc.size = Vec3(0.7f, 1.1f, 0.7f);
    capVcc.color = Vec3(0.85f, 0.85f, 0.88f);
    capVcc.shape = SubShapeType::CYLINDER_CAP;
    capVcc.mesh = Mesh3D::createCylinder(0.35f, 1.1f, 12, capVcc.color);
    addSubComponent(capVcc);

    // 3. Tu hoa loc nguon logic 5V C2
    SubComponent cap5v;
    cap5v.id = "L298N_CAP_LV";
    cap5v.name = "Tu Hoa Loc Nguon Logic 220uF/35V";
    cap5v.category = "Loc nguon 5V";
    cap5v.rating = "220 uF - 35V / Loc song hai logic";
    cap5v.roleDesc = "Loc phang nhieu cao tan o dau ra cua IC on ap 78M05, bao ve cac cong logic noi bo ben trong IC L298N khong bi nhieu xung tu cuon day dong co.";
    cap5v.liveStatusText = "Dien ap logic: 5.0V DC | Trang thai: On dinh";
    cap5v.localPos = Vec3(0.55f, 0.65f, 0.45f);
    cap5v.size = Vec3(0.7f, 1.1f, 0.7f);
    cap5v.color = Vec3(0.85f, 0.85f, 0.88f);
    cap5v.shape = SubShapeType::CYLINDER_CAP;
    cap5v.mesh = Mesh3D::createCylinder(0.35f, 1.1f, 12, cap5v.color);
    addSubComponent(cap5v);

    // 4. IC On ap tuyen tinh 78M05
    SubComponent reg7805;
    reg7805.id = "L298N_78M05";
    reg7805.name = "IC On Ap Tuyen Tinh 78M05 (5V Logic)";
    reg7805.category = "On ap nguon logic";
    reg7805.rating = "Vin 7-35V / Vout 5.0V / Iout 500mA / Goi TO-252";
    reg7805.roleDesc = "Ha ap truc tiep tu nguon pin 7.4V xuong 5.0V on dinh de nuoi mach logic dieu khien noi bo cua IC L298N.";
    reg7805.liveStatusText = "V_In: 7.4V -> V_Out: 5.0V | Hieu suat: Hoat dong tot";
    reg7805.localPos = Vec3(0.55f, 0.32f, -0.15f);
    reg7805.size = Vec3(0.65f, 0.3f, 0.6f);
    reg7805.color = Vec3(0.2f, 0.2f, 0.22f);
    reg7805.shape = SubShapeType::BOX_SMD;
    reg7805.mesh = Mesh3D::createBox(0.65f, 0.3f, 0.6f, reg7805.color);
    addSubComponent(reg7805);

    // 5. Jumper kich hoat on ap 5V Enable
    SubComponent jmp5v;
    jmp5v.id = "L298N_JMP_5V";
    jmp5v.name = "Jumper Kich Hoat On Ap 5V (5V Enable)";
    jmp5v.category = "Cau hinh nguon logic";
    jmp5v.rating = "Header 2-pin 2.54mm kem cap shunt vang/den";
    jmp5v.roleDesc = "KHI CAM: dung IC 78M05 tren mach tu ha ap 7.4V xuong 5V nuoi logic. KHI RUT: ngat 78M05, mat nguon logic 5V noi bo, bat buoc phai cap 5V tu ngoai (neu khong cap thi xe dung ngay)!";
    jmp5v.liveStatusText = "Trang thai: DANG CAM (Logic 5V noi bo BAT - Xe san sang chay)";
    jmp5v.localPos = Vec3(-0.45f, 0.42f, 0.85f);
    jmp5v.size = Vec3(0.45f, 0.55f, 0.35f);
    jmp5v.color = Vec3(0.95f, 0.85f, 0.1f);
    jmp5v.shape = SubShapeType::JUMPER_CAP;
    jmp5v.isInteractive = true;
    jmp5v.isActiveState = true;
    jmp5v.state = true;
    jmp5v.actionName = "RUT / CAM JUMPER 5V";
    jmp5v.mesh = Mesh3D::createBox(0.45f, 0.55f, 0.35f, jmp5v.color);
    addSubComponent(jmp5v);

    // 6. Cum 8 Diode xung bao ve Flyback D1-D8
    SubComponent dio;
    dio.id = "L298N_DIODES";
    dio.name = "Cum 8 Diode Xung Bao Ve (Flyback D1 - D8)";
    dio.category = "Bao ve chong dong nguoc";
    dio.rating = "8x Diode chinh luu xung nhanh / Fast Recovery Shottky";
    dio.roleDesc = "Dau song song nguoc voi cac cuon day dong co DC. Triet tieu suc dien dong cam ung nguoc (Back-EMF) cuc lon sinh ra khi ngat xung PWM hoac dao chieu dot ngot, bao ve transistor trong IC L298N.";
    dio.liveStatusText = "8x Diode xung | Bao ve: San sang dap xung Back-EMF";
    dio.localPos = Vec3(0.0f, 0.28f, -0.4f);
    dio.size = Vec3(3.6f, 0.25f, 1.4f);
    dio.color = Vec3(0.1f, 0.1f, 0.12f);
    dio.shape = SubShapeType::BOX_SMD;
    dio.mesh = Mesh3D::createBox(3.6f, 0.25f, 1.4f, dio.color);
    addSubComponent(dio);

    // 7. Domino nguon va dong co
    SubComponent term;
    term.id = "L298N_PWR_TERM";
    term.name = "Tram Domino Nguon (VCC, GND, 5V) & Dong Co (OUT1-4)";
    term.category = "Dau noi day nguon & dong luc";
    term.rating = "3x Khoi van vit 5.08mm mau xanh duong chiu dong 15A";
    term.roleDesc = "Cong VCC nhan 7.4V tu Pin qua cong tac; GND noi dat chung; 5V cap/nhan logic; OUT1-OUT2 cap nguon dong co trai; OUT3-OUT4 cap nguon dong co phai.";
    term.liveStatusText = "Ket noi: VCC 7.4V, OUT1..OUT4 noi 4 cum TT Motor";
    term.localPos = Vec3(0.0f, 0.52f, 1.1f);
    term.size = Vec3(3.8f, 0.85f, 1.1f);
    term.color = Vec3(0.1f, 0.38f, 0.85f);
    term.shape = SubShapeType::BOX_SMD;
    term.mesh = Mesh3D::createBox(3.8f, 0.85f, 1.1f, term.color);
    addSubComponent(term);

    // 8. Hang chan tin hieu Logic
    SubComponent hdrLogic;
    hdrLogic.id = "L298N_CTRL_HDR";
    hdrLogic.name = "Hang Chan Dieu Khien (ENA, IN1-IN4, ENB)";
    hdrLogic.category = "Giao tiep vi dieu khien";
    hdrLogic.rating = "6-pin Header duc 2.54mm ma vang / Logic TTL 5V";
    hdrLogic.roleDesc = "Nhan tin hieu tu Arduino: ENA/ENB nhan xung PWM de dieu toc xe; IN1/IN2 quy dinh chieu quay banh trai; IN3/IN4 quy dinh chieu quay banh phai.";
    hdrLogic.liveStatusText = "Tin hieu: PWM ENA/ENB tu Arduino D5, D6; Logic IN1..IN4 tu D4, D7, D8, D12";
    hdrLogic.localPos = Vec3(0.8f, 0.38f, 1.6f);
    hdrLogic.size = Vec3(1.5f, 0.55f, 0.35f);
    hdrLogic.color = Vec3(0.85f, 0.75f, 0.2f);
    hdrLogic.shape = SubShapeType::BOX_SMD;
    hdrLogic.mesh = Mesh3D::createBox(1.5f, 0.55f, 0.35f, hdrLogic.color);
    addSubComponent(hdrLogic);

    selectSubComponentByIndex(0);
}

void L298NDriverComp::onSubComponentAction(const std::string& subId) {
    if (subId == "L298N_JMP_5V" || subId == "JUMPER_5V") {
        jumper5VEnabled = !jumper5VEnabled;
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->isActiveState = jumper5VEnabled;
            sub->state = jumper5VEnabled;
            if (jumper5VEnabled) {
                sub->liveStatusText = "Trang thai: DANG CAM (Logic 5V noi bo BAT - Xe san sang chay)";
                sub->rating = "Trang thai: DANG CAM (Enable 78M05 ON)";
                sub->roleDesc = "Jumper DANG CAM: Kich hoat IC on ap 78M05 noi bo tao nguon 5V tu Pin VCC (7.4V) de nuoi mach logic L298N. Xe hoat dong binh thuong!";
                sub->color = Vec3(0.95f, 0.85f, 0.1f);
            } else {
                sub->liveStatusText = "Trang thai: DA RUT (MAT NGUON LOGIC 5V! Xe dung ngay)";
                sub->rating = "Trang thai: DA RUT (Disable 78M05 OFF)";
                sub->roleDesc = "Jumper DA RUT: Ngat IC on ap 78M05. Mach L298N bi mat nguon logic 5V noi bo, toan bo dong co ngat dien va xe dung ngay lap tuc!";
                sub->color = Vec3(0.4f, 0.4f, 0.4f);
            }
        }
        onElectricalUpdate();
    }
}

void L298NDriverComp::onElectricalUpdate() {
    Pin* p12v = getPin("12V");
    Pin* p5v  = getPin("5V");

    float v12 = p12v ? p12v->voltage : 0.0f;
    float v5  = p5v  ? p5v->voltage  : 0.0f;

    // Neu Jumper 5V Enable duoc cam, IC 78M05 tu cap 5V logic tu nguon 7.4V (v12)
    if (jumper5VEnabled && v12 >= 6.0f) {
        v5 = 5.0f;
        if (p5v) p5v->voltage = 5.0f;
    }

    // Can ca nguon dong luc (v12 >= 6.0V) va 5V logic (v5 >= 4.5V)
    isPowered = (v12 >= 6.0f && v5 >= 4.5f);

    if (isPowered) {
        Pin* ena = getPin("ENA");
        Pin* in1 = getPin("IN1");
        Pin* in2 = getPin("IN2");
        Pin* enb = getPin("ENB");
        Pin* in3 = getPin("IN3");
        Pin* in4 = getPin("IN4");

        float dutyL = ena ? ena->pwmDuty : 0.0f;
        float dutyR = enb ? enb->pwmDuty : 0.0f;

        float dirL = 0.0f;
        if (in1 && in2) {
            if (in1->voltage > 2.5f && in2->voltage < 1.0f) dirL = 1.0f;
            else if (in1->voltage < 1.0f && in2->voltage > 2.5f) dirL = -1.0f;
        }

        float dirR = 0.0f;
        if (in3 && in4) {
            if (in3->voltage > 2.5f && in4->voltage < 1.0f) dirR = 1.0f;
            else if (in3->voltage < 1.0f && in4->voltage > 2.5f) dirR = -1.0f;
        }

        outVoltL = dirL * dutyL * v12;
        outVoltR = dirR * dutyR * v12;

        Pin* o1 = getPin("OUT1");
        Pin* o2 = getPin("OUT2");
        Pin* o3 = getPin("OUT3");
        Pin* o4 = getPin("OUT4");

        if (o1 && o2) {
            if (outVoltL >= 0.0f) { o1->voltage = outVoltL; o2->voltage = 0.0f; }
            else { o1->voltage = 0.0f; o2->voltage = -outVoltL; }
        }
        if (o3 && o4) {
            if (outVoltR >= 0.0f) { o3->voltage = outVoltR; o4->voltage = 0.0f; }
            else { o3->voltage = 0.0f; o4->voltage = -outVoltR; }
        }
    } else {
        outVoltL = outVoltR = 0.0f;
    }

    std::ostringstream ss;
    ss << (isPowered ? "HOAT DONG" : "NGAT NGUON") 
       << " | OutL: " << std::fixed << std::setprecision(1) << outVoltL << "V"
       << " | OutR: " << outVoltR << "V";
    statusDesc = ss.str();
}

void L298NDriverComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // 1. Khoi tan nhiet nhom mau den co 6 canh xe ranh (Finned Heatsink)
    glPushMatrix();
    glTranslatef(0.0f, 1.25f, -1.05f);
    glColor3f(0.18f, 0.18f, 0.20f);
    glPushMatrix();
    glScalef(2.8f, 0.35f, 0.9f);
    glutSolidCube(1.0f);
    glPopMatrix();
    for (float fx = -1.2f; fx <= 1.25f; fx += 0.48f) {
        glPushMatrix();
        glTranslatef(fx, 0.9f, 0.0f);
        glScalef(0.12f, 1.7f, 0.9f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 2. Chip IC L298N gan op vao mat tan nhiet kem oc vit va chan kim loai
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, -0.55f);
    glColor3f(0.08f, 0.08f, 0.08f);
    glScalef(2.2f, 1.3f, 0.3f);
    glutSolidCube(1.0f);
    glColor3f(0.85f, 0.88f, 0.92f);
    glTranslatef(0.0f, 0.32f, 0.55f);
    glutSolidCylinder(0.18f, 0.2f, 8, 1);
    glPopMatrix();

    // 3. 2 Tu hoa loc nguon 220uF 35V voi nap kim loai xe ranh
    for (float x : {-0.45f, 0.55f}) {
        float z = (x < 0) ? 0.15f : 0.45f;
        glPushMatrix();
        glTranslatef(x, 0.65f, z);
        glColor3f(0.12f, 0.12f, 0.14f);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.35f, 1.1f, 14, 2);
        glTranslatef(0.0f, 0.0f, 1.11f);
        glColor3f(0.82f, 0.85f, 0.88f);
        glutSolidCylinder(0.34f, 0.05f, 14, 1);
        glPopMatrix();
    }

    // 4. IC On ap 5V 78M05 TO-252
    glPushMatrix();
    glTranslatef(0.55f, 0.32f, -0.15f);
    glColor3f(0.2f, 0.2f, 0.22f);
    glScalef(0.65f, 0.25f, 0.6f);
    glutSolidCube(1.0f);
    glColor3f(0.75f, 0.78f, 0.8f);
    glTranslatef(0.0f, 0.1f, -0.2f);
    glScalef(0.8f, 0.1f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 5. Jumper chon nguon 5V Enable
    glPushMatrix();
    glTranslatef(-0.45f, 0.42f, 0.85f);
    glColor3f(jumper5VEnabled ? 0.1f : 0.35f, jumper5VEnabled ? 0.6f : 0.35f, jumper5VEnabled ? 0.9f : 0.35f);
    glScalef(0.45f, 0.55f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 6. 3 Khoi coc dau nguon & Motor mau xanh duong kem vit M3 ma ken
    auto drawTerminal = [](float tx, float ty, float tz, float sx, float sy, float sz, int poles) {
        glPushMatrix();
        glTranslatef(tx, ty, tz);
        glColor3f(0.1f, 0.35f, 0.85f);
        glScalef(sx, sy, sz);
        glutSolidCube(1.0f);
        glPopMatrix();

        for (int p = 0; p < poles; ++p) {
            float ox = tx + (p - (poles - 1) * 0.5f) * (sx / poles);
            glPushMatrix();
            glTranslatef(ox, ty + sy * 0.52f, tz);
            glColor3f(0.85f, 0.88f, 0.92f);
            glutSolidCylinder(0.14f, 0.08f, 8, 1);
            glPopMatrix();
        }
    };
    drawTerminal(-0.6f, 0.52f, 1.6f, 1.6f, 0.85f, 0.85f, 3);
    drawTerminal(-1.65f, 0.52f, 0.6f, 0.85f, 0.85f, 1.1f, 2);
    drawTerminal( 1.65f, 0.52f, 0.6f, 0.85f, 0.85f, 1.1f, 2);

    // 7. Dan 8 Diode xung bao ve Flyback
    glColor3f(0.1f, 0.1f, 0.1f);
    for (float dx : {-1.75f, 1.75f}) {
        glPushMatrix();
        glTranslatef(dx, 0.28f, -0.4f);
        glScalef(0.35f, 0.25f, 1.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // 8. Hang rao 6 chan cam tin hieu logic (ENA, IN1-4, ENB)
    glPushMatrix();
    glTranslatef(0.8f, 0.38f, 1.6f);
    glColor3f(0.85f, 0.75f, 0.2f);
    glScalef(1.5f, 0.55f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// ===================== DONG CO DC & BANH XE =====================
DCMotorComp::DCMotorComp(bool isLeft)
    : DCMotorComp(isLeft ? "MOT_L" : "MOT_R",
                  isLeft ? "Dong Co Va Banh Trai" : "Dong Co Va Banh Phai",
                  isLeft) {}

DCMotorComp::DCMotorComp(const std::string& code, const std::string& name, bool isLeft)
    : HardwareComponent(code, name, "Dong Luc"),
      isLeftMotor(isLeft) {
    cadSize = Vec3(6.5f, 4.0f, 8.0f);
    wheelRadius = 3.3f;
    nominalVoltage = 7.4f;

    mesh = Mesh3D::createBox(2.2f, 2.4f, 5.5f, Vec3(0.96f, 0.78f, 0.1f));

    addPin("A", PinType::MOTOR_OUT, Vec3(0.0f, 0.0f, -2.4f));
    addPin("B", PinType::MOTOR_OUT, Vec3(0.0f, 0.0f, -1.6f));
}

void DCMotorComp::updatePhysics(float dt, float inputVoltage, bool driverPowered) {
    float maxVolt = 7.4f;
    float maxRpm  = 200.0f;

    rpm = (inputVoltage / maxVolt) * maxRpm;
    float rps = rpm / 60.0f;
    float omega = rps * 2.0f * PI;

    wheelAngle += omega * dt;
    linearSpeedCmS = omega * wheelRadius;

    isPowered = driverPowered;

    std::ostringstream ss;
    if (!isPowered) {
        ss << "NGAT NGUON (OFF) | V_In: 0.0V";
    } else if (std::fabs(rpm) < 1.0f) {
        ss << "SAN SANG (STANDBY) | V_In: 0.0V | Nhan WASD de chay";
    } else {
        ss << "DANG CHAY (" << std::fixed << std::setprecision(1) << inputVoltage << "V) | " 
           << std::setprecision(0) << rpm << " RPM | " 
           << std::setprecision(1) << linearSpeedCmS << " cm/s";
    }
    statusDesc = ss.str();
}

void DCMotorComp::onElectricalUpdate() {
    Pin* pa = getPin("A");
    Pin* pb = getPin("B");
    float v = (pa && pb) ? (pa->voltage - pb->voltage) : 0.0f;
    updatePhysics(0.016f, v, isPowered);
}

void DCMotorComp::render(bool wireframeMode) {
    if (!visible) return;

    glPushMatrix();
    Mat4 localM = getLocalMatrix();
    glMultMatrixf(localM.data());

    // 1. DONG CO GIAM TOC TT MOTOR
    if (wireframeMode) {
        mesh.drawWireframe(isSelected ? Vec3(1.0f, 0.85f, 0.0f) : Vec3(0.0f, 0.95f, 0.4f));
    } else {
        // Hop giam toc vang ABS
        glPushMatrix();
        glColor3f(0.96f, 0.78f, 0.08f);
        glTranslatef(0.0f, 0.0f, 0.5f);
        glScalef(2.2f, 2.2f, 3.8f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Tai bat oc hop so (Mounting ears)
        glPushMatrix();
        glColor3f(0.92f, 0.74f, 0.06f);
        glTranslatef(0.0f, 0.0f, 2.6f);
        glScalef(2.2f, 1.2f, 0.5f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Than dong co dien DC bang thep ma ken / bac
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -2.2f);
        glColor3f(0.78f, 0.80f, 0.84f);
        glRotatef(90.0f, 1, 0, 0);
        glutSolidCylinder(0.95f, 2.2f, 16, 2);
        glPopMatrix();

        // Nap duoi dong co mau den + 2 chan cuc han day dong
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -3.3f);
        glColor3f(0.15f, 0.15f, 0.15f);
        glutSolidCylinder(0.85f, 0.3f, 12, 1);
        glColor3f(0.85f, 0.55f, 0.15f);
        glTranslatef(-0.35f, 0.0f, -0.2f);
        glutSolidCube(0.25f);
        glTranslatef(0.7f, 0.0f, 0.0f);
        glutSolidCube(0.25f);
        glPopMatrix();

        // Gia do kim loai chu L bat motor vao gam xe
        glPushMatrix();
        glColor3f(0.45f, 0.48f, 0.52f);
        glTranslatef(isLeftMotor ? 1.05f : -1.05f, 0.6f, 0.5f);
        glScalef(0.2f, 1.5f, 2.8f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Truc ra hop so D-shaft mau trang noi sang vanh banh xe
        glPushMatrix();
        float shaftDir = isLeftMotor ? -1.0f : 1.0f;
        glTranslatef(shaftDir * 1.5f, 0.0f, 0.0f);
        glColor3f(0.92f, 0.92f, 0.95f);
        glRotatef(90.0f, 0, 1, 0);
        glutSolidCylinder(0.27f, 1.2f, 8, 1);
        glPopMatrix();

        if (isSelected) mesh.drawWireframe(Vec3(1.0f, 0.85f, 0.0f), 2.5f);
    }

    // 2. BANH XE CAO SU SIEU CHI TIET (HIGH-FIDELITY TT WHEEL)
    float wheelX = isLeftMotor ? -2.6f : 2.6f;
    glPushMatrix();
    glTranslatef(wheelX, 0.0f, 0.0f);

    // Quay theo goc lan cua banh xe quanh truc X
    glRotatef(wheelAngle * 57.29578f, 1.0f, 0.0f, 0.0f);

    // Xoay de banh xe nam theo mat phang YZ
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    if (wireframeMode) {
        glDisable(GL_LIGHTING);
        glColor3f(0.9f, 0.8f, 0.1f);
        glutWireTorus(0.7f, wheelRadius * 0.78f, 10, 20);
        glutWireCylinder(wheelRadius * 0.55f, 2.0f, 12, 2);
    } else {
        // A. LOP CAO SU DEN CO GAI (Treaded Rubber Tire)
        float tireRadius = wheelRadius;
        float tireWidth = 2.4f;
        float halfW = tireWidth * 0.5f;

        // Than lop chinh
        glColor3f(0.09f, 0.09f, 0.10f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -halfW);
        glutSolidCylinder(tireRadius, tireWidth, 24, 2);
        glPopMatrix();

        // 2 Mep bo tron cua lop cao su
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -halfW);
        glutSolidTorus(0.28f, tireRadius - 0.28f, 10, 24);
        glTranslatef(0.0f, 0.0f, tireWidth);
        glutSolidTorus(0.28f, tireRadius - 0.28f, 10, 24);
        glPopMatrix();

        // 20 GAI LOP CAO SU VAU MA SAT noi tren chu vi lop
        glColor3f(0.12f, 0.12f, 0.13f);
        for (int t = 0; t < 20; ++t) {
            float ang = t * (2.0f * 3.14159265f / 20.0f);
            glPushMatrix();
            glRotatef(ang * 57.29578f, 0.0f, 0.0f, 1.0f);
            glTranslatef(tireRadius, 0.0f, 0.0f);
            glScalef(0.18f, 0.42f, 2.1f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }

        // B. VANH BANH XE MAU VANG (Yellow ABS Plastic Rim)
        float rimRadius = tireRadius * 0.62f;
        glColor3f(0.96f, 0.78f, 0.06f);

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -halfW + 0.1f);
        glutSolidCylinder(rimRadius, tireWidth - 0.2f, 20, 2);
        glPopMatrix();

        // C. TAM BANH XE & 6 NAN HOA (6-Spoke Turbine Pattern)
        glColor3f(0.90f, 0.72f, 0.05f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, isLeftMotor ? (halfW - 0.4f) : (-halfW + 0.4f));
        glutSolidCylinder(0.65f, 0.6f, 12, 2);

        // Oc tan M3 ma bac o chinh giua tam banh xe
        glColor3f(0.85f, 0.88f, 0.92f);
        glTranslatef(0.0f, 0.0f, isLeftMotor ? 0.35f : -0.35f);
        glutSolidCylinder(0.32f, 0.25f, 6, 1);
        glPopMatrix();

        // 6 Nan hoa dang canh quat
        glColor3f(0.94f, 0.76f, 0.05f);
        for (int s = 0; s < 6; ++s) {
            float spkAng = s * (3.14159265f / 3.0f);
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, isLeftMotor ? (halfW - 0.35f) : (-halfW + 0.35f));
            glRotatef(spkAng * 57.29578f, 0.0f, 0.0f, 1.0f);
            glTranslatef(rimRadius * 0.5f, 0.0f, 0.0f);
            glScalef(rimRadius * 0.75f, 0.22f, 0.35f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    glPopMatrix();
    glPopMatrix();
}

// ===================== TRU NANG THANG DUNG & TAY GAP AGV CHUAN HOA =====================
ServoMG996RComp::ServoMG996RComp(const std::string& code, const std::string& name, ServoJointType type)
    : HardwareComponent(code, name, "Co Cau Nang Ha"), jointType(type) {
    cadSize = Vec3(5.0f, 3.5f, 3.0f);
    nominalVoltage = 5.0f;

    mesh = Mesh3D::createBox(cadSize.x, cadSize.y, cadSize.z, Vec3(0.18f, 0.18f, 0.2f));

    addPin("VCC", PinType::POWER_5V,  Vec3(-1.0f, 0.0f, -0.6f));
    addPin("GND", PinType::POWER_GND, Vec3( 0.0f, 0.0f, -0.6f));
    addPin("SIG", PinType::PWM_IO,    Vec3( 1.0f, 0.0f, -0.6f));

    if (jointType == ServoJointType::BASE_YAW) {
        armSegmentMesh = Mesh3D::createBox(7.6f, 0.6f, 2.6f, Vec3(0.20f, 0.22f, 0.25f));
    } else if (jointType == ServoJointType::LINEAR_LIFT) {
        armSegmentMesh = Mesh3D::createBox(7.2f, 2.6f, 1.4f, Vec3(0.72f, 0.74f, 0.78f));
    } else if (jointType == ServoJointType::GRIPPER) {
        armSegmentMesh = Mesh3D::createBox(4.6f, 1.8f, 1.6f, Vec3(0.96f, 0.46f, 0.10f));
    }
}

void ServoMG996RComp::setTargetAngle(float deg) {
    targetAngleDeg = std::clamp(deg, minAngleDeg, maxAngleDeg);
}

void ServoMG996RComp::updateMotion(float dt) {
    if (!isPowered) return;

    // Ná»™i suy mÆ°á»£t theo tá»‘c Ä‘á»™ cÆ¡ cáº¥u
    float diff = targetAngleDeg - currentAngleDeg;
    float maxStep = slewRate * dt;

    if (std::fabs(diff) <= maxStep) {
        currentAngleDeg = targetAngleDeg;
    } else {
        currentAngleDeg += (diff > 0.0f ? maxStep : -maxStep);
    }

    if (jointType == ServoJointType::BASE_YAW) {
        localRot = Vec3(0.0f, 0.0f, 0.0f);
        statusDesc = "Tru ray thang dung AGV (Tower Mast) | Chieu cao 14.5cm";
    } else if (jointType == ServoJointType::LINEAR_LIFT) {
        // BÃ n trÆ°á»£t nÃ¢ng háº¡ tháº³ng Ä‘á»©ng: di chuyá»ƒn tá»« Y = 1.5cm (sÃ¡t sÃ n) lÃªn Y = 11.5cm (trÃªn cao)
        currentLiftY = 1.5f + (currentAngleDeg / 180.0f) * 10.0f;
        localPos.x = 0.0f;
        localPos.y = currentLiftY;
        localPos.z = 0.0f;
        localRot = Vec3(0.0f, 0.0f, 0.0f); // Tuyá»‡t Ä‘á»‘i tá»‹nh tiáº¿n tháº³ng Ä‘á»©ng

        float hCm = currentLiftY - 1.5f;
        std::ostringstream ss;
        ss << "Nang ha thang dung: " << std::fixed << std::setprecision(1) << hCm << " cm / 10.0 cm";
        statusDesc = ss.str();
    } else if (jointType == ServoJointType::GRIPPER) {
        localRot = Vec3(0.0f, 0.0f, 0.0f);
        float gapCm = 2.7f + (currentAngleDeg / 90.0f) * 3.3f;
        std::ostringstream ss;
        ss << (currentAngleDeg < 25.0f ? "DANG KEP CHAT" : "DANG MO RONG")
           << " | Do mo ngam: " << std::fixed << std::setprecision(1) << gapCm << " cm";
        statusDesc = ss.str();
    }
}

void ServoMG996RComp::onElectricalUpdate() {
    Pin* vcc = getPin("VCC");
    Pin* sig = getPin("SIG");

    isPowered = (vcc && vcc->voltage >= 4.5f);

    if (isPowered && sig) {
        targetAngleDeg = sig->pwmDuty * 180.0f;
    }
    updateMotion(0.016f);
}

void ServoMG996RComp::render(bool wireframeMode) {
    if (!visible) return;

    glPushMatrix();
    Mat4 localM = getLocalMatrix();
    glMultMatrixf(localM.data());

    if (jointType == ServoJointType::BASE_YAW) {
        // =========================================================================
        // 1. KHUNG TRá»¤ RAY THáº²NG Äá»¨NG (DUAL VERTICAL RAILS & LEAD SCREW AGV MAST)
        // =========================================================================
        Vec3 metalDark(0.20f, 0.22f, 0.25f);
        Vec3 chromeRail(0.88f, 0.90f, 0.94f);
        Vec3 brassScrew(0.85f, 0.75f, 0.32f);
        Vec3 motorBlack(0.12f, 0.12f, 0.14f);
        Vec3 boltSilver(0.75f, 0.75f, 0.78f);

        // a) Bá»‡ chÃ¢n gáº¯n sÃ n Táº§ng 1 (Y = 0.0cm, Z = 0.0cm)
        drawSolidBox(0.0f, 0.0f, 0.0f, 7.6f, 0.6f, 2.6f, metalDark, wireframeMode);
        // 4 Bulong báº¯t chÃ¢n Ä‘áº¿
        for (float bx : {-3.1f, 3.1f}) {
            for (float bz : {-0.8f, 0.8f}) {
                drawSolidCylinderY(bx, 0.35f, bz, 0.18f, 0.2f, boltSilver, wireframeMode, 8);
            }
        }

        // b) Khung giáº±ng liÃªn káº¿t vá»›i mÅ©i sÃ n Táº§ng 2 (Y = 6.0cm, Z = -0.7cm)
        drawSolidBox(0.0f, 6.0f, -0.7f, 7.6f, 0.6f, 1.6f, metalDark, wireframeMode);
        drawSolidCylinderY(-3.1f, 6.35f, -0.7f, 0.18f, 0.2f, boltSilver, wireframeMode, 8);
        drawSolidCylinderY( 3.1f, 6.35f, -0.7f, 0.18f, 0.2f, boltSilver, wireframeMode, 8);

        // c) Hai thanh ray trá»¥ trÃ²n tháº³ng Ä‘á»©ng máº¡ crom (Left & Right Chrome Rails)
        // Ray tráº£i dÃ i tá»« Y = 0.0cm Ä‘áº¿n Y = 14.0cm (chiá»u dÃ i 14.0cm, tÃ¢m táº¡i Y = 7.0cm)
        drawSolidCylinderY(-2.6f, 7.0f, 0.0f, 0.30f, 14.0f, chromeRail, wireframeMode, 16);
        drawSolidCylinderY( 2.6f, 7.0f, 0.0f, 0.30f, 14.0f, chromeRail, wireframeMode, 16);

        // d) Trá»¥c vÃ­t me bi / thanh rÄƒng ren á»Ÿ trung tÃ¢m (Center Lead Screw)
        // VÃ­t me cháº¡y tá»« Y = 0.4cm Ä‘áº¿n Y = 13.8cm (chiá»u dÃ i 13.4cm, tÃ¢m táº¡i Y = 7.1cm)
        drawSolidCylinderY(0.0f, 7.1f, 0.0f, 0.22f, 13.4f, brassScrew, wireframeMode, 16);

        // e) Náº¯p náº¹p giá»¯ Ä‘á»‰nh 2 thanh ray (Top Cap Plate táº¡i Y = 14.0cm)
        drawSolidBox(0.0f, 14.0f, 0.0f, 7.6f, 0.6f, 2.6f, metalDark, wireframeMode);

        // Khá»‘i báº¡c Ä‘á»¡ trá»¥c vÃ­t me Ä‘á»‰nh
        drawSolidCylinderY(0.0f, 14.3f, 0.0f, 0.45f, 0.6f, boltSilver, wireframeMode, 12);

        // f) Cá»¥m Ä‘á»™ng cÆ¡ bÆ°á»›c kÃ©o tá»i NEMA / Servo cÃ´ng suáº¥t lá»›n á»Ÿ Ä‘á»‰nh (Top Motor)
        // ThÃ¢n Ä‘á»™ng cÆ¡ kÃ­ch thÆ°á»›c 4.0 x 2.4 x 4.0 cm
        drawSolidBox(0.0f, 15.6f, 0.0f, 4.0f, 2.4f, 4.0f, motorBlack, wireframeMode);
        // Náº¯p Ä‘áº§u Ä‘á»™ng cÆ¡ máº¡ báº¡c
        drawSolidBox(0.0f, 16.85f, 0.0f, 4.0f, 0.15f, 4.0f, boltSilver, wireframeMode);
        // Giáº¯c cáº¯m dÃ¢y Ä‘iá»u khiá»ƒn Ä‘á»™ng cÆ¡ bÃªn hÃ´ng
        drawSolidBox(2.05f, 15.6f, 0.0f, 0.3f, 1.0f, 1.4f, Vec3(0.2f, 0.5f, 0.9f), wireframeMode);

    } else if (jointType == ServoJointType::LINEAR_LIFT) {
        // =========================================================================
        // 2. BÃ€N TRÆ¯á»¢T NÃ‚NG Háº  THáº²NG Äá»¨NG (VERTICAL ELEVATOR CARRIAGE)
        // =========================================================================
        Vec3 carriageAlu(0.72f, 0.74f, 0.78f);
        Vec3 bearingSteel(0.82f, 0.84f, 0.88f);
        Vec3 brassNut(0.85f, 0.75f, 0.30f);

        // a) Táº¥m bÃ n trÆ°á»£t nhÃ´m CNC (Main Carriage Slider Plate)
        // Äáº·t á»Ÿ phÃ­a trÆ°á»›c trá»¥c ray (Z = 0.7cm)
        drawSolidBox(0.0f, 0.0f, 0.7f, 7.2f, 2.6f, 0.8f, carriageAlu, wireframeMode);

        // b) Hai khá»‘i báº¡c Ä‘áº¡n trÆ°á»£t SC8UU Ã´m khÃ­t 2 thanh ray Ä‘á»©ng (Dual Linear Bearings)
        // Báº¡c Ä‘áº¡n tháº³ng Ä‘á»©ng dá»c trá»¥c Y Ã´m láº¥y ray táº¡i X = -2.6cm vÃ  X = +2.6cm, Z = 0.0cm
        drawSolidCylinderY(-2.6f, 0.0f, 0.0f, 0.50f, 2.6f, bearingSteel, wireframeMode, 16);
        drawSolidCylinderY( 2.6f, 0.0f, 0.0f, 0.50f, 2.6f, bearingSteel, wireframeMode, 16);

        // Khá»‘i vá» nhÃ´m Ä‘á»‹nh hÃ¬nh bá»c ngoÃ i 2 báº¡c Ä‘áº¡n
        drawSolidBox(-2.6f, 0.0f, 0.2f, 1.4f, 2.6f, 1.2f, carriageAlu, wireframeMode);
        drawSolidBox( 2.6f, 0.0f, 0.2f, 1.4f, 2.6f, 1.2f, carriageAlu, wireframeMode);

        // c) á»” Ä‘ai á»‘c Ä‘á»“ng ren T8 Äƒn khá»›p trá»¥c vÃ­t me trung tÃ¢m (Center Brass Lead Nut)
        drawSolidCylinderY(0.0f, 0.0f, 0.0f, 0.44f, 2.0f, brassNut, wireframeMode, 14);

        // d) Tai gÃ¡ liÃªn káº¿t cá»¥m tay gáº¯p phÃ­a trÆ°á»›c (Gripper Mount Interface)
        drawSolidBox(0.0f, 0.0f, 1.2f, 4.6f, 2.2f, 0.4f, Vec3(0.30f, 0.32f, 0.36f), wireframeMode);

    } else if (jointType == ServoJointType::GRIPPER) {
        // =========================================================================
        // 3. Cá»¤M NGÃ€M Káº¸P ÄÃ”I SONG SONG (PARALLEL CLAMPING GRIPPER)
        // =========================================================================
        Vec3 gripBody(0.18f, 0.20f, 0.24f);
        Vec3 chromeRod(0.88f, 0.90f, 0.94f);
        Vec3 jawOrange(0.96f, 0.46f, 0.10f);
        Vec3 rubberBlack(0.12f, 0.12f, 0.12f);
        Vec3 hornWhite(0.92f, 0.92f, 0.92f);

        // a) Vá» há»™p cÆ¡ cáº¥u servo kÃ©o ngÃ m (Gripper Actuator Housing)
        drawSolidBox(0.0f, 0.0f, 0.6f, 4.6f, 1.8f, 1.4f, gripBody, wireframeMode);
        // ÄÄ©a trÃ²n xoay servo trÃªn nÃ³c
        drawSolidCylinderY(0.0f, 0.95f, 0.6f, 0.65f, 0.15f, hornWhite, wireframeMode, 14);

        // b) Hai trá»¥c trÆ°á»£t ngang dáº«n hÆ°á»›ng 2 cÃ ng káº¹p (Dual Horizontal Slide Rods)
        drawSolidCylinderX(0.0f,  0.45f, 1.4f, 0.16f, 7.2f, chromeRod, wireframeMode, 12);
        drawSolidCylinderX(0.0f, -0.45f, 1.4f, 0.16f, 7.2f, chromeRod, wireframeMode, 12);

        // c) Äá»™ má»Ÿ cá»§a 2 cÃ ng:
        // GÃ³c < 25 Ä‘á»™ -> Káº¹p cháº·t (khoáº£ng cÃ¡ch máº·t cao su = 2.8cm Ã´m cháº·t khá»‘i 3.2cm)
        // GÃ³c 90 Ä‘á»™  -> Má»Ÿ rá»™ng (khoáº£ng cÃ¡ch máº·t cao su = 6.0cm thoáº£i mÃ¡i lá»t váº­t)
        float openGap = 1.35f + (currentAngleDeg / 90.0f) * 1.65f;

        // ===== CÃ€NG Káº¸P TRÃI (LEFT JAW) =====
        // Khá»‘i báº¡c trÆ°á»£t ngang bÃªn trÃ¡i
        drawSolidBox(-openGap, 0.0f, 1.4f, 0.6f, 1.8f, 0.8f, gripBody, wireframeMode);
        // Báº£n cÃ ng káº¹p vÆ°Æ¡n dÃ i vá» phÃ­a trÆ°á»›c (+Z)
        drawSolidBox(-openGap - 0.25f, 0.0f, 3.4f, 0.5f, 2.2f, 4.0f, jawOrange, wireframeMode);
        // Äáº§u vÃ¡t nhá»n dáº«n hÆ°á»›ng á»Ÿ mÅ©i cÃ ng
        glPushMatrix();
        glTranslatef(-openGap - 0.25f, 0.0f, 5.5f);
        glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
        drawSolidBox(0.0f, 0.0f, 0.0f, 0.45f, 2.0f, 0.8f, jawOrange, wireframeMode);
        glPopMatrix();
        // Miáº¿ng Ä‘á»‡m cao su Ä‘en cÃ³ vÃ¢n bÃ¡m á»Ÿ máº·t trong cÃ ng trÃ¡i
        drawSolidBox(-openGap + 0.05f, 0.0f, 3.4f, 0.15f, 1.8f, 3.2f, rubberBlack, wireframeMode);

        // ===== CÃ€NG Káº¸P PHáº¢I (RIGHT JAW) =====
        // Khá»‘i báº¡c trÆ°á»£t ngang bÃªn pháº£i
        drawSolidBox(openGap, 0.0f, 1.4f, 0.6f, 1.8f, 0.8f, gripBody, wireframeMode);
        // Báº£n cÃ ng káº¹p vÆ°Æ¡n dÃ i vá» phÃ­a trÆ°á»›c (+Z)
        drawSolidBox(openGap + 0.25f, 0.0f, 3.4f, 0.5f, 2.2f, 4.0f, jawOrange, wireframeMode);
        // Äáº§u vÃ¡t nhá»n dáº«n hÆ°á»›ng á»Ÿ mÅ©i cÃ ng
        glPushMatrix();
        glTranslatef(openGap + 0.25f, 0.0f, 5.5f);
        glRotatef(-20.0f, 0.0f, 1.0f, 0.0f);
        drawSolidBox(0.0f, 0.0f, 0.0f, 0.45f, 2.0f, 0.8f, jawOrange, wireframeMode);
        glPopMatrix();
        // Miáº¿ng Ä‘á»‡m cao su Ä‘en cÃ³ vÃ¢n bÃ¡m á»Ÿ máº·t trong cÃ ng pháº£i
        drawSolidBox(openGap - 0.05f, 0.0f, 3.4f, 0.15f, 1.8f, 3.2f, rubberBlack, wireframeMode);
    }

    // 2. Äá»‡ quy váº½ cÃ¡c nÃºt con ná»‘i tiáº¿p
    for (auto* child : children) {
        child->render(wireframeMode);
    }

    glPopMatrix();
}
