#pragma once

#include "hittable.h"

class Sphere : public Hittable
{
public:
	Sphere() :center(0.0f), radius(0.0f), matPtr(nullptr) { }
	Sphere(glm::vec3 c, float r, std::shared_ptr<Material> m) : center(c), radius(r), matPtr(m) { }

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
    virtual bool boundingBox(AABB& outputBox) const override;

	static void getSphereUV(const glm::vec3& p, float& u, float& v);

private:
	glm::vec3 center;
	float radius;
    std::shared_ptr<Material> matPtr;
};