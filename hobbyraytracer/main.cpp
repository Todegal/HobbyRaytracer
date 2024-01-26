#include "hobbyraytracer.h"

#include <glm/gtx/polar_coordinates.hpp>

#include "ray.h"
#include "aabb.h"
#include "sphere.h"
#include "hittableList.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "bvh.h"
#include "aarect.h"
#include "box.h"
#include "translate.h"
#include "rotateY.h"
#include "rotateQuat.h"
#include "scale.h"
#include "constantMedium.h"
#include "triangle.h"
#include "mesh.h"
#include "film.h"
#include "scene.h"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <execution>
#include <ranges>

// SYSTEM CONSTANTS 
constexpr int MAX_DEPTH = 50; // Ray "bounce" depth

constexpr int NUM_THREADS = 12;

// RENDER

static glm::vec3 rayColour(ray r, const std::shared_ptr<Texture> background, const std::shared_ptr<Hittable> world)
{	
	glm::vec3 currentAttenuation = glm::vec3(1.0f);
	glm::vec3 result = glm::vec3(0.0f);

	for (int i = 0; i < MAX_DEPTH; ++i) {
		hitRecord rec;
		if (!world->hit(r, 0.001f, INFINITY, rec))
		{
			// Normalize ray direction
			glm::vec3 nD = glm::normalize(r.dir);

			// Convert normalized ray direction to polar coordinates
			float phi = atan2(nD.z, nD.x);
			float theta = acos(nD.y);

			// Convert polar coordinates to UV coordinates
			float u = phi / (2 * glm::pi<float>()) + 0.5;
			float v = theta / glm::pi<float>();

			result += currentAttenuation * background->colourValue(u, v, glm::vec3(0));
			break;
		}

		ray scattered;
		glm::vec3 attenuation;
		glm::vec3 emitted = rec.matPtr->emitted(rec.u, rec.v, rec.p);

		bool b = rec.matPtr->scatter(r, rec, attenuation, scattered);
		if (!b)
		{
			result += currentAttenuation * emitted;
			break;
		}

		result += currentAttenuation * emitted;
		currentAttenuation *= attenuation;
		r = scattered;
	}

	return result;
}

static void render(int nThreads, const std::shared_ptr<Texture> background, 
	const std::shared_ptr<Hittable> world, const Camera& camera, std::shared_ptr<Film>& film)
{
	const film_desc f = film->getFilm();

	std::atomic<int> scanlinesCompleted = 0;

	int numPixels = f.dimensions.x * f.dimensions.y;

	std::mutex m;

	std::vector<int> indices(numPixels);
	std::iota(indices.begin(), indices.end(), 0);

	std::atomic<int> pixelsCompleted = 0;

	std::thread reporter([&pixelsCompleted](int totalPixels) {
			while (pixelsCompleted <= totalPixels)
			{
				std::cout << "\rPixels rendered: " << pixelsCompleted << "/" << totalPixels << std::flush;
				
				if (pixelsCompleted == totalPixels)
					break;

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		},
		numPixels
	);

	std::for_each_n(std::execution::par, indices.begin(), numPixels,
		[&](int pIdx) {
			glm::vec3 pixelColour(0.0f);

			int x = pIdx % f.dimensions.x;
			int y = f.dimensions.y - pIdx / f.dimensions.x;

			for (size_t s = 0; s < f.samples; s++)
			{
				float u = ((float)x + glm::linearRand(0.0f, 1.0f)) / (f.dimensions.x - 1);
				float v = ((float)y + glm::linearRand(0.0f, 1.0f)) / (f.dimensions.y - 1);

				pixelColour += rayColour(camera.getRay(u, v), background, world);
			}

			pixelColour /= static_cast<float>(f.samples);

			film->tonemap(pixelColour);

			std::lock_guard<std::mutex> guard(m);
			film->writeColour(pixelColour, film->getPixels() + (pIdx * 3));

			pixelsCompleted++;
		}
	);

	reporter.join();

	std::cout << "\n";
}

int main(int argc, char** argv)
{
	auto start = std::chrono::high_resolution_clock::now();

	std::filesystem::path file = "teapot_scene.yaml";
	
	if (argc > 1)
	{
		file = argv[1];
	}

	Scene scene;

	if (scene.loadScene(file.string()) < 1)
		return -1;

	Camera camera = scene.getCamera();
	std::shared_ptr<Film> film = scene.getFilm();
	std::shared_ptr<Texture> background = scene.getBackground();

	std::shared_ptr<HittableList> world = scene.getScene();

	auto loadedEnd = std::chrono::high_resolution_clock::now();

	auto eMS = std::chrono::duration_cast<std::chrono::milliseconds>(loadedEnd - start);
	unsigned int iH = std::chrono::duration_cast<std::chrono::hours>(eMS).count();
	unsigned int iM = std::chrono::duration_cast<std::chrono::minutes>(eMS).count() - (iH * 60);
	long double fS = (eMS.count() / 1000.0) - (double)((iM * 60) + (iH * 3600));

	std::cout << std::endl << std::setprecision(6) << "Loaded scene: " << file.string() << "! (completed in "
		<< iH << ":" << iM << ":" << fS << ")" << std::endl;

	// RENDER

	render(NUM_THREADS, background, world, camera, film);

	// OUTPUT IMAGE

	int r = film->outputFilm();

	// OUTPUT TIMING

	auto end = std::chrono::high_resolution_clock::now();

	eMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	iH = std::chrono::duration_cast<std::chrono::hours>(eMS).count();
	iM = std::chrono::duration_cast<std::chrono::minutes>(eMS).count() - (iH * 60);
	fS = (eMS.count() / 1000.0) - (double)((iM * 60) + (iH * 3600));

	std::cout << std::endl << std::setprecision(6) << "Done! (completed in "
		<< iH << ":" << iM << ":" << fS << ")" << std::endl;

	return r;
}