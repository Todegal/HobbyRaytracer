#include "hobbyraytracer.h"

// STB
#ifdef _MSC_VER
#pragma warning (push, 0)
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

// Restore warning levels.
#ifdef _MSC_VER
#pragma warning (pop)
#endif

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
#include "constantMedium.h"
#include "triangle.h"
#include "mesh.h"

// IMAGE CONSTANTS 

constexpr float ASPECT_RATIO = 1.0f;
constexpr int IMAGE_WIDTH = 640, IMAGE_HEIGHT = static_cast<int>((float)IMAGE_WIDTH / ASPECT_RATIO);

constexpr int CHANNELS = 3; // RGB

constexpr int SAMPLES_PER_PIXEL = 100; // Number of rays shot through each pixel (higher = better quality but worse performance)

constexpr int MAX_DEPTH = 50; // Ray "bounce" depth

constexpr int NUM_THREADS = 10;

static std::vector<uint8_t> pixels;

static void writeColour(glm::vec3 colour, std::vector<uint8_t>& p)
{
	// Do not permit NaN!
	if (colour.r != colour.r) colour.r = 0.0f;
	if (colour.g != colour.g) colour.g = 0.0f;
	if (colour.b != colour.b) colour.b = 0.0f;

	// Tonemap https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	
	colour = glm::clamp((colour * (a * colour + b)) / (colour * (c * colour + d) + e), glm::vec3(0), glm::vec3(1));

	// Gamma Correction
	colour = glm::sqrt(colour);

	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.r, 0.0f, 0.9999f)));
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.g, 0.0f, 0.9999f)));
	p.push_back(static_cast<uint8_t>(256 * glm::clamp(colour.b, 0.0f, 0.9999f)));
}

// RENDER

static glm::vec3 rayColour(const ray& r, const glm::vec3& background, const std::shared_ptr<Hittable> world, int depth = MAX_DEPTH)
{
	if (depth <= 0) // If depth limit is exceeded just return black
		return glm::vec3(0.0f);

	hitRecord rec;
	if (!world->hit(r, 0.001f, INFINITY, rec))
	{
		return background;
	}

	ray scattered;
	glm::vec3 attenuation;
	glm::vec3 emitted = rec.matPtr->emitted(rec.u, rec.v, rec.p);

	bool b = !rec.matPtr->scatter(r, rec, attenuation, scattered);
	if (b)
	{
		return emitted;
	}

	return emitted + attenuation * rayColour(scattered, background, world, depth - 1);
}

void threadedRender(int startScanline, int nScanlines, const glm::vec3& background, const std::shared_ptr<Hittable> world, const Camera camera,
	std::promise<std::vector<uint8_t>>&& promise, std::shared_ptr<std::atomic<int>> progress
)
{
	std::vector<uint8_t> portion; int c = 0;
	for (int y = startScanline - 1; y >= startScanline - nScanlines; --y)
	{
		for (int x = 0; x < IMAGE_WIDTH; ++x)
		{
			glm::vec3 pixelColour(0.0f);

			for (size_t s = 0; s < SAMPLES_PER_PIXEL; s++)
			{
				float u = ((float)x + glm::linearRand(0.0f, 1.0f)) / (IMAGE_WIDTH - 1);
				float v = ((float)y + glm::linearRand(0.0f, 1.0f)) / (IMAGE_HEIGHT - 1);

				pixelColour += rayColour(camera.getRay(u, v), background, world);
			}

			writeColour(pixelColour / (float)SAMPLES_PER_PIXEL, portion);
		}
		progress->store(++c);
	}

	promise.set_value(portion);
}

static void render(int nThreads, const glm::vec3& background, const std::shared_ptr<Hittable> world, const Camera camera)
{
	std::vector<std::shared_ptr<std::atomic<int>>> progressMonitors;
	std::vector<std::future<std::vector<uint8_t>>> futures;
	std::vector<std::thread> threads;

	for (int i = 0; i < nThreads; i++)
	{
		std::promise<std::vector<uint8_t>> p;
		futures.emplace_back(p.get_future());

		progressMonitors.push_back(std::make_shared<std::atomic<int>>(0));

		threads.emplace_back(
			&threadedRender,
			(int)(IMAGE_HEIGHT / nThreads) * (i + 1), (int)ceil((double)IMAGE_HEIGHT / (double)nThreads), background, world, camera,
			std::move(p), progressMonitors[i]
		);
	}

	int scanlinesCompleted = 0;
	while (scanlinesCompleted < IMAGE_HEIGHT)
	{
		scanlinesCompleted = 0;
		for (size_t i = 0; i < progressMonitors.size(); i++)
		{
			scanlinesCompleted += progressMonitors[i]->load();
		}

		std::cout << "\rScanlines completed: " << scanlinesCompleted << "/" << IMAGE_HEIGHT << std::flush;

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	for (int i = nThreads - 1; i >= 0; i--)
	{
		threads[i].join();

		std::vector<uint8_t> portion = futures[i].get();

		pixels.insert(pixels.end(), portion.begin(), portion.end());
	}
}

std::shared_ptr<HittableList> randomScene(Camera& camera, glm::vec3& background) {
	HittableList world;

	auto ground_material =
		std::make_shared<Metal>(std::make_shared<CheckeredTexture>(glm::vec3(0.2f, 0.3f, 0.1f), glm::vec3(0.9f, 0.9f, 0.9f)), 0.01f);
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

	auto material1 = std::make_shared<Dielectric>(1.5, 0.0f);
	world.add(std::make_shared<Sphere>(glm::vec3(0, 1, 0), 1.0, material1));

	auto material2 = std::make_shared<Lambertian>(glm::vec3(0.4, 0.2, 0.1));
	world.add(std::make_shared<Sphere>(glm::vec3(-4, 1, 0), 1.0, material2));

	auto material3 = std::make_shared<Metal>(glm::vec3(0.7, 0.6, 0.5), 0.0);
	world.add(std::make_shared<Sphere>(glm::vec3(4, 1, 0), 1.0, material3));

	auto nodeTree = std::make_shared<BVHNode>(world);

	glm::vec3 lookfrom(13, 2, 3);
	glm::vec3 lookat(0, 0, 0);
	glm::vec3 up(0, 1, 0);

	float dist_to_focus = 10.0f;
	float aperture = 0.1f;

	Camera cam(lookfrom, lookat, up, 20, ASPECT_RATIO, aperture, dist_to_focus);
	camera = cam;

	background = glm::vec3(0.70f, 0.80f, 1.00f);

	return std::make_shared<HittableList>(nodeTree);
}

std::shared_ptr<HittableList> reflectionScene(Camera& camera, glm::vec3& background)
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

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = 0.1f;

	Camera cam(lookFrom, lookAt, up, 30, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.70f, 0.80f, 1.00f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(*world.get()));
}

std::shared_ptr<HittableList> checkeredScene(Camera& camera, glm::vec3& background)
{
	std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

	std::shared_ptr<CheckeredTexture> checkeredTexture =
		std::make_shared<CheckeredTexture>(glm::vec3(0.2f, 0.3f, 0.1f), glm::vec3(0.9f, 0.9f, 0.9f));

	world->add(std::make_shared<Sphere>(glm::vec3(0, -10, 0), 10.0f, std::make_shared<Lambertian>(checkeredTexture)));
	world->add(std::make_shared<Sphere>(glm::vec3(0, 10, 0), 10.0f, std::make_shared<Lambertian>(checkeredTexture)));

	glm::vec3 lookFrom = { 13, 2, 3 };
	glm::vec3 lookAt = { 0, 0, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = 0.0f;

	Camera cam(lookFrom, lookAt, up, 20, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.70f, 0.80f, 1.00f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(*world.get()));
}

std::shared_ptr<HittableList> earthScene(Camera& camera, glm::vec3& background)
{
	std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

	auto ground_material =
		std::make_shared<Metal>(std::make_shared<CheckeredTexture>(glm::vec3(0.2f, 0.3f, 0.1f), glm::vec3(0.9f, 0.9f, 0.9f)), 0.01f);
	world->add(std::make_shared<Sphere>(glm::vec3(0, -1000, 0), 1000, ground_material));

	std::shared_ptr<ImageTexture> earthmapTexture = std::make_shared<ImageTexture>("earthmap.jpg");
	std::shared_ptr<ImageTexture> earthmapMetallness = std::make_shared<ImageTexture>("earthmapMetallness.jpg");
	std::shared_ptr<PBR> earthmapMat = std::make_shared<PBR>(earthmapTexture, earthmapMetallness, 0.2f);

	world->add(std::make_shared<Sphere>(glm::vec3(0, 2, 0), 2, earthmapMat));

	glm::vec3 lookFrom = { 0, 2, 15 };
	glm::vec3 lookAt = { 0, 2, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = .1f;

	Camera cam(lookFrom, lookAt, up, 45, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.70f, 0.80f, 1.00f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(*world.get()));
}

std::shared_ptr<HittableList> lightScene(Camera& camera, glm::vec3& background)
{
	std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

	auto ground_material =
		std::make_shared<Metal>(std::make_shared<CheckeredTexture>(glm::vec3(0.909f, 0.007f, 0.121f), glm::vec3(0.9f, 0.9f, 0.9f)), .2f);
	world->add(std::make_shared<Sphere>(glm::vec3(0, -1000, 0), 1000, ground_material));

	std::shared_ptr<ImageTexture> earthmapTexture = std::make_shared<ImageTexture>("earthmap.jpg");
	std::shared_ptr<ImageTexture> earthmapMetallness = std::make_shared<ImageTexture>("earthmapMetallness.jpg");
	std::shared_ptr<PBR> earthmapMat = std::make_shared<PBR>(earthmapTexture, earthmapMetallness, 0.2f);

	//world->add(std::make_shared<Sphere>(glm::vec3(0, 1, 0), 1, earthmapMat));

	auto difflight = std::make_shared<DiffuseLight>(glm::vec3(1.0f), 0.5f);
	world->add(std::make_shared<Sphere>(glm::vec3(0, 1, 0), 1.0f, difflight));

	glm::vec3 lookFrom = { 10, 1, 5.5 };
	glm::vec3 lookAt = { 0, 1, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = .3f;

	Camera cam(lookFrom, lookAt, up, 15.0f, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.1f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(*world.get()));
}

std::shared_ptr<HittableList> triangleScene(Camera& camera, glm::vec3& background)
{
	std::shared_ptr<HittableList> world = std::make_shared<HittableList>();

	std::shared_ptr<Triangle> t = std::make_shared<Triangle>(
		glm::vec3(-1.0f, -0.5f, -0.1f),
		glm::vec3(0.0f, -0.5f, 0.0f),
		glm::vec3(-0.5f, 0.5f, 0.0f),
		std::make_shared<UVTest>()
		);

	std::shared_ptr<ITriangle> it = std::make_shared<ITriangle>(
		std::array<glm::vec3, 3>({
			glm::vec3(0.0f, -0.5f, 0.0f),
			glm::vec3(1.0f, -0.5f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.0f)
		}),
		std::array<glm::vec3, 3>({
			glm::vec3(0, 0, 1),
			glm::vec3(0, 0, 1),
			glm::vec3(0, 0, 1)
		}),
		std::array<glm::vec2, 3>({
			glm::vec2(0, 0),
			glm::vec2(2, 0),
			glm::vec2(0, 2)
		}), std::make_shared<UVTest>()
	);

	world->add(t);
	world->add(it);

	glm::vec3 lookFrom = { 0, 0, 2.5f };
	glm::vec3 lookAt = { 0, 0, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = .3f;

	Camera cam(lookFrom, lookAt, up, 45.0f, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.1f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(*world.get()));
	//return world;
}

std::shared_ptr<HittableList> cornellTeapotScene(Camera& camera, glm::vec3& background)
{
	HittableList world;

	std::shared_ptr<Lambertian> red = std::make_shared<Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
	std::shared_ptr<Lambertian> white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
	std::shared_ptr<Lambertian> green = std::make_shared<Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
	std::shared_ptr<DiffuseLight> light = std::make_shared<DiffuseLight>(glm::vec3(0.93725f, 0.75294f, 0.43922f), 9.0f);

	world.add(std::make_shared<YZRect>(0, 5, -2.5f, 2.5f, 2.5f, red)); // RIGHT SIDE
	world.add(std::make_shared<YZRect>(0, 5, -2.5f, 2.5f, -2.5f, green)); // LEFT SIDE
	world.add(std::make_shared<XZRect>(-1, 1, -1, 1, 4.99f, light)); // LIGHT
	world.add(std::make_shared<XZRect>(-2.5f, 2.5f, -2.5f, 2.5f, 0, white)); // FLOOR
	world.add(std::make_shared<XZRect>(-2.5f, 2.5f, -2.5f, 2.5f, 5, white)); // CEILING
	world.add(std::make_shared<XYRect>(-2.5f, 2.5f, 0, 5, -2.5f, white)); // BACK

	std::shared_ptr<Hittable> teapot =
		std::make_shared<Mesh>("teapot.obj", white);
		//std::make_shared<Sphere>(glm::vec3(0, 1, 0), 1, white);
	//teapot = std::make_shared<Translate>(teapot, glm::vec3(-1, 0.0f, 1.7f));
	//teapot = std::make_shared<Scale>(teapot, glm::vec3(.4f));
	world.add(teapot);

	glm::vec3 lookFrom = { 0, 2.5f, 9.35f };
	glm::vec3 lookAt = { 0, 2.5f, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = 0.0001f;

	Camera cam(lookFrom, lookAt, up, 40.0f, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.0f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(world));
}

std::shared_ptr<HittableList> cornellBoxScene(Camera& camera, glm::vec3& background)
{
	HittableList world;

	std::shared_ptr<Lambertian> red = std::make_shared<Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
	std::shared_ptr<Lambertian> white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
	std::shared_ptr<Lambertian> green = std::make_shared<Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
	std::shared_ptr<DiffuseLight> light = std::make_shared<DiffuseLight>(glm::vec3(0.93725f, 0.75294f, 0.43922f), 9.0f);

	world.add(std::make_shared<YZRect>(0, 5, -2.5f, 2.5f, 2.5f, red)); // RIGHT SIDE
	world.add(std::make_shared<YZRect>(0, 5, -2.5f, 2.5f, -2.5f, green)); // LEFT SIDE
	world.add(std::make_shared<XZRect>(-1, 1, -1, 1, 4.99f, light)); // LIGHT
	world.add(std::make_shared<XZRect>(-2.5f, 2.5f, -2.5f, 2.5f, 0, white)); // FLOOR
	world.add(std::make_shared<XZRect>(-2.5f, 2.5f, -2.5f, 2.5f, 5, white)); // CEILING
	world.add(std::make_shared<XYRect>(-2.5f, 2.5f, 0, 5, -2.5f, white)); // BACK

	std::shared_ptr<Hittable> box1 = std::make_shared<Box>(glm::vec3(0.0f, 1.4f, 0.0f), glm::vec3(1.4f, 2.8f, 1.4f), white);
	box1 = std::make_shared<RotateY>(box1, 195.0f);
	box1 = std::make_shared<Translate>(box1, glm::vec3(-0.7f, 0.0f, -0.7f));
	world.add(box1);

	std::shared_ptr<Hittable> box2 = std::make_shared<Box>(glm::vec3(0.0f, 0.7f, 0.0f), glm::vec3(1.4f), white);
	box2 = std::make_shared<RotateY>(box2, -18.0f);
	box2 = std::make_shared<Translate>(box2, glm::vec3(0.9f, 0.0f, 1));
	world.add(box2);

	std::shared_ptr<Hittable> glassSphere =
		std::make_shared<Sphere>(glm::vec3(-1, 0.75f, 1.7f), 0.75f, std::make_shared<Dielectric>(1.3f, 0.0f));
	world.add(glassSphere);

	glm::vec3 lookFrom = { 0, 2.5f, 9.35f };
	glm::vec3 lookAt = { 0, 2.5f, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = 0.0001f;

	Camera cam(lookFrom, lookAt, up, 40.0f, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(world));
}

std::shared_ptr<HittableList> cornellBoxSmokeScene(Camera& camera, glm::vec3& background)
{
	HittableList world;

	std::shared_ptr<Lambertian> red = std::make_shared<Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
	std::shared_ptr<Lambertian> white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
	std::shared_ptr<Lambertian> green = std::make_shared<Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
	std::shared_ptr<DiffuseLight> light = std::make_shared<DiffuseLight>(glm::vec3(0.93725f, 0.75294f, 0.43922f), 9.0f);

	world.add(std::make_shared<YZRect>(0, 5, -2.5f, 2.5f, 2.5f, red)); // RIGHT SIDE
	world.add(std::make_shared<YZRect>(0, 5, -2.5f, 2.5f, -2.5f, green)); // LEFT SIDE
	world.add(std::make_shared<XZRect>(-1, 1, -1, 1, 4.99f, light)); // LIGHT
	world.add(std::make_shared<XZRect>(-2.5f, 2.5f, -2.5f, 2.5f, 0, white)); // FLOOR
	world.add(std::make_shared<XZRect>(-2.5f, 2.5f, -2.5f, 2.5f, 5, white)); // CEILING
	world.add(std::make_shared<XYRect>(-2.5f, 2.5f, 0, 5, -2.5f, white)); // BACK

	std::shared_ptr<Hittable> box1 = std::make_shared<Box>(glm::vec3(0.0f, 1.4f, 0.0f), glm::vec3(1.4f, 2.8f, 1.4f), white);
	box1 = std::make_shared<RotateY>(box1, 195.0f);
	box1 = std::make_shared<Translate>(box1, glm::vec3(-0.7f, 0.0f, -0.7f));

	std::shared_ptr<Hittable> box2 = std::make_shared<Box>(glm::vec3(0.0f, 0.7f, 0.0f), glm::vec3(1.4f), white);
	box2 = std::make_shared<RotateY>(box2, -18.0f);
	box2 = std::make_shared<Translate>(box2, glm::vec3(0.9f, 0.0f, 1));

	world.add(std::make_shared<ConstantMedium>(box1, 1, glm::vec3(0)));
	world.add(std::make_shared<ConstantMedium>(box2, 1, glm::vec3(1)));

	glm::vec3 lookFrom = { 0, 2.5f, 9.355f };
	glm::vec3 lookAt = { 0, 2.5f, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = 0.0001f;

	Camera cam(lookFrom, lookAt, up, 40.0f, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	background = glm::vec3(0.0f);

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(world));
}

std::shared_ptr<HittableList> finalScene(Camera& camera, glm::vec3& background) {
	HittableList boxes1;
	auto ground = std::make_shared<Lambertian>(glm::vec3(0.48f, 0.83f, 0.53f));

	const int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++) 
	{
		for (int j = 0; j < boxes_per_side; j++) 
		{
			float w = 100.0f;
			float x0 = -1000.0f + i * w;
			float z0 = -1000.0f + j * w;
			float y0 = 0.0f;
			float x1 = x0 + w;
			float y1 = glm::linearRand(1.0f, 101.0f);
			float z1 = z0 + w;

			boxes1.add(Box::minMaxBox(glm::vec3(x0, y0, z0), glm::vec3(x1, y1, z1), ground));
		}
	}

	HittableList objects;

	objects.add(std::make_shared<BVHNode>(boxes1));

	auto light = std::make_shared<DiffuseLight>(glm::vec3(1), 7.0f);
	objects.add(std::make_shared<XZRect>(123, 423, 147, 412, 554, light));

	auto moving_sphere_material = std::make_shared<Metal>(glm::vec3(0.7f, 0.3f, 0.1f), 0.1f);
	objects.add(std::make_shared<Sphere>(glm::vec3(400, 400, 200), 50, moving_sphere_material));

	objects.add(std::make_shared<Sphere>(glm::vec3(260, 150, 45), 50, std::make_shared<Dielectric>(1.5, 0)));
	objects.add(std::make_shared<Sphere>(
		glm::vec3(0, 150, 145), 50, std::make_shared<Metal>(glm::vec3(0.8f, 0.8f, 0.9f), 1.0f)
		));

	auto boundary = std::make_shared<Sphere>(glm::vec3(360, 150, 145), 70, std::make_shared<Dielectric>(1.5, 0));
	objects.add(boundary);
	objects.add(std::make_shared<ConstantMedium>(boundary, 0.2f, glm::vec3(0.2f, 0.4f, 0.9f)));
	boundary = std::make_shared<Sphere>(glm::vec3(0), 5000, std::make_shared<Dielectric>(1.5f, 0));
	objects.add(std::make_shared<ConstantMedium>(boundary, 0.0001f, glm::vec3(1, 1, 1)));

	auto emat = std::make_shared<Lambertian>(std::make_shared<ImageTexture>("earthmap.jpg"));
	objects.add(std::make_shared<Sphere>(glm::vec3(400, 200, 400), 100, emat));
	auto pertext = std::make_shared<SolidColourTexture>(glm::vec3(0.1f));
	objects.add(std::make_shared<Sphere>(glm::vec3(220, 280, 300), 80, std::make_shared<Lambertian>(pertext)));

	HittableList boxes2;
	auto white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
	int ns = 1000;
	for (int j = 0; j < ns; j++) 
	{
		boxes2.add(std::make_shared<Sphere>(
			glm::vec3(glm::linearRand(0, 165), glm::linearRand(0, 165), glm::linearRand(0, 165)), 10, white)
		);
	}

	objects.add(std::make_shared<Translate>(
		std::make_shared<RotateY>(std::make_shared<BVHNode>(boxes2), 15),
		glm::vec3(-100, 270, 395)
	));

	background = glm::vec3(0);

	glm::vec3 lookFrom = { 478, 278, -600 };
	glm::vec3 lookAt = { 278, 278, 0 };
	glm::vec3 up = { 0, 1, 0 };

	float distToFocus = glm::length(lookFrom - lookAt);
	float aperture = 1;

	Camera cam(lookFrom, lookAt, up, 40.0f, ASPECT_RATIO, aperture, distToFocus);
	camera = cam;

	return std::make_shared<HittableList>(std::make_shared<BVHNode>(objects));
}

int main(int argc, char** argv)
{
	auto start = std::chrono::high_resolution_clock::now();

	// CAMERA
	Camera camera;

	// WORLD
	glm::vec3 background;
	std::shared_ptr<HittableList> world = finalScene(camera, background);

	// RENDER

	render(NUM_THREADS, background, world, camera);

	// OUTPUT IMAGE

	int r = stbi_write_png("output.png", IMAGE_WIDTH, IMAGE_HEIGHT, 3, pixels.data(), IMAGE_WIDTH * 3);

	// OUTPUT TIMING

	auto end = std::chrono::high_resolution_clock::now();
	auto eMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	unsigned int iH = std::chrono::duration_cast<std::chrono::hours>(eMS).count();

	unsigned int iM = std::chrono::duration_cast<std::chrono::minutes>(eMS).count() - (iH * 60);

	long double fS = (eMS.count() / 1000.0) - (double)((iM * 60) + (iH * 3600));

	std::cout << std::endl << std::setprecision(6) << "Done! (completed in "
		<< iH << ":" << iM << ":" << fS << ")" << std::endl;

	std::cout << "Press Enter to Continue...";
	std::cin.ignore();

	return r;
}