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
    bool h = t.hit(r, t_min, t_max, rec);
    
    if (h)
    {
        /*glm::vec3 p = r.getOrigin() + (r.getDirection() * rec.t);

        float l0 = glm::length(p - vertices[0]);
        float l1 = glm::length(p - vertices[1]);
        float l2 = glm::length(p - vertices[2]);

        float s = l0 + l1 + l2;

        float f0 = l0 / s;
        float f1 = l1 / s;
        float f2 = l2 / s;

        rec.u = uvs[0][0] * f0 + uvs[1][0] * f1 + uvs[2][0] * f2;
        rec.v = uvs[0][1] * f0 + uvs[1][1] * f1 + uvs[2][1] * f2;

        float N0 = normals[0][0] * f0 + normals[1][0] * f1 + normals[2][0] * f2;
        float N1 = normals[0][1] * f0 + normals[1][1] * f1 + normals[2][1] * f2;
        float N2 = normals[0][2] * f0 + normals[1][2] * f1 + normals[2][2] * f2;

        glm::vec3 N = glm::normalize(glm::vec3(N0, N1, N2));
        
        if (N != glm::vec3(0.0f))
        {
            rec.setFaceNormal(r, N);
        }*/

        return true;

    }

    return false;
}

bool ITriangle::boundingBox(AABB& outputBox) const
{
    return t.boundingBox(outputBox);
}
