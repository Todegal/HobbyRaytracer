#pragma once

class Texture
{
public:
	virtual glm::vec3 colourValue(float u, float v, const glm::vec3& p) const = 0;
};

class SolidColourTexture : public Texture
{
public:
	SolidColourTexture() { }
	SolidColourTexture(glm::vec3 colour) : c(colour) { }

	SolidColourTexture(float r, float g, float b) : c(glm::vec3(r, g, b)) { }

	virtual glm::vec3 colourValue(float u, float v, const glm::vec3& p) const override
	{
		return c;
	}

private:
	glm::vec3 c; // Single colour value
};

class CheckeredTexture : public Texture
{
public:
	CheckeredTexture() { }
	CheckeredTexture(std::shared_ptr<Texture> even, std::shared_ptr<Texture> odd) 
		: e(even), o(odd) { }

	CheckeredTexture(glm::vec3 colour1, glm::vec3 colour2)
		: e(std::make_shared<SolidColourTexture>(colour1)), o(std::make_shared<SolidColourTexture>(colour2)) { }

	virtual glm::vec3 colourValue(float u, float v, const glm::vec3& p) const override;

private:
	std::shared_ptr<Texture> e, o; // (e)ven texture, (o)dd texture
};

class ImageTexture : public Texture
{
public: 
	const static int bytesPerPixel = 3;
	
public:
	ImageTexture()
		: data(0), width(0), height(0), bytesPerScanline(0) { }

	ImageTexture(std::string filename);

	virtual glm::vec3 colourValue(float u, float v, const glm::vec3& p) const override;

private:
	std::vector<uint8_t> data;
	int width, height;
	int bytesPerScanline;
};