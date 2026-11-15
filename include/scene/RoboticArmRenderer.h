#pragma once
#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include "../components/ComponentPrimitives.h"
#include "../math/Vec3.h"

class RoboticArmRenderer {
public:
    /**
     * @brief Vẽ toàn bộ Cánh tay Robot 3 bậc tự do cơ khí độ nét cao:
     * @param yawDeg Góc quay mâm đế SRV_BASE (-90°..+90°)
     * @param pitchDeg Góc nâng hạ cánh tay đôi SRV_ELBOW (0°..85°)
     * @param gripOpenRatio Độ mở ngàm kẹp SRV_GRIP (0.0 = Đóng chặt, 1.0 = Mở rộng)
     */
    static void renderArmAssembly(
        float yawDeg, float pitchDeg, float gripOpenRatio,
        bool isBaseSelected, bool isElbowSelected, bool isGripSelected)
    {
        glPushMatrix();

        // ====================================================================
        // KHỚP 1: ĐẾ XOAY TRÒN & SERVO MG996R ĐẾ (SRV_BASE - GÓC YAW)
        // ====================================================================
        // 1.1. Mâm đế cố định
        glPushMatrix();
        glTranslatef(0.0f, 0.25f, 0.0f);
        CompDraw::drawElectrolyticCap(2.6f, 0.50f, {0.18f, 0.20f, 0.24f}, isBaseSelected);
        glPopMatrix();

        // Áp dụng phép quay Góc Yaw quanh trục Y cho toàn bộ phần trên
        glRotatef(yawDeg, 0.0f, 1.0f, 0.0f);

        // 1.2. Mâm xoay Turntable & Khung U-Bracket đỡ Servo
        glPushMatrix();
        glTranslatef(0.0f, 0.90f, 0.0f);
        CompDraw::drawBeveledBox(4.2f, 0.35f, 4.2f, {0.25f, 0.28f, 0.32f}, isBaseSelected);
        glPopMatrix();

        // Thân Servo MG996R đặt giữa đế xoay (Có tem xanh đặc trưng)
        glPushMatrix();
        glTranslatef(0.0f, 2.20f, 0.0f);
        CompDraw::drawBeveledBox(2.0f, 2.20f, 4.0f, {0.14f, 0.15f, 0.17f}, isBaseSelected);
        // Tem nhãn kim loại MG996R bên sườn
        glTranslatef(0.0f, 0.0f, 0.0f);
        CompDraw::drawBeveledBox(2.04f, 0.85f, 2.8f, {0.10f, 0.45f, 0.85f}, isBaseSelected);
        glPopMatrix();

        // 2 Trụ đỡ khớp khuỷu (Standoff Towers) hai bên trái/phải
        for (float side : {-1.45f, +1.45f}) {
            glPushMatrix();
            glTranslatef(side, 2.80f, 0.0f);
            CompDraw::drawBeveledBox(0.35f, 3.60f, 1.10f, {0.72f, 0.75f, 0.80f}, isBaseSelected);
            glPopMatrix();
        }

        // ====================================================================
        // KHỚP 2: CÁNH TAY ĐÒN ĐÔI SONG SONG (SRV_ELBOW - GÓC PITCH)
        // ====================================================================
        // Dịch chuyển tâm hệ trục lên chốt trục khuỷu (y = 4.30 cm)
        glTranslatef(0.0f, 4.30f, 0.0f);

        // Đĩa Servo Horn tròn hợp kim ở hai bên chốt khuỷu
        glRotatef(pitchDeg, 1.0f, 0.0f, 0.0f);

        const float armLength = 8.20f;
        // Vẽ 2 thanh dầm hợp kim song song (Twin-Boom CNC Links) vươn về phía trước (+Z)
        for (float side : {-1.15f, +1.15f}) {
            // Thanh dầm chính
            glPushMatrix();
            glTranslatef(side, 0.35f, armLength * 0.5f);
            CompDraw::drawBeveledBox(0.28f, 0.75f, armLength, {0.82f, 0.85f, 0.88f}, isElbowSelected);
            glPopMatrix();

            // Thanh giằng liên kết (Parallel Linkage Rod) màu vàng đồng phía dưới
            glPushMatrix();
            glTranslatef(side * 0.75f, -0.35f, armLength * 0.5f);
            CompDraw::drawBeveledBox(0.16f, 0.22f, armLength * 0.92f, {0.90f, 0.70f, 0.15f}, isElbowSelected);
            glPopMatrix();
        }

        // Thanh ngang gia cố giữa 2 cánh tay đòn
        glPushMatrix();
        glTranslatef(0.0f, 0.35f, armLength * 0.45f);
        CompDraw::drawBeveledBox(2.30f, 0.35f, 0.45f, {0.22f, 0.25f, 0.30f}, isElbowSelected);
        glPopMatrix();

        // ====================================================================
        // KHỚP 3: CỔ TAY & NGÀM GẮP BÁNH RĂNG ĐỐI XỨNG (SRV_GRIP - GÓC GRIP)
        // ====================================================================
        // Dịch chuyển tâm hệ trục ra đầu cổ tay (z = armLength)
        glTranslatef(0.0f, 0.0f, armLength);
        // Bù góc nghiêng để cổ tay luôn hướng song song với mặt đất (Cơ cấu bình hành)
        glRotatef(-pitchDeg * 0.65f, 1.0f, 0.0f, 0.0f);

        // 3.1. Khung gá cổ tay & Thân Servo kẹp SRV_GRIP
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.95f);
        CompDraw::drawBeveledBox(2.60f, 1.35f, 1.90f, {0.16f, 0.18f, 0.22f}, isGripSelected);
        glPopMatrix();

        // 3.2. Hai ngón kẹp cơ khí đóng/mở đối xứng (Symmetric Parallel Jaws)
        // gripOpenRatio: 0.0 (đóng sát = 6 độ) -> 1.0 (mở rộng = 46 độ)
        float jawAngle = 6.0f + gripOpenRatio * 40.0f;

        for (int dir : {-1, +1}) { // dir = -1 (Ngàm Trái), dir = +1 (Ngàm Phải)
            glPushMatrix();
            glTranslatef(dir * 0.85f, 0.0f, 1.85f);

            // Bánh răng truyền động màu cam/vàng tại gốc ngón kẹp
            glPushMatrix();
            glTranslatef(0.0f, 0.45f, 0.0f);
            CompDraw::drawElectrolyticCap(0.65f, 0.25f, {0.95f, 0.50f, 0.12f}, isGripSelected);
            glPopMatrix();

            // Xoay ngón kẹp mở ra hoặc khép vào đối xứng
            glRotatef(dir * jawAngle, 0.0f, 1.0f, 0.0f);

            // Đốt ngón kẹp ngoài (Proximal Phalanx - Hợp kim cam/xám)
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 1.15f);
            CompDraw::drawBeveledBox(0.32f, 0.65f, 2.30f, {0.95f, 0.48f, 0.12f}, isGripSelected);
            glPopMatrix();

            // Đốt mũi kẹp bẻ quặp vào trong (Distal Claw Tip)
            glTranslatef(0.0f, 0.0f, 2.25f);
            glRotatef(-dir * (jawAngle * 0.75f + 22.0f), 0.0f, 1.0f, 0.0f);

            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.95f);
            CompDraw::drawBeveledBox(0.28f, 0.65f, 1.90f, {0.85f, 0.88f, 0.92f}, isGripSelected);

            // Miếng đệm cao su ma sát màu đỏ/đen ở mặt trong ngón kẹp
            glTranslatef(-dir * 0.16f, 0.0f, 0.15f);
            CompDraw::drawBeveledBox(0.10f, 0.58f, 1.35f, {0.85f, 0.18f, 0.18f}, isGripSelected);
            glPopMatrix();

            glPopMatrix();
        }

        glPopMatrix();
    }

    /**
     * @brief Vẽ Vòng tròn Tâm Ngắm Chiếu Xuống Sàn (Drop-Target Reticle)
     */
    static void renderDropTargetReticle(const Vec3& gripWorldPos, const Vec3& targetCargoPos, bool isGrasped) {
        if (isGrasped) return;

        float dist3D = gripWorldPos.distanceTo(targetCargoPos);
        float distXZ = std::hypot(gripWorldPos.x - targetCargoPos.x, gripWorldPos.z - targetCargoPos.z);

        Vec3 reticleColor;
        float ringRadius = 2.4f;
        if (dist3D < 3.2f) {
            reticleColor = Vec3(0.2f, 1.0f, 0.3f); // Xanh lá cây sáng (Vùng gắp lý tưởng!)
            ringRadius = 1.8f;
        } else if (distXZ < 2.5f) {
            reticleColor = Vec3(1.0f, 0.65f, 0.1f); // Vàng cam (Đã căn chuẩn tâm XZ)
            ringRadius = 2.1f;
        } else {
            reticleColor = Vec3(0.15f, 0.65f, 1.0f); // Xanh dương (Còn ở xa)
            ringRadius = 2.6f;
        }

        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);

        // 1. Đường dọi thẳng đứng từ ngàm kẹp xuống sàn y = 0.05f
        glColor3f(reticleColor.x, reticleColor.y, reticleColor.z);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA); // Nét đứt
        glBegin(GL_LINES);
        glVertex3f(gripWorldPos.x, gripWorldPos.y, gripWorldPos.z);
        glVertex3f(gripWorldPos.x, 0.05f, gripWorldPos.z);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        // 2. Vòng tròn tâm ngắm trên mặt sàn
        glPushMatrix();
        glTranslatef(gripWorldPos.x, 0.05f, gripWorldPos.z);

        // Vòng ngoài
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 36; ++i) {
            float a = i * 2.0f * 3.14159265f / 36.0f;
            glVertex3f(std::cos(a) * ringRadius, 0.0f, std::sin(a) * ringRadius);
        }
        glEnd();

        // Dấu chữ thập tâm
        glBegin(GL_LINES);
        glVertex3f(-ringRadius * 1.3f, 0.0f, 0.0f); glVertex3f(ringRadius * 1.3f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, -ringRadius * 1.3f); glVertex3f(0.0f, 0.0f, ringRadius * 1.3f);
        glEnd();

        // Vòng trong đồng tâm
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 24; ++i) {
            float a = i * 2.0f * 3.14159265f / 24.0f;
            glVertex3f(std::cos(a) * (ringRadius * 0.45f), 0.0f, std::sin(a) * (ringRadius * 0.45f));
        }
        glEnd();

        glPopMatrix();
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    /**
     * @brief Vẽ Khối Vật Thể Hàng Hóa Chuẩn Kỹ Thuật (Pallet gỗ + Thùng hàng công nghiệp)
     */
    static void renderCargoCrate(const Vec3& pos, float w, float h, float d, bool isGrasped) {
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);

        // 1. Đế Pallet gỗ công nghiệp phía dưới (y = -h*0.5f)
        float palletH = 0.45f;
        glPushMatrix();
        glTranslatef(0.0f, -h * 0.5f + palletH * 0.5f, 0.0f);
        // Nan gỗ trên
        CompDraw::drawBeveledBox(w * 1.05f, 0.15f, d * 1.05f, {0.65f, 0.48f, 0.28f});
        // 3 Gối đỡ chân pallet tạo khe càng nâng
        for (float px : {-w * 0.38f, 0.0f, w * 0.38f}) {
            glPushMatrix();
            glTranslatef(px, -0.15f, 0.0f);
            CompDraw::drawBeveledBox(0.35f, 0.20f, d * 1.05f, {0.52f, 0.38f, 0.22f});
            glPopMatrix();
        }
        glPopMatrix();

        // 2. Thân thùng hàng Container / Cargo Crate
        Vec3 crateCol = isGrasped ? Vec3(0.2f, 0.85f, 0.4f) : Vec3(0.95f, 0.40f, 0.12f);
        glPushMatrix();
        glTranslatef(0.0f, palletH * 0.5f, 0.0f);
        CompDraw::drawBeveledBox(w, h - palletH, d, crateCol, isGrasped);

        // Bo viền đen chống va đập 4 góc
        for (float cx : {-w * 0.48f, w * 0.48f}) {
            for (float cz : {-d * 0.48f, d * 0.48f}) {
                glPushMatrix();
                glTranslatef(cx, 0.0f, cz);
                CompDraw::drawBeveledBox(0.25f, h - palletH + 0.05f, 0.25f, {0.14f, 0.14f, 0.16f});
                glPopMatrix();
            }
        }

        // Rãnh kẹp hai bên sườn (Clamping Channels) nơi đệm cao su ngàm gắp tiếp xúc
        for (float sx : {-w * 0.51f, w * 0.51f}) {
            glPushMatrix();
            glTranslatef(sx, 0.0f, 0.0f);
            CompDraw::drawBeveledBox(0.08f, (h - palletH) * 0.55f, d * 0.70f, {0.20f, 0.22f, 0.26f});
            glPopMatrix();
        }

        // Tem nhãn vận chuyển / Cảnh báo nguy hiểm trên nắp thùng
        glPushMatrix();
        glTranslatef(0.0f, (h - palletH) * 0.51f, 0.0f);
        CompDraw::drawBeveledBox(w * 0.60f, 0.02f, d * 0.60f, {0.92f, 0.92f, 0.95f});
        glPopMatrix();

        glPopMatrix();

        glPopMatrix();
    }
};
