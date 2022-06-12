#include "hobbyraytracer.h"
#include "constantMedium.h"

bool ConstantMedium::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    hitRecord rec1, rec2;

    if (!boundary->hit(r, -INFINITY, INFINITY, rec1))
        return false;

    if (!boundary->hit(r, rec1.t + 0.0001f, INFINITY, rec2))
        return false;

    if (rec1.t < t_min) rec1.t = t_min;
    if (rec2.t > t_max) rec2.t = t_max;

    if (rec1.t >= rec2.t)
        return false;

    if (rec1.t < 0)
        rec1.t = 0;

    const float ray_length = glm::length(r.getDirection());
    const float distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
    const float hit_distance = negInvDensity * log(glm::linearRand(0.0f, 1.0f));

    if (hit_distance > distance_inside_boundary)
        return false;

    rec.t = rec1.t + hit_distance / ray_length;
    rec.p = r.at(rec.t);

    rec.normal = glm::vec3(1, 0, 0);  // arbitrary
    rec.frontFace = true;     // also arbitrary
    rec.matPtr = phaseFunction;

    return true;
}

bool ConstantMedium::boundingBox(AABB& outputBox) const
{
    return boundary->boundingBox(outputBox);
}
