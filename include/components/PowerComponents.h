#pragma once
#include "components/HardwareComponent.h"

/**
 * @brief BAT_SW: Hộp 2 Pin 18650 mắc nối tiếp (7.4V) & Công tắc nguồn bập bênh
 * Kích thước CAD: 7.8 x 2.2 x 4.2 cm
 */
class BatterySwitchComp : public HardwareComponent {
public:
    bool switchOn = true;         // Trạng thái công tắc nguồn (Bật/Tắt)
    float batteryVoltage = 7.4f;  // Điện áp thực tế của 2 cell pin
    float capacityMah = 2600.0f;  // Dung lượng pin (mAh)
    float currentSoc = 0.92f;     // Trạng thái sạc pin (SOC 0.0 - 1.0 ~ 92%)

public:
    BatterySwitchComp();
    void toggleSwitch();
    virtual void onElectricalUpdate() override;
    virtual void renderPinsAndDetails() override;
};

/**
 * @brief DOMINO: Trạm cầu đấu dây 3 Bus nguồn (7.4V, 5.0V, GND)
 * Kích thước CAD: 6.0 x 1.5 x 2.5 cm
 */
class DominoBusComp : public HardwareComponent {
public:
    float bus7V4 = 0.0f;
    float bus5V  = 0.0f;
    float busGND = 0.0f;

public:
    DominoBusComp();
    virtual void onElectricalUpdate() override;
    virtual void renderPinsAndDetails() override;
};

/**
 * @brief BUCK: Mạch giảm áp DC-DC LM2596 (7.4V -> 5.0V cho Servo)
 * Kích thước CAD: 4.3 x 1.4 x 2.1 cm
 */
class BuckLM2596Comp : public HardwareComponent {
public:
    float targetOutVoltage = 5.0f;  // Điện áp ngõ ra tinh chỉnh bằng biến trở (V)
    float actualOutVoltage = 0.0f;  // Điện áp thực tế ngõ ra
    float efficiency = 0.91f;       // Hiệu suất chuyển đổi 91%

public:
    BuckLM2596Comp();
    void adjustVoltage(float delta);
    virtual void onElectricalUpdate() override;
    virtual void renderPinsAndDetails() override;
};
