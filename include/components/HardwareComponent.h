#pragma once
#include "scene/SceneNode.h"
#include "components/PinAndWire.h"
#include <string>
#include <vector>

/**
 * @brief Lớp cơ sở trừu tượng cho 11 linh kiện phần cứng vi điều khiển
 * Tích hợp mô phỏng mạch điện, chân cắm (Pin) và kết xuất 3D
 */
class HardwareComponent : public SceneNode {
public:
    std::string code;         // Mã chuẩn hóa: "ARD", "L298N", "BUCK"...
    std::string funcGroup;    // Nhóm chức năng: Nguồn, Vi điều khiển, Động lực...
    float nominalVoltage = 5.0f; // Điện áp danh định (V)
    float currentDrawMa = 0.0f;  // Dòng điện tiêu thụ tức thời (mA)
    bool isPowered = false;      // Trạng thái đã được cấp nguồn hợp lệ
    std::string statusDesc;      // Chuỗi mô tả trạng thái cho bảng HUD

    std::vector<Pin> pins;       // Danh sách chân cắm I/O & Nguồn

public:
    HardwareComponent(const std::string& code, const std::string& name, const std::string& group);
    virtual ~HardwareComponent() = default;

    Pin* getPin(const std::string& name);
    const Pin* getPin(const std::string& name) const;
    void addPin(const std::string& name, PinType type, const Vec3& offset, bool isOutput = false);

    // Cập nhật trạng thái logic điện toán / điện tử mỗi frame
    virtual void onElectricalUpdate() = 0;

    // Vẽ thêm các chi tiết trực quan: đầu cắm chân Pin, đèn LED chỉ báo nguồn
    virtual void renderPinsAndDetails();

    virtual void render(bool wireframeMode) override;
};
