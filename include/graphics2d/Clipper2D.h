#pragma once
#include "graphics2d/Raster2D.h"
#include <vector>

struct Rect2D {
    int xMin, yMin, xMax, yMax;
};

/**
 * @brief Cài đặt các thuật toán xén hình 2D theo giáo trình CG.3 (trang 54-76)
 * Cohen-Sutherland (mã vùng 4-bit) & Sutherland-Hodgman Polygon Clipping
 */
class Clipper2D {
public:
    // Mã vùng 4-bit cho Cohen-Sutherland (CG.3 trang 55-60)
    static const int INSIDE = 0; // 0000
    static const int LEFT   = 1; // 0001
    static const int RIGHT  = 2; // 0010
    static const int BOTTOM = 4; // 0100
    static const int TOP    = 8; // 1000

    static int computeOutCode(int x, int y, const Rect2D& clipRect);

    // 1. Xén đoạn thẳng Cohen-Sutherland
    static bool clipLineCohenSutherland(int& x0, int& y0, int& x1, int& y1, const Rect2D& clipRect);

    // 2. Xén đa giác Sutherland-Hodgman
    static std::vector<Point2D> clipPolygonSutherlandHodgman(const std::vector<Point2D>& poly, const Rect2D& clipRect);
};
