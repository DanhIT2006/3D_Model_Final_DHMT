#pragma once
#include "components/HardwareComponent.h"

/**
 * @brief L298N: Module cầu H kép điều khiển động cơ DC
 * Kích thước CAD: 4.3 x 2.7 x 4.3 cm
 */
class L298NDriverComp : public HardwareComponent {
public:
    float outVoltL = 0.0f; // Điện áp phân cực ra motor trái (-7.4V -> +7.4V)
    float outVoltR = 0.0f; // Điện áp phân cực ra motor phải (-7.4V -> +7.4V)

public:
    L298NDriverComp();
    virtual void onElectricalUpdate() override;
    virtual void renderPinsAndDetails() override;
};

/**
 * @brief MOT_L / MOT_R: Động cơ DC giảm tốc màu vàng + Bánh xe cao su
 * Kích thước CAD: 6.5 x 4.0 x 8.0 cm, Bán kính bánh xe: 3.3 cm
 */
class DCMotorComp : public HardwareComponent {
public:
    bool isLeftMotor = true;
    float wheelRadius = 3.3f;   // Bán kính bánh xe (cm)
    float wheelAngle = 0.0f;    // Góc quay hiện tại của bánh xe (radian)
    float rpm = 0.0f;           // Vòng/phút
    float linearSpeedCmS = 0.0f;// Vận tốc dài tiếp tuyến bánh xe (cm/s)

public:
    DCMotorComp(bool isLeft);
    void updatePhysics(float dt, float inputVoltage, bool driverPowered = true);
    virtual void onElectricalUpdate() override;
    virtual void render(bool wireframeMode) override;
};

enum class ServoJointType {
    BASE_YAW,    // Khung ray trụ đứng dẫn hướng (Tower Mast & Dual Rails)
    LINEAR_LIFT, // Bàn trượt nâng hạ thẳng đứng (Vertical Elevator Carriage)
    ELBOW_PITCH = LINEAR_LIFT, // Tương thích ngược với tên cũ
    GRIPPER      // Khớp kẹp ngàm gắp đồ vật (Clamping Jaws)
};

/**
 * @brief MG996R / Cơ cấu cơ học nâng hạ thẳng đứng và ngàm kẹp
 * Kích thước CAD chuẩn hóa
 */
class ServoMG996RComp : public HardwareComponent {
public:
    ServoJointType jointType;
    float currentAngleDeg = 90.0f;  // Góc quay hoặc tỷ lệ điều khiển (0 - 180)
    float targetAngleDeg  = 90.0f;  // Góc điều khiển mục tiêu từ xung PWM
    float minAngleDeg = 0.0f;
    float maxAngleDeg = 180.0f;
    float slewRate = 180.0f;        // Tốc độ di chuyển

    float currentLiftY = 0.0f;      // Độ cao tịnh tiến thẳng đứng hiện tại (cm)

    Mesh3D armSegmentMesh; // Hình học khung ray, bàn trượt hoặc ngàm kẹp

public:
    ServoMG996RComp(const std::string& code, const std::string& name, ServoJointType type);
    void setTargetAngle(float deg);
    void updateMotion(float dt);
    virtual void onElectricalUpdate() override;
    virtual void render(bool wireframeMode) override;
};
