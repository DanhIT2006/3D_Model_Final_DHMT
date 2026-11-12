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
