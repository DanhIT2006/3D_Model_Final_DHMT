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

    // Cổng USB Type-B kim loại mạ bạc góc sau-trái
    glPushMatrix();
    glTranslatef(-2.2f, 0.7f, 1.6f);
    glColor3f(0.75f, 0.75f, 0.8f);
    glutSolidCube(1.2f);
    glPopMatrix();

    // Jack nguồn DC Barrel màu đen góc sau
    glPushMatrix();
    glTranslatef(-2.2f, 0.7f, -1.6f);
    glColor3f(0.12f, 0.12f, 0.12f);
    glutSolidCube(1.3f);
    glPopMatrix();

    // Chip vi điều khiển ATmega328P DIP-28 hình chữ nhật màu đen
    glPushMatrix();
    glTranslatef(0.8f, 0.8f, 0.0f);
    glColor3f(0.15f, 0.15f, 0.15f);
    glScalef(2.6f, 0.35f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Hàng rào cắm chân cắm cái (Female Pin Headers) 2 bên mép mạch
    glDisable(GL_LIGHTING);
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, 2.2f);
    glScalef(5.5f, 0.4f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.85f, -2.2f);
    glScalef(5.5f, 0.4f, 0.35f);
    glutSolidCube(1.0f);
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
