#pragma once
#include "scene/SceneNode.h"
#include "components/PinAndWire.h"
#include <string>
#include <vector>

enum class SubShapeType {
    BOX_SMD,            // IC dán, Domino, Header, Diode
    CYLINDER_CAP,       // Tụ hóa hình trụ đứng (220uF, 47uF)
    HEATSINK_VERTICAL,  // Tản nhiệt nhôm đen của L298N
    CRYSTAL_OVAL,       // Thạch anh 16MHz hình elip dẹt
    JUMPER_CAP,         // Chốt cắm Jumper 5V (có thể rút/cắm)
    PUSH_BUTTON,        // Nút nhấn Reset hoặc biến trở tinh chỉnh
    LED_INDICATOR       // Đèn LED phát sáng trên mạch
};

/**
 * @brief Cấu trúc biểu diễn một linh kiện vi mô / thiết bị nhỏ trên bo mạch
 * (Tụ điện, IC vi điều khiển, IC ổn áp, tản nhiệt, jumper, diode, nút nhấn, cổng cắm...)
 */
struct SubComponent {
    std::string id;              // Mã định danh: "L298N_IC_HS", "L298N_JMP_5V", "ARD_RESET_BTN"...
    std::string name;            // Tên linh kiện: "IC Cau H L298N", "Tu hoa loc nguon 220uF"...
    std::string category;        // Nhóm chức năng: "Cong suat", "Loc nguon", "Dieu khien"...
    std::string rating;          // Thông số / Trị số kỹ thuật: "220uF - 35V", "DIP-28 16MHz"...
    std::string roleDesc;        // Tác dụng & nguyên lý hoạt động trong mạch
    std::string liveStatusText;  // Trạng thái giám sát thời gian thực
    Vec3 localPos;               // Tọa độ tương đối so với tâm bo mạch (cm)
    Vec3 size;                   // Kích thước khối bao W x H x D (cm)
    Vec3 color;                  // Màu sắc hiển thị
    SubShapeType shape = SubShapeType::BOX_SMD;
    bool isInteractive = false;  // Có thể click tương tác (Tháo/cắm, Nhấn, Xoay)
    bool isActiveState = true;   // Trạng thái vật lý logic (Cắm/Rút, Bật/Tắt)
    bool state = false;          // Trạng thái phụ tương thích ngược
    std::string actionName;      // Tên nhãn hành động tương tác: "RUT/CAM JUMPER 5V", "NHAN RESET"...
    Mesh3D mesh;                 // Mô hình hình học 3D của linh kiện nhỏ
    bool isSelected = false;     // Trạng thái được chọn
};

/**
 * @brief Lớp cơ sở trừu tượng cho 11 linh kiện phần cứng vi điều khiển
 * Tích hợp mô phỏng mạch điện, chân cắm (Pin), kết xuất 3D và phân tích vi mô (Workbench)
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

    // Danh sách các linh kiện vi mô con trên bo mạch
    std::vector<SubComponent> subComponents;
    int selectedSubCompIndex = -1;

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

    // === HỆ THỐNG QUAN SÁT VI MÔ (WORKBENCH MODE) ===
    virtual void initSubComponents();
    void addSubComponent(const SubComponent& sub);
    SubComponent* getSelectedSubComponent();
    void selectSubComponentByIndex(int index);
    void selectNextSubComponent();
    void selectPrevSubComponent();
    SubComponent* pickSubComponent(const Vec3& rayOrigin, const Vec3& rayDir);

    virtual void renderWorkbench(bool wireframeMode);
    virtual void onSubComponentAction(const std::string& subId);
};
