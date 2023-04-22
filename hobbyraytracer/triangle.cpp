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

bool ITriangle::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    Triangle t(vertices[0], vertices[1], vertices[2], matPtr);

    bool h = t.hit(r, t_min, t_max, rec);

    if (!h)
        return false;

    glm::vec3 iNormal = rec.u * normals[1] + rec.v * normals[2] + (1 - rec.u - rec.v) * normals[0];
    rec.setFaceNormal(r, iNormal);

    glm::vec2 iUV = rec.u * uvs[1] + rec.v * uvs[2] + (1 - rec.u - rec.v) * uvs[0];
    rec.u = iUV.x;
    rec.v = iUV.y;

    return h;
}

bool ITriangle::boundingBox(AABB& outputBox) const
{
    Triangle t(vertices[0], vertices[1], vertices[2], matPtr);

    return t.boundingBox(outputBox);
}
