#include "hobbyraytracer.h"
#include "triangle.h"

bool Triangle::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v0v2 = v2 - v0;
    
    glm::vec3 pV = glm::cross(r.getDirection(), v0v2);
    float d = glm::dot(v0v1, pV);

    if (d < 0.0001f) return false;

    if (glm::abs(d) < 0.0001f) return false;

    float invD = 1.0f / d;

    glm::vec3 tV = r.getOrigin() - v0;
    rec.u = glm::dot(tV, pV) * invD;
    if (rec.u < 0 || rec.u > 1) return false;

    glm::vec3 qV = glm::cross(tV, v0v1);
    rec.v = glm::dot(r.getDirection(), qV) * invD;
    if (rec.v < 0 || rec.u + rec.v > 1) return false;

    rec.t = glm::dot(v0v2, qV) * invD;
    rec.p = r.at(rec.t);

    rec.matPtr = matPtr;

    rec.setFaceNormal(r, glm::normalize(glm::cross(v0v1, v0v2)));

    return true;
}

bool Triangle::boundingBox(AABB& outputBox) const
{
    float minX = glm::min(glm::min(v0.x, v1.x), v2.x);
    float minY = glm::min(glm::min(v0.y, v1.y), v2.y);
    float minZ = glm::min(glm::min(v0.z, v1.z), v2.z);

    float maxX = glm::max(glm::max(v0.x, v1.x), v2.x);
    float maxY = glm::max(glm::max(v0.y, v1.y), v2.y);
    float maxZ = glm::max(glm::max(v0.z, v1.z), v2.z);

    outputBox = AABB(glm::vec3(minX - 0.0001f, minY - 0.0001f, minZ - 0.0001f), glm::vec3(maxX + 0.0001f, maxY + 0.0001f, maxZ + 0.0001f));

    return true;
}

bool ITriangle::calculateBarycentric(glm::vec3 point, glm::vec3& bc) const {
    // Retrieve the vertices for the face
    glm::vec3 v0 = vertices[0];
    glm::vec3 v1 = vertices[1];
    glm::vec3 v2 = vertices[2];

    // Calculate the barycentric coordinates of the intersection point
    glm::mat3 A(v0 - v2, v1 - v2, point - v2);
    glm::mat3 inverseA = glm::inverse(A);
    glm::vec3 barycentric = inverseA * glm::vec3(1.0f, 1.0f, 1.0f);

    // Check if the barycentric coordinates fall within the valid range for the triangle
    bool isInsideTriangle = barycentric.x >= 0.0f && barycentric.y >= 0.0f && barycentric.z >= 0.0f && barycentric.x + barycentric.y <= 1.0f;

    return isInsideTriangle;
}

bool ITriangle::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    // Calculate the normal of the triangle
    glm::vec3 normal = glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));

    // Calculate the distance from the ray origin to the plane of the triangle
    float distanceToPlane = glm::dot(vertices[0] - r.getOrigin(), normal) / glm::dot(r.getDirection(), normal);

    // If the distance to the plane is outside the valid range, return false
    if (distanceToPlane < t_min || distanceToPlane > t_max) {
        return false;
    }

    // Calculate the intersection point with the plane of the triangle
    glm::vec3 intersectionPoint = r.getOrigin() + distanceToPlane * r.getDirection();

    glm::vec3 barycentric;
    if (calculateBarycentric(intersectionPoint, barycentric))
    {
        barycentric = glm::clamp(barycentric, glm::vec3(0.0f), glm::vec3(1.0f));
    }
    else
    {
        return false;
    }

    // Update the hit record with this intersection point
    rec.t = distanceToPlane;
    rec.p = intersectionPoint;

    glm::vec3 interpolatedNormal = barycentric.x * normals[0] + barycentric.y * normals[1] + barycentric.z * normals[2];

    rec.normal = glm::normalize(interpolatedNormal);
    rec.setFaceNormal(r, rec.normal);

    glm::vec2 interpolatedUV = barycentric.x * uvs[0] + barycentric.y * uvs[1] + barycentric.z * uvs[2];
    rec.u = interpolatedUV.x; rec.v = interpolatedUV.y;
    
    rec.matPtr = matPtr;

    return true;
}

bool ITriangle::boundingBox(AABB& outputBox) const
{
    float minX = glm::min(glm::min(vertices[0].x, vertices[1].x), vertices[2].x);
    float minY = glm::min(glm::min(vertices[0].y, vertices[1].y), vertices[2].y);
    float minZ = glm::min(glm::min(vertices[0].z, vertices[1].z), vertices[2].z);

    float maxX = glm::max(glm::max(vertices[0].x, vertices[1].x), vertices[2].x);
    float maxY = glm::max(glm::max(vertices[0].y, vertices[1].y), vertices[2].y);
    float maxZ = glm::max(glm::max(vertices[0].z, vertices[1].z), vertices[2].z);

    outputBox = AABB(glm::vec3(minX - 0.0001f, minY - 0.0001f, minZ - 0.0001f), glm::vec3(maxX + 0.0001f, maxY + 0.0001f, maxZ + 0.0001f));

    return true;
}
