#pragma once
#include "math/Vec3.h"
#include "math/Mat4.h"
#include <vector>

// =========================================================================
// [SLIDE 3 CODE BLOCK]: CẤU TRÚC DỮ LIỆU ĐỐI TƯỢNG 3D THEO CG.4 TRANG 20-26
// Kết hợp Danh sách đỉnh, Danh sách cạnh (WireFrame) và Danh sách mặt (FaceModel)
// =========================================================================
struct EdgeType {
    int beginP; // Chỉ số đỉnh đầu của cạnh
    int endP;   // Chỉ số đỉnh cuối của cạnh
};

struct FaceType {
    std::vector<int> indexFace; // Danh sách chỉ số đỉnh của mặt đa giác (ngược chiều kim đồng hồ)
    Vec3 normal;                // Vector pháp tuyến mặt dùng cho mô hình chiếu sáng Lambert/Phong
    Vec3 faceColor;             // Màu mặt đa giác (RGB)
};

/**
 * @brief [SLIDE 3]: Mô hình hình học 3D kết hợp WireFrame (Khung dây) và FaceModel (Mặt đa giác)
 * theo đúng giáo trình CG.4 (trang 20-26)
 */
class Mesh3D {
public:
    std::vector<Vec3> vertices;   // 1. Danh sách đỉnh (Vertices)
    std::vector<EdgeType> edges;  // 2. Danh sách cạnh (Edges cho WireFrame mode)
    std::vector<FaceType> faces;  // 3. Danh sách mặt (Faces cho Solid mode)

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
