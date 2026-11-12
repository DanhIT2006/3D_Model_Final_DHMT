#include "graphics2d/Clipper2D.h"

int Clipper2D::computeOutCode(int x, int y, const Rect2D& r) {
    int code = INSIDE;
    if (x < r.xMin)      code |= LEFT;
    else if (x > r.xMax) code |= RIGHT;
    if (y < r.yMin)      code |= BOTTOM;
    else if (y > r.yMax) code |= TOP;
    return code;
}

bool Clipper2D::clipLineCohenSutherland(int& x0, int& y0, int& x1, int& y1, const Rect2D& r) {
    // Thuật toán xén đoạn thẳng Cohen-Sutherland (CG.3 trang 55-60)
    int code0 = computeOutCode(x0, y0, r);
    int code1 = computeOutCode(x1, y1, r);
    bool accept = false;

    while (true) {
        if ((code0 | code1) == 0) {
            // Cả 2 điểm nằm trọn vẹn bên trong cửa sổ xén
            accept = true;
            break;
        } else if (code0 & code1) {
            // Cả 2 điểm cùng nằm hẳn về một phía ngoài cửa sổ xén (Loại bỏ hiển nhiên)
            break;
        } else {
            // Một phần đoạn thẳng có thể cắt qua cửa sổ xén
            int x = 0, y = 0;
            // Chọn điểm nằm ngoài cửa sổ xén
            int codeOut = code0 ? code0 : code1;

            if (codeOut & TOP) {
                x = x0 + (int)((float)(x1 - x0) * (r.yMax - y0) / (y1 - y0));
                y = r.yMax;
            } else if (codeOut & BOTTOM) {
                x = x0 + (int)((float)(x1 - x0) * (r.yMin - y0) / (y1 - y0));
                y = r.yMin;
            } else if (codeOut & RIGHT) {
                y = y0 + (int)((float)(y1 - y0) * (r.xMax - x0) / (x1 - x0));
                x = r.xMax;
            } else if (codeOut & LEFT) {
                y = y0 + (int)((float)(y1 - y0) * (r.xMin - x0) / (x1 - x0));
                x = r.xMin;
            }

            if (codeOut == code0) {
                x0 = x;
                y0 = y;
                code0 = computeOutCode(x0, y0, r);
            } else {
                x1 = x;
                y1 = y;
                code1 = computeOutCode(x1, y1, r);
            }
        }
    }
    return accept;
}

std::vector<Point2D> Clipper2D::clipPolygonSutherlandHodgman(const std::vector<Point2D>& poly, const Rect2D& r) {
    if (poly.empty()) return {};

    std::vector<Point2D> outputList = poly;

    // 4 Cạnh của cửa sổ xén: Trái, Phải, Dưới, Trên
    enum Edge { E_LEFT, E_RIGHT, E_BOTTOM, E_TOP };

    for (int edge = 0; edge < 4; ++edge) {
        std::vector<Point2D> inputList = outputList;
        outputList.clear();
        if (inputList.empty()) break;

        Point2D s = inputList.back();

        for (const auto& p : inputList) {
            auto isInside = [&](const Point2D& pt) -> bool {
                switch (edge) {
                    case E_LEFT:   return pt.x >= r.xMin;
                    case E_RIGHT:  return pt.x <= r.xMax;
                    case E_BOTTOM: return pt.y >= r.yMin;
                    case E_TOP:    return pt.y <= r.yMax;
                }
                return false;
            };

            auto computeIntersection = [&](const Point2D& p1, const Point2D& p2) -> Point2D {
                Point2D out;
                if (edge == E_LEFT) {
                    out.x = r.xMin;
                    out.y = p1.y + (int)((float)(p2.y - p1.y) * (r.xMin - p1.x) / (p2.x - p1.x != 0 ? p2.x - p1.x : 1));
                } else if (edge == E_RIGHT) {
                    out.x = r.xMax;
                    out.y = p1.y + (int)((float)(p2.y - p1.y) * (r.xMax - p1.x) / (p2.x - p1.x != 0 ? p2.x - p1.x : 1));
                } else if (edge == E_BOTTOM) {
                    out.y = r.yMin;
                    out.x = p1.x + (int)((float)(p2.x - p1.x) * (r.yMin - p1.y) / (p2.y - p1.y != 0 ? p2.y - p1.y : 1));
                } else { // E_TOP
                    out.y = r.yMax;
                    out.x = p1.x + (int)((float)(p2.x - p1.x) * (r.yMax - p1.y) / (p2.y - p1.y != 0 ? p2.y - p1.y : 1));
                }
                return out;
            };

            if (isInside(p)) {
                if (isInside(s)) {
                    outputList.push_back(p);
                } else {
                    outputList.push_back(computeIntersection(s, p));
                    outputList.push_back(p);
                }
            } else if (isInside(s)) {
                outputList.push_back(computeIntersection(s, p));
            }
            s = p;
        }
    }

    return outputList;
}
