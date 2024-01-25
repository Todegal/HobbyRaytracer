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
	std::shared_ptr<Film> film;

public:
	Scene() : isLoaded(false) { }

	int loadScene(std::string path);

	std::shared_ptr<HittableList> getScene();

	const Camera& getCamera() { assert(isLoaded); return camera; }
	const std::shared_ptr<Texture>& getBackground() { assert(isLoaded); return background; }
	const std::shared_ptr<Film>& getFilm() { assert(isLoaded); return film; }

private:
	template<typename T>
	T getProperty(std::string name, YAML::Node node);

	bool isLoaded;
};