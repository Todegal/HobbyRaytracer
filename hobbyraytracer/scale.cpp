#include "hobbyraytracer.h"
#include "scale.h"

Scale::Scale(std::shared_ptr<Hittable> p, glm::vec3 f)
    : ptr(p), factor(f)
{

    hasBox = ptr->boundingBox(bBox);
}

bool Scale::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    glm::vec3 origin = r.getOrigin();
    glm::vec3 direction = r.getDirection();

    ray scaledRay(origin / factor, direction / factor);

    if (!ptr->hit(scaledRay, t_min, t_max, rec))
    {
        return false;
    }

    rec.p *= factor;
    rec.normal *= glm::normalize(factor);
    rec.setFaceNormal(scaledRay, rec.normal);

    return true;
}

bool Scale::boundingBox(AABB& outputBox) const
{
    outputBox = bBox;
    return hasBox;
}
