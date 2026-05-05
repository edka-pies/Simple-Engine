#pragma once
#include "glm/glm/glm.hpp"
struct AABB {
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
};

struct Sphere {
    glm::vec3 center;
    float radius;
};

struct Triangle {
    glm::vec3 a, b, c;
};

//AABB to AABB
inline bool TestAABBAABB(const AABB& a, const AABB& b) {
    return (a.minBounds.x <= b.maxBounds.x && a.maxBounds.x >= b.minBounds.x) &&
        (a.minBounds.y <= b.maxBounds.y && a.maxBounds.y >= b.minBounds.y) &&
        (a.minBounds.z <= b.maxBounds.z && a.maxBounds.z >= b.minBounds.z);
}

// Sphere to Sphere
inline bool TestSphereSphere(const Sphere& a, const Sphere& b) {
    glm::vec3 d = a.center - b.center;
    float distSq = glm::dot(d, d);
    float radiusSum = a.radius + b.radius;

    return distSq <= (radiusSum * radiusSum);
}

//Trig to Sphere
inline glm::vec3 ClosestPointOnTriangle(const glm::vec3& p, const Triangle& tri) {
    glm::vec3 ab = tri.b - tri.a;
    glm::vec3 ac = tri.c - tri.a;
    glm::vec3 ap = p - tri.a;

    float d1 = glm::dot(ab, ap);
    float d2 = glm::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return tri.a;

    glm::vec3 bp = p - tri.b;
    float d3 = glm::dot(ab, bp);
    float d4 = glm::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return tri.b;

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return tri.a + v * ab;
    }

    glm::vec3 cp = p - tri.c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return tri.c;

    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return tri.a + w * ac;
    }

    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return tri.b + w * (tri.c - tri.b);
    }

    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return tri.a + ab * v + ac * w;
}

// collision test
inline bool TestSphereTriangle(const Sphere& sphere, const Triangle& tri, glm::vec3& outHitPoint) {
    // Find the closest point on the triangle to the sphere's center
    outHitPoint = ClosestPointOnTriangle(sphere.center, tri);

    // Check if that point is inside the sphere
    glm::vec3 v = sphere.center - outHitPoint;
    float distSq = glm::dot(v, v);

    return distSq <= (sphere.radius * sphere.radius);
}

inline AABB GetWorldAABB(const AABB& localBox, const glm::mat4& modelMatrix) {
    AABB worldBox;

    glm::vec3 globalCenter = glm::vec3(modelMatrix[3]);

    glm::vec3 localCenter = (localBox.maxBounds + localBox.minBounds) * 0.5f;
    glm::vec3 localExtents = (localBox.maxBounds - localBox.minBounds) * 0.5f;

    glm::mat3 absMatrix;
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            absMatrix[col][row] = std::abs(modelMatrix[col][row]);
        }
    }

    glm::vec3 newCenter = globalCenter + glm::mat3(modelMatrix) * localCenter;
    glm::vec3 newExtents = absMatrix * localExtents;

    worldBox.minBounds = newCenter - newExtents;
    worldBox.maxBounds = newCenter + newExtents;

    return worldBox;
}