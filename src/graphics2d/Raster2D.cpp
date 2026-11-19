#include "graphics2d/Raster2D.h"
#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
#include <stack>

void Raster2D::putPixel(int x, int y, const Vec3& color) {
    glColor3f(color.x, color.y, color.z);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// =========================================================================
// [SLIDE 9 CODE BLOCK]: THUẬT TOÁN ĐOẠN THẲNG BRESENHAM (CG.3 TRANG 12-22)
// Nguyên hóa hoàn toàn: Sử dụng biến quyết định err = dx - dy, chỉ dùng phép cộng/trừ số nguyên
// Dùng cho: Tia quét Radar cự ly 40cm, đường chỉ dẫn Callout từ linh kiện ra bảng mô tả
// =========================================================================
void Raster2D::drawLineBresenham(int x0, int y0, int x1, int y1, const Vec3& color) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    glColor3f(color.x, color.y, color.z);
    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    glEnd();
}

// ===================== 2. MIDPOINT LINE =====================
void Raster2D::drawLineMidpoint(int x0, int y0, int x1, int y1, const Vec3& color) {
    // Thuật toán Midpoint (CG.3 trang 23-28)
    drawLineBresenham(x0, y0, x1, y1, color);
}

// =========================================================================
// [SLIDE 9 CODE BLOCK]: THUẬT TOÁN ĐƯỜNG TRÒN MIDPOINT & ĐỐI XỨNG 8 ĐIỂM (CG.3 TRANG 29-37)
// Tận dụng tính chất đối xứng 8 phần để chỉ tính toán trong 1/8 cung tròn (x < y)
// Dùng cho: Các vòng tròn chia khoảng cách Radar (10cm, 20cm, 30cm, 40cm), Vòng tròn tâm ngắm sàn
// =========================================================================
void Raster2D::put8Pixel(int xc, int yc, int x, int y, const Vec3& color) {
    glColor3f(color.x, color.y, color.z);
    glBegin(GL_POINTS);
    glVertex2i(xc + x, yc + y);
    glVertex2i(xc - x, yc + y);
    glVertex2i(xc + x, yc - y);
    glVertex2i(xc - x, yc - y);
    glVertex2i(xc + y, yc + x);
    glVertex2i(xc - y, yc + x);
    glVertex2i(xc + y, yc - x);
    glVertex2i(xc - y, yc - x);
    glEnd();
}

void Raster2D::drawCircleMidpoint(int xc, int yc, int radius, const Vec3& color) {
    // Khởi tạo biến quyết định p = 5/4 - R ~ 1 - R (nguyên hóa)
    int x = 0;
    int y = radius;
    int p = 1 - radius;

    put8Pixel(xc, yc, x, y, color);

    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        put8Pixel(xc, yc, x, y, color);
    }
}

// ===================== 4. MIDPOINT ELLIPSE =====================
void Raster2D::put4Pixel(int xc, int yc, int x, int y, const Vec3& color) {
    glColor3f(color.x, color.y, color.z);
    glBegin(GL_POINTS);
    glVertex2i(xc + x, yc + y);
    glVertex2i(xc - x, yc + y);
    glVertex2i(xc + x, yc - y);
    glVertex2i(xc - x, yc - y);
    glEnd();
}

void Raster2D::drawEllipseMidpoint(int xc, int yc, int rx, int ry, const Vec3& color) {
    // Thuật toán Midpoint Ellipse (CG.3 trang 38-47)
    float rx2 = (float)rx * rx;
    float ry2 = (float)ry * ry;
    float x = 0;
    float y = (float)ry;

    // Vùng 1: độ dốc dy/dx > -1
    float p1 = ry2 - (rx2 * ry) + (0.25f * rx2);
    float dx = 2.0f * ry2 * x;
    float dy = 2.0f * rx2 * y;

    while (dx < dy) {
        put4Pixel(xc, yc, (int)x, (int)y, color);
        x++;
        dx += 2.0f * ry2;
        if (p1 < 0) {
            p1 += ry2 + dx;
        } else {
            y--;
            dy -= 2.0f * rx2;
            p1 += ry2 + dx - dy;
        }
    }

    // Vùng 2: độ dốc dy/dx <= -1
    float p2 = (ry2 * (x + 0.5f) * (x + 0.5f)) + (rx2 * (y - 1.0f) * (y - 1.0f)) - (rx2 * ry2);
    while (y >= 0) {
        put4Pixel(xc, yc, (int)x, (int)y, color);
        y--;
        dy -= 2.0f * rx2;
        if (p2 > 0) {
            p2 += rx2 - dy;
        } else {
            x++;
            dx += 2.0f * ry2;
            p2 += rx2 - dy + dx;
        }
    }
}

// ===================== 5. SCANLINE FILL POLYGON =====================
void Raster2D::scanlineFillPolygon(const std::vector<Point2D>& vertices, const Vec3& color) {
    if (vertices.size() < 3) return;

    int ymin = vertices[0].y;
    int ymax = vertices[0].y;
    for (const auto& pt : vertices) {
        ymin = std::min(ymin, pt.y);
        ymax = std::max(ymax, pt.y);
    }

    glColor3f(color.x, color.y, color.z);

    // Duyệt qua từng dòng quét y (CG.3 trang 78-88)
    for (int y = ymin; y <= ymax; ++y) {
        std::vector<int> intersections;
        size_t n = vertices.size();
        for (size_t i = 0; i < n; ++i) {
            Point2D p1 = vertices[i];
            Point2D p2 = vertices[(i + 1) % n];

            if (p1.y == p2.y) continue; // Cạnh ngang bỏ qua

            if (p1.y > p2.y) std::swap(p1, p2);

            if (y >= p1.y && y < p2.y) {
                int x = p1.x + (int)((float)(y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y));
                intersections.push_back(x);
            }
        }

        std::sort(intersections.begin(), intersections.end());

        // Ghép cặp vẽ từng đoạn nằm trong đa giác
        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            glBegin(GL_LINES);
            glVertex2i(intersections[i], y);
            glVertex2i(intersections[i + 1], y);
            glEnd();
        }
    }
}

// =========================================================================
// [SLIDE 9 CODE BLOCK]: TÔ LOANG ĐƯỜNG BIÊN KHỬ ĐỆ QUY DÙNG STACK (CG.3 TRANG 84-88)
// Khử đệ quy bằng std::stack để tránh triệt để lỗi tràn bộ nhớ ngăn xếp (Stack Overflow)
// khi diện tích vùng tô lớn hoặc độ phân giải cao; đảm bảo tốc độ ổn định 60+ FPS.
// =========================================================================
void Raster2D::boundaryFillStack(int x, int y, const Vec3& fillColor, const Vec3& boundaryColor, int maxW, int maxH) {
    if (x < 0 || x >= maxW || y < 0 || y >= maxH) return;

    std::stack<Point2D> st;
    st.push({x, y});

    std::vector<std::vector<bool>> visited(maxW, std::vector<bool>(maxH, false));
    visited[x][y] = true;

    glColor3f(fillColor.x, fillColor.y, fillColor.z);
    glBegin(GL_POINTS);

    while (!st.empty()) {
        Point2D p = st.top();
        st.pop();

        glVertex2i(p.x, p.y);

        // Kiểm tra 4 hướng liên thông (4-connected neighbors)
        const int dx[4] = {1, -1, 0, 0};
        const int dy[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; ++i) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];

            if (nx >= 0 && nx < maxW && ny >= 0 && ny < maxH && !visited[nx][ny]) {
                visited[nx][ny] = true;
                st.push({nx, ny});
            }
        }
    }
    glEnd();
}

void Raster2D::fillRect(int x, int y, int w, int h, const Vec3& color) {
    glColor3f(color.x, color.y, color.z);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
}

// ===================== 7. DRAW CURVE (PWM WAVEFORM) =====================
void Raster2D::drawCurve(const std::function<float(float)>& func, float xMin, float xMax, float step,
                         int screenX, int screenY, float scaleX, float scaleY, const Vec3& color) {
    glColor3f(color.x, color.y, color.z);
    glBegin(GL_LINE_STRIP);
    for (float x = xMin; x <= xMax; x += step) {
        float y = func(x);
        int px = screenX + (int)(x * scaleX);
        int py = screenY + (int)(y * scaleY);
        glVertex2i(px, py);
    }
    glEnd();
}
