#include "hobbyraytracer.h"
#include "sphere.h"

void Sphere::getSphereUV(const glm::vec3& p, float& u, float& v)
{
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    float theta = glm::acos(-p.y);
    float phi = std::atan2(-p.z, p.x) + glm::pi<float>();

    u = phi / (2 * glm::pi<float>());
    v = theta / glm::pi<float>();
}

bool Sphere::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    glm::vec3 oc = r.o - center;
    float a = glm::length(r.dir) * glm::length(r.dir);
    float half_b = glm::dot(oc, r.dir);
    float c = glm::length(oc) * glm::length(oc) - radius * radius;

    float discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    float sqrtd = sqrtf(discriminant);

    float root = (-half_b - sqrtd) / a;
    if (root < t_min || root > t_max) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || root > t_max)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);

    glm::vec3 outwardNormal = (rec.p - center) / radius;
    rec.setFaceNormal(r, outwardNormal);

    getSphereUV(outwardNormal, rec.u, rec.v);

    rec.matPtr = matPtr;

    return true;
}

bool Sphere::boundingBox(AABB& outputBox)
{
    outputBox = AABB(
        center - glm::vec3(radius),
        center + glm::vec3(radius)
    );

    return true;
}