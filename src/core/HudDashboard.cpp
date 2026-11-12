#include "core/HudDashboard.h"
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

void HudDashboard::render(int screenW, int screenH, const RobotCarScene& scene, const Camera3D& camera, float fps) {
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

    // 6. Gợi ý phím tắt nhanh ở góc trên phải
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

bool HudDashboard::onMouseClick(int mouseX, int mouseY, RobotCarScene& scene) {
    for (const auto& btn : buttons) {
        if (mouseX >= btn.x && mouseX <= btn.x + btn.w &&
            mouseY >= btn.y && mouseY <= btn.y + btn.h) {
            switch (btn.actionId) {
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
            }
            return true;
        }
    }
    return false;
}
