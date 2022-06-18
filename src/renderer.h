#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "component.h"
#include "shader.h"

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

struct Triangle
{
	Vertex topLeft;
	Vertex bottomRight;
	Vertex bottomLeft;
};

struct Quad
{
	Triangle left;
	Triangle right;
};

struct Batch
{
	static constexpr int MAX_TRIS = 20000;

	std::array<Triangle, MAX_TRIS> buffer;
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
	void PrepareModel(glm::vec3 size, glm::vec3 position, Quaternion q, glm::vec4 color, Model* model);

	void PrepareQuad(Quad& input, int textureID);
	void PrepareQuad(glm::vec2 size, glm::vec3 position, Quaternion q, glm::vec4 color, int textureID);
	void PrepareQuad(glm::vec2 size, glm::vec3 position, Quaternion q, glm::vec4 color, int animationID, int cellX, int cellY, int cols, int rows, bool flippedX, bool flippedY);

	Bundle DetermineBatch(int textureID);

	void Display();
	void ResetBuffers();

private:
	std::vector<Batch> batches;
	
	Shader shader;

	void Flush(const Batch& batch);
};

#endif