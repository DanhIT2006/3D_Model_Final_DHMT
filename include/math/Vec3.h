#pragma once
#include <cmath>
#include <iostream>

/**
 * @brief Vector 3D và các phép toán hình học vector
 * Theo giáo trình CG.4 (3D Graphics)
 */
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    friend Vec3 operator*(float s, const Vec3& v) { return Vec3(v.x * s, v.y * s, v.z * s); }
    Vec3 operator/(float s) const { return (s != 0.0f) ? Vec3(x / s, y / s, z / s) : Vec3(); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }

    Vec3& operator+=(const Vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    Vec3& operator-=(const Vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { if (s != 0.0f) { x /= s; y /= s; z /= s; } return *this; }

    bool operator==(const Vec3& rhs) const {
        return std::fabs(x - rhs.x) < 1e-5f && std::fabs(y - rhs.y) < 1e-5f && std::fabs(z - rhs.z) < 1e-5f;
    }
    bool operator!=(const Vec3& rhs) const { return !(*this == rhs); }

    float dot(const Vec3& rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    Vec3 cross(const Vec3& rhs) const {
        return Vec3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    float lengthSq() const {
        return x * x + y * y + z * z;
    }

    float length() const {
        return std::sqrt(lengthSq());
    }

    Vec3 normalized() const {
        float len = length();
        return (len > 1e-6f) ? (*this / len) : Vec3(0.0f, 0.0f, 0.0f);
    }

    float distanceTo(const Vec3& rhs) const {
        return (*this - rhs).length();
    }

    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }

    static Vec3 zero()    { return Vec3(0.0f, 0.0f, 0.0f); }
    static Vec3 one()     { return Vec3(1.0f, 1.0f, 1.0f); }
    static Vec3 up()      { return Vec3(0.0f, 1.0f, 0.0f); }
    static Vec3 down()    { return Vec3(0.0f, -1.0f, 0.0f); }
    static Vec3 forward() { return Vec3(0.0f, 0.0f, 1.0f); } // Trục Z+ tiến tới mũi xe
    static Vec3 back()    { return Vec3(0.0f, 0.0f, -1.0f); }
    static Vec3 right()   { return Vec3(1.0f, 0.0f, 0.0f); }
    static Vec3 left()    { return Vec3(-1.0f, 0.0f, 0.0f); }
};
