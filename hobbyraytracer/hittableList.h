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
	virtual bool boundingBox(AABB& outputBox) override;

	//std::vector<std::shared_ptr<Hittable>> getObjects() const { return objects; }

public:
	std::vector<std::shared_ptr<Hittable>> objects;
};

