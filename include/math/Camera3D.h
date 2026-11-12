#pragma once
#include "math/Vec3.h"
#include "math/Mat4.h"

enum class ViewMode {
    PERSPECTIVE_ORBIT, // Góc nhìn phối cảnh tự do (xoay cầu theta, phi, R, D)
    TOP_VIEW,          // Chiếu bằng kỹ thuật (nhìn thẳng từ trên xuống)
    FRONT_VIEW,        // Chiếu đứng kỹ thuật (nhìn từ trước vào mũi xe)
    SIDE_VIEW,         // Chiếu cạnh kỹ thuật (nhìn từ bên sườn xe)
    CAR_FOLLOW         // Bám theo xe từ góc nhìn thứ 3 (chase cam)
};

/**
 * @brief Quản lý Camera quan sát theo tọa độ cầu (theta, Phi, R, D)
 * và các phép chiếu phối cảnh / trực giao theo giáo trình CG.4 (trang 28-49)
 */
class Camera3D {
public:
    ViewMode mode = ViewMode::PERSPECTIVE_ORBIT;

    // Tham số tọa độ cầu (CG.4 trang 28-49)
    float theta = 0.785f;    // Góc quay ngang (Azimuth, radian) ~ 45 độ
    float phi = 0.523f;      // Góc nâng hạ (Elevation, radian) ~ 30 độ
    float radius = 55.0f;    // Khoảng cách R từ tâm quan sát tới camera (cm)
    float focalD = 45.0f;    // Tiêu cự D dùng cho phép chiếu phối cảnh

    Vec3 target = Vec3(0.0f, 3.0f, 0.0f); // Điểm hội tụ quan sát (tâm xe)
    Vec3 eyePos = Vec3(0.0f, 0.0f, 0.0f); // Tọa độ mắt camera trong không gian thế giới
    Vec3 upVec  = Vec3(0.0f, 1.0f, 0.0f); // Vector hướng lên trên

    // Thông số màn hình
    int screenWidth = 1280;
    int screenHeight = 720;
    float fovY = 45.0f;
    float zNear = 0.5f;
    float zFar = 500.0f;

    // Dùng cho tương tác chuột
    bool isDragging = false;
    bool isPanning = false;
    int lastMouseX = 0;
    int lastMouseY = 0;

public:
    Camera3D();

    void setViewport(int width, int height);
    void setViewMode(ViewMode newMode);

    // Cập nhật vị trí mắt quan sát dựa trên tham số cầu hoặc vị trí xe
    void update(const Vec3& carPosition, float carYawRad);

    // Thiết lập ma trận chiếu (Projection Matrix) vào OpenGL
    void applyProjection() const;

    // Thiết lập ma trận quan sát (View Matrix) vào OpenGL (gluLookAt)
    void applyView() const;

    // Xử lý sự kiện chuột
    void onMouseDown(int button, int state, int x, int y);
    void onMouseMove(int x, int y);
    void onMouseWheel(int wheel, int direction, int x, int y);

    // Bắn tia Raycast từ màn hình 2D vào không gian 3D (phục vụ chọn linh kiện)
    void screenToRay(int mouseX, int mouseY, Vec3& rayOrigin, Vec3& rayDir) const;

    const char* getViewModeName() const;
};
