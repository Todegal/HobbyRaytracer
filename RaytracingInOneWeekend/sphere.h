#pragma once

#include "hittable.h"

class Sphere : public Hittable
{
public:
	Sphere() { }
	Sphere(glm::vec3 c, float r, std::shared_ptr<Material> m) : center(c), radius(r), matPtr(m) { }

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;

private:
	glm::vec3 center;
	float radius;
    std::shared_ptr<Material> matPtr;
};

bool Sphere::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    glm::vec3 oc = r.getOrigin() - center;
    float a = glm::length(r.getDirection()) * glm::length(r.getDirection());
    float half_b = glm::dot(oc, r.getDirection());
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

    rec.matPtr = matPtr;

	return true;
}