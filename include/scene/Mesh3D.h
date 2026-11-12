#pragma once
#include "math/Vec3.h"
#include "math/Mat4.h"
#include <vector>

struct EdgeType {
    int beginP;
    int endP;
};

struct FaceType {
    std::vector<int> indexFace; // Danh sách chỉ số đỉnh của mặt đa giác
    Vec3 normal;                // Pháp tuyến phục vụ tính toán ánh sáng
    Vec3 faceColor;             // Màu mặt (RGB)
};

/**
 * @brief Mô hình hình học 3D kết hợp WireFrame (Khung dây) và FaceModel (Mặt đa giác)
 * theo đúng giáo trình CG.4 (trang 20-26)
 */
class Mesh3D {
public:
    std::vector<Vec3> vertices;   // Danh sách đỉnh (Vertices)
    std::vector<EdgeType> edges;  // Danh sách cạnh (Edges cho WireFrame)
    std::vector<FaceType> faces;  // Danh sách mặt (Faces cho Solid Mesh)

    Mesh3D() = default;

    // Các hàm khởi tạo hình khối CAD chuẩn
    static Mesh3D createBox(float width, float height, float depth, Vec3 color);
    static Mesh3D createCylinder(float radius, float height, int segments, Vec3 color);
    static Mesh3D createWheel(float outerRadius, float innerRadius, float width, int segments, Vec3 rimColor, Vec3 tireColor);
    static Mesh3D createSensorEyes(float width, float height, float depth, Vec3 pcbColor, Vec3 eyeColor);
    static Mesh3D createGripperClaw(float length, float width, float thickness, bool isLeft, Vec3 color);

    // Tính toán lại pháp tuyến cho toàn bộ các mặt
    void computeNormals();

    // Vẽ khối đa giác đặc với ánh sáng
    void drawSolid(bool useLighting = true) const;

    // Vẽ khung dây wireframe bằng GL_LINES
    void drawWireframe(Vec3 lineColor = Vec3(0.0f, 1.0f, 0.5f), float lineWidth = 1.2f) const;

    // Kiểm tra giao cắt tia Raycast với khối hộp bao (AABB) của Mesh
    bool intersectRay(const Vec3& rayOrigin, const Vec3& rayDir, const Mat4& worldMat, float& hitDist) const;

    void getBounds(Vec3& minOut, Vec3& maxOut) const;
};
