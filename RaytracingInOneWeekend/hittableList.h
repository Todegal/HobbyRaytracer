#pragma once

#include "hittable.h"

class HittableList : public Hittable
{
public:
	HittableList() { }
	HittableList(std::shared_ptr<Hittable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(std::shared_ptr<Hittable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;

private:
	std::vector<std::shared_ptr<Hittable>> objects;
};

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