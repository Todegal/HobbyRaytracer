#pragma once

#include "hittable.h"

class YZRect : public Hittable
{
public:
	YZRect() :y0(0.0f), y1(0.0f), z0(0.0f), z1(0.0f), k(0.0f), mp(nullptr) { }
	YZRect(float _y0, float _y1, float _z0, float _z1, float _k, std::shared_ptr<Material> matPtr)
		: y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(matPtr) { }

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override
	{
		float t = (k - r.getOrigin().x) / r.getDirection().x;
		if (t < t_min || t > t_max)
		{
			return false;
		}

		float y = r.getOrigin().y + t * r.getDirection().y;
		float z = r.getOrigin().z + t * r.getDirection().z;

		if (y < y0 || y > y1 || z < z0 || z > z1)
		{
			return false;	
		}

		rec.u = (y - y0) / (y1 - y0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;

		auto outward_normal = glm::vec3(1, 0, 0);
		rec.setFaceNormal(r, outward_normal);

		rec.matPtr = mp;
		rec.p = r.at(t);

		return true;
	}

	virtual bool boundingBox(AABB& outputBox) override
	{
		outputBox = AABB(glm::vec3(k - 0.0001, y0, z0), glm::vec3(k + 0.0001, y1, z1));
		return true;
	}

private:
	float y0, y1, z0, z1, k;
	std::shared_ptr<Material> mp;
};

class XZRect : public Hittable
{
public:
	XZRect() :x0(0.0f), x1(0.0f), z0(0.0f), z1(0.0f), k(0.0f), mp(nullptr) { }
	XZRect(float _x0, float _x1, float _z0, float _z1, float _k, std::shared_ptr<Material> matPtr)
		: x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(matPtr) { }

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override
	{
		float t = (k - r.getOrigin().y) / r.getDirection().y;
		if (t < t_min || t > t_max)
		{
			return false;
		}

		float x = r.getOrigin().x + t * r.getDirection().x;
		float z = r.getOrigin().z + t * r.getDirection().z;

		if (x < x0 || x > x1 || z < z0 || z > z1)
		{
			return false;
		}

		rec.u = (x - x0) / (x1 - x0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;

		auto outward_normal = glm::vec3(0, 1, 0);
		rec.setFaceNormal(r, outward_normal);

		rec.matPtr = mp;
		rec.p = r.at(t);

		return true;
	}

	virtual bool boundingBox(AABB& outputBox) override
	{
		outputBox = AABB(glm::vec3(x0, k - 0.0001, z0), glm::vec3(x1, k + 0.0001, z1));
		return true;
	}

private:
	float x0, x1, z0, z1, k;
	std::shared_ptr<Material> mp;
};

class XYRect : public Hittable
{
public:
	XYRect() :x0(0.0f), x1(0.0f), y0(0.0f), y1(0.0f), k(0.0f), mp(nullptr) { }
	XYRect(float _x0, float _x1, float _y0, float _y1, float _k, std::shared_ptr<Material> matPtr)
		: x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(matPtr) { }

	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override
	{
		float t = (k - r.getOrigin().z) / r.getDirection().z;
		if (t < t_min || t > t_max)
		{
			return false;
		}

		float x = r.getOrigin().x + t * r.getDirection().x;
		float y = r.getOrigin().y + t * r.getDirection().y;

		if (x < x0 || x > x1 || y < y0 || y > y1)
		{
			return false;
		}

		rec.u = (x - x0) / (x1 - x0);
		rec.v = (y - y0) / (y1 - y0);
		rec.t = t;

		auto outward_normal = glm::vec3(0, 0, 1);
		rec.setFaceNormal(r, outward_normal);

		rec.matPtr = mp;
		rec.p = r.at(t);

		return true;
	}

	virtual bool boundingBox(AABB& outputBox) override
	{
		outputBox = AABB(glm::vec3(x0, y0, k - 0.0001f), glm::vec3(x1, y1, k + 0.0001f));
		return true;
	}

private:
	float x0, x1, y0, y1, k;
	std::shared_ptr<Material> mp;
};


