#pragma once

#include "hittable.h"
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
	static inline bool boxCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis)
	{
		AABB boxA, boxB;

		if (!a->boundingBox(boxA) || !b->boundingBox(boxB))
		{
			std::cerr << "No bounding box in BVHNode constructor" << std::endl;
		}

		return boxA.getMin()[axis] < boxB.getMin()[axis];
	}

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

BVHNode::BVHNode(const std::vector<std::shared_ptr<Hittable>>& srcObjects,
	size_t start, size_t end)
{
	std::vector<std::shared_ptr<Hittable>> objects = srcObjects;

	int axis = glm::linearRand(0, 2);
	auto comparator = (axis == 0) ? BVHNode::boxXCompare :
		(axis == 1) ? BVHNode::boxYCompare
		: BVHNode::boxZCompare;

	size_t objectSpan = end - start;

	if (objectSpan == 1)
	{
		left = right = objects[start];
	}
	else if (objectSpan == 2)
	{
		if (comparator(objects[start], objects[start + 1]))
		{
			left = objects[start];
			right = objects[start + 1];
		}
		else
		{
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else
	{
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		size_t mid = start + objectSpan / 2;
		left = std::make_shared<BVHNode>(objects, start, mid);
		right = std::make_shared<BVHNode>(objects, mid, end);
	}

	AABB boxLeft, boxRight;

	if (!left->boundingBox(boxLeft)
		|| !right->boundingBox(boxRight))
	{
		std::cerr << "No bounding box in BVHNode constructor" << std::endl;
	}

	box = AABB::surroundingBox(boxLeft, boxRight);
}

bool BVHNode::boundingBox(AABB& outputBox) const 
{
	outputBox = box;
	return true;
}

bool BVHNode::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
	if (!box.hit(r, t_min, t_max))
		return false;

	bool hitLeft = left->hit(r, t_min, t_max, rec);
	bool hitRight = right->hit(r, t_min, hitLeft ? rec.t : t_max, rec);

	return hitLeft || hitRight;
}
