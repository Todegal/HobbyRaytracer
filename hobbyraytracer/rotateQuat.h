#pragma once

#include "hittable.h"

class RotateQuat : public Hittable
{
public:
	RotateQuat(std::shared_ptr<Hittable> object, glm::quat r);

private:
	std::shared_ptr<Hittable> ptr;
	glm::quat rotation;

	bool hasBox;
	AABB bBox;

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) override;
};