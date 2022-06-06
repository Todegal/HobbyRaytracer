#pragma once

class Camera
{
public:
	Camera() { }
	Camera(
		glm::vec3 lookFrom, glm::vec3 lookAt, glm::vec3 up,
		float vfov, float aspectRatio,
		float aperture, float focusDistance
	)
	{
		float theta = glm::radians(vfov);
		float h = glm::tan(theta / 2);
		float viewportHeight = 2.0f * h;
		float viewportWidth = aspectRatio * viewportHeight;

		w = glm::normalize(lookFrom - lookAt);
		u = glm::normalize(glm::cross(up, w));
		v = glm::cross(w, u);

		origin = lookFrom;
		horizontal = focusDistance * viewportWidth * u;
		vertical = focusDistance * viewportHeight * v;
		lowerLeftCorner = origin - horizontal / 2.0f - vertical / 2.0f - focusDistance * w;

		lensRadius = aperture / 2.0f;
	}

	ray getRay(float s, float t) const
	{
		glm::vec2 rd = glm::circularRand(lensRadius);
		glm::vec3 offset = u * rd.x + v * rd.y;	

		return ray(origin + offset, lowerLeftCorner + s * horizontal + t * vertical - origin - offset);
	}

private:
	glm::vec3 origin;
	glm::vec3 lowerLeftCorner;
	glm::vec3 horizontal;
	glm::vec3 vertical;

	glm::vec3 w, u, v;

	float lensRadius;
};