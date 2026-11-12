#pragma once
#include "components/HardwareComponent.h"

enum class RobotControlMode {
    MANUAL,       // Người dùng trực tiếp điều khiển xe (WASD) và tay gắp (I/K, J/L, O/P)
    AUTO_GRAB     // Thuật toán tự hành dò đường, dừng trước vật thể và tự động gắp nhấc
};

/**
 * @brief ARD: Bo mạch vi điều khiển trung tâm Arduino Uno R3
 * Kích thước CAD: 6.8 x 1.5 x 5.3 cm
 */
class ArduinoUnoComp : public HardwareComponent {
public:
    RobotControlMode controlMode = RobotControlMode::MANUAL;
    
    // Trạng thái firmware
    int autoSeqStep = 0;
    float autoSeqTimer = 0.0f;
    float obstacleDistCm = 100.0f;

    // Giá trị góc điều khiển mục tiêu cho 3 servo (độ)
    float baseAngleDeg  = 90.0f;
    float elbowAngleDeg = 90.0f;
    float gripAngleDeg  = 45.0f; // 0: Đóng kẹp, 90: Mở rộng

    // Giá trị tốc độ động cơ DC (0 - 255)
    int motorSpeedL = 0;
    int motorSpeedR = 0;
    int motorDirL   = 0; // -1 lùi, 0 dừng, 1 tiến
    int motorDirR   = 0;

public:
    ArduinoUnoComp();
    void setControlMode(RobotControlMode mode);
    void toggleControlMode();
    void resetAutoSequence();

    void updateFirmware(float dt, float measuredDistanceCm, bool& shouldGraspObject, bool& objectGrasped);

    virtual void onElectricalUpdate() override;
    virtual void renderPinsAndDetails() override;
};

/**
 * @brief HCSR04: Cảm biến khoảng cách siêu âm HC-SR04
 * Kích thước CAD: 4.5 x 2.0 x 1.8 cm
 */
class UltrasonicComp : public HardwareComponent {
public:
    float measuredDistanceCm = 200.0f;
    float maxDetectionRange = 400.0f;
    bool obstacleDetected = false;

public:
    UltrasonicComp();
    void updateRaycast(const Vec3& sensorWorldPos, const Vec3& forwardDir, const Vec3& targetObjPos, float targetObjRadius);
    virtual void onElectricalUpdate() override;
    virtual void renderPinsAndDetails() override;
};
