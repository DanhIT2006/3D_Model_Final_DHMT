#include "components/HardwareComponent.h"
#include <GL/freeglut.h>

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
    // Đặt LED ở góc trên linh kiện
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
