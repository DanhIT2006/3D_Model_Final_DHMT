#pragma once
#include "scene/RobotCarScene.h"
#include "math/Camera3D.h"
#include "graphics2d/Raster2D.h"
#include "graphics2d/Clipper2D.h"
#include "graphics2d/FontRenderer.h"

/**
 * @brief Bảng điều khiển giao diện 2D HUD (Heads-Up Display)
 * Áp dụng đầy đủ các thuật toán CG.3: Bresenham, Midpoint, Scanline, Cohen-Sutherland
 */
class HudDashboard {
public:
    bool showHelp = false;
    float radarSweepAngle = 0.0f;
    float animTimer = 0.0f;

    // Nút bấm tương tác trên HUD
    struct HudButton {
        int x, y, w, h;
        std::string label;
        int actionId;
    };
    std::vector<HudButton> buttons;

public:
    HudDashboard();

    void update(float dt);
    void render(int screenW, int screenH, const RobotCarScene& scene, const Camera3D& camera, float fps);

    // Xử lý click chuột vào các nút trên HUD
    bool onMouseClick(int mouseX, int mouseY, RobotCarScene& scene);

private:
    void renderTopBar(int screenW, int screenH, const RobotCarScene& scene, const Camera3D& camera, float fps);
    void renderRadarMiniMap(int x, int y, int size, const RobotCarScene& scene);
    void renderComponentInspector(int x, int y, int w, int h, const RobotCarScene& scene);
    void renderOscilloscopeAndGauges(int x, int y, int w, int h, const RobotCarScene& scene);
    void renderHelpDialog(int screenW, int screenH);
};
