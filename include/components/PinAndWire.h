#pragma once
#include "math/Vec3.h"
#include "math/Mat4.h"
#include <string>
#include <vector>

enum class PinType {
    POWER_7V4,   // Nguồn động lực 7.4V từ Pin 18650
    POWER_5V,    // Nguồn hạ áp 5.0V từ LM2596 hoặc 5V LDO Arduino
    POWER_GND,   // Cực âm chung (Ground)
    DIGITAL_IO,  // Tín hiệu số HIGH / LOW
    PWM_IO,      // Tín hiệu điều rộng xung PWM (0 - 255)
    ANALOG_IO,   // Tín hiệu tương tự ADC (0 - 5V)
    MOTOR_OUT    // Ngõ ra công suất điều khiển cuộn dây động cơ
};

struct Pin {
    std::string pinName;      // Tên chân (VD: "VIN", "GND", "5V", "IN1", "TRIG"...)
    PinType type;
    Vec3 relativeOffset;      // Tọa độ tương đối chân cắm so với tâm linh kiện
    float voltage = 0.0f;     // Điện áp tức thời (V)
    float pwmDuty = 0.0f;     // Hệ số xung PWM (0.0f -> 1.0f)
    bool isOutput = false;

    Vec3 getWorldPosition(const Mat4& componentWorldMat) const {
        return componentWorldMat.transformPoint(relativeOffset);
    }
};

struct Wire {
    std::string name;
    std::string fromCompCode;
    std::string fromPinName;
    std::string toCompCode;
    std::string toPinName;
    Vec3 color;               // Màu dây (Đỏ = 7.4V, Cam = 5V, Đen = GND, Xanh/Vàng = Tín hiệu)
    bool active = false;      // Có dòng điện/tín hiệu chạy qua hay không

    void render3D(const Vec3& p1, const Vec3& p2, bool isPowered) const;
};
