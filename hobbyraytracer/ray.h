#pragma once

struct ray
{
public:
	ray() {}
	ray(const glm::vec3& origin, const glm::vec3& direction)
		: o(origin), dir(direction) { }

	glm::vec3 at(float t) const { return o + (t * dir); }

	glm::vec3 o, dir;
};