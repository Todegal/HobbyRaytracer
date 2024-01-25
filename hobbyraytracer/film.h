#pragma once

struct film_desc
{
	glm::ivec2 dimensions;
	int samples;
};

// Standard film with aces tonemapping and Gamma correction
class Film
{
public:
	Film(int w, int h, int samples, std::string output);

	// Inherited via Film
	const glm::vec3& tonemap(glm::vec3& colour);
	static void writeColour(glm::vec3 colour, std::vector<uint8_t>::iterator p);

	film_desc getFilm() const;
	float getAspectRatio() const {
		return (float)f.dimensions.x / (float)f.dimensions.y;
	}
	
	const std::vector<uint8_t>::iterator getPixels() { return pixels.begin(); }

	int outputFilm();

private:
	std::vector<uint8_t> pixels;
	film_desc f;

	std::string outputName;
};