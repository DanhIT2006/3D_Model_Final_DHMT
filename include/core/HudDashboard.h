#pragma once
#include <GL/freeglut.h>
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
    bool onMouseClick(int mouseX, int mouseY, RobotCarScene& scene, Camera3D& camera);

private:
    void renderTopBar(int screenW, int screenH, const RobotCarScene& scene, const Camera3D& camera, float fps);
    void renderRadarMiniMap(int x, int y, int size, const RobotCarScene& scene);
    void renderComponentInspector(int x, int y, int w, int h, const RobotCarScene& scene);
    void renderOscilloscopeAndGauges(int x, int y, int w, int h, const RobotCarScene& scene);
    void renderHelpDialog(int screenW, int screenH);

    // Chế độ quan sát vi mô linh kiện
    void renderWorkbenchHUD(int screenW, int screenH, RobotCarScene& scene, const Camera3D& camera);

    // Lưu ma trận 3D phục vụ phép chiếu 3D sang 2D (gluProject) vẽ Callout Line
    double lastModelView[16];
    double lastProjection[16];
    int    lastViewport[4];

    // Chiếu tọa độ 3D thành tọa độ pixel 2D trên màn hình HUD
    bool project3DToScreen(const Vec3& pos3D, int& outScreenX, int& outScreenY) const;
};
