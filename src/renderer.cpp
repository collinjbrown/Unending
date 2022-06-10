#include "renderer.h"

#include "util.h"

Renderer::Renderer(GLuint whiteTexture) : batches(1), shader("assets/shaders/base.vert", "assets/shaders/base.frag"), whiteTextureID(whiteTexture)
{
	GLuint IBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glBufferData(GL_ARRAY_BUFFER, Batch::MAX_QUADS * sizeof(Quad), nullptr, GL_DYNAMIC_DRAW);

	// Coordinates
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
	glEnableVertexAttribArray(1);

	// Texture Coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, s));
	glEnableVertexAttribArray(2);

	// Texture Index
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture));
	glEnableVertexAttribArray(3);

	unsigned int indices[Batch::MAX_QUADS * 6];
	for (int i = 0; i < Batch::MAX_QUADS; i++)
	{
		const int rightOffset = 4 * i;
		const int leftOffset = 6 * i;

		indices[leftOffset + 0] = rightOffset + 0;
		indices[leftOffset + 1] = rightOffset + 1;
		indices[leftOffset + 2] = rightOffset + 3;

		indices[leftOffset + 3] = rightOffset + 1;
		indices[leftOffset + 4] = rightOffset + 2;
		indices[leftOffset + 5] = rightOffset + 3;
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(shader.ID);
	GLint location = glGetUniformLocation(shader.ID, "batchTextures");

	int samplers[MAX_TEXTURES_PER_BATCH];
	for (int i = 0; i < MAX_TEXTURES_PER_BATCH; i++)
	{
		samplers[i] = i;
	}
	glUniform1iv(location, MAX_TEXTURES_PER_BATCH, samplers);

	this->textureIDs.push_back(whiteTexture);
	this->texturesUsed.push_back(whiteTextureID);
	whiteTextureIndex = 0.0f;
}

Batch& Renderer::DetermineBatch(int textureID)
{
	auto result = std::find(texturesUsed.rbegin(), texturesUsed.rend(), textureID);
	int location;

	if (result != texturesUsed.rend())
	{
		location = texturesUsed.rend() - result;
	}
	else
	{
		location = -1;
	}

	int textureBatch = floor(location / MAX_TEXTURES_PER_BATCH);
	if (location == -1) textureBatch = -1;

	if (location != -1 && batches[location + 1 % MAX_TEXTURES_PER_BATCH].index < Batch::MAX_QUADS)
	{
		return batches[location + 1 % MAX_TEXTURES_PER_BATCH];
	}
	else
	{
		texturesUsed.push_back(textureID);
		return batches[texturesUsed.size() % MAX_TEXTURES_PER_BATCH];
	}
}

void Renderer::PrepareCube(glm::vec3 size, glm::vec3 position, glm::vec3 rotation, glm::vec4 color, int textureID)
{
	glm::vec3 closeBottomLeft	= Util::RotateRelative(	position,	position - (size / 2.0f),											rotation);
	glm::vec3 closeBottomRight	= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, 0, -size / 2.0f),				rotation);
	glm::vec3 closeTopLeft		= Util::RotateRelative(	position,	position + glm::vec3(0, size.y / 2.0f, -size / 2.0f),				rotation);
	glm::vec3 closeTopRight		= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, size.y / 2.0f, -size / 2.0f),	rotation);

	glm::vec3 farTopRight		= Util::RotateRelative(	position,	position + (size / 2.0f),											rotation);
	glm::vec3 farBottomRight	= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, 0, size.z / 2.0f),				rotation);
	glm::vec3 farTopLeft		= Util::RotateRelative(	position,	position + glm::vec3(0, size.y / 2.0f, size.z / 2.0f),				rotation);
	glm::vec3 farTopRight		= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, size.y / 2.0f, size.z / 2.0f),	rotation);

	// Front

}

void Renderer::PrepareQuad(Quad& input, int textureID)
{
	Batch& batch = DetermineBatch(textureID);
	Quad& quad = batch.buffer[batch.index];
	batch.index++;

	
}

void Renderer::PrepareQuad(glm::vec2 size, glm::vec3 position, glm::vec3 rotation, glm::vec4 color, int textureID)
{
	Batch& batch = DetermineBatch(textureID);
	Quad& quad = batch.buffer[batch.index];
	batch.index++;

}