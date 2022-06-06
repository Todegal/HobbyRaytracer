#include "stdafx.h"

#include "ray.h"
#include "aabb.h"
#include "sphere.h"
#include "hittableList.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"

// IMAGE 

constexpr float ASPECT_RATIO = 16.0f / 9.0f;
constexpr int IMAGE_WIDTH = 640, IMAGE_HEIGHT = static_cast<int>((float)IMAGE_WIDTH / ASPECT_RATIO);

constexpr int CHANNELS = 3;

constexpr int SAMPLES_PER_PIXEL = 20;

constexpr int MAX_DEPTH = 50;

constexpr int NUM_THREADS = 20;

static std::vector<uint8_t> pixels;

static void writeColour(glm::vec3 colour, std::vector<uint8_t>& p)
{
	if (colour.r != colour.r) colour.r = 0.0f;
	if (colour.g != colour.g) colour.g = 0.0f;
	if (colour.b != colour.b) colour.b = 0.0f;

	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.r, 0.0f, 0.9999f)));
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.g, 0.0f, 0.9999f)));
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.b, 0.0f, 0.9999f)));
}

// RENDER

static glm::vec3 rayColour(const ray& r, const std::shared_ptr<Hittable> world, int depth = MAX_DEPTH)
{
	if (depth <= 0) // If depth limit is exceeded just return black
		return glm::vec3(0.0f);

	hitRecord rec;
	if (world->hit(r, 0.001f, INFINITY, rec))
	{
		ray scattered;
		glm::vec3 attenuation;
		if (rec.matPtr->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * rayColour(scattered, world, depth - 1);
		}

		return glm::vec3(0, 0, 0);
	}

	glm::vec3 unitDirection = glm::normalize(r.getDirection());
	float t = 0.5 * (unitDirection.y + 1.0);

	return (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
}

void threadedRender(int startScanline, int nScanlines, const std::shared_ptr<Hittable> world, const Camera camera,
	std::promise<std::vector<uint8_t>>&& promise, std::atomic<int>* progress
)
{
	std::vector<uint8_t> portion; int c = 0;
	for (int y = startScanline; y >= startScanline - nScanlines; --y)
	{
		for (int x = 0; x < IMAGE_WIDTH; ++x)
		{
			glm::vec3 pixelColour(0.0f);

			for (size_t s = 0; s < SAMPLES_PER_PIXEL; s++)
			{
				float u = ((float)x + glm::linearRand(0.0f, 1.0f)) / (IMAGE_WIDTH - 1);
				float v = ((float)y + glm::linearRand(0.0f, 1.0f)) / (IMAGE_HEIGHT - 1);

				pixelColour += rayColour(camera.getRay(u, v), world);
			}

			writeColour(glm::sqrt(pixelColour / (float)SAMPLES_PER_PIXEL), portion);
		}
		progress->store(++c);
	}

	promise.set_value(portion);
}

static void render(int nThreads, const const std::shared_ptr<Hittable> world, const Camera camera)
{
	std::vector<std::atomic<int>*> progressMonitors;
	std::vector<std::future<std::vector<uint8_t>>> futures;
	std::vector<std::thread> threads;

	for (int i = 0; i < nThreads; i++)
	{
		std::promise<std::vector<uint8_t>> p;
		futures.emplace_back(p.get_future());

		progressMonitors.push_back(new std::atomic<int>(0));

		threads.emplace_back(
			&threadedRender,
			(int)(IMAGE_HEIGHT / nThreads) * (i + 1), (int)(IMAGE_HEIGHT / nThreads), world, camera,
			std::move(p), progressMonitors[i]
		);
	}

	int scanlinesCompleted = 0;
	while (scanlinesCompleted < IMAGE_HEIGHT)
	{
		int count = 0;
		for (size_t i = 0; i < progressMonitors.size(); i++)
		{
			count += progressMonitors[i]->load();
		}
		scanlinesCompleted = count;

		std::cerr << "\rScanlines completed: " << scanlinesCompleted << "/" << IMAGE_HEIGHT << std::flush;
	}

	for (int i = nThreads - 1; i >= 0; i--)
	{
		threads[i].join();

		std::vector<uint8_t> portion = futures[i].get();

		pixels.insert(pixels.end(), portion.begin(), portion.end());
	}
}

std::shared_ptr<HittableList> randomScene(Camera& camera) {
	HittableList world;

	auto ground_material = std::make_shared<Lambertian>(glm::vec3(0.5, 0.5, 0.5));
	world.add(std::make_shared<Sphere>(glm::vec3(0, -1000, 0), 1000, ground_material));

	for (int a = -7; a < 7; a++) {
		for (int b = -7; b < 7; b++) {
			float choose_mat = glm::linearRand(0.0f, 1.0f);
			glm::vec3 center(a + glm::linearRand(0.0f, 0.9f), 0.2, b + glm::linearRand(0.0f, 0.9f));

			if (glm::length(center - glm::vec3(4, 0.2, 0)) > 0.9) {
				std::shared_ptr<Material> sphere_material;

				if (choose_mat < 0.8f) {
					// diffuse
					glm::vec3 albedo = glm::vec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f)) *
						glm::vec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));
					sphere_material = std::make_shared<Lambertian>(albedo);
					world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95f) {
					// metal
					glm::vec3 albedo = glm::vec3(glm::linearRand(0.5f, 1.0f), glm::linearRand(0.5f, 1.0f), glm::linearRand(0.5f, 1.0f));
					float fuzz = glm::linearRand(0.0f, 0.5f);
					sphere_material = std::make_shared<Metal>(albedo, fuzz);
					world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					float fuzz = glm::linearRand(0.0f, 0.05f);
					sphere_material = std::make_shared<Dielectric>(1.5, fuzz);
					world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = std::make_shared<Dielectric>(1.5, 0.05);
	world.add(std::make_shared<Sphere>(glm::vec3(0, 1, 0), 1.0, material1));

	auto material2 = std::make_shared<Lambertian>(glm::vec3(0.4, 0.2, 0.1));
	world.add(std::make_shared<Sphere>(glm::vec3(-4, 1, 0), 1.0, material2));

	auto material3 = std::make_shared<Metal>(glm::vec3(0.7, 0.6, 0.5), 0.0);
	world.add(std::make_shared<Sphere>(glm::vec3(4, 1, 0), 1.0, material3));

	auto nodeTree = std::make_shared<BVHNode>(world);

	glm::vec3 lookfrom(13, 2, 3);
	glm::vec3 lookat(0, 0, 0);
	glm::vec3 up(0, 1, 0);

	auto dist_to_focus = 10.0;
	auto aperture = 0.1;

	Camera cam(lookfrom, lookat, up, 20, ASPECT_RATIO, aperture, dist_to_focus);
	camera = cam;

	return std::make_shared<HittableList>(nodeTree);
}

std::shared_ptr<HittableList> testScene(Camera& camera)
{
	std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

	std::shared_ptr<Metal> mirror = std::make_shared<Metal>(glm::vec3(1.0f), 0.0f);

	std::shared_ptr<Sphere> sphere0 = std::make_shared<Sphere>(glm::vec3(-1, 1, 0), 1.0f, mirror);
	std::shared_ptr<Sphere> sphere1 = std::make_shared<Sphere>(glm::vec3(1, 1, 0), 1.0f, mirror);
	world->add(sphere0);
	world->add(sphere1);

	std::shared_ptr<Lambertian> groundMaterial = std::make_shared<Lambertian>(glm::vec3(0.3, 0.3, 0.3));
	world->add(std::make_shared<Sphere>(glm::vec3(0, -1000, 0), 1000, groundMaterial));

	std::shared_ptr<Metal> orbMaterial = std::make_shared<Metal>(glm::vec3(0.780, 0.019, 0.019), 0.4f);
	world->add(std::make_shared<Sphere>(glm::vec3(0, 0.25f, 0), 0.2f, orbMaterial));
	world->add(std::make_shared<Sphere>(glm::vec3(0, 1.75f, 0), 0.2f, orbMaterial));

	glm::vec3 lookFrom = { 0, 1, 5 };
	glm::vec3 lookAt = { 0, 1, 0 };
	glm::vec3 up = { 0, 1, 0 };

	auto distToFocus = glm::length(lookFrom - lookAt);
	auto aperture = 0.1;

	Camera cam(lookFrom, lookAt, up, 30, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(*world.get()));
}

int main(int argc, char** argv)
{
	// CAMERA
	Camera camera;

	// WORLD
	std::shared_ptr<HittableList> world = testScene(camera);

	// RENDER

	render(NUM_THREADS, world, camera);

	std::cerr << std::endl << "Done!" << std::endl;

	// OUTPUT IMAGE

	return stbi_write_bmp("output.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 3, pixels.data());
}