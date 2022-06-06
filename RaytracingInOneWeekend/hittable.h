#pragma once

#include "ray.h"

class Material;

struct hitRecord 
{
	glm::vec3 p;
	glm::vec3 normal;

	std::shared_ptr<Material> matPtr;

	float t;

	bool frontFace;

	inline void setFaceNormal(const ray& r, const glm::vec3& outward_normal) {
		frontFace = glm::dot(r.getDirection(), outward_normal) < 0;
		normal = frontFace ? outward_normal : -outward_normal;
	}
};

class Hittable
{
public:
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const = 0;
};