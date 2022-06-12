#pragma once

#include "hittableList.h"

class BVHNode : public Hittable
{
public:
	BVHNode() { }

	BVHNode(const HittableList& list)
		: BVHNode(list.objects, 0, list.objects.size())
	{ }

	BVHNode(
		const std::vector<std::shared_ptr<Hittable>>& srcObjects,
		size_t start, size_t end);

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) const override;

private:
	std::shared_ptr<Hittable> left;
	std::shared_ptr<Hittable> right;
	AABB box;

private:
	static bool boxCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis);

	static bool boxXCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b)
	{
		return boxCompare(a, b, 0);
	}

	static bool boxYCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b)
	{
		return boxCompare(a, b, 1);
	}

	static bool boxZCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b)
	{
		return boxCompare(a, b, 2);
	}
};