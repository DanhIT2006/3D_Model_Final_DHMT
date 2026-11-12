#include "components/PinAndWire.h"
#include <GL/freeglut.h>
#include <cmath>

void Wire::render3D(const Vec3& p1, const Vec3& p2, bool isPowered) const {
    glDisable(GL_LIGHTING);
    glLineWidth(active && isPowered ? 2.5f : 1.5f);

    Vec3 wireCol = color;
    if (active && isPowered) {
        // Tăng độ sáng rực rỡ khi có dòng điện/tín hiệu chạy qua
        wireCol = Vec3(
            std::min(1.0f, color.x * 1.35f + 0.1f),
            std::min(1.0f, color.y * 1.35f + 0.1f),
            std::min(1.0f, color.z * 1.35f + 0.1f)
        );
    } else {
        // Dây ngắt điện hơi tối mờ
        wireCol = wireCol * 0.65f;
    }

    glColor3f(wireCol.x, wireCol.y, wireCol.z);

    // Vẽ đường cong mềm rủ nhẹ (Catenary / Bezier 3D)
    int segments = 12;
    Vec3 mid = (p1 + p2) * 0.5f;
    // Rủ dây xuống theo trọng lực (trừ Y) tùy khoảng cách
    float dist = p1.distanceTo(p2);
    float sag = std::min(1.8f, dist * 0.12f);
    mid.y -= sag;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / segments;
        float u = 1.0f - t;
        // Đường cong bậc 2 Bezier: B(t) = (1-t)^2 P1 + 2(1-t)t Mid + t^2 P2
        Vec3 pt = p1 * (u * u) + mid * (2.0f * u * t) + p2 * (t * t);
        glVertex3f(pt.x, pt.y, pt.z);
    }
    glEnd();

    // Vẽ đầu bọc co nhiệt / cos chân cắm nhỏ tại 2 đầu dây
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(0.15f, 0.15f, 0.15f);
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
    glPointSize(1.0f);
}
