#pragma once

#include "texture.h"
#include "hittable.h"

struct hitRecord;

class Material
{
public:
	virtual bool scatter(
		const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered
	) const = 0;

	virtual glm::vec3 emitted(float u, float v, const glm::vec3& p) const
	{
		return glm::vec3(0, 0, 0);
	}
};

class Isotropic : public Material
{
public:
	Isotropic(glm::vec3 c) : albedo(std::make_shared<SolidColourTexture>(c)) { }
	Isotropic(std::shared_ptr<Texture> a) : albedo(a) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		scattered = ray(rec.p, glm::ballRand(1.0f));
		attenuation = albedo->colourValue(rec.u, rec.v, rec.p);

		return true;
	}

private:
	std::shared_ptr<Texture> albedo;
};

class DiffuseLight : public Material
{
public:
	DiffuseLight(glm::vec3 colour, float strength) : emit(std::make_shared<SolidColourTexture>(colour)), s(strength) { }
	DiffuseLight(std::shared_ptr<Texture> a, float strength) : emit(a), s(strength) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		return false;
	}

	virtual glm::vec3 emitted(float u, float v, const glm::vec3& p) const override
	{
		return emit->colourValue(u, v, p) * s;
	}

private:
	std::shared_ptr<Texture> emit;
	float s;
};

class UVTest : public Material
{
public:
	UVTest() { }

	bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 scatterDirection = rec.normal + glm::sphericalRand(1.0f);

		if (nearZero(scatterDirection))
		{
			scatterDirection = rec.normal;
		}

		scattered = ray(rec.p, scatterDirection);
		attenuation = rec.normal;

		return true;
	}
};

class Lambertian : public Material
{
public:
	Lambertian(const glm::vec3& colour) : albedo(std::make_shared<SolidColourTexture>(colour)) { }
	Lambertian(std::shared_ptr<Texture> a) :albedo(a) { }

	bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 scatterDirection = rec.normal + glm::sphericalRand(1.0f);

		if (nearZero(scatterDirection))
		{
			scatterDirection = rec.normal;
		}

		scattered = ray(rec.p, scatterDirection);
		attenuation = albedo->colourValue(rec.u, rec.v, rec.p);

		return true;
	}

private:
	std::shared_ptr<Texture> albedo;
};

class Metal : public Material
{
public:
	Metal(const glm::vec3& colour, float roughness) : 
		albedo(std::make_shared<SolidColourTexture>(colour)),
		r(std::make_shared<SolidColourTexture>(glm::vec3(roughness))) { }

	Metal(const std::shared_ptr<Texture> a, float roughness) : 
		albedo(a), r(std::make_shared<SolidColourTexture>(glm::vec3(roughness))) { }
	
	// Takes a greyscale roughness map
	Metal(const std::shared_ptr<Texture> a, const std::shared_ptr<Texture> roughness) :
		albedo(a), r(roughness) { }

	Metal(const glm::vec3& colour, const std::shared_ptr<Texture> roughness) :
		albedo(std::make_shared<SolidColourTexture>(colour)), r(roughness) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 reflected = glm::reflect(glm::normalize(r_in.getDirection()), rec.normal);

		float roughness = glm::length(r->colourValue(rec.u, rec.v, rec.p));
		roughness = roughness < 1 ? roughness : 1;

		scattered = ray(rec.p, reflected + roughness * glm::sphericalRand(1.0f));
		attenuation = albedo->colourValue(rec.u, rec.v, rec.p);

		return glm::dot(scattered.getDirection(), rec.normal) > 0;
	}

private:
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> r;
};

class PBR : public Material
{
public:
	PBR(glm::vec3 albedo, float metallness, float roughness);
	PBR(std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> metallness, float roughness);

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override;

private:
	std::shared_ptr<Metal> metal;
	std::shared_ptr<Lambertian> diffuse;

	std::shared_ptr<Texture> mix;
};

class Dielectric : public Material
{
public:
	Dielectric(float indexOfRefraction, float roughness) : ir(indexOfRefraction), r(roughness) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		attenuation = glm::vec3(1, 1, 1);
		float refractionRatio = rec.frontFace ? (1.0f / ir) : ir;

		glm::vec3 unitDirection = glm::normalize(r_in.getDirection());
		double cosTheta = glm::min(glm::dot(-unitDirection, rec.normal), 1.0f);
		double sinTheta = glm::sqrt(1.0 - cosTheta * cosTheta);

		bool cannot_refract = refractionRatio * sinTheta > 1.0;
		glm::vec3 direction;

		double ref = reflectance(cosTheta, refractionRatio);

		if (cannot_refract || ref > glm::linearRand(0.0, 1.0))
		{
			direction = reflect(unitDirection, rec.normal);
		}
		else
		{
			direction = glm::refract(unitDirection, rec.normal, refractionRatio);
		}

		scattered = ray(rec.p, direction + r * glm::sphericalRand(1.0f));
		return true;
	}

private:
	float ir; // Index of refraction
	float r; // Roughness

private:
	static double reflectance(double cosine, float refIdx)
	{
		double r0 = (1 - refIdx) / (1 + refIdx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * glm::pow((1 - cosine), 5);
	}
};