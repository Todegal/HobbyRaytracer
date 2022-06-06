#include "stdafx.h"

#include "ray.h"
#include "sphere.h"
#include "hittableList.h"
#include "camera.h"
#include "material.h"

// IMAGE 

constexpr float ASPECT_RATIO = 16.0f / 9.0f;
constexpr int IMAGE_WIDTH = 1920, IMAGE_HEIGHT = static_cast<int>((float)IMAGE_WIDTH / ASPECT_RATIO);

constexpr int CHANNELS = 3;

constexpr int SAMPLES_PER_PIXEL = 500;

constexpr int MAX_DEPTH = 50;

constexpr int NUM_THREADS = 20;

static std::vector<uint8_t> pixels;

static void writeColour(glm::vec3 colour, std::vector<uint8_t>& p)
{
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.r, 0.0f, 0.9999f)));
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.g, 0.0f, 0.9999f)));
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.b, 0.0f, 0.9999f)));
}

// RENDER

static glm::vec3 rayColour(const ray& r, const Hittable& world, int depth = MAX_DEPTH)
{
	if (depth <= 0) // If depth limit is exceeded just return black
		return glm::vec3(0.0f);

	hitRecord rec;
	if (world.hit(r, 0.001f, INFINITY, rec))
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

void threadedRender(int startScanline, int nScanlines, const HittableList world, const Camera camera,
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

static void render(int nThreads, const HittableList world, const Camera camera)
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
			(int)(IMAGE_HEIGHT / nThreads) * (i+1), (int)(IMAGE_HEIGHT / nThreads), world, camera, 
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
	
	for (int i = nThreads-1; i >= 0; i--)
	{
		threads[i].join();

		std::vector<uint8_t> portion = futures[i].get();

		pixels.insert(pixels.end(), portion.begin(), portion.end());
	}
}

HittableList randomScene() {
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

	return world;
}

int main(int argc, char** argv)
{
	// WORLD
	/*HittableList world;

	std::shared_ptr<Lambertian> materialGround = std::make_shared<Lambertian>(glm::vec3(0.8, 0.8, 0.0));
	std::shared_ptr<Lambertian> materialCenter = std::make_shared<Lambertian>(glm::vec3(0.1, 0.2, 0.5));

	std::shared_ptr<Dielectric> materialLeft = std::make_shared<Dielectric>(1.5f, 0.01f);

	std::shared_ptr<Metal> materialRight = std::make_shared<Metal>(glm::vec3(0.8, 0.6, 0.2), 0.1);

	world.add(std::make_shared<Sphere>(glm::vec3(0.0, -100.5, -1), 100.0, materialGround));
	world.add(std::make_shared<Sphere>(glm::vec3(0.0, 0.0, -1), 0.5, materialCenter));
	world.add(std::make_shared<Sphere>(glm::vec3(-1.0, 0.0, -1), 0.5, materialLeft));
	world.add(std::make_shared<Sphere>(glm::vec3(-1.0, 0.0, -1), -0.4, materialLeft));
	world.add(std::make_shared<Sphere>(glm::vec3(1.0, 0.0, -1), 0.5, materialRight));*/

	// CAMERA

	glm::vec3 lookFrom = { 13, 2, 3 };
	glm::vec3 lookAt = { 0, 0, 0 };
	glm::vec3 up = { 0, 1, 0 };

	Camera camera(
		lookFrom, lookAt, up,
		20.0f, ASPECT_RATIO,
		0.1f, 10.0f
	);

	// RENDER

	render(NUM_THREADS, randomScene(), camera);

	std::cerr << std::endl << "Done!" << std::endl;

	// OUTPUT IMAGE

	return stbi_write_bmp("output.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 3, pixels.data());
}