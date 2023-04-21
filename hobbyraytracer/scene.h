#pragma once

#include <unordered_map>

#include "hittableList.h"
#include "material.h"
#include "mesh.h"	
#include "camera.h"
#include "film.h"

class Scene
{
private:
	std::unordered_map<std::string, std::shared_ptr<Material>> materials;
	HittableList objects;

	Camera camera;
	glm::vec3 background;
	Film film;

public:
	Scene() : background(0) { }

	int loadScene(std::string path);

	std::shared_ptr<HittableList> getScene(Camera& cam, glm::vec3& b, Film& f);

};