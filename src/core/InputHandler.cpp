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
            scene.circuit.showWires = !scene.circuit.showWires;
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

        case ' ': // Phanh dừng
            if (scene.compArd) {
                scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 0;
                scene.compArd->motorDirL = scene.compArd->motorDirR = 0;
            }
            break;

        case 27: // Phím Escape -> Thoát chương trình
            exit(0);
            break;
    }
}

void InputHandler::onKeyUp(unsigned char key, int x, int y, RobotCarScene& scene) {
    (void)x; (void)y; (void)scene;
    keyStates[key] = false;
}

void InputHandler::onSpecialKeyDown(int key, int x, int y) {
    (void)key; (void)x; (void)y;
}

void InputHandler::onSpecialKeyUp(int key, int x, int y) {
    (void)key; (void)x; (void)y;
}

void InputHandler::updateContinuousInput(float dt, RobotCarScene& scene) {
    if (!scene.compArd || scene.compArd->controlMode != RobotControlMode::MANUAL) {
        return;
    }

    auto isDown = [&](char c) -> bool {
        return keyStates[c] || keyStates[std::toupper(c)] || keyStates[std::tolower(c)];
    };

    // Điều khiển động cơ xe (WASD)
    int spd = 200;
    if (isDown('w')) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = spd;
        scene.compArd->motorDirL = scene.compArd->motorDirR = 1;
    } else if (isDown('s')) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = spd;
        scene.compArd->motorDirL = scene.compArd->motorDirR = -1;
    } else if (isDown('a')) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 180;
        scene.compArd->motorDirL = -1;
        scene.compArd->motorDirR = 1;
    } else if (isDown('d')) {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 180;
        scene.compArd->motorDirL = 1;
        scene.compArd->motorDirR = -1;
    } else {
        scene.compArd->motorSpeedL = scene.compArd->motorSpeedR = 0;
        scene.compArd->motorDirL = scene.compArd->motorDirR = 0;
    }

    // Điều khiển cơ cấu nâng hạ thẳng đứng (I / K) và ngàm kẹp (O / P)
    float liftSpeed = 80.0f;
    if (isDown('i')) scene.compArd->elbowAngleDeg = std::min(180.0f, scene.compArd->elbowAngleDeg + liftSpeed * dt);
    if (isDown('k')) scene.compArd->elbowAngleDeg = std::max(0.0f,   scene.compArd->elbowAngleDeg - liftSpeed * dt);

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
        if (hud.onMouseClick(x, hudY, scene)) {
            return; // Đã click vào nút trên HUD
        }

        // Bắn tia Raycast chọn linh kiện 3D
        Vec3 rayOrigin, rayDir;
        camera.screenToRay(x, y, rayOrigin, rayDir);
        scene.pickComponent(rayOrigin, rayDir);
    }

    // Chuyển tiếp sự kiện chuột sang Camera để xoay/pan
    camera.onMouseDown(button, state, x, y);
}
