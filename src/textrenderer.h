#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <map>
#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "texture.h"
#include <ft2build.h>
#include FT_FREETYPE_H

enum class TextAlignment { left, center, right };

struct Character
{
	glm::ivec2 size;
	glm::ivec2 bearing;
	FT_Pos advance;
	float textureX;
};

class TextRenderer
{
public:
	GLuint						textureAtlas;
	GLuint						mapAtlas;

	int							texWidth;
	int							texHeight;

	int							atlasWidth;
	int							atlasHeight;

	std::map<char, Character>	characters;

	void RenderText(std::string text, float x, float y, float scaleX, float scaleY, glm::vec4 color);
	glm::vec2 CalculateAlignment(std::string text, float scaleX, float scaleY, TextAlignment alignment);

	TextRenderer(const std::string& fontpath, FT_UInt pixelSize);
};

#endif