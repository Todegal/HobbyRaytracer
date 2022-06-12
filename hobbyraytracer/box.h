#pragma once

#include "aarect.h"
#include "hittableList.h"

class Box : public Hittable
{
public:
	Box(glm::vec3 center, glm::vec3 dimensions, std::shared_ptr<Material> matPtr);

	static std::shared_ptr<Box> minMaxBox(glm::vec3 min, glm::vec3 max, std::shared_ptr<Material> matPtr)
	{
		return std::make_shared<Box>((min + max) / glm::vec3(2.0f), max - min, matPtr);
	}

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) const override;

private:
	void constructBox(glm::vec3 p0, glm::vec3 p1, std::shared_ptr<Material> matPtr);

	glm::vec3 boxMin, boxMax;
	HittableList sides;
};

void Box::constructBox(glm::vec3 min, glm::vec3 max, std::shared_ptr<Material> matPtr)
{
	boxMin = min; boxMax = max;

	sides.add(std::make_shared<XYRect>(min.x, max.x, min.y, max.y, max.z, matPtr));
	sides.add(std::make_shared<XYRect>(min.x, max.x, min.y, max.y, min.z, matPtr));

	sides.add(std::make_shared<XZRect>(min.x, max.x, min.z, max.z, max.y, matPtr));
	sides.add(std::make_shared<XZRect>(min.x, max.x, min.z, max.z, min.y, matPtr));

	sides.add(std::make_shared<YZRect>(min.y, max.y, min.z, max.z, max.x, matPtr));
	sides.add(std::make_shared<YZRect>(min.y, max.y, min.z, max.z, min.x, matPtr));
}

Box::Box(glm::vec3 center, glm::vec3 dimensions, std::shared_ptr<Material> matPtr)
{
	constructBox(center - dimensions / 2.0f, center + dimensions / 2.0f, matPtr);
}

bool Box::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
	return sides.hit(r, t_min, t_max, rec);
}

bool Box::boundingBox(AABB& outputBox) const
{
	outputBox = AABB(boxMin, boxMax);
	return true;
}

