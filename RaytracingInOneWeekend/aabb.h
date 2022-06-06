#pragma once

#include <math.h>

class AABB
{
public:
	AABB() { }
	AABB(const glm::vec3& minimum, const glm::vec3& maximum) : min(minimum), max(maximum) { }

	glm::vec3 getMin() const { return min; }
	glm::vec3 getMax() const { return max; }

	bool hit(const ray& r, float t_min, float t_max) const
	{
		for (int a = 0; a < 3; a++) {
			auto t0 = fmin((min[a] - r.getOrigin()[a]) / r.getDirection()[a],
				(max[a] - r.getOrigin()[a]) / r.getDirection()[a]);
			auto t1 = fmax((min[a] - r.getOrigin()[a]) / r.getDirection()[a],
				(max[a] - r.getOrigin()[a]) / r.getDirection()[a]);
			t_min = fmax(t0, t_min);
			t_max = fmin(t1, t_max);
			if (t_max <= t_min)
				return false;
		}
		return true;
	}

	static AABB surroundingBox(AABB box0, AABB box1)
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