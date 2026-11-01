#ifndef XEN_VA_TO_MAU_H
#define XEN_VA_TO_MAU_H

#include "cau_hinh.h"
#include "toan_hoc_do_hoa.h"
#include "raster_co_so.h"
#include <vector>
#include <algorithm>

// =========================================================================
// 1. THUẬT TOÁN XÉN ĐOẠN THẲNG COHEN-SUTHERLAND
// Giáo trình VKU CG.3 (Trang 54-61)
// =========================================================================

// Gán mã vùng 4-bit (Trang 55-56, CG.3)
inline int maVungCohen(const Diem2D& p, const Diem2D& wmin, const Diem2D& wmax) {
    int ma = 0;
    if (p.x < wmin.x) ma |= 1; // Bit 0: LEFT   (0001)
    if (p.x > wmax.x) ma |= 2; // Bit 1: RIGHT  (0010)
    if (p.y < wmin.y) ma |= 4; // Bit 2: BOTTOM (0100)
    if (p.y > wmax.y) ma |= 8; // Bit 3: TOP    (1000)
    return ma;
}

// Xén đoạn thẳng AB theo cửa sổ wmin, wmax (Trang 60-61, CG.3)
inline bool xenDoanThangCohenSutherland(Diem2D& A, Diem2D& B, const Diem2D& wmin, const Diem2D& wmax) {
    int thoat = 0, chapNhan = 1;

    while (!thoat) {
        int maA = maVungCohen(A, wmin, wmax);
        int maB = maVungCohen(B, wmin, wmax);

        // Trường hợp 1: Nằm hoàn toàn bên trong
        if ((maA | maB) == 0) {
            thoat = 1;
            chapNhan = 1;
        }
        // Trường hợp 2: Nằm hoàn toàn bên ngoài cùng một phía
        else if ((maA & maB) != 0) {
            thoat = 1;
            chapNhan = 0;
        }
        // Trường hợp 3: Cắt biên cửa sổ
        else {
            if (maA == 0) {
                std::swap(A, B);
                std::swap(maA, maB);
            }

            // Đoạn thẳng đứng (Trang 58, CG.3)
            if (std::abs(A.x - B.x) < 1e-5f) {
                A.y = (A.y > wmax.y) ? wmax.y : wmin.y;
            }
            // Đoạn thẳng xiên (Trang 59, 61, CG.3)
            else {
                double m = (double)(B.y - A.y) / (B.x - A.x);

                if (A.x < wmin.x) {
                    A.y += (wmin.x - A.x) * m;
                    A.x = wmin.x;
                } else if (A.x > wmax.x) {
                    A.y += (wmax.x - A.x) * m;
                    A.x = wmax.x;
                } else if (A.y < wmin.y) {
                    A.x += (wmin.y - A.y) / m;
                    A.y = wmin.y;
                } else if (A.y > wmax.y) {
                    A.x += (wmax.y - A.y) / m;
                    A.y = wmax.y;
                }
            }
        }
    }

    return (chapNhan == 1);
}

// =========================================================================
// 2. THUẬT TOÁN XÉN ĐA GIÁC SUTHERLAND-HODGMAN
// Giáo trình VKU CG.3 (Trang 73-76)
// =========================================================================

// Xén đa giác theo một cạnh biên (0: Trái, 1: Phải, 2: Dưới, 3: Trên)
inline std::vector<Diem2D> xenDaGiacTheoCanh(const std::vector<Diem2D>& poly, int canh, const Diem2D& wmin, const Diem2D& wmax) {
    std::vector<Diem2D> ketQua;
    if (poly.empty()) return ketQua;

    auto namBenTrong = [&](const Diem2D& p) {
        if (canh == 0) return p.x >= wmin.x;
        if (canh == 1) return p.x <= wmax.x;
        if (canh == 2) return p.y >= wmin.y;
        return p.y <= wmax.y;
    };

    auto timGiaoDiem = [&](const Diem2D& p1, const Diem2D& p2) {
        Diem2D giao = p1;
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;

        if (canh == 0) {
            float t = (wmin.x - p1.x) / (std::abs(dx) < 1e-6f ? 1e-6f : dx);
            giao.x = wmin.x;
            giao.y = p1.y + dy * t;
        } else if (canh == 1) {
            float t = (wmax.x - p1.x) / (std::abs(dx) < 1e-6f ? 1e-6f : dx);
            giao.x = wmax.x;
            giao.y = p1.y + dy * t;
        } else if (canh == 2) {
            float t = (wmin.y - p1.y) / (std::abs(dy) < 1e-6f ? 1e-6f : dy);
            giao.x = p1.x + dx * t;
            giao.y = wmin.y;
        } else {
            float t = (wmax.y - p1.y) / (std::abs(dy) < 1e-6f ? 1e-6f : dy);
            giao.x = p1.x + dx * t;
            giao.y = wmax.y;
        }
        return giao;
    };

    int n = (int)poly.size();
    for (int i = 0; i < n; ++i) {
        Diem2D A = poly[i];
        Diem2D B = poly[(i + 1) % n];

        bool trongA = namBenTrong(A);
        bool trongB = namBenTrong(B);

        // Quy tắc 1: Trong -> Trong (Lưu B)
        if (trongA && trongB) {
            ketQua.push_back(B);
        }
        // Quy tắc 2: Trong -> Ngoài (Lưu giao điểm P)
        else if (trongA && !trongB) {
            ketQua.push_back(timGiaoDiem(A, B));
        }
        // Quy tắc 3: Ngoài -> Trong (Lưu giao điểm P và B)
        else if (!trongA && trongB) {
            ketQua.push_back(timGiaoDiem(A, B));
            ketQua.push_back(B);
        }
        // Quy tắc 4: Ngoài -> Ngoài (Không lưu đỉnh nào)
    }

    return ketQua;
}

// Xén tuần tự đa giác qua 4 biên hình chữ nhật
inline std::vector<Diem2D> xenDaGiacSutherlandHodgman(std::vector<Diem2D> poly, const Diem2D& wmin, const Diem2D& wmax) {
    for (int canh = 0; canh < 4; ++canh) {
        poly = xenDaGiacTheoCanh(poly, canh, wmin, wmax);
        if (poly.empty()) break;
    }
    return poly;
}

// =========================================================================
// 3. THUẬT TOÁN TÔ MÀU THEO DÒNG QUÉT (SCANLINE FILL)
// Giáo trình VKU CG.3 (Trang 80-82)
// =========================================================================

struct CanhET {
    int yMax;
    float xOfYMin;
    float invM; // 1/m = (x2 - x1) / (y2 - y1)
};

inline void toMauScanline(const std::vector<Diem2D>& poly, MauSac c) {
    if (poly.size() < 3) return;

    // Tìm yMin và yMax của đa giác
    float yMinF = poly[0].y, yMaxF = poly[0].y;
    for (const auto& pt : poly) {
        if (pt.y < yMinF) yMinF = pt.y;
        if (pt.y > yMaxF) yMaxF = pt.y;
    }

    int yBottom = std::max(g_clipMinY, (int)std::floor(yMinF));
    int yTop    = std::min(g_clipMaxY, (int)std::ceil(yMaxF));

    int n = (int)poly.size();

    // Duyệt từng dòng quét
    for (int y = yBottom; y <= yTop; ++y) {
        std::vector<float> giaoX;

        for (int i = 0; i < n; ++i) {
            Diem2D p1 = poly[i];
            Diem2D p2 = poly[(i + 1) % n];

            // Bỏ qua cạnh ngang
            if (std::abs(p1.y - p2.y) < 1e-4f) continue;

            // Kiểm tra dòng quét y có cắt cạnh p1-p2
            if ((y >= p1.y && y < p2.y) || (y >= p2.y && y < p1.y)) {
                float x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                giaoX.push_back(x);
            }
        }

        std::sort(giaoX.begin(), giaoX.end());

        // Tô màu các cặp giao điểm theo dòng quét
        for (size_t i = 0; i + 1 < giaoX.size(); i += 2) {
            int x1 = std::max(g_clipMinX, (int)std::ceil(giaoX[i]));
            int x2 = std::min(g_clipMaxX, (int)std::floor(giaoX[i + 1]));
            for (int x = x1; x <= x2; ++x) {
                putPixel(x, y, c);
            }
        }
    }
}

// =========================================================================
// 4. THUẬT TOÁN TÔ LOANG BOUNDARY FILL DÙNG STACK (KHÔNG ĐỆ QUY)
// Giáo trình VKU CG.3 (Trang 85-88) - Dùng tô sáng linh kiện được chọn
// =========================================================================

struct Diem2i {
    int x, y;
};

inline void kiemTraVaDayStack(int x, int y, MauSac fillCol, MauSac boundCol, std::vector<Diem2i>& stack) {
    if (x < g_clipMinX || x > g_clipMaxX || y < g_clipMinY || y > g_clipMaxY) return;

    MauSac cur = getPixel(x, y);
    // Tô ngay trước khi push vào stack để không bao giờ bị push trùng (Trang 87-88, CG.3)
    if (cur != fillCol && cur != boundCol) {
        putPixel(x, y, fillCol);
        stack.push_back({x, y});
    }
}

inline void toLoangBoundaryFillStack(int x0, int y0, MauSac fillCol, MauSac boundCol) {
    if (x0 < g_clipMinX || x0 > g_clipMaxX || y0 < g_clipMinY || y0 > g_clipMaxY) return;

    MauSac startCol = getPixel(x0, y0);
    if (startCol == fillCol || startCol == boundCol) return;

    std::vector<Diem2i> stack;
    stack.reserve(16384);

    putPixel(x0, y0, fillCol);
    stack.push_back({x0, y0});

    while (!stack.empty()) {
        Diem2i p = stack.back();
        stack.pop_back();

        // Xét 4 điểm lân cận
        kiemTraVaDayStack(p.x - 1, p.y,     fillCol, boundCol, stack);
        kiemTraVaDayStack(p.x + 1, p.y,     fillCol, boundCol, stack);
        kiemTraVaDayStack(p.x,     p.y + 1, fillCol, boundCol, stack);
        kiemTraVaDayStack(p.x,     p.y - 1, fillCol, boundCol, stack);
    }
}

#endif // XEN_VA_TO_MAU_H
