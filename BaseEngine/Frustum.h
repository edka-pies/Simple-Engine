#pragma once
#include <glm/glm/glm.hpp>
#include "Mesh.h" // For AABB struct

struct Plane {
    glm::vec3 normal;
    float distance;

    // Normalizes the plane equation
    void Normalize() {
        float mag = glm::length(normal);
        normal /= mag;
        distance /= mag;
    }
};

class Frustum {
public:
    Plane planes[6];

    // Extracts the 6 planes from the Camera's View-Projection Matrix
    void Update(const glm::mat4& viewProj) {
        // Left
        planes[0].normal.x = viewProj[0][3] + viewProj[0][0];
        planes[0].normal.y = viewProj[1][3] + viewProj[1][0];
        planes[0].normal.z = viewProj[2][3] + viewProj[2][0];
        planes[0].distance = viewProj[3][3] + viewProj[3][0];

        // Right
        planes[1].normal.x = viewProj[0][3] - viewProj[0][0];
        planes[1].normal.y = viewProj[1][3] - viewProj[1][0];
        planes[1].normal.z = viewProj[2][3] - viewProj[2][0];
        planes[1].distance = viewProj[3][3] - viewProj[3][0];

        // Bottom
        planes[2].normal.x = viewProj[0][3] + viewProj[0][1];
        planes[2].normal.y = viewProj[1][3] + viewProj[1][1];
        planes[2].normal.z = viewProj[2][3] + viewProj[2][1];
        planes[2].distance = viewProj[3][3] + viewProj[3][1];

        // Top
        planes[3].normal.x = viewProj[0][3] - viewProj[0][1];
        planes[3].normal.y = viewProj[1][3] - viewProj[1][1];
        planes[3].normal.z = viewProj[2][3] - viewProj[2][1];
        planes[3].distance = viewProj[3][3] - viewProj[3][1];

        // Near
        planes[4].normal.x = viewProj[0][3] + viewProj[0][2];
        planes[4].normal.y = viewProj[1][3] + viewProj[1][2];
        planes[4].normal.z = viewProj[2][3] + viewProj[2][2];
        planes[4].distance = viewProj[3][3] + viewProj[3][2];

        // Far
        planes[5].normal.x = viewProj[0][3] - viewProj[0][2];
        planes[5].normal.y = viewProj[1][3] - viewProj[1][2];
        planes[5].normal.z = viewProj[2][3] - viewProj[2][2];
        planes[5].distance = viewProj[3][3] - viewProj[3][2];

        for (int i = 0; i < 6; i++) {
            planes[i].Normalize();
        }
    }

    // Returns TRUE if the AABB is inside or touching the frustum
    bool IsBoxVisible(const glm::vec3& minExtents, const glm::vec3& maxExtents) const {
        for (int i = 0; i < 6; i++) {
            glm::vec3 p(minExtents);
            if (planes[i].normal.x >= 0) p.x = maxExtents.x;
            if (planes[i].normal.y >= 0) p.y = maxExtents.y;
            if (planes[i].normal.z >= 0) p.z = maxExtents.z;

            // If the furthest point of the box is behind the plane, the whole box is hidden
            if (glm::dot(planes[i].normal, p) + planes[i].distance < 0) {
                return false;
            }
        }
        return true;
    }
};