#pragma once

#include "hittable.h"

class RotateY : public Hittable
{
public:
	RotateY(std::shared_ptr<Hittable> object, float angle);

private:
	std::shared_ptr<Hittable> ptr;
	float sinTheta, cosTheta;

	bool hasBox;
	AABB bBox;

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) override;
};