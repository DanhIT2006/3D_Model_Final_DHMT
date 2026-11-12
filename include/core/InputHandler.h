#pragma once
#include "scene/RobotCarScene.h"
#include "math/Camera3D.h"
#include "core/HudDashboard.h"
#include <map>

/**
 * @brief Bộ điều phối bàn phím và chuột cho toàn bộ chương trình mô phỏng
 */
class InputHandler {
public:
    std::map<unsigned char, bool> keyStates;
    bool wireframeMode = false;

public:
    InputHandler() = default;

    void onKeyDown(unsigned char key, int x, int y, RobotCarScene& scene, Camera3D& camera, HudDashboard& hud);
    void onKeyUp(unsigned char key, int x, int y, RobotCarScene& scene);
    void onSpecialKeyDown(int key, int x, int y);
    void onSpecialKeyUp(int key, int x, int y);

    void updateContinuousInput(float dt, RobotCarScene& scene);

    void onMouseClick(int button, int state, int x, int y, int screenW, int screenH,
                     RobotCarScene& scene, Camera3D& camera, HudDashboard& hud);
};
