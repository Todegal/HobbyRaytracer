#pragma once

#include <variant>

#include "texture.h"
#include "hittable.h"

struct hitRecord;

class MatVec3
{
public:
	//template <typename T, typename = std::enable_if<std::is_same_v<T, glm::vec3> || std::is_base_of_v<Texture, T>>>
	//MatVec3(T v) : variant(std::move(v))
	//{ }

	MatVec3(glm::vec3 v) : variant(std::move(v)) {}
	MatVec3(std::shared_ptr<Texture> v) : variant(v) {}

	glm::vec3 valueAt(float u, float v, glm::vec3 p) const
	{
		if (auto* color = std::get_if<glm::vec3>(&variant)) {
			return *color;
		}
		else if (auto* texture = std::get_if<std::shared_ptr<Texture>>(&variant)) {
			return (*texture)->colourValue(u, v, p);
		}
		else {
			throw std::runtime_error("Invalid Value");
		}
	}

private:
	std::variant<glm::vec3, std::shared_ptr<Texture>> variant;
};

class MatScalar
{
public:
	MatScalar(float v) : variant(std::move(v)) {}
	MatScalar(std::shared_ptr<Texture> v) : variant(v) {}

	float valueAt(float u, float v, glm::vec3 p) const
	{
		if (auto* color = std::get_if<float>(&variant)) {
			return *color;
		}
		else if (auto* texture = std::get_if<std::shared_ptr<Texture>>(&variant)) {
			return glm::length((*texture)->colourValue(u, v, p));
		}
		else {
			throw std::runtime_error("Invalid Value");
		}
	}

private:
	std::variant<float, std::shared_ptr<Texture>> variant;
};

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
	DiffuseLight(MatVec3 colour, MatScalar strength) : emit(colour), s(strength) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		return false;
	}

	virtual glm::vec3 emitted(float u, float v, const glm::vec3& p) const override
	{
		return emit.valueAt(u, v, p) * s.valueAt(u, v, p);
	}

private:
	MatVec3 emit;
	MatScalar s;
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
	Lambertian(MatVec3 a) : albedo(a) { }

	bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 scatterDirection = rec.normal + glm::sphericalRand(1.0f);

		if (nearZero(scatterDirection))
		{
			scatterDirection = rec.normal;
		}

		scattered = ray(rec.p, scatterDirection);
		attenuation = albedo.valueAt(rec.u, rec.v, rec.p);
		//attenuation = glm::vec3(rec.u, rec.v, 1 - rec.u - rec.v);
		//attenuation = rec.normal;
		//attenuation = glm::vec3(1.0f);

		return true;
	}

private:
	MatVec3 albedo;
};

class Metal : public Material
{
public:
	Metal(MatVec3 colour, MatScalar roughness) : 
		albedo(colour),
		r(roughness) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 reflected = glm::reflect(glm::normalize(r_in.dir), glm::normalize(rec.normal));

		float roughness = glm::length(r.valueAt(rec.u, rec.v, rec.p));
		roughness = roughness < 1 ? roughness : 1;

		scattered = ray(rec.p, reflected + roughness * glm::sphericalRand(1.0f) + glm::vec3(std::numeric_limits<float>::epsilon()));
		attenuation = albedo.valueAt(rec.u, rec.v, rec.p);

		return glm::dot(scattered.dir, glm::normalize(rec.normal)) > 0;
	}

private:
	MatVec3 albedo;
	MatScalar r;
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
	Dielectric(MatScalar indexOfRefraction, MatScalar roughness) : ir(indexOfRefraction), r(roughness) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		attenuation = glm::vec3(1, 1, 1);
		float refractionRatio = rec.frontFace ? (1.0f / ir.valueAt(rec.u, rec.v, rec.p)) : ir.valueAt(rec.u, rec.v, rec.p);

		glm::vec3 unitDirection = glm::normalize(r_in.dir);
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

		scattered = ray(rec.p, direction + r.valueAt(rec.u, rec.v, rec.p) * glm::sphericalRand(1.0f));
		return true;
	}

private:
	MatScalar ir; // Index of refraction
	MatScalar r; // Roughness

private:
	static double reflectance(double cosine, float refIdx)
	{
		double r0 = (1 - refIdx) / (1 + refIdx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * glm::pow((1 - cosine), 5);
	}
};