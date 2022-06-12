#include "hobbyraytracer.h"
#include "hittableList.h"

bool HittableList::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
	hitRecord tempRec;
	bool hitAnything = false;
	float closest = t_max;

	for (const auto& object : objects)
	{
		if (object->hit(r, t_min, closest, tempRec))
		{
			hitAnything = true;
			closest = tempRec.t;
			rec = tempRec;
		}
	}

	return hitAnything;
}

bool HittableList::boundingBox(AABB& outputBox) const
{
	if (objects.empty()) return false;

	AABB tempBox;
	bool firstBox = true;

	for (const auto& object : objects)
	{
		if (!object->boundingBox(tempBox)) return false;
		outputBox = firstBox ? tempBox : AABB::surroundingBox(outputBox, tempBox);
		firstBox = false;
	}

	return true;
}