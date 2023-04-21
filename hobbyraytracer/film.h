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
	Film() : 
		f(), outputName("") { }
	Film(int w, int h, int samples, std::string output);

	// Inherited via Film
	static void writeColour(glm::vec3 colour, std::vector<uint8_t>& p);

	void writeFilm(std::vector<uint8_t>& pix) { pixels = pix; }
	
	film_desc getFilm() const;
	float getAspectRatio() const {
		return (float)f.dimensions.x / (float)f.dimensions.y;
	}
	
	int outputFilm();

private:
	std::vector<uint8_t> pixels;
	film_desc f;

	std::string outputName;
};