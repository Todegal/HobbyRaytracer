#pragma once

#include <glm/vec3.hpp>

#include <algorithm>

class AABB
{
public:
	AABB() : min(0.0f), max(0.0f) { }
	AABB(const glm::vec3& minimum, const glm::vec3& maximum) : min(minimum), max(maximum) { }
	static AABB createFromDimensions(const glm::vec3& center, const glm::vec3& dimensions)
	{
		glm::vec3 min = center - dimensions / 2.0f;
		glm::vec3 max = center + dimensions / 2.0f;

		return AABB(min, max);
	}

	glm::vec3 getMin() const { return min; }
	glm::vec3 getMax() const { return max; }

	void setMin(const glm::vec3& v) { min = v; }
	void setMax(const glm::vec3& v) { max = v; }

	bool hit(const ray& r, float t_min, float t_max) const
	{
		for (int a = 0; a < 3; a++) {
			auto t0 = std::min((min[a] - r.getOrigin()[a]) / r.getDirection()[a],
				(max[a] - r.getOrigin()[a]) / r.getDirection()[a]);
			auto t1 = std::max((min[a] - r.getOrigin()[a]) / r.getDirection()[a],
				(max[a] - r.getOrigin()[a]) / r.getDirection()[a]);
			t_min = std::max(t0, t_min);
			t_max = std::min(t1, t_max);
			if (t_max <= t_min)
				return false;
		}
		return true;
	}

	static AABB surroundingBox(const AABB& box0, const AABB& box1)
	{
		glm::vec3 small(
			glm::min(box0.getMin().x, box1.getMin().x),
			glm::min(box0.getMin().y, box1.getMin().y),
			glm::min(box0.getMin().z, box1.getMin().z)
		);

		glm::vec3 big(
			glm::max(box0.getMax().x, box1.getMax().x),
			glm::max(box0.getMax().y, box1.getMax().y),
			glm::max(box0.getMax().z, box1.getMax().z)
		);

		return AABB(small, big);
	}

private:
	glm::vec3 min, max;
};