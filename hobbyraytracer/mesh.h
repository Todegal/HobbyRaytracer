#pragma once

#include "hittable.h"
#include "hittableList.h"
#include "bvh.h"
#include "triangle.h"

namespace Assimp {
	class Importer;
}

struct aiScene;

class Mesh : public Hittable
{
public:
	Mesh() { }
	Mesh(std::string filepath, std::shared_ptr<Material> matPtr);

	// Inherited via Hittable
	virtual bool hit(const ray& r, float t_min, float t_max, hitRecord& rec) const override;
	virtual bool boundingBox(AABB& outputBox) const override;

private:
	static Assimp::Importer importer;

	static bool assimpLoadFile(
		std::string path,
		std::vector<glm::vec3>& vertices,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec2>& uvs,
		std::vector<unsigned int>& indices);

	std::shared_ptr<BVHNode> tree;
	std::shared_ptr<Material> matPtr;
};