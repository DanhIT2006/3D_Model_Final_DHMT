#include "math/Camera3D.h"
#include <GL/freeglut.h>
#include <algorithm>

Camera3D::Camera3D() {
    update(Vec3(0.0f, 0.0f, 0.0f), 0.0f);
}

void Camera3D::setViewport(int width, int height) {
    screenWidth = (width > 0) ? width : 1;
    screenHeight = (height > 0) ? height : 1;
}

void Camera3D::setViewMode(ViewMode newMode) {
    mode = newMode;
    if (mode == ViewMode::TOP_VIEW) {
        theta = 0.0f;
        phi = 1.55f; // Gần 90 độ (tránh suy biến gimbal lock)
    } else if (mode == ViewMode::FRONT_VIEW) {
        theta = 0.0f;
        phi = 0.08f;
    } else if (mode == ViewMode::SIDE_VIEW) {
        theta = 1.5708f; // 90 độ
        phi = 0.08f;
    }
}

const char* Camera3D::getViewModeName() const {
    switch (mode) {
        case ViewMode::PERSPECTIVE_ORBIT: return "Perspective Orbit (Free 3D)";
        case ViewMode::TOP_VIEW:          return "Top View (Technical Ortho)";
        case ViewMode::FRONT_VIEW:        return "Front View (Technical Ortho)";
        case ViewMode::SIDE_VIEW:         return "Side View (Technical Ortho)";
        case ViewMode::CAR_FOLLOW:        return "Chase Camera (Behind Car)";
    }
    return "Unknown";
}

void Camera3D::update(const Vec3& carPosition, float carYawRad) {
    if (mode == ViewMode::CAR_FOLLOW) {
        // Camera đặt ở phía sau đuôi xe, cao hơn một chút và nhìn về mũi xe
        float followDist = 32.0f;
        float followHeight = 16.0f;
        
        // Hướng lùi của xe theo góc yaw:
        // Xe có mũi hướng theo góc Yaw
        float sinYaw = std::sin(carYawRad);
        float cosYaw = std::cos(carYawRad);

        eyePos = carPosition + Vec3(-followDist * sinYaw, followHeight, -followDist * cosYaw);
        target = carPosition + Vec3(followDist * 0.5f * sinYaw, 4.0f, followDist * 0.5f * cosYaw);
        upVec = Vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    // Tọa độ cầu chuẩn hóa (CG.4 trang 28-49)
    // Giới hạn góc phi tránh lật ngược camera
    phi = std::clamp(phi, -1.45f, 1.45f);

    float cosPhi = std::cos(phi);
    float sinPhi = std::sin(phi);
    float cosTheta = std::cos(theta);
    float sinTheta = std::sin(theta);

    eyePos.x = target.x + radius * cosPhi * sinTheta;
    eyePos.y = target.y + radius * sinPhi;
    eyePos.z = target.z + radius * cosPhi * cosTheta;
    upVec = Vec3(0.0f, 1.0f, 0.0f);
}

void Camera3D::applyProjection() const {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)screenWidth / (float)screenHeight;

    if (mode == ViewMode::TOP_VIEW || mode == ViewMode::FRONT_VIEW || mode == ViewMode::SIDE_VIEW) {
        // Chiếu trực giao kỹ thuật (Orthographic Projection - CG.4)
        float orthoSize = radius * 0.55f;
        glOrtho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, zNear, zFar);
    } else {
        // Chiếu phối cảnh (Perspective Projection - CG.4)
        gluPerspective(fovY, aspect, zNear, zFar);
    }

    glMatrixMode(GL_MODELVIEW);
}

void Camera3D::applyView() const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        eyePos.x, eyePos.y, eyePos.z,
        target.x, target.y, target.z,
        upVec.x,  upVec.y,  upVec.z
    );
}

void Camera3D::onMouseDown(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        lastMouseX = x;
        lastMouseY = y;
        if (button == GLUT_LEFT_BUTTON) {
            isDragging = true;
        } else if (button == GLUT_RIGHT_BUTTON || button == GLUT_MIDDLE_BUTTON) {
            isPanning = true;
        }
    } else if (state == GLUT_UP) {
        isDragging = false;
        isPanning = false;
    }
}

void Camera3D::onMouseMove(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;

    if (isDragging && mode == ViewMode::PERSPECTIVE_ORBIT) {
        float rotSpeed = 0.008f;
        theta -= dx * rotSpeed;
        phi   += dy * rotSpeed;
    } else if (isPanning) {
        float panSpeed = radius * 0.0018f;
        Vec3 forward = (target - eyePos).normalized();
        Vec3 right = forward.cross(upVec).normalized();
        Vec3 up = right.cross(forward).normalized();

        target -= right * (dx * panSpeed);
        target += up * (dy * panSpeed);
    }
}

void Camera3D::onMouseWheel(int wheel, int direction, int x, int y) {
    (void)wheel; (void)x; (void)y;
    float zoomSpeed = 2.5f;
    if (direction > 0) {
        radius = std::max(10.0f, radius - zoomSpeed);
    } else {
        radius = std::min(200.0f, radius + zoomSpeed);
    }
}

void Camera3D::screenToRay(int mouseX, int mouseY, Vec3& rayOrigin, Vec3& rayDir) const {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLfloat winX = (float)mouseX;
    GLfloat winY = (float)viewport[3] - (float)mouseY;

    GLdouble nearX, nearY, nearZ;
    GLdouble farX, farY, farZ;

    gluUnProject(winX, winY, 0.0, modelview, projection, viewport, &nearX, &nearY, &nearZ);
    gluUnProject(winX, winY, 1.0, modelview, projection, viewport, &farX, &farY, &farZ);

    rayOrigin = Vec3((float)nearX, (float)nearY, (float)nearZ);
    Vec3 farPt((float)farX, (float)farY, (float)farZ);
    rayDir = (farPt - rayOrigin).normalized();
}
