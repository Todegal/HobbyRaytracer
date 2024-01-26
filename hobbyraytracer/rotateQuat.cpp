#include "hobbyraytracer.h"
#include "rotateQuat.h"

#include <glm\gtc\quaternion.hpp>

RotateQuat::RotateQuat(std::shared_ptr<Hittable> p, glm::quat r)
    : ptr(p), rotation(r)
{
    // Get the original axis - aligned bounding box of the object
    hasBox = ptr->boundingBox(bBox);

    // Get the vertices of the original box in world coordinates
    glm::vec3 minPoint = bBox.getMin();
    glm::vec3 maxPoint = bBox.getMax();
    glm::vec3 vertices[8] = {
        glm::vec3(minPoint.x, minPoint.y, minPoint.z),
        glm::vec3(minPoint.x, minPoint.y, maxPoint.z),
        glm::vec3(minPoint.x, maxPoint.y, minPoint.z),
        glm::vec3(minPoint.x, maxPoint.y, maxPoint.z),
        glm::vec3(maxPoint.x, minPoint.y, minPoint.z),
        glm::vec3(maxPoint.x, minPoint.y, maxPoint.z),
        glm::vec3(maxPoint.x, maxPoint.y, minPoint.z),
        glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z)
    };
    for (int i = 0; i < 8; i++) {
        vertices[i] = rotation * vertices[i];
    }

    // Calculate the new axis-aligned bounding box
    glm::vec3 newMinPoint = vertices[0];
    glm::vec3 newMaxPoint = vertices[0];
    for (int i = 1; i < 8; i++) {
        newMinPoint.x = std::min(newMinPoint.x, vertices[i].x);
        newMinPoint.y = std::min(newMinPoint.y, vertices[i].y);
        newMinPoint.z = std::min(newMinPoint.z, vertices[i].z);
        newMaxPoint.x = std::max(newMaxPoint.x, vertices[i].x);
        newMaxPoint.y = std::max(newMaxPoint.y, vertices[i].y);
        newMaxPoint.z = std::max(newMaxPoint.z, vertices[i].z);
    }

    bBox = AABB(newMinPoint, newMaxPoint);
}

bool RotateQuat::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    // Rotate the ray according to the object's rotation
    glm::vec3 origin = r.o;
    glm::vec3 direction = r.dir;
    glm::quat invRotation = glm::conjugate(rotation);
    glm::vec3 newOrigin = invRotation * origin;
    glm::vec3 newDirection = glm::normalize(invRotation * direction);
    ray rotatedRay(newOrigin, newDirection);

    // Check for intersection with the rotated object
    if (!ptr->hit(rotatedRay, t_min, t_max, rec)) {
        return false;
    }

    // Rotate the hit point and surface normal back to world coordinates
    rec.p = rotation * rec.p;
    rec.normal = rotation * rec.normal;

    rec.setFaceNormal(rotatedRay, rec.normal);

    return true;
}

bool RotateQuat::boundingBox(AABB& outputBox)
{
    outputBox = bBox;
    return hasBox;
}
