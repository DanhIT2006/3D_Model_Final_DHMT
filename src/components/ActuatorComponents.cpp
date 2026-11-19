#include "components/ActuatorComponents.h"
#include "scene/RoboticArmRenderer.h"
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

// =========================================================================
// [SLIDE 6 CODE BLOCK]: GIẢI PHẪU BO MẠCH CẦU H L298N (8 KHỐI PHẦN TỬ CON)
// Mô hình hóa chi tiết: Tản nhiệt nhôm 7 lá, Cặp tụ hóa 220uF/35V tách rời,
// 8 Diode bảo vệ dòng ngược Back-EMF (D1-D8), Chốt Jumper 5V Enable rút/cắm được
// =========================================================================
void L298NDriverComp::initSubComponents() {
    subComponents.clear();

    // 1. IC Cau H L298N & Tan nhiet nhom 7 la xe ranh
    SubComponent heatsink;
    heatsink.id = "L298N_IC_HS";
    heatsink.name = "IC Cau H L298N & Tan Nhiet Nhom";
    heatsink.category = "Cong suat & Dao chieu";
    heatsink.rating = "Multiwatt15 / 46V max / 2A moi kenh / Nhom Anodized 7 la";
    heatsink.roleDesc = "Chua 2 mach cau H cong suat (Dual Full-Bridge) dieu khien toc do PWM va dao chieu 2 dong co DC. Tan nhiet nhom 7 la giup giai phong nhiet luong BJT khi dong co tai nang.";
    heatsink.liveStatusText = "Dien ap dong luc: 7.4V | Nhiet do: 42°C (Binh thuong)";
    heatsink.localPos = Vec3(0.0f, 0.12f + 1.05f, -0.95f);
    heatsink.size = Vec3(2.2f, 2.1f, 1.1f);
    heatsink.color = Vec3(0.16f, 0.17f, 0.19f);
    heatsink.shape = SubShapeType::HEATSINK_VERTICAL;
    heatsink.mesh = Mesh3D::createBox(2.2f, 2.1f, 1.1f, heatsink.color);
    addSubComponent(heatsink);

    // 2. Tu hoa loc nguon dong luc VCC 220uF/35V
    SubComponent capVcc;
    capVcc.id = "L298N_CAP_HV";
    capVcc.name = "Tu Hoa Loc Nguon Dong Luc 220uF/35V";
    capVcc.category = "Loc nguon dau vao";
    capVcc.rating = "220 uF - 35V / 105°C / Nap nhom co ranh chong no";
    capVcc.roleDesc = "Nam tren duong nguon VCC (7.4V), bu dong tuc thoi khi dong co DC khoi dong hoac dao chieu, ngan ngua sut ap he thong.";
    capVcc.liveStatusText = "Dien ap nap: 7.4V DC | San phang tot";
    capVcc.localPos = Vec3(-0.52f, 0.12f + 0.525f, -0.05f);
    capVcc.size = Vec3(0.76f, 1.05f, 0.76f);
    capVcc.color = Vec3(0.12f, 0.14f, 0.18f);
    capVcc.shape = SubShapeType::CYLINDER_CAP;
    capVcc.mesh = Mesh3D::createCylinder(0.38f, 1.05f, 16, capVcc.color);
    addSubComponent(capVcc);

    // 3. Tu hoa loc nguon logic 5V 220uF/35V
    SubComponent cap5v;
    cap5v.id = "L298N_CAP_LV";
    cap5v.name = "Tu Hoa Loc Nguon Logic 220uF/35V";
    cap5v.category = "Loc nguon 5V";
    cap5v.rating = "220 uF - 35V / Loc song hai logic";
    cap5v.roleDesc = "Loc phang nhieu cao tan o dau ra IC 78M05, bao ve cac cong logic ben trong IC L298N.";
    cap5v.liveStatusText = "Dien ap logic: 5.0V DC | On dinh";
    cap5v.localPos = Vec3(+0.52f, 0.12f + 0.525f, +0.65f);
    cap5v.size = Vec3(0.76f, 1.05f, 0.76f);
    cap5v.color = Vec3(0.12f, 0.14f, 0.18f);
    cap5v.shape = SubShapeType::CYLINDER_CAP;
    cap5v.mesh = Mesh3D::createCylinder(0.38f, 1.05f, 16, cap5v.color);
    addSubComponent(cap5v);

    // 4. IC On ap tuyen tinh 78M05
    SubComponent reg7805;
    reg7805.id = "L298N_78M05";
    reg7805.name = "IC On Ap Tuyen Tinh 78M05 (5V Logic)";
    reg7805.category = "On ap nguon logic";
    reg7805.rating = "Vin 7-35V / Vout 5.0V / Iout 500mA / TO-252";
    reg7805.roleDesc = "Ha ap truc tiep tu pin 7.4V xuong 5.0V on dinh de nuoi mach logic dieu khien noi bo L298N.";
    reg7805.liveStatusText = "V_In: 7.4V -> V_Out: 5.0V | Hieu suat tot";
    reg7805.localPos = Vec3(+0.52f, 0.12f + 0.11f, -0.15f);
    reg7805.size = Vec3(0.65f, 0.22f, 0.55f);
    reg7805.color = Vec3(0.18f, 0.18f, 0.20f);
    reg7805.shape = SubShapeType::BOX_SMD;
    reg7805.mesh = Mesh3D::createBox(0.65f, 0.22f, 0.55f, reg7805.color);
    addSubComponent(reg7805);

    // 5. Jumper kich hoat on ap 5V Enable
    SubComponent jmp5v;
    jmp5v.id = "L298N_JMP_5V";
    jmp5v.name = "Jumper Kich Hoat On Ap 5V (5V Enable)";
    jmp5v.category = "Cau hinh nguon logic";
    jmp5v.rating = "Header 2-pin 2.54mm kem cap shunt vang/den";
    jmp5v.roleDesc = "KHI CAM: IC 78M05 tu tao 5V logic. KHI RUT: ngat 78M05, mat 5V logic noi bo, xe dung ngay!";
    jmp5v.liveStatusText = "Trang thai: DANG CAM (Logic 5V BAT - Xe san sang chay)";
    jmp5v.localPos = Vec3(-1.05f, 0.12f + 0.275f, +0.65f);
    jmp5v.size = Vec3(0.30f, 0.55f, 0.25f);
    jmp5v.color = Vec3(0.95f, 0.85f, 0.1f);
    jmp5v.shape = SubShapeType::JUMPER_CAP;
    jmp5v.isInteractive = true;
    jmp5v.isActiveState = true;
    jmp5v.state = true;
    jmp5v.actionName = "RUT / CAM JUMPER 5V";
    jmp5v.mesh = Mesh3D::createBox(0.30f, 0.55f, 0.25f, jmp5v.color);
    addSubComponent(jmp5v);

    // 6. Cum 8 Diode xung bao ve Flyback D1-D8
    SubComponent dio;
    dio.id = "L298N_DIODES";
    dio.name = "Cum 8 Diode Xung Bao Ve (Flyback D1 - D8)";
    dio.category = "Bao ve chong dong nguoc";
    dio.rating = "8x Diode M7 SMD / 4 con moi ben suon tan nhiet";
    dio.roleDesc = "Xep 2 ben suon tan nhiet. Triet tieu suc dien dong cam ung nguoc (Back-EMF) sinh ra khi ngat xung PWM hoac dao chieu, bao ve L298N.";
    dio.liveStatusText = "8x Diode xung 2 ben suon | San sang dap Back-EMF";
    dio.localPos = Vec3(0.0f, 0.12f + 0.125f, -0.8f);
    dio.size = Vec3(3.60f, 0.25f, 1.50f);
    dio.color = Vec3(0.12f, 0.12f, 0.14f);
    dio.shape = SubShapeType::BOX_SMD;
    dio.mesh = Mesh3D::createBox(3.60f, 0.25f, 1.50f, dio.color);
    addSubComponent(dio);

    // 7. Domino nguon va dong co
    SubComponent term;
    term.id = "L298N_PWR_TERM";
    term.name = "3 Tram Domino Xanh (VCC/GND/5V & OUT1-4)";
    term.category = "Dau noi day nguon & dong luc";
    term.rating = "3x Tram van vit 5.08mm mau xanh duong";
    term.roleDesc = "Tram nguon 3 vit (VCC, GND, 5V) va 2 tram 2 vit OUT1-2, OUT3-4 cap dong cho 4 dong co TT Motor.";
    term.liveStatusText = "Ket noi: VCC 7.4V, OUT1..OUT4 noi 4 TT Motor";
    term.localPos = Vec3(-0.65f, 0.12f + 0.525f, +1.55f);
    term.size = Vec3(1.50f, 1.05f, 0.85f);
    term.color = Vec3(0.12f, 0.38f, 0.85f);
    term.shape = SubShapeType::BOX_SMD;
    term.mesh = Mesh3D::createBox(1.50f, 1.05f, 0.85f, term.color);
    addSubComponent(term);

    // 8. Hang chan tin hieu Logic
    SubComponent hdrLogic;
    hdrLogic.id = "L298N_CTRL_HDR";
    hdrLogic.name = "Hang Chan Dieu Khien (ENA, IN1-IN4, ENB)";
    hdrLogic.category = "Giao tiep vi dieu khien";
    hdrLogic.rating = "6 chân kim loại vàng đồng cách đều 2.54mm + 2 Jumper cap";
    hdrLogic.roleDesc = "Nhan tin hieu tu Arduino: ENA/ENB nhan PWM; IN1-IN4 nhan logic chieu quay. Hai dau ENA, ENB co 2 jumper chup.";
    hdrLogic.liveStatusText = "PWM D5/D6, Logic D4/D7/D8/D12 tu Arduino";
    hdrLogic.localPos = Vec3(+0.95f, 0.12f + 0.325f, +1.60f);
    hdrLogic.size = Vec3(1.55f, 0.65f, 0.25f);
    hdrLogic.color = Vec3(0.85f, 0.75f, 0.2f);
    hdrLogic.shape = SubShapeType::BOX_SMD;
    hdrLogic.mesh = Mesh3D::createBox(1.55f, 0.65f, 0.25f, hdrLogic.color);
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

        float dutyL = ena ? ena->pwmDuty : 0.0f;
        float dutyR = enb ? enb->pwmDuty : 0.0f;
        if (ena && ena->voltage > 2.0f && dutyL < 0.01f) dutyL = ena->voltage / 5.0f;
        if (enb && enb->voltage > 2.0f && dutyR < 0.01f) dutyR = enb->voltage / 5.0f;
        if (dirL != 0.0f && dutyL < 0.05f) dutyL = 0.8f;
        if (dirR != 0.0f && dutyR < 0.05f) dutyR = 0.8f;

        Pin* out1 = getPin("OUT1");
        Pin* out2 = getPin("OUT2");
        Pin* out3 = getPin("OUT3");
        Pin* out4 = getPin("OUT4");

        if (out1 && out2) {
            out1->voltage = (dirL > 0) ? (v12 * dutyL) : 0.0f;
            out2->voltage = (dirL < 0) ? (v12 * dutyL) : 0.0f;
        }
        if (out3 && out4) {
            out3->voltage = (dirR > 0) ? (v12 * dutyR) : 0.0f;
            out4->voltage = (dirR < 0) ? (v12 * dutyR) : 0.0f;
        }

        outVoltL = dirL * (v12 * dutyL);
        outVoltR = dirR * (v12 * dutyR);

        currentDrawMa = 35.0f + (dutyL + dutyR) * 250.0f;
        statusDesc = "DANG DIEU KHIEN DONG CO 4WD (7.4V PWM)";
    } else {
        outVoltL = 0.0f;
        outVoltR = 0.0f;
        Pin* out1 = getPin("OUT1");
        Pin* out2 = getPin("OUT2");
        Pin* out3 = getPin("OUT3");
        Pin* out4 = getPin("OUT4");
        if (out1) out1->voltage = 0.0f;
        if (out2) out2->voltage = 0.0f;
        if (out3) out3->voltage = 0.0f;
        if (out4) out4->voltage = 0.0f;

        currentDrawMa = 0.0f;
        statusDesc = jumper5VEnabled ? "MAT NGUON CAP DONG LUC 12V (THIEU AP)"
                                     : "MAT NGUON LOGIC 5V (JUMPER 5V DA RUT) - DONG CO DUNG";
    }
}

void L298NDriverComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // 0. 4 Lo khoan bat oc mau bac o 4 goc PCB (±1.85, 0.11, ±1.85)
    glColor3f(0.82f, 0.85f, 0.88f);
    for (float hx : {-1.85f, 1.85f}) {
        for (float hz : {-1.85f, 1.85f}) {
            glPushMatrix();
            glTranslatef(hx, 0.11f, hz);
            glRotatef(-90.0f, 1, 0, 0);
            glutSolidCylinder(0.16f, 0.02f, 12, 1);
            glPopMatrix();
        }
    }

    // 1. Khoi tan nhiet nhom mau den co 7 canh xe ranh doc (Finned Heatsink)
    // Tọa độ chuẩn: (0.0f, 0.12f, -0.95f), kích thước 2.2 x 2.1 x 1.1 cm
    glPushMatrix();
    glTranslatef(0.0f, 0.12f + 1.05f, -0.95f);
    glColor3f(0.16f, 0.17f, 0.19f);
    // Tam lung phang truoc
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.35f);
    glScalef(2.2f, 2.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 7 La tan nhiet doc phia sau
    for (float fx = -0.95f; fx <= 0.98f; fx += 0.32f) {
        glPushMatrix();
        glTranslatef(fx, 0.0f, -0.15f);
        glScalef(0.09f, 2.1f, 0.85f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 2. Chip IC L298N Multiwatt-15 gan ap vao mat truoc tan nhiet kem oc vit & 15 chan
    glPushMatrix();
    glTranslatef(0.0f, 0.12f + 0.65f, -0.65f);
    glColor3f(0.08f, 0.08f, 0.09f);
    glPushMatrix();
    glScalef(2.0f, 1.25f, 0.28f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Oc vit ma bac o giua chip
    glColor3f(0.85f, 0.88f, 0.92f);
    glPushMatrix();
    glTranslatef(0.0f, 0.32f, 0.16f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCylinder(0.16f, 0.08f, 10, 1);
    glPopMatrix();
    // 15 Chan kim loai ma bac uon cong xuong PCB
    for (int p = 0; p < 15; ++p) {
        float px = -0.85f + p * 0.12f;
        float pz = (p % 2 == 0) ? 0.05f : 0.16f;
        glPushMatrix();
        glTranslatef(px, -0.45f, pz);
        glScalef(0.04f, 0.55f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 3. 2 Tu hoa loc nguon 220uF/35V tach roi ro rang, chong Z-fighting tuyet doi
    // Tụ 1 (HV): (-0.52f, 0.12f, -0.05f), Tụ 2 (LV): (+0.52f, 0.12f, +0.65f)
    struct CapDef { float x, z; };
    CapDef caps[2] = { {-0.52f, -0.05f}, {+0.52f, +0.65f} };
    for (int i = 0; i < 2; ++i) {
        float cx = caps[i].x;
        float cz = caps[i].z;
        glPushMatrix();
        glTranslatef(cx, 0.12f, cz);
        glRotatef(-90.0f, 1, 0, 0);

        // Than tru mau den/xanh den duy nhat (1 lop)
        glColor3f(0.12f, 0.14f, 0.18f);
        glutSolidCylinder(0.38f, 0.96f, 16, 2);

        // Nap nhom tron mau bac o tren dinh
        glTranslatef(0.0f, 0.0f, 0.965f);
        glColor3f(0.85f, 0.88f, 0.90f);
        glutSolidCylinder(0.375f, 0.085f, 16, 1);

        // Ran chu thap chong no tren dinh nap nhom
        glDisable(GL_LIGHTING);
        glColor3f(0.45f, 0.48f, 0.52f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex3f(-0.24f, 0.0f, 0.088f); glVertex3f(0.24f, 0.0f, 0.088f);
        glVertex3f(0.0f, -0.24f, 0.088f); glVertex3f(0.0f, 0.24f, 0.088f);
        glEnd();
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }

    // 4. IC On ap tuyen tinh 78M05 tai (+0.52f, 0.12f, -0.15f)
    glPushMatrix();
    glTranslatef(0.52f, 0.12f + 0.11f, -0.15f);
    glColor3f(0.18f, 0.18f, 0.20f);
    glPushMatrix();
    glScalef(0.65f, 0.22f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // La tan nhiet lung TO-252 mau bac
    glColor3f(0.85f, 0.88f, 0.92f);
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, -0.28f);
    glScalef(0.55f, 0.06f, 0.16f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 3 Chan kim loai ra phia truoc
    for (float px : {-0.20f, 0.0f, 0.20f}) {
        glPushMatrix();
        glTranslatef(px, -0.06f, 0.32f);
        glScalef(0.08f, 0.05f, 0.18f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // 5. Cum 8 Diode M7 xung bao ve Flyback D1-D8 dat thanh 2 cot o 2 BEN SUON TAN NHIET
    // 4 diode trai x = -1.55f, 4 diode phai x = +1.55f, z = {-1.4f, -1.0f, -0.6f, -0.2f}
    float diodeZ[4] = {-1.35f, -0.98f, -0.62f, -0.25f};
    for (float dx : {-1.55f, 1.55f}) {
        for (float dz : diodeZ) {
            glPushMatrix();
            glTranslatef(dx, 0.12f + 0.10f, dz);
            // Than diode mau den
            glColor3f(0.12f, 0.12f, 0.14f);
            glPushMatrix();
            glScalef(0.42f, 0.20f, 0.25f);
            glutSolidCube(1.0f);
            glPopMatrix();
            // Vach bac cuc Cathode
            glColor3f(0.85f, 0.88f, 0.92f);
            glPushMatrix();
            glTranslatef(0.0f, 0.01f, -0.08f);
            glScalef(0.44f, 0.21f, 0.05f);
            glutSolidCube(1.0f);
            glPopMatrix();
            // 2 Chan han kim loai o 2 dau
            for (float pz : {-0.16f, 0.16f}) {
                glPushMatrix();
                glTranslatef(0.0f, -0.05f, pz);
                glScalef(0.24f, 0.05f, 0.10f);
                glutSolidCube(1.0f);
                glPopMatrix();
            }
            glPopMatrix();
        }
    }

    // 6. 3 Tram Domino xanh duong bat oc
    // OUT1-OUT2 (Trai): (-1.65, 0.12, +0.75)
    glPushMatrix();
    glTranslatef(-1.65f, 0.12f + 0.525f, +0.75f);
    glColor3f(0.12f, 0.38f, 0.85f);
    glPushMatrix();
    glScalef(0.85f, 1.05f, 1.00f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 2 Oc vit tren dau
    glColor3f(0.82f, 0.85f, 0.88f);
    for (float oz : {-0.25f, 0.25f}) {
        glPushMatrix();
        glTranslatef(0.0f, 0.53f, oz);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.18f, 0.04f, 10, 1);
        glPopMatrix();
    }
    glPopMatrix();

    // OUT3-OUT4 (Phai): (+1.65, 0.12, +0.75)
    glPushMatrix();
    glTranslatef(1.65f, 0.12f + 0.525f, +0.75f);
    glColor3f(0.12f, 0.38f, 0.85f);
    glPushMatrix();
    glScalef(0.85f, 1.05f, 1.00f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 2 Oc vit tren dau
    glColor3f(0.82f, 0.85f, 0.88f);
    for (float oz : {-0.25f, 0.25f}) {
        glPushMatrix();
        glTranslatef(0.0f, 0.53f, oz);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.18f, 0.04f, 10, 1);
        glPopMatrix();
    }
    glPopMatrix();

    // Domino Nguon VCC/GND/5V (Truoc-Trai): (-0.65, 0.12, +1.55)
    glPushMatrix();
    glTranslatef(-0.65f, 0.12f + 0.525f, +1.55f);
    glColor3f(0.12f, 0.38f, 0.85f);
    glPushMatrix();
    glScalef(1.50f, 1.05f, 0.85f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 3 Oc vit tren dau
    glColor3f(0.82f, 0.85f, 0.88f);
    for (float ox : {-0.45f, 0.0f, 0.45f}) {
        glPushMatrix();
        glTranslatef(ox, 0.53f, 0.0f);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.18f, 0.04f, 10, 1);
        glPopMatrix();
    }
    glPopMatrix();

    // 7. Jumper 5V Enable tai (-1.05f, 0.12f, +0.65f)
    glPushMatrix();
    glTranslatef(-1.05f, 0.12f, +0.65f);
    // 2 Chan kim loai vang dung dung
    glColor3f(0.92f, 0.78f, 0.25f);
    for (float jz : {-0.07f, 0.07f}) {
        glPushMatrix();
        glTranslatef(0.0f, 0.20f, jz);
        glScalef(0.06f, 0.40f, 0.06f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    // Shunt Jumper cap: neu dang cam thi nam o y = 0.24f; neu rut ra thi nhac cao len y = 0.65f
    float shuntY = jumper5VEnabled ? 0.24f : 0.65f;
    Vec3 shuntCol = jumper5VEnabled ? Vec3(0.95f, 0.85f, 0.1f) : Vec3(0.4f, 0.4f, 0.4f);
    glColor3f(shuntCol.x, shuntCol.y, shuntCol.z);
    glPushMatrix();
    glTranslatef(0.0f, shuntY, 0.0f);
    glScalef(0.30f, 0.32f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // 8. Hang chan tin hieu Logic ENA, IN1..4, ENB tai (+0.95f, 0.12f, +1.60f)
    // De nhua den + 6 chan kim loai vang dong rieng biet + 2 jumper ENA/ENB
    glPushMatrix();
    glTranslatef(0.95f, 0.12f, 1.60f);
    // De nhua den
    glColor3f(0.12f, 0.12f, 0.14f);
    glPushMatrix();
    glTranslatef(0.0f, 0.11f, 0.0f);
    glScalef(1.55f, 0.22f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // 6 Chan kim loai vang vuong 0.07 x 0.65 x 0.07 cm
    glColor3f(0.95f, 0.82f, 0.22f);
    for (int pin = 0; pin < 6; ++pin) {
        float px = -0.635f + pin * 0.254f;
        glPushMatrix();
        glTranslatef(px, 0.45f, 0.0f);
        glScalef(0.07f, 0.65f, 0.07f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Jumper chup o ENA (pin 0) va ENB (pin 5)
        if (pin == 0 || pin == 5) {
            glColor3f(0.18f, 0.18f, 0.20f);
            glPushMatrix();
            glTranslatef(px, 0.42f, 0.0f);
            glScalef(0.18f, 0.38f, 0.20f);
            glutSolidCube(1.0f);
            glPopMatrix();
            glColor3f(0.95f, 0.82f, 0.22f);
        }
    }
    glPopMatrix();

    // 9. LED bao nguon Logic tai (+0.05f, 0.12f, +0.45f)
    glPushMatrix();
    glTranslatef(0.05f, 0.12f + 0.08f, 0.45f);
    glColor3f(isPowered ? 1.0f : 0.3f, 0.15f, 0.15f);
    glScalef(0.22f, 0.14f, 0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// ==================== DONG CO DC & BANH XE =====================
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
    float omega = rps * 2.0f * 3.14159265f;

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

    initSubComponents();
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

// =========================================================================
// [SLIDE 7 CODE BLOCK]: RÀNG BUỘC ĐIỆN ÁP HOẠT ĐỘNG SERVO MG996R (VCC >= 4.8V)
// Servo MG996R yêu cầu dải nguồn 4.8V - 7.2V (dòng đỉnh 2.5A khi nâng tải).
// Được cấp nguồn riêng từ mạch hạ áp BUCK LM2596 (5.00V / 3A). Khi BUCK bị giảm áp
// dưới 4.8V hoặc tắt công tắc BAT_SW (7.4V), isPowered = false và cánh tay lập tức bị khóa cứng.
// =========================================================================
void ServoMG996RComp::onElectricalUpdate() {
    Pin* vcc = getPin("VCC");
    Pin* sig = getPin("SIG");

    isPowered = (vcc && vcc->voltage >= 4.8f);

    if (isPowered && sig) {
        targetAngleDeg = sig->pwmDuty * 180.0f;
    }
    updateMotion(0.016f);
}

void ServoMG996RComp::initSubComponents() {
    subComponents.clear();

    SubComponent motor;
    motor.id = "MG996R_BODY";
    motor.name = "Dong Co Servo MG996R High-Torque Metal Gear";
    motor.category = "Chap hanh co khi";
    motor.rating = "Momen xoan: 11.0 kg.cm / 4.8V-7.2V / Hop so kim loai";
    motor.roleDesc = "Dong co servo goc quay voi he thong giam toc banh rang kim loai, chiu luc tai lon, phan hoi goc chinh xac bang bien tro ben trong va mach dieu khien xung PWM 50Hz.";
    motor.liveStatusText = "Goc hien tai: " + std::to_string((int)currentAngleDeg) + " do | Dien ap: 5.0V";
    motor.localPos = Vec3(0.0f, 0.0f, 0.0f);
    motor.size = Vec3(4.0f, 4.3f, 2.0f);
    motor.color = Vec3(0.14f, 0.15f, 0.17f);
    motor.shape = SubShapeType::BOX_SMD;
    motor.mesh = Mesh3D::createBox(4.0f, 4.3f, 2.0f, motor.color);
    addSubComponent(motor);

    SubComponent gear;
    gear.id = "MG996R_GEAR";
    gear.name = "Hop So Banh Rang Kim Loai (Metal Gear Train)";
    gear.category = "Giam toc truyen dong";
    gear.rating = "Hop kim dong + thep nung / Ty so truyen 1:300";
    gear.roleDesc = "He thong 5 tang banh rang kim loai sieu ben giup nhan mo-men xoan tu dong co DC toc do cao thanh luc keo lon de nang canh tay va kep giu vat the.";
    gear.liveStatusText = "Ty so truyen: 1:300 | Kha nang chiu tai: 11kg.cm";
    gear.localPos = Vec3(1.0f, 2.2f, 0.0f);
    gear.size = Vec3(1.6f, 0.8f, 1.6f);
    gear.color = Vec3(0.85f, 0.75f, 0.32f);
    gear.shape = SubShapeType::CYLINDER_CAP;
    gear.mesh = Mesh3D::createCylinder(0.8f, 0.8f, 16, gear.color);
    addSubComponent(gear);

    SubComponent horn;
    horn.id = "MG996R_HORN";
    horn.name = "Dia Quay Servo Horn (Aluminum 25T Disc)";
    horn.category = "Khop noi co hoc";
    horn.rating = "Truc 25 rang (25T) / Hop kim nhom CNC / 4 lo bat vit M3";
    horn.roleDesc = "Dia quay gan chat vao truc servo, truyen dong truc tiep sang cac thanh dam CNC cua canh tay robot.";
    horn.liveStatusText = "Goc quay: " + std::to_string((int)currentAngleDeg) + " do";
    horn.localPos = Vec3(1.0f, 2.7f, 0.0f);
    horn.size = Vec3(2.0f, 0.3f, 2.0f);
    horn.color = Vec3(0.92f, 0.25f, 0.22f);
    horn.shape = SubShapeType::CYLINDER_CAP;
    horn.mesh = Mesh3D::createCylinder(1.0f, 0.3f, 16, horn.color);
    addSubComponent(horn);

    selectSubComponentByIndex(0);
}

void ServoMG996RComp::renderPinsAndDetails() {
    HardwareComponent::renderPinsAndDetails();

    // 1. Than dong co Servo MG996R mau den nham 4.07 x 4.29 x 1.97 cm
    glColor3f(0.14f, 0.15f, 0.17f);
    glPushMatrix();
    glScalef(4.07f, 4.29f, 1.97f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 2. Dai tem kim loai xanh duong/vang dac trung MG996R
    glColor3f(0.10f, 0.45f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glScalef(4.10f, 1.6f, 2.02f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 3. Tai ga bat vit 2 ben (Mounting Flanges)
    glColor3f(0.14f, 0.15f, 0.17f);
    glPushMatrix();
    glTranslatef(0.0f, 1.25f, 0.0f);
    glScalef(5.45f, 0.32f, 1.97f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Lo bat vit dem cao su va bac dong
    glColor3f(0.85f, 0.75f, 0.32f);
    for (float ex : {-2.4f, 2.4f}) {
        for (float ez : {-0.55f, 0.55f}) {
            glPushMatrix();
            glTranslatef(ex, 1.25f, ez);
            glRotatef(-90.0f, 1, 0, 0);
            glutSolidCylinder(0.18f, 0.36f, 10, 1);
            glPopMatrix();
        }
    }

    // 4. Truc ra kim loai 25T va Dia tron Servo Horn
    glPushMatrix();
    glTranslatef(1.05f, 2.15f, 0.0f);
    // Truc ra kim loai dong/thep
    glColor3f(0.85f, 0.75f, 0.32f);
    glRotatef(-90.0f, 1, 0, 0);
    glutSolidCylinder(0.30f, 0.50f, 16, 2);

    // Dia quay Servo Horn tron mau do CNC
    glTranslatef(0.0f, 0.0f, 0.45f);
    glRotatef(currentAngleDeg, 0, 0, 1);
    glColor3f(0.90f, 0.22f, 0.20f);
    glutSolidCylinder(1.10f, 0.20f, 18, 2);

    // 4 Lo oc M3 tren dia quay
    glColor3f(0.85f, 0.88f, 0.92f);
    for (int h = 0; h < 4; ++h) {
        float a = h * 1.5707963f;
        glPushMatrix();
        glTranslatef(0.75f * std::cos(a), 0.75f * std::sin(a), 0.18f);
        glutSolidCylinder(0.10f, 0.05f, 8, 1);
        glPopMatrix();
    }
    glPopMatrix();

    // 5. Cap 3 day servo (Nau GND, Do VCC, Cam PWM)
    glPushMatrix();
    glTranslatef(-2.05f, -1.6f, 0.0f);
    glColor3f(0.40f, 0.25f, 0.15f); // Nau
    glPushMatrix(); glTranslatef(0.0f, 0.0f, -0.2f); glScalef(0.6f, 0.15f, 0.15f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.85f, 0.15f, 0.15f); // Do
    glPushMatrix(); glTranslatef(0.0f, 0.0f,  0.0f); glScalef(0.6f, 0.15f, 0.15f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.95f, 0.55f, 0.12f); // Cam
    glPushMatrix(); glTranslatef(0.0f, 0.0f,  0.2f); glScalef(0.6f, 0.15f, 0.15f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();
}

void ServoMG996RComp::render(bool wireframeMode) {
    (void)wireframeMode;
    if (!visible) return;

    if (jointType == ServoJointType::BASE_YAW) {
        // Tim khop khuyu va khop kep la con trong cay SceneNode
        ServoMG996RComp* elbowComp = (!children.empty()) ? dynamic_cast<ServoMG996RComp*>(children[0]) : nullptr;
        ServoMG996RComp* gripComp = (elbowComp && !elbowComp->children.empty()) ? dynamic_cast<ServoMG996RComp*>(elbowComp->children[0]) : nullptr;

        float yawDeg = currentAngleDeg - 90.0f; // 90 do la huong thang phia truoc (+Z)
        float liftDeg = elbowComp ? elbowComp->currentAngleDeg : 0.0f;

        float gripRatio = (gripComp) ? (gripComp->currentAngleDeg / 85.0f) : 0.5f;
        if (gripRatio < 0.0f) gripRatio = 0.0f;
        if (gripRatio > 1.0f) gripRatio = 1.0f;

        bool isBaseSelected = isSelected;
        bool isElbowSelected = (elbowComp && elbowComp->isSelected);
        bool isGripSelected = (gripComp && gripComp->isSelected);

        glPushMatrix();
        Mat4 localM = getLocalMatrix();
        glMultMatrixf(localM.data());

        RoboticArmRenderer::renderArmAssembly(
            yawDeg, liftDeg, gripRatio,
            isBaseSelected, isElbowSelected, isGripSelected
        );

        glPopMatrix();
        return; // Toan bo 3 khop da duoc ket xuat dong bo trong renderArmAssembly
    }
}
