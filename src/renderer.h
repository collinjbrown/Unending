#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "component.h"
#include "shader.h"
#include "texture.h"
#include "animation.h"

struct Vertex
{
	float x;
	float y;
	float z;

	float r;
	float g;
	float b;
	float a;

	float s;
	float t;

	float texture;
};

struct Quad
{
	Vertex topRight;
	Vertex bottomRight;
	Vertex bottomLeft;
	Vertex topLeft;
};

struct Batch
{
	static constexpr int MAX_QUADS = 10000;

	std::array<Quad, MAX_QUADS> buffer;
	int index = 0;
};

struct Bundle
{
	int batch;
	float location;
};

class Renderer
{
public:
	static constexpr int MAX_TEXTURES_PER_BATCH = 32;

	GLuint VAO;
	GLuint VBO;

	std::vector<GLuint> textureIDs;
	std::vector<GLuint> texturesUsed;

	float whiteTextureIndex;
	GLuint whiteTextureID;

	Renderer(GLuint whiteTexture);

	void PrepareCube(glm::vec3 size, glm::vec3 position, Quaternion q, glm::vec4 color, int textureID);

	void PrepareQuad(Quad& input, int textureID);
	void PrepareQuad(glm::vec2 size, glm::vec3 position, Quaternion q, glm::vec4 color, int textureID);
	void PrepareQuad(glm::vec2 size, glm::vec3 position, Quaternion q, glm::vec4 color, int animationID, int cellX, int cellY, int cols, int rows, bool flippedX, bool flippedY);
	void PrepareCube(int rows, int cols, int aisles, glm::vec3 size, glm::vec3 position, Quaternion q, glm::vec4 color, int textureID);

	Bundle DetermineBatch(int textureID);

	void Display();
	void ResetBuffers();

private:
	std::vector<Batch> batches;
	
	Shader shader;

	void Flush(const Batch& batch);
};

#endif