#include "components/HardwareComponent.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>

HardwareComponent::HardwareComponent(const std::string& code, const std::string& name, const std::string& group)
    : SceneNode(code, name), code(code), funcGroup(group) {}

Pin* HardwareComponent::getPin(const std::string& name) {
    for (auto& pin : pins) {
        if (pin.pinName == name) return &pin;
    }
    return nullptr;
}

const Pin* HardwareComponent::getPin(const std::string& name) const {
    for (const auto& pin : pins) {
        if (pin.pinName == name) return &pin;
    }
    return nullptr;
}

void HardwareComponent::addPin(const std::string& name, PinType type, const Vec3& offset, bool isOutput) {
    Pin p;
    p.pinName = name;
    p.type = type;
    p.relativeOffset = offset;
    p.voltage = 0.0f;
    p.pwmDuty = 0.0f;
    p.isOutput = isOutput;
    pins.push_back(p);
}

void HardwareComponent::renderPinsAndDetails() {
    glDisable(GL_LIGHTING);

    // Vẽ các đầu cắm Pin mạ vàng / bạc
    for (const auto& pin : pins) {
        glPushMatrix();
        glTranslatef(pin.relativeOffset.x, pin.relativeOffset.y, pin.relativeOffset.z);

        if (pin.type == PinType::POWER_7V4) {
            glColor3f(1.0f, 0.2f, 0.2f);
        } else if (pin.type == PinType::POWER_5V) {
            glColor3f(1.0f, 0.6f, 0.1f);
        } else if (pin.type == PinType::POWER_GND) {
            glColor3f(0.1f, 0.1f, 0.1f);
        } else {
            glColor3f(0.9f, 0.8f, 0.2f); // Chân tín hiệu mạ vàng
        }

        glPointSize(4.0f);
        glBegin(GL_POINTS);
        glVertex3f(0, 0, 0);
        glEnd();
        glPointSize(1.0f);

        glPopMatrix();
    }

    // Vẽ LED chỉ thị trạng thái nguồn (Xanh lá khi có điện, Đỏ khi ngắt điện)
    glPushMatrix();
    glTranslatef(cadSize.x * 0.35f, cadSize.y * 0.52f, cadSize.z * 0.35f);
    if (isPowered) {
        glColor3f(0.1f, 1.0f, 0.2f); // LED xanh lá rực sáng
    } else {
        glColor3f(0.35f, 0.05f, 0.05f); // LED tắt / đỏ tối
    }
    glutSolidSphere(0.18f, 8, 8);
    glPopMatrix();
}

void HardwareComponent::render(bool wireframeMode) {
    if (!visible) return;

    glPushMatrix();
    Mat4 localM = getLocalMatrix();
    glMultMatrixf(localM.data());

    if (wireframeMode) {
        mesh.drawWireframe(isSelected ? Vec3(1.0f, 0.85f, 0.0f) : Vec3(0.0f, 0.95f, 0.4f), isSelected ? 2.5f : 1.0f);
    } else {
        mesh.drawSolid(true);
        if (isSelected) {
            mesh.drawWireframe(Vec3(1.0f, 0.85f, 0.0f), 2.5f);
        }
        renderPinsAndDetails();
    }

    // Vẽ các nút con (nếu có)
    for (auto* child : children) {
        child->render(wireframeMode);
    }

    glPopMatrix();
}

// =========================================================================
// === HỆ THỐNG QUAN SÁT VI MÔ (WORKBENCH MODE) ===
// =========================================================================

void HardwareComponent::initSubComponents() {
    // Được nạp chồng ở các lớp con cụ thể
}

void HardwareComponent::addSubComponent(const SubComponent& sub) {
    subComponents.push_back(sub);
}

SubComponent* HardwareComponent::getSelectedSubComponent() {
    if (selectedSubCompIndex >= 0 && selectedSubCompIndex < (int)subComponents.size()) {
        return &subComponents[selectedSubCompIndex];
    }
    return nullptr;
}

void HardwareComponent::selectSubComponentByIndex(int index) {
    for (auto& s : subComponents) {
        s.isSelected = false;
    }
    if (index >= 0 && index < (int)subComponents.size()) {
        selectedSubCompIndex = index;
        subComponents[selectedSubCompIndex].isSelected = true;
    } else {
        selectedSubCompIndex = -1;
    }
}

void HardwareComponent::selectNextSubComponent() {
    if (subComponents.empty()) return;
    int nextIdx = (selectedSubCompIndex + 1) % (int)subComponents.size();
    selectSubComponentByIndex(nextIdx);
}

void HardwareComponent::selectPrevSubComponent() {
    if (subComponents.empty()) return;
    int prevIdx = (selectedSubCompIndex - 1 + (int)subComponents.size()) % (int)subComponents.size();
    selectSubComponentByIndex(prevIdx);
}

SubComponent* HardwareComponent::pickSubComponent(const Vec3& rayOrigin, const Vec3& rayDir) {
    float closestDist = 1e9f;
    int bestIndex = -1;

    for (int i = 0; i < (int)subComponents.size(); ++i) {
        const auto& sub = subComponents[i];
        Vec3 pos = sub.localPos + Vec3(0.0f, 0.1f, 0.0f);
        Vec3 halfSize = sub.size * 0.5f + Vec3(0.25f, 0.25f, 0.25f);

        Vec3 minPt = pos - halfSize;
        Vec3 maxPt = pos + halfSize;

        float tmin = (minPt.x - rayOrigin.x) / (std::fabs(rayDir.x) > 1e-8f ? rayDir.x : 1e-8f);
        float tmax = (maxPt.x - rayOrigin.x) / (std::fabs(rayDir.x) > 1e-8f ? rayDir.x : 1e-8f);
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (minPt.y - rayOrigin.y) / (std::fabs(rayDir.y) > 1e-8f ? rayDir.y : 1e-8f);
        float tymax = (maxPt.y - rayOrigin.y) / (std::fabs(rayDir.y) > 1e-8f ? rayDir.y : 1e-8f);
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) continue;
        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (minPt.z - rayOrigin.z) / (std::fabs(rayDir.z) > 1e-8f ? rayDir.z : 1e-8f);
        float tzmax = (maxPt.z - rayOrigin.z) / (std::fabs(rayDir.z) > 1e-8f ? rayDir.z : 1e-8f);
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) continue;
        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        if (tmax < 0.0f) continue;
        float hitDist = (tmin < 0.0f) ? tmax : tmin;
        if (hitDist < closestDist) {
            closestDist = hitDist;
            bestIndex = i;
        }
    }

    if (bestIndex != -1) {
        selectSubComponentByIndex(bestIndex);
        return &subComponents[bestIndex];
    }
    return nullptr;
}

void HardwareComponent::renderWorkbench(bool wireframeMode) {
    glPushMatrix();
    // Đặt linh kiện cách mặt bàn 0.1cm
    glTranslatef(0.0f, 0.1f, 0.0f);

    // 1. Vẽ bo mạch chủ (Base Board)
    if (wireframeMode) {
        mesh.drawWireframe(Vec3(0.0f, 0.8f, 0.4f), 1.2f);
    } else {
        mesh.drawSolid(true);
        mesh.drawWireframe(Vec3(0.1f, 0.4f, 0.2f), 1.0f);
    }

    // 2. Vẽ tất cả linh kiện nhỏ (SubComponents)
    for (size_t i = 0; i < subComponents.size(); ++i) {
        auto& sub = subComponents[i];
        glPushMatrix();
        glTranslatef(sub.localPos.x, sub.localPos.y, sub.localPos.z);

        if (wireframeMode) {
            sub.mesh.drawWireframe(sub.isSelected ? Vec3(1.0f, 0.85f, 0.0f) : sub.color, sub.isSelected ? 2.5f : 1.0f);
        } else {
            sub.mesh.drawSolid(true);
            if (sub.isSelected) {
                // Viền vàng nổi bật khi được chọn
                sub.mesh.drawWireframe(Vec3(1.0f, 0.9f, 0.0f), 2.5f);
            }
        }
        glPopMatrix();

        // 3. Nếu đang được chọn: vẽ khung hộp bao định vị phát sáng (Bounding Box Glow)
        if (sub.isSelected) {
            glDisable(GL_LIGHTING);
            glPushMatrix();
            glTranslatef(sub.localPos.x, sub.localPos.y, sub.localPos.z);
            glColor3f(1.0f, 0.85f, 0.1f);
            glLineWidth(2.0f);

            // Hộp bao dây nét đứt xung quanh linh kiện
            float hx = sub.size.x * 0.5f + 0.12f;
            float hy = sub.size.y * 0.5f + 0.12f;
            float hz = sub.size.z * 0.5f + 0.12f;

            glBegin(GL_LINE_LOOP);
            glVertex3f(-hx, -hy, -hz); glVertex3f(hx, -hy, -hz);
            glVertex3f(hx, hy, -hz);   glVertex3f(-hx, hy, -hz);
            glEnd();

            glBegin(GL_LINE_LOOP);
            glVertex3f(-hx, -hy, hz);  glVertex3f(hx, -hy, hz);
            glVertex3f(hx, hy, hz);    glVertex3f(-hx, hy, hz);
            glEnd();

            glBegin(GL_LINES);
            glVertex3f(-hx, -hy, -hz); glVertex3f(-hx, -hy, hz);
            glVertex3f(hx, -hy, -hz);  glVertex3f(hx, -hy, hz);
            glVertex3f(hx, hy, -hz);   glVertex3f(hx, hy, hz);
            glVertex3f(-hx, hy, -hz);  glVertex3f(-hx, hy, hz);
            glEnd();

            // Mũi tên chỉ thị thẳng đứng từ trên trỏ xuống linh kiện
            glBegin(GL_LINES);
            glVertex3f(0.0f, hy + 1.2f, 0.0f);
            glVertex3f(0.0f, hy + 0.2f, 0.0f);
            glVertex3f(0.0f, hy + 0.2f, 0.0f);
            glVertex3f(-0.15f, hy + 0.45f, 0.0f);
            glVertex3f(0.0f, hy + 0.2f, 0.0f);
            glVertex3f(0.15f, hy + 0.45f, 0.0f);
            glEnd();

            glLineWidth(1.0f);
            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }

    // 4. Kết xuất chân pin thực tế và linh kiện nổi (Tản nhiệt nhôm, tụ nhôm, IC socket...)
    renderPinsAndDetails();

    glPopMatrix();
}

void HardwareComponent::onSubComponentAction(const std::string& subId) {
    (void)subId;
}
