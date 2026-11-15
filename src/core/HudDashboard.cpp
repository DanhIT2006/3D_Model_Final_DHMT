#include "core/HudDashboard.h"
#include "scene/RoboticArmRenderer.h"
#include <GL/freeglut.h>
#include <cmath>
#include <sstream>
#include <iomanip>

HudDashboard::HudDashboard() {
    buttons.clear();
}

void HudDashboard::update(float dt) {
    animTimer += dt;
    radarSweepAngle += 2.5f * dt;
    if (radarSweepAngle > 3.14159265f * 2.0f) {
        radarSweepAngle -= 3.14159265f * 2.0f;
    }
}

bool HudDashboard::project3DToScreen(const Vec3& pos3D, int& outScreenX, int& outScreenY) const {
    GLdouble sx, sy, sz;
    if (gluProject((GLdouble)pos3D.x, (GLdouble)pos3D.y, (GLdouble)pos3D.z,
                   lastModelView, lastProjection, lastViewport,
                   &sx, &sy, &sz) == GL_TRUE) {
        if (sz >= 0.0 && sz <= 1.0) {
            outScreenX = (int)std::round(sx);
            outScreenY = (int)std::round(sy);
            return true;
        }
    }
    return false;
}

void HudDashboard::render(int screenW, int screenH, const RobotCarScene& scene, const Camera3D& camera, float fps) {
    // Lưu lại ma trận chiếu phối cảnh 3D hiện tại để thực hiện phép chiếu 3D sang 2D (gluProject)
    glGetDoublev(GL_MODELVIEW_MATRIX, lastModelView);
    glGetDoublev(GL_PROJECTION_MATRIX, lastProjection);
    glGetIntegerv(GL_VIEWPORT, lastViewport);

    int pipW = 230;
    int pipH = 165;
    int pipX = screenW - pipW - 15;
    int pipY = screenH - pipH - 55;

    // KET XUAT CAMERA CAN CANH NGAM GAP (PiP CLAW-EYE VIEWPORT)
    if (!scene.isWorkbenchMode && showClawCam) {
        glViewport(pipX, pipY, pipW, pipH);
        glEnable(GL_SCISSOR_TEST);
        glScissor(pipX, pipY, pipW, pipH);
        glClear(GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(48.0, (double)pipW / (double)pipH, 0.5, 300.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        float yawDeg = scene.compSrvBase ? (scene.compSrvBase->currentAngleDeg - 90.0f) : 0.0f;
        float totalYawRad = scene.carYawRad + (yawDeg * 3.14159265f / 180.0f);
        Vec3 gripPt = scene.gripWorldPos;
        Vec3 eyePt = gripPt + Vec3(-3.0f * std::sin(totalYawRad), 7.5f, -3.0f * std::cos(totalYawRad));
        Vec3 targetPt = gripPt + Vec3(1.5f * std::sin(totalYawRad), -1.2f, 1.5f * std::cos(totalYawRad));

        gluLookAt(eyePt.x, eyePt.y, eyePt.z, targetPt.x, targetPt.y, targetPt.z, 0, 1, 0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        // San phu
        glColor3f(0.18f, 0.20f, 0.24f);
        glBegin(GL_LINES);
        for (float g = -15.0f; g <= 15.0f; g += 3.0f) {
            glVertex3f(targetPt.x + g, -3.3f, targetPt.z - 15.0f);
            glVertex3f(targetPt.x + g, -3.3f, targetPt.z + 15.0f);
            glVertex3f(targetPt.x - 15.0f, -3.3f, targetPt.z + g);
            glVertex3f(targetPt.x + 15.0f, -3.3f, targetPt.z + g);
        }
        glEnd();

        const_cast<RobotCarScene&>(scene).carRoot->render(false);
        RoboticArmRenderer::renderCargoCrate(scene.targetObject->localPos, 3.4f, 3.2f, 3.4f, scene.isObjectGrasped);
        RoboticArmRenderer::renderDropTargetReticle(scene.gripWorldPos, scene.targetObject->localPos, scene.isObjectGrasped);

        glDisable(GL_LIGHTING);
        glDisable(GL_SCISSOR_TEST);

        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();

        glViewport(0, 0, screenW, screenH);
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screenW, 0, screenH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    buttons.clear();

    // Nếu đang ở chế độ quan sát vi mô: hiển thị giao diện bàn làm việc linh kiện chi tiết
    if (scene.isWorkbenchMode && scene.inspectedComponent) {
        renderWorkbenchHUD(screenW, screenH, const_cast<RobotCarScene&>(scene), camera);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        return;
    }

    // 1. Thanh trạng thái phía trên (Top Bar)
    renderTopBar(screenW, screenH, scene, camera, fps);

    // 2. Góc trên-trái: Radar siêu âm HC-SR04 & Mini-Map (Midpoint Circle, Bresenham Line, Cohen-Sutherland Clip)
    renderRadarMiniMap(15, screenH - 250, 200, scene);

    // 3. Góc dưới-trái: Bảng chi tiết linh kiện được chọn (Component Inspector)
    renderComponentInspector(15, 15, 360, 230, scene);

    // 4. Góc dưới-phải: Đồng hồ đo góc Servo & Dao động ký xung PWM (Oscilloscope)
    renderOscilloscopeAndGauges(screenW - 350, 15, 335, 230, scene);

    // 5. Bảng hướng dẫn phím tắt khi người dùng nhấn 'H'
    if (showHelp) {
        renderHelpDialog(screenW, screenH);
    }

    // 6. Khung vien cong nghe cao cho PiP Claw Cam tren 2D HUD
    if (!scene.isWorkbenchMode && showClawCam) {
        Raster2D::drawLineBresenham(pipX, pipY, pipX + pipW, pipY, Vec3(0.0f, 0.7f, 0.95f));
        Raster2D::drawLineBresenham(pipX + pipW, pipY, pipX + pipW, pipY + pipH, Vec3(0.0f, 0.7f, 0.95f));
        Raster2D::drawLineBresenham(pipX + pipW, pipY + pipH, pipX, pipY + pipH, Vec3(0.0f, 0.7f, 0.95f));
        Raster2D::drawLineBresenham(pipX, pipY + pipH, pipX, pipY, Vec3(0.0f, 0.7f, 0.95f));

        int cornerLen = 12;
        Raster2D::drawLineBresenham(pipX, pipY, pipX + cornerLen, pipY, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX, pipY, pipX, pipY + cornerLen, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX + pipW, pipY, pipX + pipW - cornerLen, pipY, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX + pipW, pipY, pipX + pipW, pipY + cornerLen, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX, pipY + pipH, pipX + cornerLen, pipY + pipH, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX, pipY + pipH, pipX, pipY + pipH - cornerLen, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX + pipW, pipY + pipH, pipX + pipW - cornerLen, pipY + pipH, Vec3(1.0f, 0.85f, 0.2f));
        Raster2D::drawLineBresenham(pipX + pipW, pipY + pipH, pipX + pipW, pipY + pipH - cornerLen, Vec3(1.0f, 0.85f, 0.2f));

        int midX = pipX + pipW / 2;
        int midY = pipY + pipH / 2;
        Raster2D::drawCircleMidpoint(midX, midY, 6, Vec3(0.0f, 1.0f, 0.8f));
        Raster2D::drawLineBresenham(midX - 10, midY, midX - 6, midY, Vec3(0.0f, 1.0f, 0.8f));
        Raster2D::drawLineBresenham(midX + 6, midY, midX + 10, midY, Vec3(0.0f, 1.0f, 0.8f));
        Raster2D::drawLineBresenham(midX, midY - 10, midX, midY - 6, Vec3(0.0f, 1.0f, 0.8f));
        Raster2D::drawLineBresenham(midX, midY + 6, midX, midY + 10, Vec3(0.0f, 1.0f, 0.8f));

        Raster2D::fillRect(pipX + 2, pipY + pipH - 22, pipW - 4, 20, Vec3(0.06f, 0.10f, 0.16f));
        FontRenderer::drawText2DWithShadow(pipX + 8, pipY + pipH - 16, "CAM NGAN GAP (PIP) [C]", Vec3(0.0f, 0.9f, 1.0f), FontRenderer::SMALL);

        float d = scene.gripWorldPos.distanceTo(scene.targetObject->localPos);
        if (scene.isObjectGrasped) {
            FontRenderer::drawText2DWithShadow(pipX + 8, pipY + 8, "[DA KEP DONG BO]", Vec3(0.2f, 1.0f, 0.4f), FontRenderer::SMALL);
        } else if (d < 3.2f) {
            FontRenderer::drawText2DWithShadow(pipX + 8, pipY + 8, "[READY TO GRAB - BAM G]", Vec3(0.2f, 1.0f, 0.3f), FontRenderer::SMALL);
        } else {
            std::string dStr = "Cu ly: " + std::to_string((int)d) + "cm";
            FontRenderer::drawText2DWithShadow(pipX + 8, pipY + 8, dStr, Vec3(1.0f, 0.85f, 0.2f), FontRenderer::SMALL);
        }
    }

    // 7. Gợi ý phím tắt nhanh ở góc trên phải
    FontRenderer::drawText2DWithShadow(screenW - 220, screenH - 55, "Nhan [H]: Xem phim tat", Vec3(0.9f, 0.9f, 0.2f), FontRenderer::NORMAL);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void HudDashboard::renderTopBar(int screenW, int screenH, const RobotCarScene& scene, const Camera3D& camera, float fps) {
    // Nền thanh trên cùng
    Raster2D::fillRect(0, screenH - 42, screenW, 42, Vec3(0.08f, 0.09f, 0.12f));
    Raster2D::drawLineBresenham(0, screenH - 43, screenW, screenH - 43, Vec3(0.0f, 0.55f, 0.85f));

    // Tiêu đề hệ thống
    FontRenderer::drawText2DWithShadow(15, screenH - 26, "DIGITAL TWIN 3D - XE ROBOT GAP DO VAT (11 LINH KIEN)", Vec3(0.0f, 0.85f, 1.0f), FontRenderer::NORMAL);

    // Các thông số giám sát tức thời
    std::ostringstream ss;
    float batV = (scene.compBatSw && scene.compBatSw->switchOn) ? scene.compBatSw->batteryVoltage : 0.0f;
    float buckV = scene.compBuck ? scene.compBuck->actualOutVoltage : 0.0f;

    ss << "PIN: " << std::fixed << std::setprecision(1) << batV << "V"
       << " | BUCK: " << std::setprecision(2) << buckV << "V"
       << " | DONG: " << std::setprecision(0) << scene.circuit.totalCurrentMa << "mA"
       << " | GOC NHIN: " << camera.getViewModeName()
       << " | " << std::setprecision(0) << fps << " FPS";

    FontRenderer::drawText2DWithShadow(screenW - 680, screenH - 26, ss.str(), Vec3(0.85f, 0.88f, 0.9f), FontRenderer::SMALL);
}

void HudDashboard::renderRadarMiniMap(int x, int y, int size, const RobotCarScene& scene) {
    // Khung nền Radar
    Raster2D::fillRect(x, y, size, size, Vec3(0.06f, 0.08f, 0.1f));
    
    // Viền khung màu xanh Neon
    Raster2D::drawLineBresenham(x, y, x + size, y, Vec3(0.0f, 0.7f, 0.4f));
    Raster2D::drawLineBresenham(x + size, y, x + size, y + size, Vec3(0.0f, 0.7f, 0.4f));
    Raster2D::drawLineBresenham(x + size, y + size, x, y + size, Vec3(0.0f, 0.7f, 0.4f));
    Raster2D::drawLineBresenham(x, y + size, x, y, Vec3(0.0f, 0.7f, 0.4f));

    int cx = x + size / 2;
    int cy = y + size / 2;
    int maxR = size / 2 - 10;

    // Vùng cửa sổ xén (Clipping Window cho Cohen-Sutherland)
    Rect2D clipWin = { x + 2, y + 2, x + size - 2, y + size - 2 };

    // Vẽ các vòng cự ly siêu âm bằng thuật toán Midpoint Circle (CG.3 trang 29-37)
    Vec3 ringCol(0.12f, 0.35f, 0.25f);
    Raster2D::drawCircleMidpoint(cx, cy, maxR / 3, ringCol);
    Raster2D::drawCircleMidpoint(cx, cy, (maxR * 2) / 3, ringCol);
    Raster2D::drawCircleMidpoint(cx, cy, maxR, ringCol);

    // Trục chữ thập tâm
    Raster2D::drawLineBresenham(cx - maxR, cy, cx + maxR, cy, ringCol);
    Raster2D::drawLineBresenham(cx, cy - maxR, cx, cy + maxR, ringCol);

    // Tia quét Radar (Bresenham Line + Cohen-Sutherland Clip)
    int rx0 = cx;
    int ry0 = cy;
    int rx1 = cx + (int)(maxR * std::cos(radarSweepAngle));
    int ry1 = cy + (int)(maxR * std::sin(radarSweepAngle));

    if (Clipper2D::clipLineCohenSutherland(rx0, ry0, rx1, ry1, clipWin)) {
        Raster2D::drawLineBresenham(rx0, ry0, rx1, ry1, Vec3(0.2f, 1.0f, 0.5f));
    }

    // Vẽ điểm blip vật cản phía trước mũi xe / tay gắp tới vật thể mẫu
    if (scene.targetObject) {
        Vec3 frontPos = scene.compSrvBase ? scene.compSrvBase->getWorldMatrix().transformPoint(Vec3(0.0f, 1.5f, 3.4f)) : scene.carPosition;
        float dist = frontPos.distanceTo(scene.targetObject->localPos);
        if (dist < 40.0f) {
            float normD = dist / 40.0f;
            int blipY = cy + (int)(normD * maxR);
            // Điểm vật cản nhấp nháy màu đỏ
            Raster2D::drawCircleMidpoint(cx, blipY, 4, Vec3(1.0f, 0.2f, 0.2f));
            Raster2D::drawCircleMidpoint(cx, blipY, 6, Vec3(1.0f, 0.6f, 0.2f));
        }
    }

    // Nhãn RADAR
    FontRenderer::drawText2DWithShadow(x + 8, y + size - 18, "RADAR CU LY TRUOC (40cm)", Vec3(0.2f, 0.9f, 0.6f), FontRenderer::SMALL);
}

void HudDashboard::renderComponentInspector(int x, int y, int w, int h, const RobotCarScene& scene) {
    Raster2D::fillRect(x, y, w, h, Vec3(0.07f, 0.08f, 0.11f));
    Raster2D::drawLineBresenham(x, y + h, x + w, y + h, Vec3(0.2f, 0.45f, 0.75f));
    Raster2D::drawLineBresenham(x + w, y, x + w, y + h, Vec3(0.2f, 0.45f, 0.75f));

    FontRenderer::drawText2DWithShadow(x + 12, y + h - 22, "THONG SO LINH KIEN (INSPECTOR)", Vec3(0.0f, 0.8f, 1.0f), FontRenderer::NORMAL);

    HardwareComponent* comp = scene.selectedComponent;
    if (!comp) {
        FontRenderer::drawText2D(x + 15, y + h - 55, "Click chuot vao linh kien 3D de xem", Vec3(0.6f, 0.6f, 0.65f), FontRenderer::NORMAL);
        FontRenderer::drawText2D(x + 15, y + h - 80, "Danh sach 10 linh kien: ARD, L298N, BUCK,", Vec3(0.5f, 0.5f, 0.55f), FontRenderer::SMALL);
        FontRenderer::drawText2D(x + 15, y + h - 100, "BAT_SW, DOMINO, MOT_L, MOT_R,", Vec3(0.5f, 0.5f, 0.55f), FontRenderer::SMALL);
        FontRenderer::drawText2D(x + 15, y + h - 120, "SRV_BASE (Tru), SRV_ELBOW, SRV_GRIP.", Vec3(0.5f, 0.5f, 0.55f), FontRenderer::SMALL);

        // Nút chuyển chế độ
        buttons.push_back({x + 15, y + 15, 150, 28, "DOI CHE DO [M]", 1});
        buttons.push_back({x + 180, y + 15, 150, 28, "RESET XE [R]", 2});

        for (const auto& btn : buttons) {
            Raster2D::fillRect(btn.x, btn.y, btn.w, btn.h, Vec3(0.15f, 0.25f, 0.38f));
            Raster2D::drawLineBresenham(btn.x, btn.y, btn.x + btn.w, btn.y, Vec3(0.3f, 0.6f, 0.9f));
            FontRenderer::drawText2DWithShadow(btn.x + 15, btn.y + 8, btn.label, Vec3(1, 1, 1), FontRenderer::SMALL);
        }
        return;
    }

    // Hiển thị thông số linh kiện được chọn
    std::string title = comp->code + ": " + comp->name;
    FontRenderer::drawText2DWithShadow(x + 15, y + h - 48, title, Vec3(1.0f, 0.85f, 0.1f), FontRenderer::NORMAL);

    std::ostringstream ssGrp;
    ssGrp << "Nhom: " << comp->funcGroup << " | CAD: " 
          << comp->cadSize.x << "x" << comp->cadSize.y << "x" << comp->cadSize.z << " cm";
    FontRenderer::drawText2D(x + 15, y + h - 70, ssGrp.str(), Vec3(0.75f, 0.78f, 0.82f), FontRenderer::SMALL);

    std::ostringstream ssPwr;
    std::string pwrState;
    Vec3 pwrCol;
    if (!comp->isPowered) {
        pwrState = "NGAT DIEN (OFF)";
        pwrCol = Vec3(1.0f, 0.3f, 0.3f);
    } else {
        const auto* mot = dynamic_cast<const DCMotorComp*>(comp);
        if (mot) {
            if (std::fabs(mot->rpm) > 1.0f) {
                pwrState = "DANG CHAY (RUNNING - 7.4V)";
                pwrCol = Vec3(0.2f, 1.0f, 0.3f);
            } else {
                pwrState = "SAN SANG (STANDBY - 7.4V)";
                pwrCol = Vec3(0.2f, 0.85f, 1.0f);
            }
        } else {
            pwrState = "CO DIEN (ACTIVE)";
            pwrCol = Vec3(0.2f, 1.0f, 0.3f);
        }
    }
    ssPwr << "Nguon cap: " << pwrState << " | Dinh muc: " << std::fixed << std::setprecision(1) << comp->nominalVoltage << "V";
    FontRenderer::drawText2D(x + 15, y + h - 90, ssPwr.str(), pwrCol, FontRenderer::SMALL);

    FontRenderer::drawText2D(x + 15, y + h - 110, "Trang thai: " + comp->statusDesc, Vec3(0.9f, 0.9f, 0.95f), FontRenderer::SMALL);

    // Bảng chân cắm (Pins) kèm điện áp tức thời
    std::string pinStr = "Cac chan Pin: ";
    for (size_t i = 0; i < comp->pins.size() && i < 6; ++i) {
        std::ostringstream vss;
        vss << std::fixed << std::setprecision(1) << comp->pins[i].voltage << "V";
        pinStr += comp->pins[i].pinName + "[" + vss.str() + "] ";
    }
    if (comp->pins.size() > 6) pinStr += "... (" + std::to_string(comp->pins.size()) + " pins)";
    FontRenderer::drawText2D(x + 15, y + h - 130, pinStr, Vec3(0.7f, 0.7f, 0.75f), FontRenderer::SMALL);

    // Nút tương tác nhanh riêng cho linh kiện
    if (!comp->subComponents.empty()) {
        buttons.push_back({x + 15, y + 48, 260, 28, "QUAN SAT CHI TIET [Phim I / Q / Enter]", 99});
    }

    if (comp->code == "BAT_SW") {
        buttons.push_back({x + 15, y + 15, 150, 28, "BAT/TAT NGUON [T]", 3});
    } else if (comp->code == "BUCK") {
        buttons.push_back({x + 15, y + 15, 100, 28, "GIAM AP [-]", 4});
        buttons.push_back({x + 125, y + 15, 100, 28, "TANG AP [+]", 5});
    } else if (comp->code == "ARD") {
        buttons.push_back({x + 15, y + 15, 160, 28, "DOI CHE DO [M]", 1});
    }
    buttons.push_back({x + 235, y + 15, 100, 28, "RESET [R]", 2});

    for (const auto& btn : buttons) {
        Raster2D::fillRect(btn.x, btn.y, btn.w, btn.h, Vec3(0.15f, 0.25f, 0.38f));
        Raster2D::drawLineBresenham(btn.x, btn.y, btn.x + btn.w, btn.y, Vec3(0.3f, 0.6f, 0.9f));
        FontRenderer::drawText2DWithShadow(btn.x + 10, btn.y + 8, btn.label, Vec3(1, 1, 1), FontRenderer::SMALL);
    }
}

void HudDashboard::renderOscilloscopeAndGauges(int x, int y, int w, int h, const RobotCarScene& scene) {
    Raster2D::fillRect(x, y, w, h, Vec3(0.07f, 0.08f, 0.11f));
    Raster2D::drawLineBresenham(x, y + h, x + w, y + h, Vec3(0.2f, 0.45f, 0.75f));
    Raster2D::drawLineBresenham(x, y, x, y + h, Vec3(0.2f, 0.45f, 0.75f));

    FontRenderer::drawText2DWithShadow(x + 12, y + h - 22, "DONG HO GOC & DAO DONG KY PWM", Vec3(0.0f, 0.8f, 1.0f), FontRenderer::NORMAL);

    // 1. Ba đồng hồ đo góc Servo (BASE, ELBOW, GRIP) sử dụng Midpoint Circle
    int gaugeY = y + h - 68;
    int r = 24;

    auto drawGauge = [&](int gx, const std::string& name, float angleDeg, float minA, float maxA) {
        // Cung tròn nền bằng Midpoint Circle
        Raster2D::drawCircleMidpoint(gx, gaugeY, r, Vec3(0.25f, 0.28f, 0.32f));
        // Kim chỉ góc bằng Bresenham Line
        float rad = (angleDeg - 90.0f) * 3.14159265f / 180.0f;
        int kx = gx + (int)((r - 3) * std::sin(rad));
        int ky = gaugeY + (int)((r - 3) * std::cos(rad));
        Raster2D::drawLineBresenham(gx, gaugeY, kx, ky, Vec3(1.0f, 0.85f, 0.2f));

        FontRenderer::drawText2D(gx - 18, gaugeY - r - 14, name, Vec3(0.75f, 0.75f, 0.8f), FontRenderer::SMALL);
        std::string valStr = std::to_string((int)angleDeg) + "o";
        FontRenderer::drawText2D(gx - 10, gaugeY - 6, valStr, Vec3(0.2f, 1.0f, 0.4f), FontRenderer::SMALL);
    };

    float baseA  = scene.compSrvBase  ? scene.compSrvBase->currentAngleDeg : 90.0f;
    float liftA  = scene.compSrvElbow ? scene.compSrvElbow->currentAngleDeg : 0.0f;
    float gripA  = scene.compSrvGrip  ? scene.compSrvGrip->currentAngleDeg : 45.0f;

    drawGauge(x + 55,  "MAST", baseA, 0, 180);
    drawGauge(x + 165, "LIFT", liftA, 0, 180);
    drawGauge(x + 275, "GRIP", gripA, 0, 90);

    // 2. Dao động ký hiển thị dạng sóng PWM (y = f(x)) cắt bởi Cohen-Sutherland
    int oscX = x + 15;
    int oscY = y + 15;
    int oscW = w - 30;
    int oscH = 75;

    Raster2D::fillRect(oscX, oscY, oscW, oscH, Vec3(0.04f, 0.05f, 0.07f));
    Raster2D::drawLineBresenham(oscX, oscY, oscX + oscW, oscY, Vec3(0.15f, 0.3f, 0.2f));
    Raster2D::drawLineBresenham(oscX, oscY + oscH, oscX + oscW, oscY + oscH, Vec3(0.15f, 0.3f, 0.2f));

    FontRenderer::drawText2D(oscX + 8, oscY + oscH - 14, "XUNG PWM DONG CO & SERVO (500Hz)", Vec3(0.2f, 0.8f, 0.4f), FontRenderer::SMALL);

    // Hàm vẽ xung vuông PWM
    float dutyL = (scene.compArd && scene.compArd->isPowered) ? ((float)scene.compArd->motorSpeedL / 255.0f) : 0.0f;
    float period = 30.0f;

    for (int px = 0; px < oscW - 10; ++px) {
        float phase = std::fmod((float)px + animTimer * 60.0f, period);
        bool high = (phase < period * std::max(0.05f, dutyL));
        int py = oscY + 12 + (high ? 38 : 5);
        Raster2D::putPixel(oscX + 5 + px, py, Vec3(0.0f, 1.0f, 0.5f));
    }
}

void HudDashboard::renderHelpDialog(int screenW, int screenH) {
    int w = 560;
    int h = 380;
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;

    // Nền hộp thoại mờ tối
    Raster2D::fillRect(x, y, w, h, Vec3(0.08f, 0.10f, 0.14f));
    Raster2D::drawLineBresenham(x, y, x + w, y, Vec3(0.0f, 0.8f, 1.0f));
    Raster2D::drawLineBresenham(x + w, y, x + w, y + h, Vec3(0.0f, 0.8f, 1.0f));
    Raster2D::drawLineBresenham(x + w, y + h, x, y + h, Vec3(0.0f, 0.8f, 1.0f));
    Raster2D::drawLineBresenham(x, y + h, x, y, Vec3(0.0f, 0.8f, 1.0f));

    FontRenderer::drawText2DWithShadow(x + 20, y + h - 30, "BANG HUONG DAN PHIM TAT & DIEU KHIEN MO PHONG", Vec3(1.0f, 0.9f, 0.1f), FontRenderer::NORMAL);

    std::vector<std::pair<std::string, std::string>> guides = {
        {"W / A / S / D", "Dieu khien xe Tien / Re Trai / Lui / Re Phai"},
        {"Space", "Phanh dung xe khan cap (Brake)"},
        {"I / K", "Nang len / Ha xuong thang dung (Linear Lift)"},
        {"J / L", "Canh chinh vi tri tru nang (Base Trim)"},
        {"O / P", "Mo rong / Kep chat ngam gap (Gripper)"},
        {"T", "Bat / Tat cong tac nguon Pin 18650 (BAT_SW)"},
        {"M", "Chuyen doi Thu cong (Manual) <-> Tu hanh gap (Auto Grab)"},
        {"R", "Dat lai toan bo trang thai mo phong ban dau (Reset)"},
        {"1, 2, 3, 4, 5", "Doi Camera: 3D Tu do, Chieu bang, Chieu dung, Chieu canh, Bam sau xe"},
        {"F", "Bat / Tat che do xem khung day Wireframe (CG.4)"},
        {"C", "An / Hien cac duong day dien 3D"},
        {"+ / -", "Tang / Giam dien ap mach Buck LM2596"},
        {"Q", "Vao / Thoat che do quan sat vi mo linh kien 3D (Workbench)"},
        {"[ / ]", "Chon linh kien con truoc / tiep theo trong che do quan sat"},
        {"Chuot Trai", "Click chon linh kien 3D / Giu keo de xoay Camera"},
        {"Chuot Phai / Cuon", "Truot Camera (Pan) / Phong to - Thu nho (Zoom)"}
    };

    int curY = y + h - 60;
    for (const auto& g : guides) {
        FontRenderer::drawText2D(x + 25, curY, g.first, Vec3(0.0f, 1.0f, 0.8f), FontRenderer::SMALL);
        FontRenderer::drawText2D(x + 180, curY, "- " + g.second, Vec3(0.85f, 0.85f, 0.88f), FontRenderer::SMALL);
        curY -= 20;
    }

    FontRenderer::drawText2DWithShadow(x + 180, y + 15, "Nhan [H] de dong bang nay", Vec3(1.0f, 0.8f, 0.2f), FontRenderer::SMALL);
}

namespace {
    void drawWrappedText(int x, int y, int maxCharsPerLine, int lineSpacing, const std::string& text, const Vec3& color) {
        std::string remaining = text;
        int curY = y;
        while (!remaining.empty()) {
            if ((int)remaining.length() <= maxCharsPerLine) {
                FontRenderer::drawText2D(x, curY, remaining, color, FontRenderer::SMALL);
                break;
            }
            size_t splitPos = remaining.rfind(' ', maxCharsPerLine);
            if (splitPos == std::string::npos) splitPos = maxCharsPerLine;
            std::string line = remaining.substr(0, splitPos);
            FontRenderer::drawText2D(x, curY, line, color, FontRenderer::SMALL);
            curY -= lineSpacing;
            if (splitPos + 1 < remaining.length()) {
                remaining = remaining.substr(splitPos + 1);
            } else {
                break;
            }
        }
    }
}

void HudDashboard::renderWorkbenchHUD(int screenW, int screenH, RobotCarScene& scene, const Camera3D& camera) {
    (void)camera;
    HardwareComponent* comp = scene.inspectedComponent;
    if (!comp) return;

    // 1. Thanh tiêu đề trên cùng
    Raster2D::fillRect(0, screenH - 52, screenW, 52, Vec3(0.07f, 0.09f, 0.13f));
    Raster2D::drawLineBresenham(0, screenH - 53, screenW, screenH - 53, Vec3(0.0f, 0.65f, 0.95f));

    std::string titleStr = "CHE DO CO LAP LINH KIEN: " + comp->name + " [" + comp->code + "]";
    FontRenderer::drawText2DWithShadow(25, screenH - 26, titleStr, Vec3(0.0f, 0.9f, 1.0f), FontRenderer::NORMAL);
    FontRenderer::drawText2DWithShadow(screenW - 550, screenH - 26, "[X] X-RAY PCB: " + std::string(scene.isXrayMode ? "DANG BAT (ALPHA 0.30)" : "DANG TAT"), scene.isXrayMode ? Vec3(0.2f, 1.0f, 0.4f) : Vec3(0.6f, 0.7f, 0.8f), FontRenderer::NORMAL);
    FontRenderer::drawText2D(25, screenH - 45, "Chuot trai: Xoay 3D | Chuot phai: Pan | Cuon: Zoom (6-25cm) | Click chi tiet hoac danh muc ben trai", Vec3(0.72f, 0.76f, 0.82f), FontRenderer::SMALL);

    // Nút quay lại ở góc trên phải
    buttons.push_back({screenW - 235, screenH - 44, 220, 32, "QUAY LAI XE ROBOT [ESC]", 100});

    // 2. Bảng bên trái: Danh mục linh kiện con (Sub-components list)
    int listW = 320;
    int listH = screenH - 120;
    int listX = 20;
    int listY = 50;

    Raster2D::fillRect(listX, listY, listW, listH, Vec3(0.06f, 0.08f, 0.12f));
    Raster2D::drawLineBresenham(listX, listY + listH, listX + listW, listY + listH, Vec3(0.2f, 0.5f, 0.8f));
    Raster2D::drawLineBresenham(listX + listW, listY, listX + listW, listY + listH, Vec3(0.2f, 0.5f, 0.8f));
    Raster2D::drawLineBresenham(listX, listY, listX, listY + listH, Vec3(0.2f, 0.5f, 0.8f));
    Raster2D::drawLineBresenham(listX, listY, listX + listW, listY, Vec3(0.2f, 0.5f, 0.8f));

    std::string listTitle = "DANH MUC LINH KIEN CON (" + std::to_string(comp->subComponents.size()) + ")";
    FontRenderer::drawText2DWithShadow(listX + 15, listY + listH - 25, listTitle, Vec3(1.0f, 0.85f, 0.2f), FontRenderer::NORMAL);
    FontRenderer::drawText2D(listX + 15, listY + listH - 44, "Dung Tab / [ / ] de duyet nhanh", Vec3(0.6f, 0.65f, 0.7f), FontRenderer::SMALL);

    for (size_t i = 0; i < comp->subComponents.size(); ++i) {
        int itemY = listY + listH - 74 - (int)i * 28;
        if (itemY < listY + 10) break;

        const auto& sub = comp->subComponents[i];
        buttons.push_back({listX + 8, itemY, listW - 16, 24, sub.name, 200 + (int)i});

        if (sub.isSelected) {
            Raster2D::fillRect(listX + 8, itemY, listW - 16, 24, Vec3(0.12f, 0.32f, 0.52f));
            Raster2D::fillRect(listX + 8, itemY, 4, 24, Vec3(1.0f, 0.85f, 0.1f));
            FontRenderer::drawText2DWithShadow(listX + 18, itemY + 6, std::to_string(i + 1) + ". " + sub.name, Vec3(1.0f, 0.95f, 0.3f), FontRenderer::SMALL);
        } else {
            Raster2D::fillRect(listX + 8, itemY, listW - 16, 24, Vec3(0.08f, 0.10f, 0.15f));
            FontRenderer::drawText2D(listX + 18, itemY + 6, std::to_string(i + 1) + ". " + sub.name, Vec3(0.8f, 0.85f, 0.9f), FontRenderer::SMALL);
        }
    }

    // 3. Card phía dưới bên phải: Bảng phân tích kỹ thuật
    int cardX = 360;
    int cardY = 15;
    int cardW = screenW - 380;
    int cardH = 155;

    Raster2D::fillRect(cardX, cardY, cardW, cardH, Vec3(0.06f, 0.08f, 0.12f));
    Raster2D::drawLineBresenham(cardX, cardY + cardH, cardX + cardW, cardY + cardH, Vec3(0.1f, 0.7f, 0.95f));
    Raster2D::drawLineBresenham(cardX, cardY, cardX, cardY + cardH, Vec3(0.1f, 0.7f, 0.95f));
    Raster2D::drawLineBresenham(cardX + cardW, cardY, cardX + cardW, cardY + cardH, Vec3(0.1f, 0.7f, 0.95f));
    Raster2D::drawLineBresenham(cardX, cardY, cardX + cardW, cardY, Vec3(0.1f, 0.7f, 0.95f));

    SubComponent* sub = comp->getSelectedSubComponent();
    if (sub) {
        // === 4. THUẬT TOÁN ĐỒ HỌA CALLOUT LINE (3D -> 2D Projection + Bresenham Line + Cohen-Sutherland Clip) ===
        Vec3 subWorldPos = sub->localPos + Vec3(0.0f, 0.1f, 0.0f);
        int sx = 0, sy = 0;
        if (project3DToScreen(subWorldPos, sx, sy)) {
            // A. Điểm ngắm tâm định vị (Target Reticle)
            Raster2D::drawCircleMidpoint(sx, sy, 8, Vec3(1.0f, 0.85f, 0.15f));
            Raster2D::drawCircleMidpoint(sx, sy, 3, Vec3(1.0f, 0.45f, 0.1f));

            Raster2D::drawLineBresenham(sx - 14, sy, sx - 8, sy, Vec3(1.0f, 0.85f, 0.15f));
            Raster2D::drawLineBresenham(sx + 8, sy, sx + 14, sy, Vec3(1.0f, 0.85f, 0.15f));
            Raster2D::drawLineBresenham(sx, sy - 14, sx, sy - 8, Vec3(1.0f, 0.85f, 0.15f));
            Raster2D::drawLineBresenham(sx, sy + 8, sx, sy + 14, Vec3(1.0f, 0.85f, 0.15f));

            // B. Đường dẫn Callout dạng Gấp khúc kỹ thuật 2 đoạn (Elbow Callout)
            int topBoxY = cardY + cardH;
            int elbowX = std::max(cardX + 45, std::min(sx, cardX + cardW - 45));
            int elbowY = topBoxY + 24;

            Rect2D screenRect = {0, 0, screenW, screenH};

            // Đoạn 1: Từ tâm phần tử 3D (sx, sy) kéo chéo tới điểm gãy (elbowX, elbowY)
            int x0 = sx, y0 = sy, x1 = elbowX, y1 = elbowY;
            if (Clipper2D::clipLineCohenSutherland(x0, y0, x1, y1, screenRect)) {
                Raster2D::drawLineBresenham(x0, y0, x1, y1, Vec3(1.0f, 0.78f, 0.2f));
            }

            // Đoạn 2: Từ điểm gãy (elbowX, elbowY) kẻ thẳng đứng xuống chạm đúng mép trên hộp tại (elbowX, topBoxY)
            int x2 = elbowX, y2 = elbowY, x3 = elbowX, y3 = topBoxY;
            if (Clipper2D::clipLineCohenSutherland(x2, y2, x3, y3, screenRect)) {
                Raster2D::drawLineBresenham(x2, y2, x3, y3, Vec3(0.0f, 0.85f, 1.0f));
            }

            // Điểm chốt khớp nối (Knot) và điểm tiếp xúc trên mép hộp
            Raster2D::drawCircleMidpoint(elbowX, elbowY, 3, Vec3(1.0f, 0.85f, 0.2f));
            Raster2D::drawCircleMidpoint(elbowX, topBoxY, 3, Vec3(0.0f, 0.85f, 1.0f));
            FontRenderer::drawText2DWithShadow(elbowX + 6, elbowY - 2, sub->id, Vec3(1.0f, 0.92f, 0.3f), FontRenderer::SMALL);
        }

        // === 5. Bảng thông số chi tiết của phần tử con ===
        std::string catText = sub->category.empty() ? "Linh kien dien tu" : sub->category;
        FontRenderer::drawText2DWithShadow(cardX + 18, cardY + cardH - 22, "CHI TIET PHAN TU: " + sub->name, Vec3(1.0f, 0.88f, 0.15f), FontRenderer::NORMAL);
        FontRenderer::drawText2D(cardX + 18, cardY + cardH - 40, "Nhom: " + catText + " | Ma: " + sub->id + " | Tri so: " + sub->rating, Vec3(0.75f, 0.85f, 0.95f), FontRenderer::SMALL);
        FontRenderer::drawText2D(cardX + 18, cardY + cardH - 58, "Tac dung: " + sub->roleDesc.substr(0, (cardW - 40) / 7), Vec3(0.85f, 0.88f, 0.92f), FontRenderer::SMALL);
        if (sub->roleDesc.length() > (size_t)((cardW - 40) / 7)) {
            std::string line2 = sub->roleDesc.substr((cardW - 40) / 7, (cardW - 40) / 7);
            FontRenderer::drawText2D(cardX + 18, cardY + cardH - 74, line2, Vec3(0.85f, 0.88f, 0.92f), FontRenderer::SMALL);
        }

        // Dòng giám sát thời gian thực
        std::string liveStr = sub->liveStatusText.empty() ? ("Dang hoat dong binh thuong (5.0V)") : sub->liveStatusText;
        FontRenderer::drawText2DWithShadow(cardX + 18, cardY + 48, "[GIAM SAT]: " + liveStr, Vec3(0.2f, 1.0f, 0.4f), FontRenderer::SMALL);

        // Nút bấm tương tác vật lý nếu phần tử hỗ trợ (Nhấn Reset, Rút/Cắm Jumper, Vặn biến trở...)
        if (sub->isInteractive) {
            std::string actLabel = sub->actionName.empty() ? "TUONG TAC [ENTER]" : (sub->actionName + " [ENTER]");
            buttons.push_back({cardX + 18, cardY + 12, 270, 28, actLabel, 101});
        }

        // Nút chuyển phần tử Trước / Sau
        buttons.push_back({cardX + cardW - 230, cardY + 12, 105, 28, "TRUOC [ [ ]", 102});
        buttons.push_back({cardX + cardW - 115, cardY + 12, 105, 28, "TIEP [ ] ]", 103});
    }

    // Vẽ tất cả các nút trong chế độ Workbench
    for (const auto& btn : buttons) {
        if (btn.actionId < 100) continue;
        Vec3 btnCol = (btn.actionId == 100) ? Vec3(0.6f, 0.15f, 0.15f) :
                      (btn.actionId == 101) ? Vec3(0.12f, 0.55f, 0.35f) : Vec3(0.15f, 0.28f, 0.42f);
        Raster2D::fillRect(btn.x, btn.y, btn.w, btn.h, btnCol);
        Raster2D::drawLineBresenham(btn.x, btn.y, btn.x + btn.w, btn.y, Vec3(0.3f, 0.6f, 0.9f));
        FontRenderer::drawText2DWithShadow(btn.x + 10, btn.y + 8, btn.label, Vec3(1, 1, 1), FontRenderer::SMALL);
    }
}
bool HudDashboard::onMouseClick(int mouseX, int mouseY, RobotCarScene& scene, Camera3D& camera) {
    for (const auto& btn : buttons) {
        if (mouseX >= btn.x && mouseX <= btn.x + btn.w &&
            mouseY >= btn.y && mouseY <= btn.y + btn.h) {
            switch (btn.actionId) {
                case 99: // Vào chế độ quan sát vi mô
                    if (scene.selectedComponent) {
                        scene.enterWorkbench(scene.selectedComponent);
                        camera.enterWorkbenchMode();
                    }
                    break;
                case 100: // Thoát chế độ quan sát vi mô
                    scene.exitWorkbench();
                    camera.exitWorkbenchMode();
                    break;
                case 101: // Tương tác linh kiện nhỏ
                    if (scene.inspectedComponent) {
                        SubComponent* sub = scene.inspectedComponent->getSelectedSubComponent();
                        if (sub) {
                            scene.inspectedComponent->onSubComponentAction(sub->id);
                        }
                    }
                    break;
                case 102: // Linh kiện trước
                    if (scene.inspectedComponent) scene.inspectedComponent->selectPrevSubComponent();
                    break;
                case 103: // Linh kiện tiếp
                    if (scene.inspectedComponent) scene.inspectedComponent->selectNextSubComponent();
                    break;
                case 1: // Đổi chế độ
                    if (scene.compArd) scene.compArd->toggleControlMode();
                    break;
                case 2: // Reset
                    scene.resetSimulation();
                    break;
                case 3: // Bật/Tắt nguồn pin
                    if (scene.compBatSw) scene.compBatSw->toggleSwitch();
                    break;
                case 4: // Giảm áp Buck
                    if (scene.compBuck) scene.compBuck->adjustVoltage(-0.2f);
                    break;
                case 5: // Tăng áp Buck
                    if (scene.compBuck) scene.compBuck->adjustVoltage(0.2f);
                    break;
                default:
                    if (btn.actionId >= 200 && btn.actionId < 300) {
                        int idx = btn.actionId - 200;
                        if (scene.inspectedComponent) scene.inspectedComponent->selectSubComponentByIndex(idx);
                    }
                    break;
            }
            return true;
        }
    }
    return false;
}
