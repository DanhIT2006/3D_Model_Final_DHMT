#include <GL/freeglut.h>
#include <iostream>
#include <chrono>

#include "math/Camera3D.h"
#include "scene/RobotCarScene.h"
#include "core/HudDashboard.h"
#include "core/InputHandler.h"

// Biến toàn cục hệ thống
static Camera3D g_camera;
static RobotCarScene* g_scene = nullptr;
static HudDashboard g_hud;
static InputHandler g_input;

static int g_windowWidth = 1280;
static int g_windowHeight = 720;
static float g_fps = 60.0f;
static auto g_lastTime = std::chrono::high_resolution_clock::now();

void initOpenGL() {
    glClearColor(0.10f, 0.12f, 0.16f, 1.0f); // Màu nền xám xanh hiện đại
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // Bật khử răng cưa cho đường nét
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Thiết lập hệ thống chiếu sáng đa nguồn (Dual-Source Lighting)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Key Light (Nguồn sáng chính từ góc trên - trước - phải)
    GLfloat light0Pos[]     = { 30.0f, 65.0f, 45.0f, 1.0f };
    GLfloat light0Ambient[] = { 0.25f, 0.27f, 0.32f, 1.0f };
    GLfloat light0Diffuse[] = { 0.88f, 0.88f, 0.90f, 1.0f };
    GLfloat light0Specular[]= { 0.55f, 0.55f, 0.55f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light0Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

    // Fill Light (Nguồn sáng bù mềm từ phía sau - trái - thấp hơn, chống bóng tối gắt ở gầm và bánh)
    GLfloat light1Pos[]     = { -25.0f, 35.0f, -35.0f, 1.0f };
    GLfloat light1Ambient[] = { 0.10f, 0.10f, 0.12f, 1.0f };
    GLfloat light1Diffuse[] = { 0.35f, 0.38f, 0.42f, 1.0f };
    GLfloat light1Specular[]= { 0.20f, 0.20f, 0.20f, 1.0f };

    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  light1Ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);

    // Cho phép tô màu vật liệu theo glColor (glColorMaterial)
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat matSpecular[] = { 0.45f, 0.45f, 0.45f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 48);

    g_camera.setViewport(g_windowWidth, g_windowHeight);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Kết xuất không gian 3D
    glEnable(GL_LIGHTING);
    g_camera.applyProjection();
    g_camera.applyView();

    if (g_scene) {
        g_scene->render(g_input.wireframeMode);
    }

    // 2. Kết xuất giao diện HUD 2D
    g_hud.render(g_windowWidth, g_windowHeight, *g_scene, g_camera, g_fps);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    g_windowWidth = (w > 0) ? w : 1;
    g_windowHeight = (h > 0) ? h : 1;
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    g_camera.setViewport(g_windowWidth, g_windowHeight);
}

void timer(int value) {
    (void)value;
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - g_lastTime).count();
    g_lastTime = now;

    // Giới hạn dt tránh giật lag
    if (dt > 0.1f) dt = 0.1f;
    g_fps = 0.9f * g_fps + 0.1f * (1.0f / (dt > 1e-4f ? dt : 0.016f));

    // Cập nhật điều khiển liên tục (WASD, Arm)
    if (g_scene) {
        g_input.updateContinuousInput(dt, *g_scene);
        g_scene->update(dt);
        g_camera.update(g_scene->carPosition, g_scene->carYawRad);
    }

    g_hud.update(dt);

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    if (g_scene) {
        g_input.onKeyDown(key, x, y, *g_scene, g_camera, g_hud);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (g_scene) {
        g_input.onKeyUp(key, x, y, *g_scene);
    }
}

void mouse(int button, int state, int x, int y) {
    if (g_scene) {
        g_input.onMouseClick(button, state, x, y, g_windowWidth, g_windowHeight, *g_scene, g_camera, g_hud);
    }
}

void motion(int x, int y) {
    g_camera.onMouseMove(x, y);
}

void mouseWheel(int wheel, int direction, int x, int y) {
    g_camera.onMouseWheel(wheel, direction, x, y);
}

int main(int argc, char** argv) {
    std::cout << "==================================================================" << std::endl;
    std::cout << "  DIGITAL TWIN 3D - XE ROBOT GAP DO VAT (11 LINH KIEN PHAN CUNG)  " << std::endl;
    std::cout << "  DO HOA MAY TINH C++/OPENGL (CG.3-2D & CG.4-3D GRAPHICS)          " << std::endl;
    std::cout << "==================================================================" << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Digital Twin 3D - Xe Robot Gap Do Vat (OpenGL / C++)");

    initOpenGL();

    g_scene = new RobotCarScene();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMouseWheelFunc(mouseWheel);
    glutTimerFunc(16, timer, 0);

    std::cout << "[Khoi tao] Hoan tat! Nhan [H] trong chuong trinh de xem phim tat." << std::endl;

    glutMainLoop();

    delete g_scene;
    return 0;
}
