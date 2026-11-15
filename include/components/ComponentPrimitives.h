#pragma once
#include <GL/freeglut.h>
#include <cmath>
#include "../math/Vec3.h"

namespace CompDraw {

    // 1. Vẽ khối hộp có vát viền cạnh nổi (Chống bệt màu đen trên nền tối & Khử Z-fighting)
    inline void drawBeveledBox(float w, float h, float d, Vec3 color, bool highlight = false) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        if (highlight) {
            glColor3f(1.0f, 0.85f, 0.15f); // Sáng vàng khi đang được chọn
        } else {
            glColor3f(color.x, color.y, color.z);
        }

        glPushMatrix();
        glScalef(w, h, d);
        glutSolidCube(1.0f);

        // Vẽ đường viền cạnh mảnh sáng hơn 25% để tách khối rõ nét
        glDisable(GL_POLYGON_OFFSET_FILL);
        glLineWidth(highlight ? 2.2f : 1.1f);
        if (highlight) {
            glColor3f(1.0f, 0.95f, 0.40f);
        } else {
            glColor3f(std::fmin(1.0f, color.x + 0.22f),
                      std::fmin(1.0f, color.y + 0.22f),
                      std::fmin(1.0f, color.z + 0.22f));
        }
        glutWireCube(1.002f);
        glPopMatrix();
    }

    // 2. Vẽ Tụ Hóa Thực Tế (220uF / 47uF) - KHỬ TRIỆT ĐỂ SỌC ĐEN TRẮNG (Z-FIGHTING)
    inline void drawElectrolyticCap(float radius, float height, Vec3 sleeveColor, bool highlight = false) {
        const int slices = 24;
        float capMetalY = height * 0.88f; // 88% dưới là vỏ bọc cách điện, 12% đỉnh là nắp nhôm bạc

        // A. Vẽ thân vỏ bọc cách điện (Từ y = 0 đến y = capMetalY)
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float angle = (2.0f * 3.14159265f * i) / slices;
            float nx = std::cos(angle);
            float nz = std::sin(angle);
            // Dải sọc bạc báo cực âm (-) ở góc từ -25 độ đến +25 độ
            bool isNegativeStripe = (i <= 2 || i >= slices - 2);
            if (highlight) {
                glColor3f(1.0f, 0.82f, 0.15f);
            } else if (isNegativeStripe) {
                glColor3f(0.82f, 0.85f, 0.88f); // Sọc trắng bạc cực âm
            } else {
                glColor3f(sleeveColor.x, sleeveColor.y, sleeveColor.z);
            }
            glNormal3f(nx, 0.0f, nz);
            glVertex3f(nx * radius, 0.0f, nz * radius);
            glVertex3f(nx * radius, capMetalY, nz * radius);
        }
        glEnd();

        // B. Vẽ vành nắp nhôm bạc phía trên (Từ y = capMetalY đến y = height) -> KHÔNG TRÙNG VỎ!
        glColor3f(highlight ? 1.0f : 0.88f, highlight ? 0.92f : 0.90f, highlight ? 0.45f : 0.94f);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float angle = (2.0f * 3.14159265f * i) / slices;
            float nx = std::cos(angle);
            float nz = std::sin(angle);
            glNormal3f(nx, 0.0f, nz);
            glVertex3f(nx * radius * 0.96f, capMetalY, nz * radius * 0.96f);
            glVertex3f(nx * radius * 0.96f, height, nz * radius * 0.96f);
        }
        glEnd();

        // C. Mặt phẳng đỉnh nắp nhôm
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, height, 0.0f);
        for (int i = 0; i <= slices; ++i) {
            float angle = (2.0f * 3.14159265f * i) / slices;
            glVertex3f(std::cos(angle) * radius * 0.96f, height, std::sin(angle) * radius * 0.96f);
        }
        glEnd();

        // D. Rãnh chữ thập chống nổ trên đỉnh nắp nhôm (Cao hơn đỉnh 0.005f để không Z-fighting)
        glDisable(GL_LIGHTING);
        glColor3f(0.35f, 0.38f, 0.42f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex3f(-radius * 0.65f, height + 0.005f, 0.0f);
        glVertex3f(+radius * 0.65f, height + 0.005f, 0.0f);
        glVertex3f(0.0f, height + 0.005f, -radius * 0.65f);
        glVertex3f(0.0f, height + 0.005f, +radius * 0.65f);
        glEnd();
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    // 3. Vẽ Khối Tản Nhiệt Nhôm Xẻ Rãnh 7 Lá & IC L298N
    inline void drawL298NHeatsinkAndIC(bool highlight = false) {
        Vec3 hsColor = {0.16f, 0.18f, 0.20f};
        // Tấm lưng nhôm phía trước (Nơi bắt vít IC L298N)
        glPushMatrix();
        glTranslatef(0.0f, 0.95f, 0.35f);
        drawBeveledBox(2.20f, 1.90f, 0.18f, hsColor, highlight);
        glPopMatrix();

        // 7 Lá tản nhiệt xẻ rãnh dọc phía sau
        for (int i = 0; i < 7; ++i) {
            float fx = -0.96f + i * 0.32f;
            glPushMatrix();
            glTranslatef(fx, 0.95f, -0.15f);
            drawBeveledBox(0.10f, 1.90f, 0.85f, hsColor, highlight);
            glPopMatrix();
        }

        // Thân IC công suất L298N (Multiwatt-15) ốp trước tấm nhôm + Ốc bắt tản nhiệt
        glPushMatrix();
        glTranslatef(0.0f, 0.85f, 0.52f);
        drawBeveledBox(1.55f, 1.05f, 0.22f, {0.10f, 0.10f, 0.12f}, highlight);
        // Ốc vít bắt tản nhiệt mạ bạc ở giữa
        glColor3f(0.85f, 0.88f, 0.92f);
        glPushMatrix();
        glTranslatef(0.0f, 0.28f, 0.12f);
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(0.16f, 0.06f, 10, 1);
        glPopMatrix();
        glPopMatrix();

        // 15 Chân kim loại màu bạc cắm xuống mạch
        glColor3f(0.80f, 0.82f, 0.85f);
        for (int p = 0; p < 15; ++p) {
            float px = -0.70f + p * 0.10f;
            float pz = (p % 2 == 0) ? 0.55f : 0.65f;
            glPushMatrix();
            glTranslatef(px, 0.22f, pz);
            glScalef(0.045f, 0.44f, 0.05f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    // 4. Vẽ Hàng Chân Kim Loại Rời (Pin Header)
    inline void drawPinHeaderMale(int numPins, float pitch, bool hasJumpersOnEnds, bool highlight = false) {
        float totalLen = numPins * pitch;
        // Đế nhựa đen dưới chân
        glPushMatrix();
        glTranslatef(0.0f, 0.10f, 0.0f);
        drawBeveledBox(totalLen, 0.20f, 0.24f, {0.12f, 0.12f, 0.14f}, highlight);
        glPopMatrix();

        // Từng chân kim loại mạ vàng dựng đứng rời nhau
        float startX = -((numPins - 1) * pitch) * 0.5f;
        for (int i = 0; i < numPins; ++i) {
            float px = startX + i * pitch;
            glPushMatrix();
            glTranslatef(px, 0.38f, 0.0f);
            drawBeveledBox(0.065f, 0.68f, 0.065f, {0.92f, 0.78f, 0.20f}, highlight);
            glPopMatrix();
        }

        // Mũ Jumper đen cắm ở 2 đầu ENA và ENB (nếu có)
        if (hasJumpersOnEnds && numPins >= 6) {
            glPushMatrix();
            glTranslatef(startX, 0.52f, 0.0f);
            drawBeveledBox(0.20f, 0.45f, 0.22f, {0.18f, 0.20f, 0.25f}, highlight);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(-startX, 0.52f, 0.0f);
            drawBeveledBox(0.20f, 0.45f, 0.22f, {0.18f, 0.20f, 0.25f}, highlight);
            glPopMatrix();
        }
    }

    // 5. Vẽ ốc vít kim loại mạ bạc có rãnh tua-vít
    inline void drawScrew(float radius, float height) {
        glColor3f(0.82f, 0.85f, 0.88f);
        glPushMatrix();
        glRotatef(-90.0f, 1, 0, 0);
        glutSolidCylinder(radius, height, 12, 1);
        glPopMatrix();

        // Rãnh tua-vít
        glDisable(GL_LIGHTING);
        glColor3f(0.35f, 0.38f, 0.42f);
        glLineWidth(1.4f);
        glBegin(GL_LINES);
        glVertex3f(-radius * 0.75f, height + 0.002f, 0.0f);
        glVertex3f( radius * 0.75f, height + 0.002f, 0.0f);
        glEnd();
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }
}
