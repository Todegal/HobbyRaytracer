#pragma once

#include "hittable.h"

class Translate : public Hittable
{
public:
	Translate(std::shared_ptr<Hittable> object, const glm::vec3& displacement)
		: ptr(object), offset(displacement) { }

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) const override;

private:
	std::shared_ptr<Hittable> ptr;
	glm::vec3 offset;
};