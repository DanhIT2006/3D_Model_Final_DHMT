#pragma once
#include "math/Vec3.h"
#include <vector>
#include <functional>

struct Point2D {
    int x, y;
};

/**
 * @brief [SLIDE 9]: Cài đặt đầy đủ các thuật toán vẽ 2D cơ sở theo giáo trình CG.3
 * Bresenham Line, Midpoint Line, Midpoint Circle (put8pixel), Midpoint Ellipse (put4pixel),
 * Tô màu dòng quét (Scanline Fill), Tô loang khử đệ quy bằng Stack (Boundary Fill Stack).
 */
class Raster2D {
public:
    // Vẽ điểm pixel tại tọa độ (x, y)
    static void putPixel(int x, int y, const Vec3& color);

    // 1. Đoạn thẳng Bresenham (CG.3 trang 12-22) - Dùng cho tia quét Radar & Callout lines
    static void drawLineBresenham(int x0, int y0, int x1, int y1, const Vec3& color);

    // 2. Đoạn thẳng Midpoint (CG.3 trang 23-28)
    static void drawLineMidpoint(int x0, int y0, int x1, int y1, const Vec3& color);

    // 3. Đường tròn Midpoint với thuật toán 8 điểm đối xứng put8pixel (CG.3 trang 29-37) - Dùng cho vòng cự ly Radar & Tâm ngắm
    static void drawCircleMidpoint(int xc, int yc, int radius, const Vec3& color);
    static void put8Pixel(int xc, int yc, int x, int y, const Vec3& color);

    // 4. Ellipse Midpoint với thuật toán 4 điểm đối xứng put4pixel (CG.3 trang 38-47)
    static void drawEllipseMidpoint(int xc, int yc, int rx, int ry, const Vec3& color);
    static void put4Pixel(int xc, int yc, int x, int y, const Vec3& color);

    // 5. Tô màu đa giác theo dòng quét Scanline Fill (CG.3 trang 78-88) - Dùng cho nút bấm & thanh trạng thái
    static void scanlineFillPolygon(const std::vector<Point2D>& vertices, const Vec3& color);

    // 6. Tô loang đường biên khử đệ quy dùng Stack (CG.3 trang 84-88) - Tránh lỗi tràn ngăn xếp (Stack Overflow)
    static void boundaryFillStack(int x, int y, const Vec3& fillColor, const Vec3& boundaryColor, int maxW, int maxH);
    static void fillRect(int x, int y, int w, int h, const Vec3& color);

    // 7. Vẽ đường cong biểu diễn đồ thị sóng xung PWM y = f(x)
    static void drawCurve(const std::function<float(float)>& func, float xMin, float xMax, float step,
                          int screenX, int screenY, float scaleX, float scaleY, const Vec3& color);
};
