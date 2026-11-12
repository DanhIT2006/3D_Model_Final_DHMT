#pragma once
#include "math/Vec3.h"
#include <cstring>
#include <cmath>

/**
 * @brief Ma trận thuần nhất 4x4 biểu diễn các phép biến đổi Affine 3D
 * Lưu trữ theo thứ tự Column-Major tương thích hoàn toàn với OpenGL (glMultMatrixf)
 * Theo giáo trình CG.4 (3D Graphics, trang 5-16)
 */
class Mat4 {
public:
    // Mảng 16 phần tử theo chuẩn Column-major
    // [ 0  4  8 12 ]
    // [ 1  5  9 13 ]
    // [ 2  6 10 14 ]
    // [ 3  7 11 15 ]
    float m[16];

    Mat4();
    Mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33);

    static Mat4 identity();
    static Mat4 translate(float tx, float ty, float tz);
    static Mat4 translate(const Vec3& t);
    static Mat4 scale(float sx, float sy, float sz);
    static Mat4 scale(const Vec3& s);
    static Mat4 rotateX(float angleRad);
    static Mat4 rotateY(float angleRad);
    static Mat4 rotateZ(float angleRad);

    /**
     * @brief Quay quanh trục bất kỳ đi qua 2 điểm P0 và P1 (CG.4 trang 13-14)
     * M = Tr(-P0) * Rx(phi) * Ry(-theta) * Rz(alpha) * Ry(theta) * Rx(-phi) * Tr(P0)
     */
    static Mat4 rotateArbitraryAxis(const Vec3& p0, const Vec3& p1, float angleRad);

    /**
     * @brief Quay quanh trục đơn vị đi qua gốc tọa độ theo công thức Rodrigues
     */
    static Mat4 rotateAxis(const Vec3& axis, float angleRad);

    Mat4 operator*(const Mat4& rhs) const;
    Vec3 transformPoint(const Vec3& p) const;   // Biến đổi điểm (w = 1)
    Vec3 transformVector(const Vec3& v) const;  // Biến đổi vector hướng (w = 0)

    Mat4 inverse() const;
    Mat4 transpose() const;

    const float* data() const { return m; }
    float* data() { return m; }

    float get(int row, int col) const { return m[col * 4 + row]; }
    void set(int row, int col, float val) { m[col * 4 + row] = val; }

    Vec3 getTranslation() const { return Vec3(m[12], m[13], m[14]); }
};
