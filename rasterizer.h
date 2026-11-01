#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "config.h"
#include <GL/glut.h>
#include <cstring>
#include <vector>

// =========================================================================
// 1. FRAMEBUFFER & PIPELINE RASTER HOÁ ĐIỂM ẢNH (GIAI ĐOẠN 0.1)
// =========================================================================

// Mảng đệm màu pixel trong RAM để đạt hiệu năng tối đa và hỗ trợ getPixel O(1)
extern MauSac g_frameBuffer[CUA_SO_CAO][CUA_SO_RONG];

// Cửa sổ xén hình hiện hành (mặc định là toàn màn hình hoặc CenterViewport)
extern int g_clipMinX, g_clipMaxX, g_clipMinY, g_clipMaxY;

inline void datVungXen(int minX, int maxX, int minY, int maxY) {
    g_clipMinX = std::max(0, minX);
    g_clipMaxX = std::min(CUA_SO_RONG - 1, maxX);
    g_clipMinY = std::max(0, minY);
    g_clipMaxY = std::min(CUA_SO_CAO - 1, maxY);
}

inline void xoaFrameBuffer(MauSac mauNen) {
    for (int y = 0; y < CUA_SO_CAO; ++y) {
        for (int x = 0; x < CUA_SO_RONG; ++x) {
            g_frameBuffer[y][x] = mauNen;
        }
    }
}

// Hàm ghi pixel duy nhất thỏa mãn kiểm tra biên và ghi vào RAM
inline void putPixel(int x, int y, MauSac c) {
    if (x < g_clipMinX || x > g_clipMaxX || y < g_clipMinY || y > g_clipMaxY) return;
    g_frameBuffer[y][x] = c;
}

// Đọc pixel trực tiếp từ RAM O(1) phục vụ Boundary Fill
inline MauSac getPixel(int x, int y) {
    if (x < 0 || x >= CUA_SO_RONG || y < 0 || y >= CUA_SO_CAO) return MauSac(0, 0, 0);
    return g_frameBuffer[y][x];
}

// Đẩy toàn bộ mảng frameBuffer ra màn hình qua OpenGL glDrawPixels
inline void renderFrameBuffer() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(CUA_SO_RONG, CUA_SO_CAO, GL_RGBA, GL_UNSIGNED_BYTE, g_frameBuffer);
}

// =========================================================================
// 2. THUẬT TOÁN VẼ ĐOẠN THẲNG BRESENHAM TỔNG QUÁT 8 OCTANT
// Giáo trình VKU CG.3 (Trang 10-18)
// =========================================================================

inline void veDoanThangBresenham(int x1, int y1, int x2, int y2, MauSac c) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int x = x1;
    int y = y1;

    // Trường hợp 1: |m| <= 1 (dx >= dy)
    if (dx >= dy) {
        int P = 2 * dy - dx;
        int const1 = 2 * dy;
        int const2 = 2 * (dy - dx);

        for (int i = 0; i <= dx; ++i) {
            putPixel(x, y, c);
            if (P < 0) {
                P += const1;
            } else {
                P += const2;
                y += sy;
            }
            x += sx;
        }
    }
    // Trường hợp 2: |m| > 1 (dx < dy)
    else {
        int P = 2 * dx - dy;
        int const1 = 2 * dx;
        int const2 = 2 * (dx - dy);

        for (int i = 0; i <= dy; ++i) {
            putPixel(x, y, c);
            if (P < 0) {
                P += const1;
            } else {
                P += const2;
                x += sx;
            }
            y += sy;
        }
    }
}

// Vẽ đoạn thẳng có độ dày (t = 2..4 px) cho dây điện 3D
inline void veDoanThangDay(int x1, int y1, int x2, int y2, MauSac c, int doDay = 2) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int banKinh = doDay / 2;

    for (int off = -banKinh; off <= banKinh; ++off) {
        if (dx >= dy) {
            veDoanThangBresenham(x1, y1 + off, x2, y2 + off, c);
        } else {
            veDoanThangBresenham(x1 + off, y1, x2 + off, y2, c);
        }
    }
}

// =========================================================================
// 3. THUẬT TOÁN VẼ ĐƯỜNG TRÒN BRESENHAM & MIDPOINT (Trang 23-28, CG.3)
// =========================================================================

inline void put8DiemTron(int xc, int yc, int x, int y, MauSac c) {
    putPixel(xc + x, yc + y, c);
    putPixel(xc - x, yc + y, c);
    putPixel(xc + x, yc - y, c);
    putPixel(xc - x, yc - y, c);
    putPixel(xc + y, yc + x, c);
    putPixel(xc - y, yc + x, c);
    putPixel(xc + y, yc - x, c);
    putPixel(xc - y, yc - x, c);
}

inline void veDuongTronBresenham(int xc, int yc, int r, MauSac c) {
    int x = 0;
    int y = r;
    int p = 3 - 2 * r;

    while (x <= y) {
        put8DiemTron(xc, yc, x, y, c);
        if (p < 0) {
            p += 4 * x + 6;
        } else {
            p += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// Vẽ đường tròn đặc (tô đầy) cho các điểm nút/chân cắm
inline void veHinhTronDac(int xc, int yc, int r, MauSac c) {
    for (int y = -r; y <= r; ++y) {
        int xBound = (int)std::sqrt(r * r - y * y);
        for (int x = -xBound; x <= xBound; ++x) {
            putPixel(xc + x, yc + y, c);
        }
    }
}

// =========================================================================
// 4. VẼ HÌNH CHỮ NHẬT VÀ VIỀN
// =========================================================================

inline void veHCN(int x0, int y0, int x1, int y1, MauSac c) {
    int xMin = std::min(x0, x1), xMax = std::max(x0, x1);
    int yMin = std::min(y0, y1), yMax = std::max(y0, y1);
    for (int y = yMin; y <= yMax; ++y) {
        for (int x = xMin; x <= xMax; ++x) {
            putPixel(x, y, c);
        }
    }
}

inline void veVienHCN(int x0, int y0, int x1, int y1, MauSac c) {
    veDoanThangBresenham(x0, y0, x1, y0, c);
    veDoanThangBresenham(x1, y0, x1, y1, c);
    veDoanThangBresenham(x1, y1, x0, y1, c);
    veDoanThangBresenham(x0, y1, x0, y0, c);
}

// =========================================================================
// 5. VẼ CHỮ BẰNG FONT RASTER BITMAP (Trang 93-95, CG.3)
// =========================================================================

inline void veChu(int x, int y, const char* str, MauSac c, void* font = GLUT_BITMAP_HELVETICA_12) {
    // Vẽ trực tiếp qua OpenGL Bitmap text khi render UI
    glRasterPos2i(x, y);
    glColor3ub(c.r, c.g, c.b);
    while (*str) {
        glutBitmapCharacter(font, *str);
        ++str;
    }
}

#endif // RASTERIZER_H
