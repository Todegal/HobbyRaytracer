#include "hobbyraytracer.h"
#include "translate.h"

#include "ray.h"
#include "aabb.h"

bool Translate::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
    ray movedR(r.o - offset, r.dir);
	if (!ptr->hit(movedR, t_min, t_max, rec))
	{
		return false;
	}

	rec.p += offset;
	rec.setFaceNormal(movedR, rec.normal);

	return true;
}

bool Translate::boundingBox(AABB& outputBox)
{
	if (!ptr->boundingBox(outputBox))
	{
		return false;
	}

	outputBox = AABB(
		outputBox.getMin() + offset,
		outputBox.getMax() + offset
	);

	return true;
}
