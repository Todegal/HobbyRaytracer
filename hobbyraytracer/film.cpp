#include "hobbyraytracer.h"

#include "film.h"

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

Film::Film(int w, int h, int samples, std::string output)
{
	outputName = output;
	f = { glm::ivec2(w, h), samples };
}

void Film::writeColour(glm::vec3 colour, std::vector<uint8_t>& p)
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

film_desc Film::getFilm() const
{
	return f;
}

int Film::outputFilm()
{
	if (ends_with(outputName, ".png"))
	{
		return stbi_write_png(outputName.c_str(), f.dimensions.x, f.dimensions.y, 3, pixels.data(), f.dimensions.x * 3);
	}

	if (ends_with(outputName, ".tga"))
	{
		return stbi_write_tga(outputName.c_str(), f.dimensions.x, f.dimensions.y, 3, pixels.data());
	}

	if (!ends_with(outputName, ".bmp"))
	{
		std::cout << "File type not supported, generating bitmap!" << std::endl;
	}

	return stbi_write_bmp(outputName.c_str(), f.dimensions.x, f.dimensions.y, 3, pixels.data());
}
