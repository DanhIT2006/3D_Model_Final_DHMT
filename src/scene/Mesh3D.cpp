#include "scene/Mesh3D.h"
#include <GL/freeglut.h>
#include <cmath>
#include <limits>
#include <algorithm>

void Mesh3D::computeNormals() {
    for (auto& face : faces) {
        if (face.indexFace.size() < 3) continue;
        const Vec3& v0 = vertices[face.indexFace[0]];
        const Vec3& v1 = vertices[face.indexFace[1]];
        const Vec3& v2 = vertices[face.indexFace[2]];
        Vec3 e1 = v1 - v0;
        Vec3 e2 = v2 - v0;
        face.normal = e1.cross(e2).normalized();
    }
}

Mesh3D Mesh3D::createBox(float width, float height, float depth, Vec3 color) {
    Mesh3D mesh;
    float hx = width * 0.5f;
    float hy = height * 0.5f;
    float hz = depth * 0.5f;

    // 8 Đỉnh của khối hộp CAD (CG.4 trang 20-22)
    mesh.vertices = {
        Vec3(-hx, -hy, -hz), // 0
        Vec3( hx, -hy, -hz), // 1
        Vec3( hx,  hy, -hz), // 2
        Vec3(-hx,  hy, -hz), // 3
        Vec3(-hx, -hy,  hz), // 4
        Vec3( hx, -hy,  hz), // 5
        Vec3( hx,  hy,  hz), // 6
        Vec3(-hx,  hy,  hz)  // 7
    };

    // 12 Cạnh cho WireFrame
    mesh.edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Mặt sau
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Mặt trước
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Nối trước - sau
    };

    // 6 Mặt đa giác (FaceModel - CG.4 trang 24-26)
    mesh.faces = {
        // Mặt trước (+Z)
        {{4, 5, 6, 7}, Vec3(0, 0, 1), color},
        // Mặt sau (-Z)
        {{1, 0, 3, 2}, Vec3(0, 0, -1), color * 0.85f},
        // Mặt trên (+Y)
        {{3, 7, 6, 2}, Vec3(0, 1, 0), color * 1.15f},
        // Mặt dưới (-Y)
        {{0, 1, 5, 4}, Vec3(0, -1, 0), color * 0.70f},
        // Mặt phải (+X)
        {{5, 1, 2, 6}, Vec3(1, 0, 0), color * 0.95f},
        // Mặt trái (-X)
        {{0, 4, 7, 3}, Vec3(-1, 0, 0), color * 0.90f}
    };

    return mesh;
}

Mesh3D Mesh3D::createCylinder(float radius, float height, int segments, Vec3 color) {
    Mesh3D mesh;
    float halfH = height * 0.5f;
    float angleStep = 2.0f * 3.14159265f / segments;

    // Đỉnh vòng dưới: 0 .. segments-1
    for (int i = 0; i < segments; ++i) {
        float a = i * angleStep;
        mesh.vertices.push_back(Vec3(radius * std::cos(a), -halfH, radius * std::sin(a)));
    }
    // Đỉnh vòng trên: segments .. 2*segments-1
    for (int i = 0; i < segments; ++i) {
        float a = i * angleStep;
        mesh.vertices.push_back(Vec3(radius * std::cos(a), halfH, radius * std::sin(a)));
    }
    // Tâm đáy dưới: 2*segments, tâm nắp trên: 2*segments+1
    int bottomCenter = (int)mesh.vertices.size();
    mesh.vertices.push_back(Vec3(0, -halfH, 0));
    int topCenter = (int)mesh.vertices.size();
    mesh.vertices.push_back(Vec3(0, halfH, 0));

    // Cạnh WireFrame
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        mesh.edges.push_back({i, next});
        mesh.edges.push_back({segments + i, segments + next});
        mesh.edges.push_back({i, segments + i});
    }

    // Mặt hông hình trụ
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int iBot1 = i;
        int iBot2 = next;
        int iTop2 = segments + next;
        int iTop1 = segments + i;
        float midAngle = (i + 0.5f) * angleStep;
        Vec3 norm(std::cos(midAngle), 0.0f, std::sin(midAngle));
        mesh.faces.push_back({{iBot1, iBot2, iTop2, iTop1}, norm, color});
    }

    // Nắp trên
    std::vector<int> topIndices;
    for (int i = 0; i < segments; ++i) {
        topIndices.push_back(segments + i);
    }
    mesh.faces.push_back({topIndices, Vec3(0, 1, 0), color * 1.1f});

    // Đáy dưới
    std::vector<int> botIndices;
    for (int i = segments - 1; i >= 0; --i) {
        botIndices.push_back(i);
    }
    mesh.faces.push_back({botIndices, Vec3(0, -1, 0), color * 0.8f});

    return mesh;
}

Mesh3D Mesh3D::createWheel(float outerRadius, float innerRadius, float width, int segments, Vec3 rimColor, Vec3 tireColor) {
    // Bánh xe robot: lốp cao su đen bên ngoài + vành mâm màu vàng/bạc bên trong
    Mesh3D mesh = createCylinder(outerRadius, width, segments, tireColor);
    
    // Thêm vành mâm bên trong
    Mesh3D rim = createCylinder(innerRadius, width * 1.05f, segments, rimColor);
    int offset = (int)mesh.vertices.size();
    for (const auto& v : rim.vertices) mesh.vertices.push_back(v);
    for (const auto& e : rim.edges) mesh.edges.push_back({e.beginP + offset, e.endP + offset});
    for (auto f : rim.faces) {
        for (int& idx : f.indexFace) idx += offset;
        mesh.faces.push_back(f);
    }
    return mesh;
}

Mesh3D Mesh3D::createSensorEyes(float width, float height, float depth, Vec3 pcbColor, Vec3 eyeColor) {
    // Bo mạch PCB màu xanh của HC-SR04
    Mesh3D mesh = createBox(width, height, depth, pcbColor);

    // 2 mắt siêu âm dạng ống trụ bạc (Trig & Echo)
    float eyeR = height * 0.38f;
    float eyeLen = depth * 1.2f;
    Mesh3D eye1 = createCylinder(eyeR, eyeLen, 16, eyeColor);
    Mesh3D eye2 = createCylinder(eyeR, eyeLen, 16, eyeColor);

    // Quay trục ống trụ hướng về phía trước (+Z)
    Mat4 rotToZ = Mat4::rotateX(1.5708f);

    float eyeDist = width * 0.26f;
    Mat4 tr1 = Mat4::translate(-eyeDist, 0.0f, depth * 0.5f + eyeLen * 0.5f) * rotToZ;
    Mat4 tr2 = Mat4::translate( eyeDist, 0.0f, depth * 0.5f + eyeLen * 0.5f) * rotToZ;

    auto appendTransformed = [&](const Mesh3D& subMesh, const Mat4& mat) {
        int offset = (int)mesh.vertices.size();
        for (const auto& v : subMesh.vertices) {
            mesh.vertices.push_back(mat.transformPoint(v));
        }
        for (const auto& e : subMesh.edges) {
            mesh.edges.push_back({e.beginP + offset, e.endP + offset});
        }
        for (auto f : subMesh.faces) {
            f.normal = mat.transformVector(f.normal).normalized();
            for (int& idx : f.indexFace) idx += offset;
            mesh.faces.push_back(f);
        }
    };

    appendTransformed(eye1, tr1);
    appendTransformed(eye2, tr2);
    return mesh;
}

Mesh3D Mesh3D::createGripperClaw(float length, float width, float thickness, bool isLeft, Vec3 color) {
    // Ngàm kẹp cong nhẹ
    Mesh3D base = createBox(width, thickness, length * 0.6f, color);
    
    // Đoạn ngàm uốn vào trong để giữ chặt vật thể
    float tipLen = length * 0.5f;
    Mesh3D tip = createBox(width * 0.9f, thickness, tipLen, color * 1.15f);

    float angle = isLeft ? -0.45f : 0.45f;
    float xOffset = isLeft ? width * 0.5f : -width * 0.5f;
    Mat4 trTip = Mat4::translate(xOffset, 0.0f, length * 0.45f) * Mat4::rotateY(angle);

    int offset = (int)base.vertices.size();
    for (const auto& v : tip.vertices) base.vertices.push_back(trTip.transformPoint(v));
    for (const auto& e : tip.edges) base.edges.push_back({e.beginP + offset, e.endP + offset});
    for (auto f : tip.faces) {
        f.normal = trTip.transformVector(f.normal).normalized();
        for (int& idx : f.indexFace) idx += offset;
        base.faces.push_back(f);
    }
    return base;
}

void Mesh3D::drawSolid(bool useLighting) const {
    if (faces.empty()) return;

    for (const auto& face : faces) {
        if (face.indexFace.size() < 3) continue;

        if (useLighting) {
            glNormal3f(face.normal.x, face.normal.y, face.normal.z);
            glColor3f(face.faceColor.x, face.faceColor.y, face.faceColor.z);
        } else {
            glColor3f(face.faceColor.x, face.faceColor.y, face.faceColor.z);
        }

        if (face.indexFace.size() == 3) {
            glBegin(GL_TRIANGLES);
            for (int idx : face.indexFace) {
                const Vec3& v = vertices[idx];
                glVertex3f(v.x, v.y, v.z);
            }
            glEnd();
        } else if (face.indexFace.size() == 4) {
            glBegin(GL_QUADS);
            for (int idx : face.indexFace) {
                const Vec3& v = vertices[idx];
                glVertex3f(v.x, v.y, v.z);
            }
            glEnd();
        } else {
            glBegin(GL_POLYGON);
            for (int idx : face.indexFace) {
                const Vec3& v = vertices[idx];
                glVertex3f(v.x, v.y, v.z);
            }
            glEnd();
        }
    }
}

void Mesh3D::drawWireframe(Vec3 lineColor, float lineWidth) const {
    glDisable(GL_LIGHTING);
    glLineWidth(lineWidth);
    glColor3f(lineColor.x, lineColor.y, lineColor.z);

    glBegin(GL_LINES);
    for (const auto& edge : edges) {
        if (edge.beginP >= 0 && edge.beginP < (int)vertices.size() &&
            edge.endP >= 0 && edge.endP < (int)vertices.size()) {
            const Vec3& v0 = vertices[edge.beginP];
            const Vec3& v1 = vertices[edge.endP];
            glVertex3f(v0.x, v0.y, v0.z);
            glVertex3f(v1.x, v1.y, v1.z);
        }
    }
    glEnd();
}

void Mesh3D::getBounds(Vec3& minOut, Vec3& maxOut) const {
    if (vertices.empty()) {
        minOut = maxOut = Vec3::zero();
        return;
    }
    minOut = Vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    maxOut = Vec3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for (const auto& v : vertices) {
        minOut.x = std::min(minOut.x, v.x);
        minOut.y = std::min(minOut.y, v.y);
        minOut.z = std::min(minOut.z, v.z);

        maxOut.x = std::max(maxOut.x, v.x);
        maxOut.y = std::max(maxOut.y, v.y);
        maxOut.z = std::max(maxOut.z, v.z);
    }
}

bool Mesh3D::intersectRay(const Vec3& rayOrigin, const Vec3& rayDir, const Mat4& worldMat, float& hitDist) const {
    if (vertices.empty()) return false;

    // Chuyển tia ray từ không gian thế giới về không gian cục bộ của Mesh
    Mat4 invWorld = worldMat.inverse();
    Vec3 localRayOrigin = invWorld.transformPoint(rayOrigin);
    Vec3 localRayDir = invWorld.transformVector(rayDir).normalized();

    Vec3 boxMin, boxMax;
    getBounds(boxMin, boxMax);

    // Mở rộng nhẹ bounding box để dễ click chuột
    boxMin -= Vec3(0.3f, 0.3f, 0.3f);
    boxMax += Vec3(0.3f, 0.3f, 0.3f);

    float tmin = (boxMin.x - localRayOrigin.x) / (localRayDir.x != 0.0f ? localRayDir.x : 1e-8f);
    float tmax = (boxMax.x - localRayOrigin.x) / (localRayDir.x != 0.0f ? localRayDir.x : 1e-8f);
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (boxMin.y - localRayOrigin.y) / (localRayDir.y != 0.0f ? localRayDir.y : 1e-8f);
    float tymax = (boxMax.y - localRayOrigin.y) / (localRayDir.y != 0.0f ? localRayDir.y : 1e-8f);
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) return false;
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (boxMin.z - localRayOrigin.z) / (localRayDir.z != 0.0f ? localRayDir.z : 1e-8f);
    float tzmax = (boxMax.z - localRayOrigin.z) / (localRayDir.z != 0.0f ? localRayDir.z : 1e-8f);
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) return false;
    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    if (tmax < 0.0f) return false;
    hitDist = (tmin < 0.0f) ? tmax : tmin;
    return true;
}
