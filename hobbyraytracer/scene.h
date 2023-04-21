#pragma once

#include <unordered_map>

#include "hittableList.h"
#include "material.h"
#include "mesh.h"	
#include "camera.h"
#include "film.h"

#include <yaml-cpp/yaml.h>

class Scene
{
private:
	std::unordered_map<std::string, std::shared_ptr<Material>> materials;
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	HittableList objects;

	Camera camera;
	std::shared_ptr<Texture> background;
	Film film;

public:
	Scene() { }

	int loadScene(std::string path);

	std::shared_ptr<HittableList> getScene(Camera& cam, std::shared_ptr<Texture>& b, Film& f);

private:
	template<typename T>
	T getProperty(std::string name, YAML::Node node);
};