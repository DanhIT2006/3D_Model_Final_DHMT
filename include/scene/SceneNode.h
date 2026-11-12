#pragma once
#include "math/Vec3.h"
#include "math/Mat4.h"
#include "scene/Mesh3D.h"
#include <string>
#include <vector>

/**
 * @brief Nút trong đồ thị phân cảnh (Scene Graph)
 * Quản lý quan hệ phân cấp cha - con và tích lũy ma trận biến đổi thế giới
 * Theo giáo trình CG.4 (trang 16 & trang 28)
 */
class SceneNode {
public:
    std::string id;
    std::string name;

    Vec3 localPos = Vec3(0, 0, 0);
    Vec3 localRot = Vec3(0, 0, 0); // Góc quay (Pitch X, Yaw Y, Roll Z) tính bằng Radian
    Vec3 localScale = Vec3(1, 1, 1);
    Vec3 cadSize = Vec3(1, 1, 1);  // Kích thước thực tế W x H x D (cm)

    Mesh3D mesh;
    bool visible = true;
    bool isSelected = false;

    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;

public:
    SceneNode(const std::string& id = "", const std::string& name = "");
    virtual ~SceneNode();

    void addChild(SceneNode* child);
    void removeChild(SceneNode* child);

    Mat4 getLocalMatrix() const;
    Mat4 getWorldMatrix() const;

    virtual void update(float dt);
    virtual void render(bool wireframeMode);

    SceneNode* findNode(const std::string& targetId);

    // Kiểm tra Raycast đệ quy để chọn đối tượng trong 3D
    virtual SceneNode* raycast(const Vec3& rayOrigin, const Vec3& rayDir, float& closestDist);
};
