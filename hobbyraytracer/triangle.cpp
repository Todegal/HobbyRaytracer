#include "hobbyraytracer.h"
#include "triangle.h"

bool Triangle::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v0v2 = v2 - v0;
    
    glm::vec3 pV = glm::normalize(glm::cross(r.getDirection(), v0v2));
    float d = glm::dot(glm::normalize(v0v1), pV);

    if (d < 0.0001f) return false;

    if (glm::abs(d) < 0.0001f) return false;

    float invD = 1.0f / d;

    glm::vec3 tV = glm::normalize(r.getOrigin() - v0);
    rec.u = glm::dot(tV, pV) * invD;
    if (rec.u < 0 || rec.u > 1) return false;

    glm::vec3 qV = glm::cross(tV, glm::normalize(v0v1));
    rec.v = glm::dot(glm::normalize(r.getDirection()), qV) * invD;
    if (rec.v < 0 || rec.u + rec.v > 1) return false;

    rec.t = glm::dot(v0v2, qV) * invD;

    if (rec.t < t_min) return false;
    if (rec.t > t_max) return false;

    rec.p = r.at(rec.t);

    rec.matPtr = matPtr;

    assert(rec.u + rec.v <= 1);

    rec.setFaceNormal(r, glm::cross(v0v1, v0v2));

    return true;
}

bool Triangle::boundingBox(AABB& outputBox)
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
    // See: https://pbr-book.org/3ed-2018/Shapes/Triangle_Meshes

    glm::vec3 d = r.getDirection();
    glm::vec3 o = r.getOrigin();

    // Translate ray to origin
    glm::vec3 p0t = vertices[0] - o;
    glm::vec3 p1t = vertices[1] - o;
    glm::vec3 p2t = vertices[2] - o;

    // Re-orientate vector around the +Z axis
    int kZ = o.x > o.z ? o.x > o.y ? 0 : 1 : 2;
    int kX = kZ + 1 == 3 ? 0 : kZ + 1;
    int kY = kX + 1 == 3 ? 0 : kX + 1;

    d = { d[kX], d[kY], d[kZ] };
    p0t = { p0t[kX], p0t[kY], p0t[kZ] };
    p1t = { p1t[kX], p1t[kY], p1t[kZ] };
    p2t = { p2t[kX], p2t[kY], p2t[kZ] };

    // Manually apply shearing matrix
    float sX = -d.x / d.z;
    float sY = -d.y / d.z;
    float sZ = 1.0f / d.z;

    p0t.x += sX * p0t.z;
    p0t.y += sY * p0t.z;
    p1t.x += sX * p1t.z;
    p1t.y += sY * p1t.z;
    p2t.x += sX * p2t.z;
    p2t.y += sY * p2t.z;

    // Compute edge functions
    float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

    // Perform triangle edge and determinant tests
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    float det = e0 + e1 + e2;
    if (det == 0) return false;

    p0t.z *= sZ;
    p1t.z *= sZ;
    p2t.z *= sZ;
    float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < t_max * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > t_max * det))
        return false;

    // Compute barycentric coordinates and $t$ value for triangle intersection
    float invDet = 1 / det;
    float b0 = e0 * invDet;
    float b1 = e1 * invDet;
    float b2 = e2 * invDet;
    float t = tScaled * invDet;

    rec.t = t;
    rec.p = r.at(rec.t);
    rec.matPtr = matPtr;
    
    glm::vec3 normal = b0 * normals[0] + b1 * normals[1] + b2 * normals[2];
    glm::vec2 uv = b0 * uvs[0] + b1 * uvs[1] + b2 * uvs[2];

    rec.normal = normal;

    rec.u = uv.r;
    rec.v = uv.g;

    return true;
}

bool ITriangle::boundingBox(AABB& outputBox)
{
    if (bBox.getMax() == bBox.getMin())
    {
        float minX = glm::min(glm::min(vertices[0].x, vertices[1].x), vertices[2].x);
        float minY = glm::min(glm::min(vertices[0].y, vertices[1].y), vertices[2].y);
        float minZ = glm::min(glm::min(vertices[0].z, vertices[1].z), vertices[2].z);

        float maxX = glm::max(glm::max(vertices[0].x, vertices[1].x), vertices[2].x);
        float maxY = glm::max(glm::max(vertices[0].y, vertices[1].y), vertices[2].y);
        float maxZ = glm::max(glm::max(vertices[0].z, vertices[1].z), vertices[2].z);

        bBox = AABB(glm::vec3(minX - 0.0001f, minY - 0.0001f, minZ - 0.0001f), glm::vec3(maxX + 0.0001f, maxY + 0.0001f, maxZ + 0.0001f));
    }

    outputBox = bBox;

    return true;
}
