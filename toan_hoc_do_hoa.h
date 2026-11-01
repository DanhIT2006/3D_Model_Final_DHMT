#ifndef TOAN_HOC_DO_HOA_H
#define TOAN_HOC_DO_HOA_H

#include "cau_hinh.h"
#include <cmath>

// =========================================================================
// 1. CẤU TRÚC ĐIỂM & VECTOR 2D, 3D
// =========================================================================

struct Diem2D {
    float x, y;

    Diem2D() : x(0), y(0) {}
    Diem2D(float _x, float _y) : x(_x), y(_y) {}

    Diem2D operator+(const Diem2D& o) const { return Diem2D(x + o.x, y + o.y); }
    Diem2D operator-(const Diem2D& o) const { return Diem2D(x - o.x, y - o.y); }
    Diem2D operator*(float s) const         { return Diem2D(x * s, y * s); }
};

struct Diem3D {
    float x, y, z;

    Diem3D() : x(0), y(0), z(0) {}
    Diem3D(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Diem3D operator+(const Diem3D& o) const { return Diem3D(x + o.x, y + o.y, z + o.z); }
    Diem3D operator-(const Diem3D& o) const { return Diem3D(x - o.x, y - o.y, z - o.z); }
    Diem3D operator*(float s) const         { return Diem3D(x * s, y * s, z * s); }

    float doDai() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Diem3D chuanHoa() const {
        float d = doDai();
        if (d < 1e-6f) return Diem3D(0, 0, 0);
        return Diem3D(x / d, y / d, z / d);
    }

    static float tichVoHuong(const Diem3D& a, const Diem3D& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Diem3D tichCoHuong(const Diem3D& a, const Diem3D& b) {
        return Diem3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
};

typedef Diem3D Vec3;

// =========================================================================
// 2. MA TRẬN 4x4 BIẾN ĐỔI THUẦN NHẤT 3D (QUY ƯỚC VECTOR HÀNG: Q = P * M)
// Giáo trình VKU CG.4 (Trang 5-16)
// =========================================================================

struct MaTran4x4 {
    float m[4][4];

    MaTran4x4() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    // Nhân ma trận từ trái sang phải: M = M1 * M2
    MaTran4x4 operator*(const MaTran4x4& o) const {
        MaTran4x4 res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res.m[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    res.m[i][j] += m[i][k] * o.m[k][j];
                }
            }
        }
        return res;
    }

    // Biến đổi điểm 3D theo vector hàng: Q = [x, y, z, 1] * M
    Diem3D bienDoi(const Diem3D& p) const {
        float nx = p.x * m[0][0] + p.y * m[1][0] + p.z * m[2][0] + m[3][0];
        float ny = p.x * m[0][1] + p.y * m[1][1] + p.z * m[2][1] + m[3][1];
        float nz = p.x * m[0][2] + p.y * m[1][2] + p.z * m[2][2] + m[3][2];
        float nw = p.x * m[0][3] + p.y * m[1][3] + p.z * m[2][3] + m[3][3];
        if (std::abs(nw) > 1e-6f && std::abs(nw - 1.0f) > 1e-6f) {
            nx /= nw; ny /= nw; nz /= nw;
        }
        return Diem3D(nx, ny, nz);
    }

    // Nghịch đảo ma trận biến đổi cứng (Rigid Body: R chuyển vị, t' = -t * R^T)
    MaTran4x4 nghichDaoBienDoiCung() const {
        MaTran4x4 inv;
        // Chuyển vị phần quay 3x3
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                inv.m[i][j] = m[j][i];

        // Vector tịnh tiến mới: -t * R^T
        float tx = m[3][0], ty = m[3][1], tz = m[3][2];
        inv.m[3][0] = -(tx * inv.m[0][0] + ty * inv.m[1][0] + tz * inv.m[2][0]);
        inv.m[3][1] = -(tx * inv.m[0][1] + ty * inv.m[1][1] + tz * inv.m[2][1]);
        inv.m[3][2] = -(tx * inv.m[0][2] + ty * inv.m[1][2] + tz * inv.m[2][2]);
        inv.m[3][3] = 1.0f;
        return inv;
    }
};

// 2.1. Ma trận Tịnh tiến 3D (Trang 6, CG.4)
inline MaTran4x4 taoMaTranTinhTien(float tx, float ty, float tz) {
    MaTran4x4 mat;
    mat.m[3][0] = tx; mat.m[3][1] = ty; mat.m[3][2] = tz;
    return mat;
}

// 2.2. Ma trận Tỉ lệ 3D (Trang 9, CG.4)
inline MaTran4x4 taoMaTranTiLe(float sx, float sy, float sz) {
    MaTran4x4 mat;
    mat.m[0][0] = sx; mat.m[1][1] = sy; mat.m[2][2] = sz;
    return mat;
}

// Tỉ lệ quanh tâm bất kỳ (Trang 10, CG.4)
inline MaTran4x4 taoMaTranTiLeQuanhTam(const Diem3D& tam, float sx, float sy, float sz) {
    MaTran4x4 mat = taoMaTranTiLe(sx, sy, sz);
    mat.m[3][0] = (1.0f - sx) * tam.x;
    mat.m[3][1] = (1.0f - sy) * tam.y;
    mat.m[3][2] = (1.0f - sz) * tam.z;
    return mat;
}

// 2.3. Ma trận Quay quanh các trục Ox, Oy, Oz (Trang 12, CG.4)
inline MaTran4x4 taoMaTranQuayX(float rad) {
    MaTran4x4 mat;
    float c = std::cos(rad), s = std::sin(rad);
    mat.m[1][1] = c;  mat.m[1][2] = s;
    mat.m[2][1] = -s; mat.m[2][2] = c;
    return mat;
}

inline MaTran4x4 taoMaTranQuayY(float rad) {
    MaTran4x4 mat;
    float c = std::cos(rad), s = std::sin(rad);
    mat.m[0][0] = c;  mat.m[0][2] = -s;
    mat.m[2][0] = s;  mat.m[2][2] = c;
    return mat;
}

inline MaTran4x4 taoMaTranQuayZ(float rad) {
    MaTran4x4 mat;
    float c = std::cos(rad), s = std::sin(rad);
    mat.m[0][0] = c;  mat.m[0][1] = s;
    mat.m[1][0] = -s; mat.m[1][1] = c;
    return mat;
}

// 2.4. Ma trận Quay quanh trục bất kỳ k = P0 -> P1 (Trang 13-14, CG.4)
inline MaTran4x4 taoMaTranQuayQuanhTrucBatKy(const Diem3D& p0, const Diem3D& p1, float alphaRad) {
    float kx = p1.x - p0.x, ky = p1.y - p0.y, kz = p1.z - p0.z;
    float L = std::sqrt(kx * kx + ky * ky + kz * kz);
    if (L < 1e-6f) return MaTran4x4();
    kx /= L; ky /= L; kz /= L;
    float d = std::sqrt(ky * ky + kz * kz);

    MaTran4x4 T1 = taoMaTranTinhTien(-p0.x, -p0.y, -p0.z);
    MaTran4x4 T2 = taoMaTranTinhTien(p0.x, p0.y, p0.z);

    if (d < 1e-6f) {
        return T1 * taoMaTranQuayX((kx > 0 ? 1.0f : -1.0f) * alphaRad) * T2;
    }

    float cosPhi = kz / d, sinPhi = ky / d;
    MaTran4x4 Rx, Rx_inv;
    Rx.m[1][1] = cosPhi;     Rx.m[1][2] = sinPhi;
    Rx.m[2][1] = -sinPhi;    Rx.m[2][2] = cosPhi;
    Rx_inv.m[1][1] = cosPhi; Rx_inv.m[1][2] = -sinPhi;
    Rx_inv.m[2][1] = sinPhi; Rx_inv.m[2][2] = cosPhi;

    float cosTheta = d, sinTheta = kx;
    MaTran4x4 Ry, Ry_inv;
    Ry.m[0][0] = cosTheta;     Ry.m[0][2] = sinTheta;
    Ry.m[2][0] = -sinTheta;    Ry.m[2][2] = cosTheta;
    Ry_inv.m[0][0] = cosTheta; Ry_inv.m[0][2] = -sinTheta;
    Ry_inv.m[2][0] = sinTheta; Ry_inv.m[2][2] = cosTheta;

    return T1 * Rx * Ry * taoMaTranQuayZ(alphaRad) * Ry_inv * Rx_inv * T2;
}

// =========================================================================
// 3. MA TRẬN 3x3 BIẾN ĐỔI THUẦN NHẤT 2D (PHỤC VỤ SƠ ĐỒ NGUYÊN LÝ 2D)
// Giáo trình VKU CG.3 (Trang 120-132)
// =========================================================================

struct MaTran3x3 {
    float m[3][3];

    MaTran3x3() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    MaTran3x3 operator*(const MaTran3x3& o) const {
        MaTran3x3 res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                res.m[i][j] = 0.0f;
                for (int k = 0; k < 3; ++k) {
                    res.m[i][j] += m[i][k] * o.m[k][j];
                }
            }
        }
        return res;
    }

    Diem2D bienDoi(const Diem2D& p) const {
        float nx = p.x * m[0][0] + p.y * m[1][0] + m[2][0];
        float ny = p.x * m[0][1] + p.y * m[1][1] + m[2][1];
        float nw = p.x * m[0][2] + p.y * m[1][2] + m[2][2];
        if (std::abs(nw) > 1e-6f && std::abs(nw - 1.0f) > 1e-6f) {
            nx /= nw; ny /= nw;
        }
        return Diem2D(nx, ny);
    }

    MaTran3x3 nghichDao() const {
        float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                  - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                  + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        if (std::abs(det) < 1e-6f) return MaTran3x3();
        float invDet = 1.0f / det;
        MaTran3x3 res;
        res.m[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
        res.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
        res.m[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
        res.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
        res.m[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
        res.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;
        res.m[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
        res.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
        res.m[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;
        return res;
    }
};

inline MaTran3x3 taoMaTranTinhTien2D(float tx, float ty) {
    MaTran3x3 mat;
    mat.m[2][0] = tx; mat.m[2][1] = ty;
    return mat;
}

inline MaTran3x3 taoMaTranTiLeQuanhTam2D(float x0, float y0, float sx, float sy) {
    MaTran3x3 mat;
    mat.m[0][0] = sx; mat.m[1][1] = sy;
    mat.m[2][0] = (1.0f - sx) * x0;
    mat.m[2][1] = (1.0f - sy) * y0;
    return mat;
}

inline MaTran3x3 taoMaTranQuayQuanhTam2D(float x0, float y0, float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    MaTran3x3 mat;
    mat.m[0][0] = c;  mat.m[0][1] = s;
    mat.m[1][0] = -s; mat.m[1][1] = c;
    mat.m[2][0] = (1.0f - c) * x0 + s * y0;
    mat.m[2][1] = -s * x0 + (1.0f - c) * y0;
    return mat;
}

inline MaTran3x3 taoMaTranDoiXung2D(float x0, float y0, bool latNgang, bool latDoc) {
    float sx = latNgang ? -1.0f : 1.0f;
    float sy = latDoc   ? -1.0f : 1.0f;
    return taoMaTranTiLeQuanhTam2D(x0, y0, sx, sy);
}

// =========================================================================
// 4. HỆ TỌA ĐỘ QUAN SÁT 3D (CAMERA) & PHÉP CHIẾU MÀN HÌNH
// Giáo trình VKU CG.4 (Trang 38-49)
// =========================================================================

// Biến đổi điểm từ thế giới sang hệ tọa độ quan sát (Mắt nhìn đặt tại khoảng cách R, góc theta, phi)
inline Diem3D chuyenHeToaDoQuanSat(const Diem3D& p, const Diem3D& target, float thetaRad, float phiRad, float R) {
    float dx = p.x - target.x;
    float dy = p.y - target.y;
    float dz = p.z - target.z;

    float sinT = std::sin(thetaRad), cosT = std::cos(thetaRad);
    float sinP = std::sin(phiRad),   cosP = std::cos(phiRad);

    Diem3D p0;
    // Đồng bộ quy ước trục Y hướng lên, mặt sàn là XZ
    p0.x =  dx * cosT - dz * sinT;
    p0.y = -dx * sinT * sinP + dy * cosP - dz * cosT * sinP;
    p0.z = -dx * sinT * cosP - dy * sinP - dz * cosT * cosP + R;
    return p0;
}

// Phép chiếu lên màn hình: Phối cảnh (Perspective) hoặc Song song (Parallel)
inline Diem2D chieuLenManHinh(const Diem3D& p0, float D, bool laPhoiCanh, float cx, float cy, float scalePx = 18.0f) {
    Diem2D p2d;
    if (laPhoiCanh) {
        float zSafe = (p0.z < 1.0f) ? 1.0f : p0.z;
        p2d.x = cx + (p0.x * D / zSafe) * scalePx;
        p2d.y = cy + (p0.y * D / zSafe) * scalePx;
    } else {
        p2d.x = cx + p0.x * scalePx * 0.75f;
        p2d.y = cy + p0.y * scalePx * 0.75f;
    }
    return p2d;
}

#endif // TOAN_HOC_DO_HOA_H
