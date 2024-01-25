#pragma once

#include "hittable.h"
#include "material.h"
#include "texture.h"

class ConstantMedium : public Hittable
{
public:
	ConstantMedium(std::shared_ptr<Hittable> b, float d, std::shared_ptr<Texture> a)
		: boundary(b), negInvDensity(-1/d), phaseFunction(std::make_shared<Isotropic>(a)) { }

	ConstantMedium(std::shared_ptr<Hittable> b, float d, glm::vec3 colour)
		: boundary(b), negInvDensity(-1/d), phaseFunction(std::make_shared<Isotropic>(colour)) { }

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) override;

private:
	std::shared_ptr<Hittable> boundary;
	std::shared_ptr<Material> phaseFunction;
	float negInvDensity;
};