#include "stdafx.h"
#include "texture.h"

// Disable pedantic warnings
#ifdef _MSC_VER
#pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Restore warning levels.
#ifdef _MSC_VER
#pragma warning (pop)
#endif

glm::vec3 CheckeredTexture::colourValue(float u, float v, const glm::vec3& p) const
{
	float sines = glm::sin(10 * p.x) * glm::sin(10 * p.y) * glm::sin(10 * p.z);
	if (sines < 0)
	{
		return o->colourValue(u, v, p);
	}
	else
	{
		return e->colourValue(u, v, p);
	}
}

ImageTexture::ImageTexture(std::string filename)
{
	int componentsPerPixel = bytesPerPixel;

	uint8_t* d = stbi_load(
		filename.c_str(), &width, &height, &componentsPerPixel, componentsPerPixel
	);

	if (!d)
	{
		std::cout << "ERROR: Could not load texture image file: " << filename << std::endl;
		width = height = 0;
	}

	data = std::vector<uint8_t>(d, d + (width * height * bytesPerPixel));

	bytesPerScanline = bytesPerPixel * width;
}

glm::vec3 ImageTexture::colourValue(float u, float v, const glm::vec3& p) const
{
	// If we have no texture data, then return solid cyan as a debugging aid.
	if (data.size() == 0)
		return glm::vec3(0, 1, 1);

	// Clamp input texture coordinates to [0,1] x [1,0]
	u = glm::clamp(u, 0.0f, 1.0f);
	v = 1.0f - glm::clamp(v, 0.0f, 1.0f);  // Flip V to image coordinates

	int i = static_cast<int>(u * width);
	int j = static_cast<int>(v * height);

	// Clamp integer mapping, since actual coordinates should be less than 1.0
	if (i >= width)  i = width - 1;
	if (j >= height) j = height - 1;

	const float colourScale = 1.0f / 255.0f;
	int pixel = j * bytesPerScanline + i * bytesPerPixel;

	return glm::vec3(colourScale * data[pixel], colourScale * data[pixel + 1], colourScale * data[pixel + 2]);
}
