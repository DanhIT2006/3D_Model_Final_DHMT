#pragma once
#include "scene/SceneNode.h"
#include "core/CircuitSimulator.h"
#include <memory>

/**
 * @brief Quản lý toàn bộ cấu trúc cơ khí, cây phân cảnh và động học của Xe Robot
 * Tầng 1, Tầng 2, 4 cột trụ đồng, bánh mắt trâu, 11 linh kiện và vật thể gắp mẫu
 */
class RobotCarScene {
public:
    // Nút gốc xe
    SceneNode* carRoot = nullptr;

    // Các tầng khung gầm
    SceneNode* chassisFloor1 = nullptr;
    SceneNode* chassisFloor2 = nullptr;

    // 11 Linh kiện
    BatterySwitchComp* compBatSw   = nullptr;
    DominoBusComp*     compDomino  = nullptr;
    BuckLM2596Comp*    compBuck    = nullptr;
    ArduinoUnoComp*    compArd     = nullptr;
    L298NDriverComp*   compL298N   = nullptr;
    UltrasonicComp*    compHcsr04  = nullptr;
    DCMotorComp*       compMotL    = nullptr;
    DCMotorComp*       compMotR    = nullptr;
    ServoMG996RComp*   compSrvBase = nullptr;
    ServoMG996RComp*   compSrvElbow= nullptr;
    ServoMG996RComp*   compSrvGrip = nullptr;

    // Vật thể mẫu để gắp (Target Object)
    SceneNode* targetObject = nullptr;
    bool isObjectGrasped = false;
    Vec3 initialObjectPos = Vec3(0.0f, 1.5f, 38.0f);

    // Thông số động học xe vi sai (Differential Drive Kinematics)
    Vec3 carPosition = Vec3(0.0f, 0.0f, 0.0f);
    float carYawRad = 0.0f; // Hướng mũi xe (radian)
    float wheelbaseWidth = 16.0f; // Khoảng cách giữa 2 tâm bánh xe (cm)

    // Bộ giải mạch điện
    CircuitSimulator circuit;

    // Linh kiện hiện đang được người dùng click chọn trong 3D
    HardwareComponent* selectedComponent = nullptr;

public:
    RobotCarScene();
    ~RobotCarScene();

    void initScene();
    void update(float dt);
    void render(bool wireframeMode);

    // Động học vi sai xe 2 bánh
    void updateDifferentialDrive(float dt);

    // Động học tay gắp & cơ chế hút/kẹp vật thể
    void updateArmAndGrasping(float dt);

    // Reset toàn bộ xe, tay gắp và vật thể về vị trí xuất phát ban đầu
    void resetSimulation();

    // Xử lý chọn linh kiện bằng Raycast từ màn hình 2D
    HardwareComponent* pickComponent(const Vec3& rayOrigin, const Vec3& rayDir);
};
