#include "scene/RobotCarScene.h"
#include "scene/RoboticArmRenderer.h"
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
    // 1. Nut goc xe tai tam san Tang 1
    carRoot = new SceneNode("CAR_ROOT", "Xe Robot Gap Do Vat 4WD");
    carPosition = Vec3(0.0f, 0.0f, 0.0f);
    carYawRad = 0.0f;

    // 2. San Tang 1 (Chassis Floor 1)
    chassisFloor1 = new SceneNode("FLOOR_1", "San Khung Gam Tang 1");
    chassisFloor1->localPos = Vec3(0.0f, 0.0f, 0.0f);
    // Tam mica den bong cat laser cao cap 16.5 x 0.35 x 25.0 cm
    chassisFloor1->mesh = Mesh3D::createBox(16.5f, 0.35f, 25.0f, Vec3(0.14f, 0.16f, 0.22f));
    carRoot->addChild(chassisFloor1);

    // 2b. Thanh xe bao ve hai ben suon (Chassis Side Guard Panels / Fenders)
    chassisSideLeft = new SceneNode("SIDE_L", "Thanh Xe Ben Trai");
    chassisSideLeft->localPos = Vec3(-7.4f, 3.0f, 0.0f);
    chassisSideLeft->mesh = Mesh3D::createBox(0.35f, 5.6f, 18.0f, Vec3(0.18f, 0.22f, 0.28f));
    chassisFloor1->addChild(chassisSideLeft);

    chassisSideRight = new SceneNode("SIDE_R", "Thanh Xe Ben Phai");
    chassisSideRight->localPos = Vec3(7.4f, 3.0f, 0.0f);
    chassisSideRight->mesh = Mesh3D::createBox(0.35f, 5.6f, 18.0f, Vec3(0.18f, 0.22f, 0.28f));
    chassisFloor1->addChild(chassisSideRight);

    // 2c. Thanh can truoc & can sau xe (Front & Rear Impact Bumpers)
    frontBumper = new SceneNode("BUMP_F", "Thanh Can Truoc AGV");
    frontBumper->localPos = Vec3(0.0f, 0.2f, 12.8f);
    frontBumper->mesh = Mesh3D::createBox(14.0f, 0.7f, 1.2f, Vec3(0.20f, 0.22f, 0.26f));
    chassisFloor1->addChild(frontBumper);

    rearBumper = new SceneNode("BUMP_R", "Thanh Can Sau AGV");
    rearBumper->localPos = Vec3(0.0f, 0.2f, -12.4f);
    rearBumper->mesh = Mesh3D::createBox(14.0f, 0.7f, 1.2f, Vec3(0.20f, 0.22f, 0.26f));
    chassisFloor1->addChild(rearBumper);

    // 4 Cot tru dong luc giac ket noi Tang 1 len Tang 2 (cao 6.0 cm)
    for (float x : {-6.5f, 6.5f}) {
        for (float z : {-9.5f, 9.5f}) {
            SceneNode* pillar = new SceneNode("PILLAR", "Tru Dong Luc Giac Tang 1-2");
            pillar->localPos = Vec3(x, 3.0f, z);
            pillar->mesh = Mesh3D::createCylinder(0.32f, 6.0f, 6, Vec3(0.85f, 0.72f, 0.25f));
            chassisFloor1->addChild(pillar);
        }
    }

    // 3. He thong 4 Dong co DC giam toc TT Motor & 4 Banh xe cao su (4WD Chassis)
    compMotFL = new DCMotorComp("MOT_FL", "Dong Co Truoc Trai", true);
    compMotFL->localPos = Vec3(-7.8f, -1.0f, 5.0f);
    chassisFloor1->addChild(compMotFL);

    compMotRL = new DCMotorComp("MOT_RL", "Dong Co Sau Trai", true);
    compMotRL->localPos = Vec3(-7.8f, -1.0f, -5.0f);
    chassisFloor1->addChild(compMotRL);

    compMotFR = new DCMotorComp("MOT_FR", "Dong Co Truoc Phai", false);
    compMotFR->localPos = Vec3(7.8f, -1.0f, 5.0f);
    chassisFloor1->addChild(compMotFR);

    compMotRR = new DCMotorComp("MOT_RR", "Dong Co Sau Phai", false);
    compMotRR->localPos = Vec3(7.8f, -1.0f, -5.0f);
    chassisFloor1->addChild(compMotRR);

    compMotL = compMotFL;
    compMotR = compMotFR;

    // Khoi tao cac linh kien tren Tang 1
    compDomino = new DominoBusComp();
    compDomino->localPos = Vec3(0.0f, 0.75f, 0.0f);
    chassisFloor1->addChild(compDomino);

    compL298N = new L298NDriverComp();
    compL298N->localPos = Vec3(-2.5f, 0.8f, -7.0f);
    chassisFloor1->addChild(compL298N);

    compBuck = new BuckLM2596Comp();
    compBuck->localPos = Vec3(3.0f, 0.7f, -7.0f);
    chassisFloor1->addChild(compBuck);

    compHcsr04 = new UltrasonicComp();
    compHcsr04->localPos = Vec3(0.0f, 1.10f, 11.20f);
    chassisFloor1->addChild(compHcsr04);

    // 4. San Tang 2 (Chassis Floor 2) cach Tang 1 delta Y = 6.0 cm (Phia truoc khuyet ngan hon de tay gap khong va cham)
    chassisFloor2 = new SceneNode("FLOOR_2", "San Khung Gam Tang 2");
    chassisFloor2->localPos = Vec3(0.0f, 6.0f, 0.0f);
    chassisFloor2->mesh = Mesh3D::createBox(15.0f, 0.35f, 21.0f, Vec3(0.08f, 0.09f, 0.12f));
    carRoot->addChild(chassisFloor2);

    // 5. Khoi tao va lap rap linh kien tren Tang 2
    compArd = new ArduinoUnoComp();
    compArd->localPos = Vec3(-3.5f, 0.75f, 0.0f);
    chassisFloor2->addChild(compArd);

    compBatSw = new BatterySwitchComp();
    compBatSw->localPos = Vec3(0.0f, 1.1f, -8.5f);
    chassisFloor2->addChild(compBatSw);

    // 6. Canh tay robot 3 bac tu do co khi chuan (Base Yaw -> Twin-Boom Pitch -> Gear Gripper)
    compSrvBase = new ServoMG996RComp("SRV_BASE", "Khop De Xoay (Base Yaw)", ServoJointType::BASE_YAW);
    compSrvBase->localPos = Vec3(0.0f, 0.35f, 4.60f);
    chassisFloor2->addChild(compSrvBase);

    compSrvElbow = new ServoMG996RComp("SRV_ELBOW", "Khop Khuyu Tay Doi (Twin-Boom Pitch)", ServoJointType::LINEAR_LIFT);
    compSrvElbow->localPos = Vec3(0.0f, 0.0f, 0.0f);
    compSrvBase->addChild(compSrvElbow);

    compSrvGrip = new ServoMG996RComp("SRV_GRIP", "Cum Ngam Kep Banh Rang (Gripper)", ServoJointType::GRIPPER);
    compSrvGrip->localPos = Vec3(0.0f, 0.0f, 0.0f);
    compSrvElbow->addChild(compSrvGrip);

    // 7. Khoi vat the mau can gap (Target Cargo Crate)
    targetObject = new SceneNode("TARGET_OBJ", "Vat The Can Gap");
    targetObject->localPos = initialObjectPos;
    targetObject->mesh = Mesh3D::createBox(3.4f, 3.2f, 3.4f, Vec3(0.95f, 0.40f, 0.12f));

    // 8. Dang ky toan bo cac linh kien vao bo giai mach dien CircuitSimulator
    circuit.registerComponent(compBatSw);
    circuit.registerComponent(compDomino);
    circuit.registerComponent(compBuck);
    circuit.registerComponent(compArd);
    circuit.registerComponent(compL298N);
    circuit.registerComponent(compHcsr04);
    circuit.registerComponent(compMotFL);
    circuit.registerComponent(compMotRL);
    circuit.registerComponent(compMotFR);
    circuit.registerComponent(compMotRR);
    circuit.registerComponent(compSrvBase);
    circuit.registerComponent(compSrvElbow);
    circuit.registerComponent(compSrvGrip);

    circuit.setupStandardCarCircuit();
}

void RobotCarScene::updateDifferentialDrive(float dt) {
    if (!compMotFL || !compMotFR) return;

    // Van toc tiep tuyen 2 banh truoc (cm/s)
    float vL = compMotFL->linearSpeedCmS;
    float vR = compMotFR->linearSpeedCmS;

    // Cong thuc dong hoc vi sai (Differential Drive Kinematics)
    float v = (vR + vL) * 0.5f;
    float omega = (vR - vL) / wheelbaseWidth;

    carYawRad += omega * dt;

    carPosition.x += v * std::sin(carYawRad) * dt;
    carPosition.z += v * std::cos(carYawRad) * dt;

    // Dong bo 2 banh sau cung toc do va goc quay voi 2 banh truoc (4WD)
    if (compMotRL) {
        compMotRL->linearSpeedCmS = compMotFL->linearSpeedCmS;
        compMotRL->rpm = compMotFL->rpm;
        compMotRL->wheelAngle = compMotFL->wheelAngle;
        compMotRL->isPowered = compMotFL->isPowered;
        compMotRL->statusDesc = compMotFL->statusDesc;
    }
    if (compMotRR) {
        compMotRR->linearSpeedCmS = compMotFR->linearSpeedCmS;
        compMotRR->rpm = compMotFR->rpm;
        compMotRR->wheelAngle = compMotFR->wheelAngle;
        compMotRR->isPowered = compMotFR->isPowered;
        compMotRR->statusDesc = compMotFR->statusDesc;
    }

    carRoot->localPos = carPosition;
    carRoot->localRot.y = carYawRad;
}

void RobotCarScene::updateArmAndGrasping(float dt) {
    if (!compSrvGrip || !targetObject) return;

    float yawDeg = compSrvBase ? (compSrvBase->currentAngleDeg - 90.0f) : 0.0f;
    float liftProgress = compSrvElbow ? ((140.0f - compSrvElbow->currentAngleDeg) / 140.0f) : 0.0f;
    if (liftProgress < 0.0f) liftProgress = 0.0f;
    if (liftProgress > 1.0f) liftProgress = 1.0f;

    float shoulderUpDeg = 58.0f - liftProgress * 30.0f;
    float forearmDownDeg = 35.0f + liftProgress * 48.0f;

    // Tọa độ mâm xoay trên sàn tầng 2 trong không gian thế giới (z = +4.60cm trên xe)
    float totalYawRad = carYawRad + (yawDeg * 3.14159265f / 180.0f);
    Vec3 basePos = carPosition + Vec3(4.60f * std::sin(carYawRad), 6.35f, 4.60f * std::cos(carYawRad));

    const float L1 = 6.80f;
    const float L2 = 6.50f;
    const float L3 = 3.80f;

    float radShoulder = shoulderUpDeg * 3.14159265f / 180.0f;
    float dY1 = L1 * std::sin(radShoulder);
    float dZ1 = L1 * std::cos(radShoulder);

    float radForearmNet = (shoulderUpDeg - forearmDownDeg) * 3.14159265f / 180.0f;
    float dY2 = L2 * std::sin(radForearmNet);
    float dZ2 = L2 * std::cos(radForearmNet);

    float radClaw = 35.0f * 3.14159265f / 180.0f;
    float dY3 = -L3 * std::sin(radClaw);
    float dZ3 =  L3 * std::cos(radClaw);

    float horizDist = dZ1 + dZ2 + dZ3;
    float gripY = std::max(1.5f, basePos.y + 3.00f + dY1 + dY2 + dY3);
    float gripX = basePos.x + horizDist * std::sin(totalYawRad);
    float gripZ = basePos.z + horizDist * std::cos(totalYawRad);

    gripWorldPos = Vec3(gripX, gripY, gripZ);

    bool shouldGrasp = false;
    float distToObj = gripWorldPos.distanceTo(targetObject->localPos);
    float measuredDist = distToObj;

    // Cap nhat firmware Arduino
    if (compArd) {
        compArd->updateFirmware(dt, measuredDist, shouldGrasp, isObjectGrasped);

        if (compSrvBase)  compSrvBase->setTargetAngle(compArd->baseAngleDeg);
        if (compSrvElbow) compSrvElbow->setTargetAngle(compArd->elbowAngleDeg);
        if (compSrvGrip)  compSrvGrip->setTargetAngle(compArd->gripAngleDeg);
    }

    float distXZ = std::hypot(gripWorldPos.x - targetObject->localPos.x, gripWorldPos.z - targetObject->localPos.z);
    bool inGraspZone = (distXZ < 2.8f && std::abs(gripWorldPos.y - targetObject->localPos.y) < 2.5f);

    // Kiem tra dieu kien gap vat
    if (!isObjectGrasped) {
        if ((shouldGrasp || compSrvGrip->currentAngleDeg < 35.0f) && inGraspZone) {
            isObjectGrasped = true;
        }
    }

    if (isObjectGrasped) {
        if (compSrvGrip->currentAngleDeg > 55.0f) {
            isObjectGrasped = false;
            targetObject->localPos.y = 1.6f;
        } else {
            targetObject->localPos = gripWorldPos + Vec3(0.0f, -0.4f, 0.0f);
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

    // 2. Cập nhật động học cánh tay và firmware Arduino (tính toán lệnh PWM trước khi mô phỏng mạch)
    updateArmAndGrasping(dt);

    // 3. Chạy bước mô phỏng mạch điện (Truyền tải tín hiệu Arduino -> L298N -> Động cơ DC)
    circuit.stepCircuit(dt);

    // 4. Động học vi sai xe (Cập nhật vị trí và góc quay xe theo vận tốc động cơ)
    updateDifferentialDrive(dt);

    // 5. Cập nhật cây đồ thị phân cảnh
    carRoot->update(dt);
    targetObject->update(dt);
}

void RobotCarScene::render(bool wireframeMode) {
    // Nếu đang ở chế độ quan sát vi mô: chỉ vẽ bàn làm việc và linh kiện được cô lập
    if (isWorkbenchMode && inspectedComponent) {
        renderWorkbenchScene(wireframeMode);
        return;
    }

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

    // 4. Vẽ vật thể cần gắp & Vòng tròn tâm ngắm chiếu xuống sàn
    RoboticArmRenderer::renderCargoCrate(targetObject->localPos, 3.4f, 3.2f, 3.4f, isObjectGrasped);
    RoboticArmRenderer::renderDropTargetReticle(gripWorldPos, targetObject->localPos, isObjectGrasped);

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

    if (compMotFL) compMotFL->wheelAngle = 0.0f;
    if (compMotRL) compMotRL->wheelAngle = 0.0f;
    if (compMotFR) compMotFR->wheelAngle = 0.0f;
    if (compMotRR) compMotRR->wheelAngle = 0.0f;
}

void RobotCarScene::enterWorkbench(HardwareComponent* comp) {
    if (!comp) return;
    inspectedComponent = comp;
    isWorkbenchMode = true;
    if (inspectedComponent->selectedSubCompIndex == -1 && !inspectedComponent->subComponents.empty()) {
        inspectedComponent->selectSubComponentByIndex(0);
    }
}

void RobotCarScene::exitWorkbench() {
    isWorkbenchMode = false;
    inspectedComponent = nullptr;
}

void RobotCarScene::renderWorkbenchScene(bool wireframeMode) {
    glDisable(GL_LIGHTING);

    // 1. Dia xoay ky thuat ban trong suot (Holo-Turntable Grid)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.04f, 0.12f, 0.22f, 0.35f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, -0.01f, 0.0f);
    for (int i = 0; i <= 64; ++i) {
        float a = i * 2.0f * 3.14159265f / 64.0f;
        glVertex3f(8.8f * std::cos(a), -0.01f, 8.8f * std::sin(a));
    }
    glEnd();
    glDisable(GL_BLEND);

    // Vong tron dong tam
    glLineWidth(1.4f);
    for (float r : {2.0f, 4.0f, 6.0f, 8.5f}) {
        glColor3f(0.18f, 0.45f, 0.68f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 64; ++i) {
            float a = i * 2.0f * 3.14159265f / 64.0f;
            glVertex3f(r * std::cos(a), 0.0f, r * std::sin(a));
        }
        glEnd();
    }

    // Vach chia do tren vanh ngoai R = 8.5cm (0 - 360 do)
    glColor3f(0.20f, 0.65f, 0.85f);
    glBegin(GL_LINES);
    for (int deg = 0; deg < 360; deg += 5) {
        float rad = deg * 3.14159265f / 180.0f;
        float rInner = (deg % 30 == 0) ? 7.8f : ((deg % 15 == 0) ? 8.05f : 8.25f);
        glVertex3f(rInner * std::cos(rad), 0.0f, rInner * std::sin(rad));
        glVertex3f(8.5f   * std::cos(rad), 0.0f, 8.5f   * std::sin(rad));
    }
    glEnd();

    // Cac nan hoa huong tam 45 do
    glColor3f(0.14f, 0.30f, 0.42f);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; ++i) {
        float a = i * 2.0f * 3.14159265f / 8.0f;
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(8.5f * std::cos(a), 0.0f, 8.5f * std::sin(a));
    }
    glEnd();

    // Luoi vuong ky thuat ti le 1.0cm (-7cm den +7cm)
    glColor3f(0.10f, 0.22f, 0.32f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float v = -7.0f; v <= 7.0f; v += 1.0f) {
        glVertex3f(v, 0.0f, -7.0f);
        glVertex3f(v, 0.0f,  7.0f);
        glVertex3f(-7.0f, 0.0f, v);
        glVertex3f( 7.0f, 0.0f, v);
    }
    glEnd();

    // 2. He truc toa do 3D (RGB Axes)
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    // Truc X (Do)
    glColor3f(1.0f, 0.2f, 0.2f);
    glVertex3f(-6.0f, 0.05f, 6.0f); glVertex3f(-4.5f, 0.05f, 6.0f);
    // Truc Y (Xanh la)
    glColor3f(0.2f, 1.0f, 0.2f);
    glVertex3f(-6.0f, 0.05f, 6.0f); glVertex3f(-6.0f, 1.55f, 6.0f);
    // Truc Z (Xanh duong)
    glColor3f(0.2f, 0.5f, 1.0f);
    glVertex3f(-6.0f, 0.05f, 6.0f); glVertex3f(-6.0f, 0.05f, 4.5f);
    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_LIGHTING);

    // 3. Ket xuat linh kien duoc co lap
    if (inspectedComponent) {
        inspectedComponent->renderWorkbench(wireframeMode, isXrayMode);
    }
}

SubComponent* RobotCarScene::pickSubComponent(const Vec3& rayOrigin, const Vec3& rayDir) {
    if (isWorkbenchMode && inspectedComponent) {
        return inspectedComponent->pickSubComponent(rayOrigin, rayDir);
    }
    return nullptr;
}

HardwareComponent* RobotCarScene::pickComponent(const Vec3& rayOrigin, const Vec3& rayDir) {
    if (isWorkbenchMode && inspectedComponent) {
        inspectedComponent->pickSubComponent(rayOrigin, rayDir);
        return inspectedComponent;
    }

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
