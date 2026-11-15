#pragma once
#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include "../components/ComponentPrimitives.h"
#include "../math/Vec3.h"

class RoboticArmRenderer {
public:
    /**
     * @brief Vẽ Cánh tay Robot 2 khâu (Boom + Forearm) không đâm xuyên sàn Tầng 2 & vươn qua đầu HC-SR04
     * @param yawDeg Góc xoay mâm đế SRV_BASE (-90°..+90°)
     * @param liftDeg Góc nâng/hạ cánh tay: 0.0° (Hạ gắp sát đất) -> 90.0° (Nâng cao thu về)
     * @param gripOpenRatio Độ mở ngàm kẹp SRV_GRIP (0.0 = Đóng chặt, 1.0 = Mở rộng)
     */
    static void renderArmAssembly(
        float yawDeg, float liftDeg, float gripOpenRatio,
        bool isBaseSelected, bool isElbowSelected, bool isGripSelected)
    {
        // t = 0.0f khi hạ thấp nhất để gắp (liftDeg = 0), t = 1.0f khi nâng cao tối đa (liftDeg = 90)
        float t = std::max(0.0f, std::min(1.0f, liftDeg / 90.0f));

        // Khi t = 0 (Hạ gắp): shoulderUp = 14 độ (vươn xa), forearmBend = 68 độ (hạ đúng tâm khối hàng y = 1.65cm)
        // Khi t = 1 (Nâng cao): shoulderUp = 62 độ (nâng bổng lên cao), forearmBend = 32 độ
        float shoulderUpDeg  = 14.0f + t * 48.0f;
        float forearmBendDeg = 68.0f - t * 36.0f;
        float pitchDownDeg   = forearmBendDeg - shoulderUpDeg;

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
        glTranslatef(0.0f, 2.00f, 0.0f);
        CompDraw::drawBeveledBox(2.0f, 2.00f, 4.0f, {0.14f, 0.15f, 0.17f}, isBaseSelected);
        // Tem nhãn kim loại MG996R bên sườn
        glTranslatef(0.0f, 0.0f, 0.0f);
        CompDraw::drawBeveledBox(2.04f, 0.85f, 2.8f, {0.10f, 0.45f, 0.85f}, isBaseSelected);
        glPopMatrix();

        // 2 Trụ đỡ khớp khuỷu (Standoff Towers) hai bên trái/phải
        for (float side : {-1.45f, +1.45f}) {
            glPushMatrix();
            glTranslatef(side, 2.20f, 0.0f);
            CompDraw::drawBeveledBox(0.35f, 2.40f, 1.10f, {0.72f, 0.75f, 0.80f}, isBaseSelected);
            glPopMatrix();
        }

        // ====================================================================
        // KHỚP 2: CÁNH TAY 2 KHÂU (BOOM ARM + FOREARM LINK)
        // ====================================================================
        // Dịch chuyển tâm hệ trục lên chốt vai (y = 3.00 cm trên sàn Tầng 2)
        glTranslatef(0.0f, 3.00f, 0.0f);

        // --------------------------------------------------------------------
        // KHÂU 1 (BOOM ARM - Dài L1 = 8.20 cm):
        // Dùng -shoulderUpDeg để vươn LÊN TRÊN (+Y) và RA TRƯỚC (+Z)
        // --------------------------------------------------------------------
        glRotatef(-shoulderUpDeg, 1.0f, 0.0f, 0.0f);

        const float L1 = 8.20f;
        // Vẽ 2 thanh dầm hợp kim song song (Twin-Boom CNC Links)
        for (float side : {-1.10f, +1.10f}) {
            glPushMatrix();
            glTranslatef(side, 0.0f, L1 * 0.5f);
            CompDraw::drawBeveledBox(0.30f, 0.70f, L1, {0.82f, 0.85f, 0.90f}, isElbowSelected);
            glPopMatrix();
        }

        // Thanh ngang gia cố giữa 2 cánh tay đòn
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, L1 * 0.5f);
        CompDraw::drawBeveledBox(2.20f, 0.30f, 0.40f, {0.22f, 0.25f, 0.30f}, isElbowSelected);
        glPopMatrix();

        // Dịch tới đỉnh Khâu 1 (Đầu mút vươn qua mép trước Sàn Tầng 2)
        glTranslatef(0.0f, 0.0f, L1);

        // --------------------------------------------------------------------
        // KHÂU 2 (FOREARM + SRV_ELBOW - Dài L2 = 8.60 cm):
        // Từ trên cao chúc xuống vượt qua đầu HC-SR04 tới điểm gắp
        // --------------------------------------------------------------------
        glRotatef(+forearmBendDeg, 1.0f, 0.0f, 0.0f);

        const float L2 = 8.60f;
        for (float side : {-0.85f, +0.85f}) {
            glPushMatrix();
            glTranslatef(side, 0.0f, L2 * 0.5f);
            CompDraw::drawBeveledBox(0.26f, 0.58f, L2, {0.92f, 0.72f, 0.15f}, isElbowSelected);
            glPopMatrix();
        }

        // Dịch ra đầu cổ tay SRV_GRIP (Lúc này ở ngoài mũi xe, vượt qua HC-SR04!)
        glTranslatef(0.0f, 0.0f, L2);

        // ====================================================================
        // KHỚP 3: CỔ TAY & NGÀM GẮP BÁNH RĂNG ĐỐI XỨNG (SRV_GRIP)
        // ====================================================================
        // Góc bù cổ tay: Giữ cho 2 ngón kẹp nghiêng nhẹ -25.0 độ hướng về phía trước
        glRotatef(25.0f - pitchDownDeg, 1.0f, 0.0f, 0.0f);

        // 3.1. Khung gá cổ tay & Thân Servo kẹp SRV_GRIP
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.85f);
        CompDraw::drawBeveledBox(2.50f, 1.20f, 1.70f, {0.16f, 0.18f, 0.22f}, isGripSelected);
        glPopMatrix();

        // 3.2. Hai ngón kẹp cơ khí đóng/mở đối xứng (Symmetric Parallel Jaws)
        float jawAngle = 6.0f + gripOpenRatio * 40.0f;

        for (int dir : {-1, +1}) { // dir = -1 (Ngàm Trái), dir = +1 (Ngàm Phải)
            glPushMatrix();
            glTranslatef(dir * 0.80f, 0.0f, 1.70f);

            // Bánh răng truyền động màu cam/vàng tại gốc ngón kẹp
            glPushMatrix();
            glTranslatef(0.0f, 0.35f, 0.0f);
            CompDraw::drawElectrolyticCap(0.55f, 0.20f, {0.95f, 0.50f, 0.12f}, isGripSelected);
            glPopMatrix();

            // Xoay ngón kẹp mở ra hoặc khép vào đối xứng
            glRotatef(dir * jawAngle, 0.0f, 1.0f, 0.0f);

            // Đốt ngón kẹp ngoài (Proximal Phalanx - Hợp kim cam/xám)
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.95f);
            CompDraw::drawBeveledBox(0.28f, 0.55f, 1.90f, {0.95f, 0.48f, 0.12f}, isGripSelected);
            glPopMatrix();

            // Đốt mũi kẹp bẻ quặp vào trong (Distal Claw Tip)
            glTranslatef(0.0f, 0.0f, 1.85f);
            glRotatef(-dir * (jawAngle * 0.75f + 20.0f), 0.0f, 1.0f, 0.0f);

            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.80f);
            CompDraw::drawBeveledBox(0.24f, 0.55f, 1.60f, {0.85f, 0.88f, 0.92f}, isGripSelected);

            // Miếng đệm cao su ma sát màu đỏ ở mặt trong ngón kẹp
            glTranslatef(-dir * 0.14f, 0.0f, 0.10f);
            CompDraw::drawBeveledBox(0.08f, 0.48f, 1.20f, {0.85f, 0.18f, 0.18f}, isGripSelected);
            glPopMatrix();

            glPopMatrix();
        }

        glPopMatrix();
    }

    /**
     * @brief Hàm tính chính xác tọa độ 3D của Tâm Điểm Kẹp (Giữa 2 miếng đệm đỏ của ngàm gắp)
     * Đồng bộ tuyệt đối với các lệnh glTranslatef / glRotatef trong renderArmAssembly
     */
    static Vec3 computeGripTipWorldPos(
        const Vec3& carWorldPos, float carYawRad,
        float floor2Height,      // Độ cao mặt sàn Tầng 2 (6.35f cm)
        float baseYawDeg,        // Góc xoay đế SRV_BASE (0 độ khi nhìn thẳng)
        float liftDeg)           // Góc nâng trên đồng hồ LIFT (0..90 độ, 0 = Hạ thấp nhất để gắp)
    {
        float t = std::max(0.0f, std::min(1.0f, liftDeg / 90.0f));

        // Góc khớp đã tối ưu tầm vươn xa (z ≈ 18.2cm) và hạ sát tâm khối hàng (y ≈ 1.65cm) khi liftDeg = 0
        float shoulderUpDeg  = 14.0f + t * 48.0f;
        float forearmBendDeg = 68.0f - t * 36.0f;

        const float baseLocalZ = 6.20f; // Đế xoay đặt tiến về mũi Sàn Tầng 2
        const float baseTowerY = 3.00f; // Chiều cao chốt khớp Vai trên mặt Sàn Tầng 2
        const float L1         = 8.20f; // Chiều dài Khâu 1 (Thanh xám)
        const float L2         = 8.60f; // Chiều dài Khâu 2 (Thanh vàng)
        const float clawReach  = 2.65f; // Khoảng cách từ cổ tay tới tâm 2 miếng đệm cao su đỏ

        float sRad = shoulderUpDeg * 3.14159265f / 180.0f;
        // Góc tổng của Khâu 2 so với phương ngang (chúc xuống mặt đất một góc pitchDownRad)
        float pitchDownDeg = forearmBendDeg - shoulderUpDeg;
        float fRad = pitchDownDeg * 3.14159265f / 180.0f;
        // Góc nghiêng của bàn kẹp (chúc nhẹ 25 độ về phía trước)
        float cRad = 25.0f * 3.14159265f / 180.0f;

        // 1. Tính tầm vươn dọc theo mặt phẳng cánh tay (forwardDist, heightY)
        float forwardFromBase = L1 * std::cos(sRad) + L2 * std::cos(fRad) + clawReach * std::cos(cRad);
        float localY = floor2Height + baseTowerY
                     + L1 * std::sin(sRad)
                     - L2 * std::sin(fRad)
                     - clawReach * std::sin(cRad);

        // 2. Xoay theo góc Yaw của đế SRV_BASE
        float bYawRad = baseYawDeg * 3.14159265f / 180.0f;
        float carLocalX = std::sin(bYawRad) * forwardFromBase;
        float carLocalZ = baseLocalZ + std::cos(bYawRad) * forwardFromBase;

        // 3. Chuyển từ hệ tọa độ cục bộ của xe sang hệ tọa độ Thế giới (World Space)
        Vec3 worldGrip;
        worldGrip.x = carWorldPos.x + carLocalX * std::cos(carYawRad) + carLocalZ * std::sin(carYawRad);
        worldGrip.y = carWorldPos.y + std::max(0.85f, localY); // Không bao giờ chạm xuyên mặt đất
        worldGrip.z = carWorldPos.z - carLocalX * std::sin(carYawRad) + carLocalZ * std::cos(carYawRad);
        return worldGrip;
    }

    /**
     * @brief Vẽ Vòng tròn Tâm Ngắm Chiếu Xuống Sàn (Drop-Target Reticle)
     */
    static void renderDropTargetReticle(const Vec3& gripWorldPos, const Vec3& targetCargoPos, bool isGrasped) {
        if (isGrasped) return;

        float distXZ = std::hypot(gripWorldPos.x - targetCargoPos.x, gripWorldPos.z - targetCargoPos.z);
        float distY  = std::abs(gripWorldPos.y - targetCargoPos.y);

        Vec3 reticleColor;
        float ringRadius = 2.4f;
        if (distXZ <= 3.8f && distY <= 3.2f) {
            reticleColor = Vec3(0.15f, 1.0f, 0.35f); // Xanh lá cây sáng (RGB: 0.15, 1.0, 0.35 - Vùng gắp lý tưởng!)
            ringRadius = 1.9f;
        } else if (distXZ <= 5.5f) {
            reticleColor = Vec3(1.0f, 0.65f, 0.1f);  // Vàng cam (Đang tiến gần)
            ringRadius = 2.2f;
        } else {
            reticleColor = Vec3(0.15f, 0.65f, 1.0f); // Xanh dương (Còn ở xa)
            ringRadius = 2.6f;
        }

        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);

        // 1. Đường dọi thẳng đứng từ ngàm kẹp xuống sàn y = 0.02f
        glColor3f(reticleColor.x, reticleColor.y, reticleColor.z);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA); // Nét đứt
        glBegin(GL_LINES);
        glVertex3f(gripWorldPos.x, gripWorldPos.y, gripWorldPos.z);
        glVertex3f(gripWorldPos.x, 0.02f, gripWorldPos.z);
        glEnd();
        glDisable(GL_LINE_STIPPLE);

        // 2. Vòng tròn tâm ngắm trên mặt sàn
        glPushMatrix();
        glTranslatef(gripWorldPos.x, 0.02f, gripWorldPos.z);

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
