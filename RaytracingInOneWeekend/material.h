#pragma once

struct hitRecord;

class Material
{
public:
	virtual bool scatter(
		const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered
	) const = 0;
};

class Lambertian : public Material
{
public:
	Lambertian(const glm::vec3& colour) : albedo(colour) { }

	bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 scatterDirection = rec.normal + glm::sphericalRand(1.0f);

		if (nearZero(scatterDirection))
		{
			scatterDirection = rec.normal;
		}

		scattered = ray(rec.p, scatterDirection);
		attenuation = albedo;

		return true;
	}

private:
	glm::vec3 albedo;
};

class Metal : public Material
{
public:
	Metal(const glm::vec3& colour, float roughness) : albedo(colour), r(roughness < 1 ? roughness : 1) { }

	virtual bool scatter(const ray& r_in, const hitRecord& rec, glm::vec3& attenuation, ray& scattered) const override
	{
		glm::vec3 reflected = glm::reflect(glm::normalize(r_in.getDirection()), rec.normal);
		scattered = ray(rec.p, reflected + r * glm::sphericalRand(1.0f));
		attenuation = albedo;

		return glm::dot(scattered.getDirection(), rec.normal) > 0;
	}

private:
	glm::vec3 albedo;
	float r;
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
		float cosTheta = glm::min(glm::dot(-unitDirection, rec.normal), 1.0f);
		float sinTheta = glm::sqrt(1.0 - cosTheta * cosTheta);

		bool cannot_refract = refractionRatio * sinTheta > 1.0;
		glm::vec3 direction;

		float ref = reflectance(cosTheta, refractionRatio);

		if (cannot_refract || ref > glm::linearRand(0.0f, 1.0f))
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
	static float reflectance(float cosine, float refIdx)
	{
		float r0 = (1 - refIdx) / (1 + refIdx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * glm::pow((1 - cosine), 5);
	}
};