#include "stdafx.h"
#include "material.h"

PBR::PBR(std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> metallness, float roughness)
	: mix(metallness)
{
	metal = std::make_shared<Metal>(albedo, roughness);
	diffuse = std::make_shared<Lambertian>(albedo);
}

bool PBR::scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const
{
	bool m = glm::length(mix->colourValue(rec.u, rec.v, rec.p)) > 0.5f;
	if (m)
	{
		return metal->scatter(r_in, rec, attenuation, scattered);
	}

	return diffuse->scatter(r_in, rec, attenuation, scattered);

}