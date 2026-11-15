#include "core/InputHandler.h"
#include <GL/freeglut.h>
#include <algorithm>

void InputHandler::onKeyDown(unsigned char key, int x, int y, RobotCarScene& scene, Camera3D& camera, HudDashboard& hud) {
    (void)x; (void)y;
    keyStates[key] = true;

    switch (key) {
        case '1': camera.setViewMode(ViewMode::PERSPECTIVE_ORBIT); break;
        case '2': camera.setViewMode(ViewMode::TOP_VIEW); break;
        case '3': camera.setViewMode(ViewMode::FRONT_VIEW); break;
        case '4': camera.setViewMode(ViewMode::SIDE_VIEW); break;
        case '5': camera.setViewMode(ViewMode::CAR_FOLLOW); break;

        case 'f':
        case 'F':
            wireframeMode = !wireframeMode;
            break;

        case 'c':
        case 'C':
            hud.showClawCam = !hud.showClawCam;
            break;

        case 'x':
        case 'X':
            if (scene.isWorkbenchMode) {
                scene.isXrayMode = !scene.isXrayMode;
                hud.xrayMode = scene.isXrayMode;
            }
            break;

        case 'g':
        case 'G':
            if (scene.isObjectGrasped) {
                scene.isObjectGrasped = false;
                scene.targetObject->localPos.y = 1.6f;
            } else {
                float distXZ = std::hypot(scene.gripWorldPos.x - scene.targetObject->localPos.x,
                                          scene.gripWorldPos.z - scene.targetObject->localPos.z);
                if (distXZ < 3.8f) {
                    scene.isObjectGrasped = true;
                }
            }
            break;

        case 'h':
        case 'H':
            hud.showHelp = !hud.showHelp;
            break;

        case 't':
        case 'T':
            if (scene.compBatSw) scene.compBatSw->toggleSwitch();
            break;

        case 'm':
        case 'M':
            if (scene.compArd) scene.compArd->toggleControlMode();
            break;

        case 'r':
        case 'R':
            scene.resetSimulation();
            break;

        case '+':
        case '=':
            if (scene.compBuck) scene.compBuck->adjustVoltage(0.2f);
            break;

        case '-':
        case '_':
            if (scene.compBuck) scene.compBuck->adjustVoltage(-0.2f);
            break;

        case ' ': // Phanh dừng hoặc Tương tác linh kiện
            if (scene.isWorkbenchMode && scene.inspectedComponent) {
                SubComponent* sub = scene.inspectedComponent->getSelectedSubComponent();
                if (sub && sub->isInteractive) {
                    scene.inspectedComponent->onSubComponentAction(sub->id);
                }
            } else if (scene.compArd) {
                scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 0;
                scene.compArd->motorDirL = scene.compArd->motorDirR = 0;
            }
            break;

        case 'q':
        case 'Q':
            if (scene.isWorkbenchMode) {
                scene.exitWorkbench();
                camera.exitWorkbenchMode();
            } else {
                HardwareComponent* targetComp = scene.selectedComponent;
                if (!targetComp) {
                    targetComp = scene.compL298N ? (HardwareComponent*)scene.compL298N : (HardwareComponent*)scene.compArd;
                    if (targetComp) {
                        scene.selectedComponent = targetComp;
                        targetComp->isSelected = true;
                    }
                }
                if (targetComp) {
                    scene.enterWorkbench(targetComp);
                    camera.enterWorkbenchMode();
                }
            }
            break;

        case '[':
            if (scene.isWorkbenchMode && scene.inspectedComponent) {
                scene.inspectedComponent->selectPrevSubComponent();
            }
            break;

        case ']':
        case '\t':
            if (scene.isWorkbenchMode && scene.inspectedComponent) {
                scene.inspectedComponent->selectNextSubComponent();
            }
            break;

        case 13: // Enter
            if (scene.isWorkbenchMode && scene.inspectedComponent) {
                SubComponent* sub = scene.inspectedComponent->getSelectedSubComponent();
                if (sub && sub->isInteractive) {
                    scene.inspectedComponent->onSubComponentAction(sub->id);
                }
            } else if (scene.selectedComponent) {
                scene.enterWorkbench(scene.selectedComponent);
                camera.enterWorkbenchMode();
            }
            break;

        case 27: // Phím Escape -> Thoát chế độ quan sát hoặc thoát chương trình
            if (scene.isWorkbenchMode) {
                scene.exitWorkbench();
                camera.exitWorkbenchMode();
            } else {
                exit(0);
            }
            break;
    }
}

void InputHandler::onKeyUp(unsigned char key, int x, int y, RobotCarScene& scene) {
    (void)x; (void)y; (void)scene;
    keyStates[key] = false;
    keyStates[std::tolower(key)] = false;
    keyStates[std::toupper(key)] = false;
}

void InputHandler::onSpecialKeyDown(int key, int x, int y) {
    (void)x; (void)y;
    specialKeyStates[key] = true;
}

void InputHandler::onSpecialKeyUp(int key, int x, int y) {
    (void)x; (void)y;
    specialKeyStates[key] = false;
}

void InputHandler::updateContinuousInput(float dt, RobotCarScene& scene) {
    // Đóng băng chuyển động khi đang ở chế độ soi chi tiết bo mạch (Workbench)
    if (scene.isWorkbenchMode) {
        return;
    }

    if (!scene.compArd || scene.compArd->controlMode != RobotControlMode::MANUAL) {
        return;
    }

    auto isDown = [&](char c) -> bool {
        return keyStates[c] || keyStates[std::toupper(c)] || keyStates[std::tolower(c)];
    };

    bool fwd   = isDown('w') || specialKeyStates[GLUT_KEY_UP];
    bool bwd   = isDown('s') || specialKeyStates[GLUT_KEY_DOWN];
    bool left  = isDown('a') || specialKeyStates[GLUT_KEY_LEFT];
    bool right = isDown('d') || specialKeyStates[GLUT_KEY_RIGHT];

    // Điều khiển động cơ xe (WASD hoặc Phím mũi tên)
    int spd = 200;
    if (fwd) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = spd;
        scene.compArd->motorDirL = scene.compArd->motorDirR = 1;
    } else if (bwd) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = spd;
        scene.compArd->motorDirL = scene.compArd->motorDirR = -1;
    } else if (left) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 180;
        scene.compArd->motorDirL = -1;
        scene.compArd->motorDirR = 1;
    } else if (right) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 180;
        scene.compArd->motorDirL = 1;
        scene.compArd->motorDirR = -1;
    } else {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 0;
        scene.compArd->motorDirL = scene.compArd->motorDirR = 0;
    }

    // Điều khiển cơ cấu nâng hạ cánh tay (I / K) và ngàm kẹp (O / P)
    float liftSpeed = 80.0f;
    if (isDown('i')) scene.compArd->elbowAngleDeg = std::min(90.0f, scene.compArd->elbowAngleDeg + liftSpeed * dt);
    if (isDown('k')) scene.compArd->elbowAngleDeg = std::max(0.0f,  scene.compArd->elbowAngleDeg - liftSpeed * dt);

    if (isDown('j')) scene.compArd->baseAngleDeg  = std::min(180.0f, scene.compArd->baseAngleDeg + 50.0f * dt);
    if (isDown('l')) scene.compArd->baseAngleDeg  = std::max(0.0f,   scene.compArd->baseAngleDeg - 50.0f * dt);

    if (isDown('o')) scene.compArd->gripAngleDeg  = std::min(90.0f,  scene.compArd->gripAngleDeg + 80.0f * dt);
    if (isDown('p')) scene.compArd->gripAngleDeg  = std::max(10.0f,  scene.compArd->gripAngleDeg - 80.0f * dt);
}

void InputHandler::onMouseClick(int button, int state, int x, int y, int screenW, int screenH,
                                RobotCarScene& scene, Camera3D& camera, HudDashboard& hud) {
    if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        // Tọa độ HUD: gốc (0, 0) ở góc dưới-trái
        int hudY = screenH - y;
        if (hud.onMouseClick(x, hudY, scene, camera)) {
            return; // Đã click vào nút trên HUD
        }

        // Bắn tia Raycast chọn linh kiện 3D hoặc linh kiện con trong Workbench
        Vec3 rayOrigin, rayDir;
        camera.screenToRay(x, y, rayOrigin, rayDir);
        if (scene.isWorkbenchMode) {
            scene.pickSubComponent(rayOrigin, rayDir);
        } else {
            scene.pickComponent(rayOrigin, rayDir);
        }
    }

    // Chuyển tiếp sự kiện chuột sang Camera để xoay/pan
    camera.onMouseDown(button, state, x, y);
}
