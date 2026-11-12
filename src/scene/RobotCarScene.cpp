#include "scene/RobotCarScene.h"
#include <GL/freeglut.h>
#include <cmath>
#include <iostream>

RobotCarScene::RobotCarScene() {
    initScene();
}

RobotCarScene::~RobotCarScene() {
    delete carRoot;
    delete targetObject;
}

void RobotCarScene::initScene() {
    // 1. Nút gốc xe tại tâm sàn Tầng 1
    carRoot = new SceneNode("CAR_ROOT", "Xe Robot Gắp Đồ Vật");
    carPosition = Vec3(0.0f, 0.0f, 0.0f);
    carYawRad = 0.0f;

    // 2. Sàn Tầng 1 (Chassis Floor 1)
    chassisFloor1 = new SceneNode("FLOOR_1", "San Khung Gam Tang 1");
    chassisFloor1->localPos = Vec3(0.0f, 0.0f, 0.0f);
    // Tấm mica đen bóng kích thước 16 x 0.4 x 24 cm
    chassisFloor1->mesh = Mesh3D::createBox(16.0f, 0.4f, 24.0f, Vec3(0.12f, 0.12f, 0.14f));
    carRoot->addChild(chassisFloor1);

    // 4 Cột trụ đồng kết nối Tầng 1 lên Tầng 2 (cao 6.0 cm, đường kính 0.5 cm)
    for (float x : {-6.5f, 6.5f}) {
        for (float z : {-9.5f, 9.5f}) {
            SceneNode* pillar = new SceneNode("PILLAR", "Tru Dong Tang 1-2");
            pillar->localPos = Vec3(x, 3.0f, z);
            pillar->mesh = Mesh3D::createCylinder(0.3f, 6.0f, 8, Vec3(0.85f, 0.72f, 0.25f));
            chassisFloor1->addChild(pillar);
        }
    }

    // Bánh xe đa hướng mắt trâu (Caster Wheel) đặt dưới mũi xe Tầng 1
    SceneNode* caster = new SceneNode("CASTER", "Banh Mat Trau Dan Huong");
    caster->localPos = Vec3(0.0f, -1.8f, 9.5f);
    caster->mesh = Mesh3D::createCylinder(0.8f, 1.2f, 12, Vec3(0.75f, 0.75f, 0.8f));
    chassisFloor1->addChild(caster);

    // 3. Khởi tạo và lắp ráp các linh kiện trên Tầng 1
    compDomino = new DominoBusComp();
    compDomino->localPos = Vec3(0.0f, 0.75f, 0.0f);
    chassisFloor1->addChild(compDomino);

    compL298N = new L298NDriverComp();
    compL298N->localPos = Vec3(-2.5f, 0.8f, -7.0f);
    chassisFloor1->addChild(compL298N);

    compBuck = new BuckLM2596Comp();
    compBuck->localPos = Vec3(3.0f, 0.7f, -7.0f);
    chassisFloor1->addChild(compBuck);

    // Bo cam bien sieu am o mui xe theo yeu cau, thay the bang truc tru nang va tay gap
    compHcsr04 = nullptr;

    compMotL = new DCMotorComp(true);
    compMotL->localPos = Vec3(-8.0f, -1.0f, -4.0f);
    chassisFloor1->addChild(compMotL);

    compMotR = new DCMotorComp(false);
    compMotR->localPos = Vec3(8.0f, -1.0f, -4.0f);
    chassisFloor1->addChild(compMotR);

    // 4. San Tang 2 (Chassis Floor 2) cach Tang 1 delta Y = 6.0 cm
    chassisFloor2 = new SceneNode("FLOOR_2", "San Khung Gam Tang 2");
    chassisFloor2->localPos = Vec3(0.0f, 6.0f, 0.0f);
    chassisFloor2->mesh = Mesh3D::createBox(16.0f, 0.4f, 24.0f, Vec3(0.12f, 0.12f, 0.14f));
    carRoot->addChild(chassisFloor2);

    // 5. Khoi tao va lap rap linh kien tren Tang 2
    compArd = new ArduinoUnoComp();
    compArd->localPos = Vec3(-3.5f, 0.75f, 0.0f);
    chassisFloor2->addChild(compArd);

    compBatSw = new BatterySwitchComp();
    compBatSw->localPos = Vec3(0.0f, 1.1f, -8.5f);
    chassisFloor2->addChild(compBatSw);

    // 6. Co cau nang ha thang dung va tay gap AGV (Linear Elevator Gripper)
    // Lap ngay tai mui xe Tang 1 (Z = 11.5cm) thay the vi tri cu cua cam bien sieu am
    compSrvBase = new ServoMG996RComp("SRV_BASE", "Tru Ray Dan Huong AGV", ServoJointType::BASE_YAW);
    compSrvBase->localPos = Vec3(0.0f, 0.0f, 11.5f);
    chassisFloor1->addChild(compSrvBase);

    // SRV_ELBOW: Ban truot nang ha thang dung (Vertical Elevator Carriage) truot doc truc Y
    compSrvElbow = new ServoMG996RComp("SRV_ELBOW", "Ban Truot Nang Ha (Linear Lift)", ServoJointType::LINEAR_LIFT);
    compSrvElbow->localPos = Vec3(0.0f, 1.5f, 0.0f);
    compSrvBase->addChild(compSrvElbow);

    // SRV_GRIP: Cum ngam kep gap doi gan o mat truoc ban truot
    compSrvGrip = new ServoMG996RComp("SRV_GRIP", "Cum Ngam Kep Gap Do Vat", ServoJointType::GRIPPER);
    compSrvGrip->localPos = Vec3(0.0f, 0.0f, 1.4f);
    compSrvElbow->addChild(compSrvGrip);

    // 7. Khoi vat the mau can gap (Hop qua/khoi kim loai mau cam noi bat)
    targetObject = new SceneNode("TARGET_OBJ", "Vat The Can Gap");
    targetObject->localPos = initialObjectPos;
    // Khoi hop kich thuoc 3.2 x 3.0 x 3.2 cm
    targetObject->mesh = Mesh3D::createBox(3.2f, 3.0f, 3.2f, Vec3(0.95f, 0.35f, 0.1f));

    // 8. Dang ky toan bo cac linh kien vao bo giai mach dien CircuitSimulator
    circuit.registerComponent(compBatSw);
    circuit.registerComponent(compDomino);
    circuit.registerComponent(compBuck);
    circuit.registerComponent(compArd);
    circuit.registerComponent(compL298N);
    circuit.registerComponent(compMotL);
    circuit.registerComponent(compMotR);
    circuit.registerComponent(compSrvBase);
    circuit.registerComponent(compSrvElbow);
    circuit.registerComponent(compSrvGrip);

    circuit.setupStandardCarCircuit();
}

void RobotCarScene::updateDifferentialDrive(float dt) {
    if (!compMotL || !compMotR) return;

    // Vận tốc tiếp tuyến 2 bánh xe (cm/s)
    float vL = compMotL->linearSpeedCmS;
    float vR = compMotR->linearSpeedCmS;

    // Công thức động học vi sai (Differential Drive Kinematics)
    // Vận tốc dài trung bình xe
    float v = (vR + vL) * 0.5f;
    // Vận tốc góc quay thân xe quanh trục thẳng đứng
    float omega = (vR - vL) / wheelbaseWidth;

    carYawRad += omega * dt;

    // Hướng chuyển động mũi xe theo trục Z cục bộ
    carPosition.x += v * std::sin(carYawRad) * dt;
    carPosition.z += v * std::cos(carYawRad) * dt;

    // Áp dụng vị trí và hướng vào nút gốc xe
    carRoot->localPos = carPosition;
    carRoot->localRot.y = carYawRad;
}

void RobotCarScene::updateArmAndGrasping(float dt) {
    if (!compSrvGrip || !targetObject) return;

    Mat4 gripWorldM = compSrvGrip->getWorldMatrix();
    // Vi tri tam diem kep ngam trong khong gian the gioi (Z = 3.4cm trong he toa do ngam)
    Vec3 gripWorldPos = gripWorldM.transformPoint(Vec3(0.0f, 0.0f, 3.4f));

    bool shouldGrasp = false;
    // Cu ly truc tiep tu mui xe / ngam kep toi vat the mau
    float distToObj = gripWorldPos.distanceTo(targetObject->localPos);
    float measuredDist = distToObj;

    // Cap nhat firmware Arduino
    if (compArd) {
        compArd->updateFirmware(dt, measuredDist, shouldGrasp, isObjectGrasped);

        // Truyen goc dieu khien sang cac servo
        if (compSrvBase)  compSrvBase->setTargetAngle(compArd->baseAngleDeg);
        if (compSrvElbow) compSrvElbow->setTargetAngle(compArd->elbowAngleDeg);
        if (compSrvGrip)  compSrvGrip->setTargetAngle(compArd->gripAngleDeg);
    }

    // Kiem tra dieu kien gap vat
    if (!isObjectGrasped) {
        // Nếu ngàm đóng lại (góc < 25 độ) và cự ly ngàm tới vật thể < 4.2 cm -> Kẹp dính vật thể!
        if ((shouldGrasp || compSrvGrip->currentAngleDeg < 25.0f) && distToObj < 4.2f) {
            isObjectGrasped = true;
        }
    }

    if (isObjectGrasped) {
        // Khi đã kẹp, vật thể di chuyển hoàn toàn đồng bộ theo ngàm kẹp
        if (compSrvGrip->currentAngleDeg > 55.0f) {
            // Người dùng hoặc máy mở ngàm ra -> nhả vật thể rơi xuống mặt sàn
            isObjectGrasped = false;
            targetObject->localPos.y = 1.5f; // Đặt nằm trên mặt sàn
        } else {
            // Vật thể bám theo tâm ngàm
            targetObject->localPos = gripWorldPos;
        }
    }
}

void RobotCarScene::update(float dt) {
    // 1. Cập nhật cảm biến siêu âm HCSR04
    if (compHcsr04 && targetObject) {
        Mat4 hcsrWorldM = compHcsr04->getWorldMatrix();
        Vec3 sensorPos = hcsrWorldM.transformPoint(Vec3(0, 0, 0));
        Vec3 forwardDir = hcsrWorldM.transformVector(Vec3(0, 0, 1)).normalized();
        compHcsr04->updateRaycast(sensorPos, forwardDir, targetObject->localPos, 1.6f);
    }

    // 2. Chạy bước mô phỏng mạch điện
    circuit.stepCircuit(dt);

    // 3. Động học vi sai xe
    updateDifferentialDrive(dt);

    // 4. Động học cánh tay và gắp vật
    updateArmAndGrasping(dt);

    // 5. Cập nhật cây đồ thị phân cảnh
    carRoot->update(dt);
    targetObject->update(dt);
}

void RobotCarScene::render(bool wireframeMode) {
    // 1. Vẽ mặt sàn lưới Grid (sân đấu / mặt bàn thực nghiệm)
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);
    glColor3f(0.22f, 0.24f, 0.28f);

    float gridSize = 80.0f;
    float step = 10.0f;
    glBegin(GL_LINES);
    for (float i = -gridSize; i <= gridSize; i += step) {
        glVertex3f(i, -3.3f, -gridSize);
        glVertex3f(i, -3.3f,  gridSize);
        glVertex3f(-gridSize, -3.3f, i);
        glVertex3f( gridSize, -3.3f, i);
    }
    glEnd();

    // 2. Vẽ xe robot và 11 linh kiện theo cây SceneNode
    carRoot->render(wireframeMode);

    // 3. Vẽ các đường dây điện 3D
    circuit.render3DWires();

    // 4. Vẽ vật thể cần gắp
    targetObject->render(wireframeMode);

    // 5. Vẽ tia phát sóng siêu âm HC-SR04 trực quan
    if (compHcsr04 && compHcsr04->isPowered) {
        Mat4 hcsrM = compHcsr04->getWorldMatrix();
        Vec3 sensorPos = hcsrM.transformPoint(Vec3(0, 0, 0));
        Vec3 forward = hcsrM.transformVector(Vec3(0, 0, 1)).normalized();

        float beamLen = compHcsr04->measuredDistanceCm;
        Vec3 endBeam = sensorPos + forward * beamLen;

        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);
        if (compHcsr04->obstacleDetected) {
            glColor3f(1.0f, 0.2f, 0.2f); // Đỏ cảnh báo vật cản gần
        } else {
            glColor3f(0.2f, 0.8f, 1.0f); // Xanh dương bình thường
        }

        glBegin(GL_LINES);
        glVertex3f(sensorPos.x, sensorPos.y, sensorPos.z);
        glVertex3f(endBeam.x, endBeam.y, endBeam.z);
        glEnd();

        // Vẽ vòng tròn nón quét ở đầu chùm tia
        glPushMatrix();
        glTranslatef(endBeam.x, endBeam.y, endBeam.z);
        glutWireTorus(0.08f, beamLen * 0.15f, 6, 12);
        glPopMatrix();
    }
}

void RobotCarScene::resetSimulation() {
    carPosition = Vec3(0.0f, 0.0f, 0.0f);
    carYawRad = 0.0f;
    carRoot->localPos = carPosition;
    carRoot->localRot.y = 0.0f;

    isObjectGrasped = false;
    targetObject->localPos = initialObjectPos;

    if (compArd) compArd->resetAutoSequence();
    if (compBatSw) compBatSw->switchOn = true;
    if (compBuck) compBuck->targetOutVoltage = 5.0f;

    if (compSrvBase)  compSrvBase->setTargetAngle(90.0f);
    if (compSrvElbow) compSrvElbow->setTargetAngle(90.0f);
    if (compSrvGrip)  compSrvGrip->setTargetAngle(45.0f);
}

HardwareComponent* RobotCarScene::pickComponent(const Vec3& rayOrigin, const Vec3& rayDir) {
    float closestDist = 1e9f;
    SceneNode* hitNode = carRoot->raycast(rayOrigin, rayDir, closestDist);

    if (selectedComponent) {
        selectedComponent->isSelected = false;
        selectedComponent = nullptr;
    }

    if (hitNode) {
        // Tìm xem nút được chọn hoặc tổ tiên của nó có phải là HardwareComponent không
        SceneNode* cur = hitNode;
        while (cur) {
            auto* comp = dynamic_cast<HardwareComponent*>(cur);
            if (comp) {
                selectedComponent = comp;
                selectedComponent->isSelected = true;
                return selectedComponent;
            }
            cur = cur->parent;
        }
    }
    return nullptr;
}
