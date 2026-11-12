#pragma once
#include "components/HardwareComponent.h"
#include "components/PowerComponents.h"
#include "components/ControlComponents.h"
#include "components/ActuatorComponents.h"
#include <vector>
#include <map>
#include <string>

/**
 * @brief Bộ giải mạch mô phỏng điện toán thời gian thực (Real-time Circuit Simulator)
 * Quản lý kết nối dây dẫn (Wires), lan truyền điện áp và phân phối dòng điện
 */
class CircuitSimulator {
public:
    std::map<std::string, HardwareComponent*> componentMap;
    std::vector<Wire> wires;
    bool showWires = true;

    // Tổng dòng điện tiêu thụ toàn xe (mA)
    float totalCurrentMa = 0.0f;

public:
    CircuitSimulator() = default;

    void registerComponent(HardwareComponent* comp);
    void addWire(const std::string& name,
                 const std::string& fromComp, const std::string& fromPin,
                 const std::string& toComp, const std::string& toPin,
                 const Vec3& color);

    void setupStandardCarCircuit();
    void stepCircuit(float dt);
    void render3DWires() const;

    HardwareComponent* getComponent(const std::string& code);
};
