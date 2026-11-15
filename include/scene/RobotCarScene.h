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

    // Các tầng khung gầm và thành xe bảo vệ
    SceneNode* chassisFloor1    = nullptr;
    SceneNode* chassisFloor2    = nullptr;
    SceneNode* chassisSideLeft  = nullptr; // Thành xe bên trái
    SceneNode* chassisSideRight = nullptr; // Thành xe bên phải
    SceneNode* frontBumper      = nullptr; // Thành cản trước
    SceneNode* rearBumper       = nullptr; // Thành cản sau

    // 11 Linh kiện & Hệ thống 4 Bánh xe (4WD)
    BatterySwitchComp* compBatSw   = nullptr;
    DominoBusComp*     compDomino  = nullptr;
    BuckLM2596Comp*    compBuck    = nullptr;
    ArduinoUnoComp*    compArd     = nullptr;
    L298NDriverComp*   compL298N   = nullptr;
    UltrasonicComp*    compHcsr04  = nullptr;

    // 4 Động cơ DC & 4 Bánh xe cao su (4WD Robot Car)
    DCMotorComp*       compMotFL   = nullptr; // Trước trái
    DCMotorComp*       compMotRL   = nullptr; // Sau trái
    DCMotorComp*       compMotFR   = nullptr; // Trước phải
    DCMotorComp*       compMotRR   = nullptr; // Sau phải
    DCMotorComp*       compMotL    = nullptr; // Tương thích ngược: Trỏ compMotFL
    DCMotorComp*       compMotR    = nullptr; // Tương thích ngược: Trỏ compMotFR

    ServoMG996RComp*   compSrvBase = nullptr;
    ServoMG996RComp*   compSrvElbow= nullptr;
    ServoMG996RComp*   compSrvGrip = nullptr;

    // Vật thể mẫu để gắp (Target Object)
    SceneNode* targetObject = nullptr;
    bool isObjectGrasped = false;
    Vec3 gripWorldPos = Vec3(0.0f, 2.5f, 16.0f);
    Vec3 initialObjectPos = Vec3(0.0f, 1.5f, 38.0f);

    // Thông số động học xe vi sai (Differential Drive Kinematics)
    Vec3 carPosition = Vec3(0.0f, 0.0f, 0.0f);
    float carYawRad = 0.0f; // Hướng mũi xe (radian)
    float wheelbaseWidth = 16.0f; // Khoảng cách giữa 2 tâm bánh xe (cm)

    // Bộ giải mạch điện
    CircuitSimulator circuit;

    // Linh kiện hiện đang được người dùng click chọn trong 3D
    HardwareComponent* selectedComponent = nullptr;

    // Chế độ quan sát vi mô (Micro-Component Workbench Inspection Mode)
    bool isWorkbenchMode = false;
    bool isXrayMode = false; // Chế độ X-Ray bo mạch (Phím X)
    HardwareComponent* inspectedComponent = nullptr;

public:
    RobotCarScene();
    ~RobotCarScene();

    void initScene();
    void update(float dt);
    void render(bool wireframeMode);

    // Chế độ quan sát vi mô bàn làm việc CAD
    void enterWorkbench(HardwareComponent* comp);
    void exitWorkbench();
    void renderWorkbenchScene(bool wireframeMode);
    SubComponent* pickSubComponent(const Vec3& rayOrigin, const Vec3& rayDir);

    // Động học vi sai xe 2 bánh
    void updateDifferentialDrive(float dt);

    // Động học tay gắp & cơ chế hút/kẹp vật thể
    void updateArmAndGrasping(float dt);

    // Reset toàn bộ xe, tay gắp và vật thể về vị trí xuất phát ban đầu
    void resetSimulation();

    // Xử lý chọn linh kiện bằng Raycast từ màn hình 2D
    HardwareComponent* pickComponent(const Vec3& rayOrigin, const Vec3& rayDir);
};
