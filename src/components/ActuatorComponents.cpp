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

    // 1. Tan nhiet nhom xe ranh
    SubComponent heatsink;
    heatsink.id = "HEATSINK";
    heatsink.name = "Tan nhiet nhom xe ranh (Aluminium Heatsink)";
    heatsink.rating = "Nhom Anodized den - 6 ranh thoat nhiet";
    heatsink.roleDesc = "Giai phong luong nhiet lon sinh ra tu 2 mach cau H ben trong IC L298N khi dong co hoat dong mang tai hoac bam xung PWM tan so cao, chong qua nhiet tu ngat.";
    heatsink.localPos = Vec3(0.0f, 1.25f, -1.05f);
    heatsink.size = Vec3(2.8f, 2.1f, 0.9f);
    heatsink.color = Vec3(0.18f, 0.18f, 0.20f);
    heatsink.mesh = Mesh3D::createBox(2.8f, 2.1f, 0.9f, heatsink.color);
    addSubComponent(heatsink);

    // 2. IC Cong suat L298N
    SubComponent icL298n;
    icL298n.id = "IC_L298N";
    icL298n.name = "IC Cong suat cau H kep L298N";
    icL298n.rating = "Multiwatt15 / 46V max / 2A dinh muc moi kenh";
    icL298n.roleDesc = "Trai tim mach dong luc: Nhan tin hieu logic 5V tu Arduino Uno va khuếch dai cong suat (7.4V, toi da 2A) dieu khien chieu quay tien/lui va toc do PWM cho 2 dong co DC banh xe.";
    icL298n.localPos = Vec3(0.0f, 0.85f, -0.55f);
    icL298n.size = Vec3(2.2f, 1.3f, 0.3f);
    icL298n.color = Vec3(0.08f, 0.08f, 0.08f);
    icL298n.mesh = Mesh3D::createBox(2.2f, 1.3f, 0.3f, icL298n.color);
    addSubComponent(icL298n);

    // 3. Tu hoa loc nguon VCC C1
    SubComponent capVcc;
    capVcc.id = "CAP_VCC";
    capVcc.name = "Tu hoa loc nguon VCC C1 (220uF 35V)";
    capVcc.rating = "220 uF - 35V / Chiu nhiet 105 do C";
    capVcc.roleDesc = "San phang gơn song dien ap tu pin 7.4V. Triet tieu hien tuong sut ap tuc thoi khi dong co DC khoi dong hoac dao chieu (Inrush current), tranh gay reset vi dieu khien.";
    capVcc.localPos = Vec3(-0.45f, 0.65f, 0.15f);
    capVcc.size = Vec3(0.7f, 1.1f, 0.7f);
    capVcc.color = Vec3(0.12f, 0.12f, 0.15f);
    capVcc.mesh = Mesh3D::createCylinder(0.35f, 1.1f, 12, capVcc.color);
    addSubComponent(capVcc);

    // 4. Tu hoa loc nguon 5V C2
    SubComponent cap5v;
    cap5v.id = "CAP_5V";
    cap5v.name = "Tu hoa loc nguon logic C2 (220uF 35V)";
    cap5v.rating = "220 uF - 35V / Loc dien ap logic 5V";
    cap5v.roleDesc = "On dinh dien ap 5V ngõ ra cua IC 78M05, giu cho muc logic cua cac cong AND/OR ben trong IC L298N luon chuan xac, chong nhieu cao tan tu cuon day dong co.";
    cap5v.localPos = Vec3(0.55f, 0.65f, 0.45f);
    cap5v.size = Vec3(0.7f, 1.1f, 0.7f);
    cap5v.color = Vec3(0.12f, 0.12f, 0.15f);
    cap5v.mesh = Mesh3D::createCylinder(0.35f, 1.1f, 12, cap5v.color);
    addSubComponent(cap5v);

    // 5. IC On ap 78M05
    SubComponent reg7805;
    reg7805.id = "REG_78M05";
    reg7805.name = "IC On ap tuyen tinh 78M05 (5V Regulator)";
    reg7805.rating = "Vin max 35V / Vout 5.0V / Iout 500mA / TO-252";
    reg7805.roleDesc = "Ha ap truc tiep tu nguon Pin VCC (7.4V) xuong 5.0V on dinh de nuoi mach logic L298N va co the cap nguon 5V phu ra ngoai qua chan 5V.";
    reg7805.localPos = Vec3(0.55f, 0.32f, -0.15f);
    reg7805.size = Vec3(0.65f, 0.3f, 0.6f);
    reg7805.color = Vec3(0.2f, 0.2f, 0.22f);
    reg7805.mesh = Mesh3D::createBox(0.65f, 0.3f, 0.6f, reg7805.color);
    addSubComponent(reg7805);

    // 6. Jumper chon nguon 5V Enable
    SubComponent jmp5v;
    jmp5v.id = "JUMPER_5V";
    jmp5v.name = "Jumper chon nguon 5V Enable (5V Regulator Jumper)";
    jmp5v.rating = "Header 2-pin 2.54mm kem cap shunt mau den";
    jmp5v.roleDesc = "KHI CAM: Cho phep IC 78M05 lay nguon tu VCC tao 5V logic. KHI RUT (khi pin > 12V): Ngat 78M05 de tranh chay IC; khi do bat buoc phai cap nguon 5V ngoai vao coc 5V.";
    jmp5v.localPos = Vec3(-0.45f, 0.42f, 0.85f);
    jmp5v.size = Vec3(0.45f, 0.55f, 0.35f);
    jmp5v.color = Vec3(0.1f, 0.6f, 0.9f);
    jmp5v.isInteractive = true;
    jmp5v.state = true;
    jmp5v.actionName = "THAO / CAM JUMPER 5V";
    jmp5v.mesh = Mesh3D::createBox(0.45f, 0.55f, 0.35f, jmp5v.color);
    addSubComponent(jmp5v);

    // 7. Dan 4 Diode bao ve Motor A
    SubComponent dioA;
    dioA.id = "FLYBACK_DIODES_L";
    dioA.name = "Dan 4 Diode bao ve Motor A (Flyback D1-D4)";
    dioA.rating = "4x 1N4007 / Fast Recovery Shottky / 1A";
    dioA.roleDesc = "Dau song song nguoc voi cuon cam Dong co Trai (Motor A). Tieu tan suc dien dong tu cam nguoc (Back-EMF) khi ngat xung PWM, chong danh thung transistor cong suat.";
    dioA.localPos = Vec3(-1.75f, 0.28f, -0.4f);
    dioA.size = Vec3(0.35f, 0.25f, 1.5f);
    dioA.color = Vec3(0.1f, 0.1f, 0.1f);
    dioA.mesh = Mesh3D::createBox(0.35f, 0.25f, 1.5f, dioA.color);
    addSubComponent(dioA);

    // 8. Dan 4 Diode bao ve Motor B
    SubComponent dioB;
    dioB.id = "FLYBACK_DIODES_R";
    dioB.name = "Dan 4 Diode bao ve Motor B (Flyback D5-D8)";
    dioB.rating = "4x 1N4007 / Fast Recovery Shottky / 1A";
    dioB.roleDesc = "Dau song song nguoc voi cuon cam Dong co Phai (Motor B). Ngan chan xung dien ap nguoc hang chuc volt dot ngot phong nguoc ve nguon.";
    dioB.localPos = Vec3(1.75f, 0.28f, -0.4f);
    dioB.size = Vec3(0.35f, 0.25f, 1.5f);
    dioB.color = Vec3(0.1f, 0.1f, 0.1f);
    dioB.mesh = Mesh3D::createBox(0.35f, 0.25f, 1.5f, dioB.color);
    addSubComponent(dioB);

    // 9. Coc dau nguon 3-pin Screw Terminal
    SubComponent termPwr;
    termPwr.id = "TERMINAL_PWR";
    termPwr.name = "Coc dau nguon 3-pin Screw Terminal (VCC, GND, 5V)";
    termPwr.rating = "Khoi van vit 5.08mm mau xanh duong / Dong 15A";
    termPwr.roleDesc = "Chan VCC nhan 7.4V tu Pin qua cong tac; chan GND noi dat chung toan xe; chan 5V cap nguon logic hoac nhan 5V phu tuy thuoc vao Jumper 5V Enable.";
    termPwr.localPos = Vec3(-0.6f, 0.52f, 1.6f);
    termPwr.size = Vec3(1.6f, 0.85f, 0.85f);
    termPwr.color = Vec3(0.1f, 0.35f, 0.85f);
    termPwr.mesh = Mesh3D::createBox(1.6f, 0.85f, 0.85f, termPwr.color);
    addSubComponent(termPwr);

    // 10. Coc ra Dong co A
    SubComponent termMotA;
    termMotA.id = "TERMINAL_MOTA";
    termMotA.name = "Coc ra Dong co A (OUT1, OUT2 - Motor A)";
    termMotA.rating = "Khoi van vit 2-pin 5.08mm mau xanh duong";
    termMotA.roleDesc = "Dau ra noi truc tiep voi 2 cuc cua Dong co DC giam toc Banh Trai (MOT_L). Dien ap dao cuc (-7.4V den +7.4V) cho phep banh quay tien/lui.";
    termMotA.localPos = Vec3(-1.65f, 0.52f, 0.6f);
    termMotA.size = Vec3(0.85f, 0.85f, 1.1f);
    termMotA.color = Vec3(0.1f, 0.35f, 0.85f);
    termMotA.mesh = Mesh3D::createBox(0.85f, 0.85f, 1.1f, termMotA.color);
    addSubComponent(termMotA);

    // 11. Coc ra Dong co B
    SubComponent termMotB;
    termMotB.id = "TERMINAL_MOTB";
    termMotB.name = "Coc ra Dong co B (OUT3, OUT4 - Motor B)";
    termMotB.rating = "Khoi van vit 2-pin 5.08mm mau xanh duong";
    termMotB.roleDesc = "Dau ra noi truc tiep voi 2 cuc cua Dong co DC giam toc Banh Phai (MOT_R). Dong dien duoc bam xung PWM de dieu khien van toc goc.";
    termMotB.localPos = Vec3(1.65f, 0.52f, 0.6f);
    termMotB.size = Vec3(0.85f, 0.85f, 1.1f);
    termMotB.color = Vec3(0.1f, 0.35f, 0.85f);
    termMotB.mesh = Mesh3D::createBox(0.85f, 0.85f, 1.1f, termMotB.color);
    addSubComponent(termMotB);

    // 12. Hang chan tin hieu Logic
    SubComponent hdrLogic;
    hdrLogic.id = "HEADER_LOGIC";
    hdrLogic.name = "Hang chan tin hieu Logic (ENA, IN1, IN2, IN3, IN4, ENB)";
    hdrLogic.rating = "6-pin Header duc 2.54mm ma vang / Logic TTL 5V";
    hdrLogic.roleDesc = "Nhan tin hieu tu Arduino: ENA/ENB nhan xung PWM de dieu toc xe; IN1/IN2 quy dinh chieu quay banh trai; IN3/IN4 quy dinh chieu quay banh phai.";
    hdrLogic.localPos = Vec3(0.8f, 0.38f, 1.6f);
    hdrLogic.size = Vec3(1.5f, 0.55f, 0.35f);
    hdrLogic.color = Vec3(0.85f, 0.75f, 0.2f);
    hdrLogic.mesh = Mesh3D::createBox(1.5f, 0.55f, 0.35f, hdrLogic.color);
    addSubComponent(hdrLogic);

    // 13. LED bao nguon Logic
    SubComponent ledPwr;
    ledPwr.id = "LED_PWR";
    ledPwr.name = "LED bao nguon Logic (Power LED Indicator)";
    ledPwr.rating = "SMD LED 0805 mau do / Dong tieu thu 10mA";
    ledPwr.roleDesc = "Phat sang mau do de xac nhan mach logic L298N dang duoc cap nguon 5V on dinh va san sang nhan lenh dieu khien tu vi dieu khien.";
    ledPwr.localPos = Vec3(0.05f, 0.28f, 0.65f);
    ledPwr.size = Vec3(0.25f, 0.2f, 0.25f);
    ledPwr.color = Vec3(1.0f, 0.2f, 0.2f);
    ledPwr.mesh = Mesh3D::createBox(0.25f, 0.2f, 0.25f, ledPwr.color);
    addSubComponent(ledPwr);

    // Mac dinh chon linh kien dau tien (Tan nhiet)
    selectSubComponentByIndex(0);
}

void L298NDriverComp::onSubComponentAction(const std::string& subId) {
    if (subId == "JUMPER_5V") {
        jumper5VEnabled = !jumper5VEnabled;
        SubComponent* sub = getSelectedSubComponent();
        if (sub) {
            sub->state = jumper5VEnabled;
            if (jumper5VEnabled) {
                sub->rating = "Trang thai: DANG CAM (Enable 78M05 ON)";
                sub->roleDesc = "Jumper DANG CAM: Kich hoat IC on ap 78M05 noi bo tao nguon 5V tu Pin VCC (7.4V) de nuoi mach logic L298N. Chan 5V co the cap nguon 5V ra ngoai.";
                sub->color = Vec3(0.1f, 0.6f, 0.9f);
            } else {
                sub->rating = "Trang thai: DA RUT (Disable 78M05 OFF)";
                sub->roleDesc = "Jumper DA RUT: Ngat IC on ap 78M05. Che do nay bat buoc khi pin > 12V de bao ve 78M05 khong bi chay. Can cap nguon 5V ngoai vao coc 5V!";
                sub->color = Vec3(0.4f, 0.4f, 0.4f);
            }
        }
    }
}

void L298NDriverComp::onElectricalUpdate() {
    Pin* p12v = getPin("12V");
    Pin* p5v  = getPin("5V");

    float v12 = p12v ? p12v->voltage : 0.0f;
    float v5  = p5v  ? p5v->voltage  : 0.0f;

    // Cáº§n cáº£ nguá»“n Ä‘á»™ng lá»±c 12V (7.4V) vÃ  5V logic tá»« Arduino
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

    // Khá»‘i táº£n nhiá»‡t nhÃ´m mÃ u Ä‘en cÃ³ cÃ¡c cÃ¡nh xáº» rÃ£nh á»Ÿ giá»¯a máº¡ch
    glPushMatrix();
    glTranslatef(0.0f, 1.4f, 0.0f);
    glColor3f(0.18f, 0.18f, 0.2f);
    glScalef(2.4f, 2.2f, 1.6f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Chip IC L298N gáº¯n á»‘p vÃ o máº·t táº£n nhiá»‡t
    glPushMatrix();
    glTranslatef(0.0f, 1.2f, 0.9f);
    glColor3f(0.1f, 0.1f, 0.1f);
    glScalef(1.8f, 1.6f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 2 Tá»¥ hÃ³a lá»c nguá»“n mÃ u Ä‘en/vÃ ng
    for (float x : {-1.2f, 1.2f}) {
        glPushMatrix();
        glTranslatef(x, 0.9f, 0.8f);
        glColor3f(0.15f, 0.15f, 0.2f);
        glutSolidCylinder(0.35f, 1.2f, 10, 1);
        glPopMatrix();
    }
}

// ===================== DONG CO DC & BANH XE =====================
DCMotorComp::DCMotorComp(bool isLeft)
    : HardwareComponent(isLeft ? "MOT_L" : "MOT_R",
                        isLeft ? "Dong Co & Banh Trai" : "Dong Co & Banh Phai",
                        "Dong Luc"),
      isLeftMotor(isLeft) {
    cadSize = Vec3(6.5f, 4.0f, 8.0f);
    nominalVoltage = 5.0f;

    // Há»™p sá»‘ mÃ u vÃ ng Ä‘áº·c trÆ°ng Ä‘á»™ng cÆ¡ TT Motor
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

    // 1. Váº½ há»™p sá»‘ mÃ u vÃ ng
    if (wireframeMode) {
        mesh.drawWireframe(isSelected ? Vec3(1.0f, 0.85f, 0.0f) : Vec3(0.0f, 0.95f, 0.4f));
    } else {
        mesh.drawSolid(true);
        if (isSelected) mesh.drawWireframe(Vec3(1.0f, 0.85f, 0.0f), 2.5f);
    }

    // 2. Váº½ trá»¥c vÃ  bÃ¡nh xe cao su
    float wheelXOffset = isLeftMotor ? -2.2f : 2.2f;
    glPushMatrix();
    glTranslatef(wheelXOffset, 0.0f, 0.0f);
    glRotatef(wheelAngle * 57.29578f, 1.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    if (wireframeMode) {
        glDisable(GL_LIGHTING);
        glColor3f(0.8f, 0.8f, 0.2f);
        glutWireTorus(0.9f, wheelRadius * 0.75f, 10, 16);
    } else {
        glColor3f(0.12f, 0.12f, 0.12f);
        glutSolidTorus(0.85f, wheelRadius * 0.72f, 12, 20);

        glColor3f(0.92f, 0.76f, 0.08f);
        glutSolidCylinder(wheelRadius * 0.65f, 0.8f, 16, 2);
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
