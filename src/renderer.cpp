#include "renderer.h"

#include <iostream>

#include "game.h"
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

Bundle Renderer::DetermineBatch(int textureID)
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

	int currentBatch = floor(texturesUsed.size() / MAX_TEXTURES_PER_BATCH);

	if (location != -1 && batches[textureBatch].index < Batch::MAX_QUADS)
	{
		return { textureBatch, (float)(static_cast<int>(location - 1) % MAX_TEXTURES_PER_BATCH) };
	}
	else
	{
		location = texturesUsed.size() + 1;
		texturesUsed.push_back(textureID);
		return { currentBatch, (float)(static_cast<int>(location - 1) % MAX_TEXTURES_PER_BATCH) };
	}
}

void Renderer::PrepareCube(glm::vec3 size, glm::vec3 position, glm::vec3 rotation, glm::vec4 color, int textureID)
{
	glm::vec3 closeTopRight		= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f),		rotation) * Game::main.zoom;
	glm::vec3 closeBottomRight	= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f),	rotation) * Game::main.zoom;
	glm::vec3 closeBottomLeft	= Util::RotateRelative(	position,	position - (size / 2.0f),												rotation) * Game::main.zoom;
	glm::vec3 closeTopLeft		= Util::RotateRelative(	position,	position + glm::vec3(-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f),	rotation) * Game::main.zoom;

	glm::vec3 farTopRight		= Util::RotateRelative(	position,	position + (size / 2.0f),												rotation) * Game::main.zoom;
	glm::vec3 farBottomRight	= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f),		rotation) * Game::main.zoom;
	glm::vec3 farBottomLeft		= Util::RotateRelative(	position,	position + glm::vec3(-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f),	rotation) * Game::main.zoom;
	glm::vec3 farTopLeft		= Util::RotateRelative(	position,	position + glm::vec3(-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f),		rotation) * Game::main.zoom;

	// Front	- All the close verts.
	Quad front
	{
		{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
		{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.0,	0.25,	(float)textureID },
		{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.0,	0.5,	(float)textureID },
		{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID }
	};

	// Back		- All the far verts.
	Quad back
	{
		{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID },
		{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.25,	(float)textureID },
		{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.5,	(float)textureID },
		{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID }
	};

	// Left		- Close left, top and bottom, and far left, top and bottom.		- The far verts will be treated as the quad's left, top and bottom.
	Quad left
	{
		{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID },
		{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.75,	(float)textureID },
		{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.75,	(float)textureID },
		{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID }
	};

	// Right	- Close right, top and bottom, and far right, top and bottom.	- The close verts will be treated as the quad's left, top and bottom.
	Quad right
	{
		{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
		{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.0,	(float)textureID },
		{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.0,	(float)textureID },
		{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID }
	};

	// Top		- Close top, left and right, and far top, left and right.		- The left verts, far and close, will be treated as the quad's left, top and bottom.
	Quad top
	{
		{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID },
		{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID },
		{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
		{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID }
	};

	// Bottom		- Close bottom, left and right, and far bottom, left and right.	- The right verts, far and close, will be treated as the quad's left, top and bottom.
	Quad bottom
	{
		{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.25,	(float)textureID },
		{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.5,	(float)textureID },
		{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	1.0,	0.5,	(float)textureID },
		{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	1.0,	0.25,	(float)textureID }
	};

	PrepareQuad(front,	textureID);
	PrepareQuad(left,	textureID);
	PrepareQuad(back,	textureID);
	PrepareQuad(right,	textureID);
	PrepareQuad(top,	textureID);
	PrepareQuad(bottom,	textureID);
}

void Renderer::PrepareQuad(Quad& input, int textureID)
{
	Bundle bundle = DetermineBatch(textureID);
	Batch& batch = batches[bundle.batch];
	Quad& quad = batch.buffer[batch.index];

	input.topRight.texture		= bundle.location;
	input.bottomRight.texture	= bundle.location;
	input.bottomLeft.texture	= bundle.location;
	input.topLeft.texture		= bundle.location;

	batch.buffer[batch.index] = input;
	batch.index++;
}

void Renderer::PrepareQuad(glm::vec2 size, glm::vec3 position, glm::vec3 rotation, glm::vec4 color, int textureID)
{
	/*Batch& batch = DetermineBatch(textureID);
	Quad& quad = batch.buffer[batch.index];
	batch.index++;*/

}

void Renderer::Display()
{
	shader.Use();
	shader.SetMatrix("MVP", Game::main.projection * Game::main.view);

	int currentBatch = 0;
	int texUnit = 0;

	for (int i = 0; i < texturesUsed.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + texUnit);
		glBindTexture(GL_TEXTURE_2D, textureIDs[texturesUsed[i] - 1]);

		if (texUnit >= MAX_TEXTURES_PER_BATCH - 1)
		{
			Flush(batches[currentBatch]);
			currentBatch++;
			texUnit = 0;
		}
		else
		{
			texUnit++;
		}
	}

	Flush(batches[currentBatch]);
}

void Renderer::Flush(const Batch& batch)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batch.index * sizeof(Quad), &batch.buffer[0]);
	glDrawElements(GL_TRIANGLES, batch.index * 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer::ResetBuffers()
{
	texturesUsed.clear();
	texturesUsed.push_back(whiteTextureID);

	for (Batch& batch : batches)
	{
		batch.index = 0;
	}
}