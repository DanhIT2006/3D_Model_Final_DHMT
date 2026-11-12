#include "scene/SceneNode.h"
#include <GL/freeglut.h>
#include <algorithm>

SceneNode::SceneNode(const std::string& id, const std::string& name)
    : id(id), name(name) {}

SceneNode::~SceneNode() {
    for (auto* child : children) {
        delete child;
    }
    children.clear();
}

void SceneNode::addChild(SceneNode* child) {
    if (!child) return;
    if (child->parent) {
        child->parent->removeChild(child);
    }
    child->parent = this;
    children.push_back(child);
}

void SceneNode::removeChild(SceneNode* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}

Mat4 SceneNode::getLocalMatrix() const {
    // Thứ tự biến đổi: Tịnh tiến * Quay Y * Quay X * Quay Z * Tỉ lệ (CG.4 trang 16)
    Mat4 tr = Mat4::translate(localPos);
    Mat4 rotY = Mat4::rotateY(localRot.y);
    Mat4 rotX = Mat4::rotateX(localRot.x);
    Mat4 rotZ = Mat4::rotateZ(localRot.z);
    Mat4 sc = Mat4::scale(localScale);
    return tr * rotY * rotX * rotZ * sc;
}

Mat4 SceneNode::getWorldMatrix() const {
    if (parent) {
        return parent->getWorldMatrix() * getLocalMatrix();
    }
    return getLocalMatrix();
}

void SceneNode::update(float dt) {
    for (auto* child : children) {
        child->update(dt);
    }
}

void SceneNode::render(bool wireframeMode) {
    if (!visible) return;

    glPushMatrix();
    Mat4 localM = getLocalMatrix();
    glMultMatrixf(localM.data());

    // Vẽ hình học của nút hiện tại
    if (wireframeMode) {
        mesh.drawWireframe(isSelected ? Vec3(1.0f, 0.85f, 0.0f) : Vec3(0.0f, 0.95f, 0.4f), isSelected ? 2.5f : 1.0f);
    } else {
        mesh.drawSolid(true);
        if (isSelected) {
            // Hiển thị khung viền màu vàng cam báo hiệu đang được chọn
            mesh.drawWireframe(Vec3(1.0f, 0.85f, 0.0f), 2.5f);
        }
    }

    // Đệ quy vẽ các nút con
    for (auto* child : children) {
        child->render(wireframeMode);
    }

    glPopMatrix();
}

SceneNode* SceneNode::findNode(const std::string& targetId) {
    if (id == targetId) return this;
    for (auto* child : children) {
        SceneNode* found = child->findNode(targetId);
        if (found) return found;
    }
    return nullptr;
}

SceneNode* SceneNode::raycast(const Vec3& rayOrigin, const Vec3& rayDir, float& closestDist) {
    if (!visible) return nullptr;

    SceneNode* bestHitNode = nullptr;
    Mat4 worldM = getWorldMatrix();
    float curHit = -1.0f;

    if (mesh.intersectRay(rayOrigin, rayDir, worldM, curHit)) {
        if (curHit > 0.0f && curHit < closestDist) {
            closestDist = curHit;
            bestHitNode = this;
        }
    }

    for (auto* child : children) {
        SceneNode* childHit = child->raycast(rayOrigin, rayDir, closestDist);
        if (childHit) {
            bestHitNode = childHit;
        }
    }

    return bestHitNode;
}
