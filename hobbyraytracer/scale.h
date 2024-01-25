#pragma once

#include "hittable.h"

class Scale : public Hittable
{
public:
	Scale(std::shared_ptr<Hittable> object, glm::vec3 factor);

private:
	std::shared_ptr<Hittable> ptr;
	glm::vec3 factor;

	bool hasBox;
	AABB bBox;

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) override;
};