#include "math/Mat4.h"

Mat4::Mat4() {
    std::memset(m, 0, sizeof(m));
    m[0]  = 1.0f;
    m[5]  = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

Mat4::Mat4(float m00, float m01, float m02, float m03,
           float m10, float m11, float m12, float m13,
           float m20, float m21, float m22, float m23,
           float m30, float m31, float m32, float m33) {
    // Column-major
    m[0]  = m00; m[4]  = m01; m[8]  = m02; m[12] = m03;
    m[1]  = m10; m[5]  = m11; m[9]  = m12; m[13] = m13;
    m[2]  = m20; m[6]  = m21; m[10] = m22; m[14] = m23;
    m[3]  = m30; m[7]  = m31; m[11] = m32; m[15] = m33;
}

Mat4 Mat4::identity() {
    return Mat4();
}

Mat4 Mat4::translate(float tx, float ty, float tz) {
    Mat4 res = identity();
    res.m[12] = tx;
    res.m[13] = ty;
    res.m[14] = tz;
    return res;
}

Mat4 Mat4::translate(const Vec3& t) {
    return translate(t.x, t.y, t.z);
}

Mat4 Mat4::scale(float sx, float sy, float sz) {
    Mat4 res = identity();
    res.m[0]  = sx;
    res.m[5]  = sy;
    res.m[10] = sz;
    return res;
}

Mat4 Mat4::scale(const Vec3& s) {
    return scale(s.x, s.y, s.z);
}

Mat4 Mat4::rotateX(float angleRad) {
    Mat4 res = identity();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    res.m[5]  = c;
    res.m[6]  = s;
    res.m[9]  = -s;
    res.m[10] = c;
    return res;
}

Mat4 Mat4::rotateY(float angleRad) {
    Mat4 res = identity();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    res.m[0]  = c;
    res.m[2]  = -s;
    res.m[8]  = s;
    res.m[10] = c;
    return res;
}

Mat4 Mat4::rotateZ(float angleRad) {
    Mat4 res = identity();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    res.m[0]  = c;
    res.m[1]  = s;
    res.m[4]  = -s;
    res.m[5]  = c;
    return res;
}

Mat4 Mat4::rotateAxis(const Vec3& axis, float angleRad) {
    Vec3 a = axis.normalized();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    float t = 1.0f - c;

    Mat4 res;
    res.m[0]  = t * a.x * a.x + c;
    res.m[1]  = t * a.x * a.y + s * a.z;
    res.m[2]  = t * a.x * a.z - s * a.y;
    res.m[3]  = 0.0f;

    res.m[4]  = t * a.x * a.y - s * a.z;
    res.m[5]  = t * a.y * a.y + c;
    res.m[6]  = t * a.y * a.z + s * a.x;
    res.m[7]  = 0.0f;

    res.m[8]  = t * a.x * a.z + s * a.y;
    res.m[9]  = t * a.y * a.z - s * a.x;
    res.m[10] = t * a.z * a.z + c;
    res.m[11] = 0.0f;

    res.m[12] = 0.0f;
    res.m[13] = 0.0f;
    res.m[14] = 0.0f;
    res.m[15] = 1.0f;

    return res;
}

Mat4 Mat4::rotateArbitraryAxis(const Vec3& p0, const Vec3& p1, float angleRad) {
    // Theo CG.4 trang 13-14:
    // M = Tr(-P0) * R_axis(P1 - P0, angle) * Tr(P0)
    Vec3 axis = p1 - p0;
    if (axis.lengthSq() < 1e-6f) {
        return identity();
    }
    Mat4 trInv = translate(-p0);
    Mat4 rot = rotateAxis(axis, angleRad);
    Mat4 tr = translate(p0);
    return tr * rot * trInv;
}

Mat4 Mat4::operator*(const Mat4& rhs) const {
    Mat4 res;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += m[k * 4 + row] * rhs.m[col * 4 + k];
            }
            res.m[col * 4 + row] = sum;
        }
    }
    return res;
}

Vec3 Mat4::transformPoint(const Vec3& p) const {
    float x = m[0] * p.x + m[4] * p.y + m[8]  * p.z + m[12];
    float y = m[1] * p.x + m[5] * p.y + m[9]  * p.z + m[13];
    float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14];
    float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
    if (std::fabs(w) > 1e-6f && std::fabs(w - 1.0f) > 1e-6f) {
        return Vec3(x / w, y / w, z / w);
    }
    return Vec3(x, y, z);
}

Vec3 Mat4::transformVector(const Vec3& v) const {
    float x = m[0] * v.x + m[4] * v.y + m[8]  * v.z;
    float y = m[1] * v.x + m[5] * v.y + m[9]  * v.z;
    float z = m[2] * v.x + m[6] * v.y + m[10] * v.z;
    return Vec3(x, y, z);
}

Mat4 Mat4::transpose() const {
    Mat4 res;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            res.m[col * 4 + row] = m[row * 4 + col];
        }
    }
    return res;
}

Mat4 Mat4::inverse() const {
    float inv[16], det;
    const float* src = m;

    inv[0] = src[5]  * src[10] * src[15] - 
             src[5]  * src[11] * src[14] - 
             src[9]  * src[6]  * src[15] + 
             src[9]  * src[7]  * src[14] +
             src[13] * src[6]  * src[11] - 
             src[13] * src[7]  * src[10];

    inv[4] = -src[4]  * src[10] * src[15] + 
              src[4]  * src[11] * src[14] + 
              src[8]  * src[6]  * src[15] - 
              src[8]  * src[7]  * src[14] - 
              src[12] * src[6]  * src[11] + 
              src[12] * src[7]  * src[10];

    inv[8] = src[4]  * src[9] * src[15] - 
             src[4]  * src[11] * src[13] - 
             src[8]  * src[5] * src[15] + 
             src[8]  * src[7] * src[13] + 
             src[12] * src[5] * src[11] - 
             src[12] * src[7] * src[9];

    inv[12] = -src[4]  * src[9] * src[14] + 
               src[4]  * src[10] * src[13] +
               src[8]  * src[5] * src[14] - 
               src[8]  * src[6] * src[13] - 
               src[12] * src[5] * src[10] + 
               src[12] * src[6] * src[9];

    inv[1] = -src[1]  * src[10] * src[15] + 
              src[1]  * src[11] * src[14] + 
              src[9]  * src[2] * src[15] - 
              src[9]  * src[3] * src[14] - 
              src[13] * src[2] * src[11] + 
              src[13] * src[3] * src[10];

    inv[5] = src[0]  * src[10] * src[15] - 
             src[0]  * src[11] * src[14] - 
             src[8]  * src[2] * src[15] + 
             src[8]  * src[3] * src[14] + 
             src[12] * src[2] * src[11] - 
             src[12] * src[3] * src[10];

    inv[9] = -src[0]  * src[9] * src[15] + 
              src[0]  * src[11] * src[13] + 
              src[8]  * src[1] * src[15] - 
              src[8]  * src[3] * src[13] - 
              src[12] * src[1] * src[11] + 
              src[12] * src[3] * src[9];

    inv[13] = src[0]  * src[9] * src[14] - 
              src[0]  * src[10] * src[13] - 
              src[8]  * src[1] * src[14] + 
              src[8]  * src[2] * src[13] + 
              src[12] * src[1] * src[10] - 
              src[12] * src[2] * src[9];

    inv[2] = src[1]  * src[6] * src[15] - 
             src[1]  * src[7] * src[14] - 
             src[5]  * src[2] * src[15] + 
             src[5]  * src[3] * src[14] + 
             src[13] * src[2] * src[7] - 
             src[13] * src[3] * src[6];

    inv[6] = -src[0]  * src[6] * src[15] + 
              src[0]  * src[7] * src[14] + 
              src[4]  * src[2] * src[15] - 
              src[4]  * src[3] * src[14] - 
              src[12] * src[2] * src[7] + 
              src[12] * src[3] * src[6];

    inv[10] = src[0]  * src[5] * src[15] - 
              src[0]  * src[7] * src[13] - 
              src[4]  * src[1] * src[15] + 
              src[4]  * src[3] * src[13] + 
              src[12] * src[1] * src[7] - 
              src[12] * src[3] * src[5];

    inv[14] = -src[0]  * src[5] * src[14] + 
               src[0]  * src[6] * src[13] + 
               src[4]  * src[1] * src[14] - 
               src[4]  * src[2] * src[13] - 
               src[12] * src[1] * src[6] + 
               src[12] * src[2] * src[5];

    inv[3] = -src[1] * src[6] * src[11] + 
              src[1] * src[7] * src[10] + 
              src[5] * src[2] * src[11] - 
              src[5] * src[3] * src[10] - 
              src[9] * src[2] * src[7] + 
              src[9] * src[3] * src[6];

    inv[7] = src[0] * src[6] * src[11] - 
             src[0] * src[7] * src[10] - 
             src[4] * src[2] * src[11] + 
             src[4] * src[3] * src[10] + 
             src[8] * src[2] * src[7] - 
             src[8] * src[3] * src[6];

    inv[11] = -src[0] * src[5] * src[11] + 
               src[0] * src[7] * src[9] + 
               src[4] * src[1] * src[11] - 
               src[4] * src[3] * src[9] - 
               src[8] * src[1] * src[7] + 
               src[8] * src[3] * src[5];

    inv[15] = src[0] * src[5] * src[10] - 
              src[0] * src[6] * src[9] - 
              src[4] * src[1] * src[10] + 
              src[4] * src[2] * src[9] + 
              src[8] * src[1] * src[6] - 
              src[8] * src[2] * src[5];

    det = src[0] * inv[0] + src[1] * inv[4] + src[2] * inv[8] + src[3] * inv[12];
    if (std::fabs(det) < 1e-8f) {
        return identity();
    }

    det = 1.0f / det;
    Mat4 res;
    for (int i = 0; i < 16; i++) {
        res.m[i] = inv[i] * det;
    }
    return res;
}
