#pragma once

#include "hittable.h"

class Triangle : public Hittable
{
public:
	Triangle() : v0(0.0f), v1(0.0f), v2(0.0f), matPtr(nullptr) { }
	Triangle(glm::vec3 _v0, glm::vec3 _v1, glm::vec3 _v2, std::shared_ptr<Material> _matPtr) : v0(_v0), v1(_v1), v2(_v2), matPtr(_matPtr) { }

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) const override;

private:
	glm::vec3 v0, v1, v2;
	std::shared_ptr<Material> matPtr;
};

class ITriangle : public Hittable
{
public:
	ITriangle();
	ITriangle(std::array<glm::vec3, 3> _vertices, std::array<glm::vec3, 3> _normals, std::array<glm::vec2, 3> _uvs, std::shared_ptr<Material> _matPtr)
		: vertices(_vertices), normals(_normals), uvs(_uvs), matPtr(_matPtr) { }

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) const override;

private:
	std::array<glm::vec3, 3> vertices, normals;
	std::array<glm::vec2, 3> uvs;

	std::shared_ptr<Material> matPtr;

};