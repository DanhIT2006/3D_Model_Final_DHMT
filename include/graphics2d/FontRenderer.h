#pragma once
#include "math/Vec3.h"
#include <string>

/**
 * @brief Vẽ font chữ 2D Bitmap trên HUD và nhãn 3D trong không gian
 * Theo giáo trình CG.3 (trang 96-102)
 */
class FontRenderer {
public:
    enum FontSize {
        SMALL,  // Helvetica 10/12
        NORMAL, // Helvetica 12/Bitmap 9x15
        LARGE   // Helvetica 18
    };

    // Vẽ chuỗi ký tự 2D tại tọa độ màn hình (x, y)
    static void drawText2D(int x, int y, const std::string& text, const Vec3& color, FontSize size = NORMAL);

    // Vẽ chuỗi ký tự kèm đổ bóng tương phản giúp chữ luôn rõ nét trên mọi nền 3D
    static void drawText2DWithShadow(int x, int y, const std::string& text, const Vec3& color, FontSize size = NORMAL);

    // Vẽ nhãn chữ 3D tại tọa độ thế giới (Billboard text)
    static void drawText3D(const Vec3& worldPos, const std::string& text, const Vec3& color);

    static int getTextWidth(const std::string& text, FontSize size = NORMAL);
};
