#include "renderer.h"

#include <iostream>

#include "game.h"
#include "util.h"
#include <glm/gtx/norm.hpp>

Renderer::Renderer(GLuint whiteTexture) : batches(1), shader("assets/shaders/base.vert", "assets/shaders/base.frag"),
whiteTextureID(whiteTexture)
{
	GLuint IBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glBufferData(GL_ARRAY_BUFFER, Batch::MAX_TRIS * sizeof(Triangle), nullptr, GL_DYNAMIC_DRAW);

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

	unsigned int indices[Batch::MAX_TRIS * 3];
	for (int i = 0; i < Batch::MAX_TRIS; i++)
	{
		const int offset = 3 * i;

		indices[offset + 0] = offset + 0;
		indices[offset + 1] = offset + 1;
		indices[offset + 2] = offset + 2;
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

	if (location != -1 && batches[textureBatch].index + 2 < Batch::MAX_TRIS)
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

void Renderer::PrepareModel(glm::vec3 size, glm::vec3 position, Quaternion q, glm::vec4 color, Model* model)
{
	Bundle bundle = DetermineBatch(whiteTextureID);
	Batch& batch = batches[bundle.batch];

	int triCount = model->vertices.size() / 3;

	for (int i = 0; i < triCount; i++)
	{
		glm::vec3 aPos = model->vertices[(i * 3) + 0] * size.x;
		glm::vec3 bPos = model->vertices[(i * 3) + 1] * size.y;
		glm::vec3 cPos = model->vertices[(i * 3) + 2] * size.z;

		aPos.x *= -1.0f;
		bPos.x *= -1.0f;
		cPos.x *= -1.0f;
		aPos.y *= -1.0f;
		bPos.y *= -1.0f;
		cPos.y *= -1.0f;

		aPos = Util::RotateRelative(position, position + aPos, q);
		bPos = Util::RotateRelative(position, position + bPos, q);
		cPos = Util::RotateRelative(position, position + cPos, q);

		Vertex a = { aPos.x, aPos.y, aPos.z, color.r, color.g, color.b, color.a, 0.0f, 0.0f, bundle.location };
		Vertex b = { bPos.x, bPos.y, bPos.z, color.r, color.g, color.b, color.a, 0.0f, 0.0f, bundle.location };
		Vertex c = { cPos.x, cPos.y, cPos.z, color.r, color.g, color.b, color.a, 0.0f, 0.0f, bundle.location };

		batch.buffer[batch.index] = { a, b, c };
		batch.index++;
	}
}

void Renderer::PrepareCube(glm::vec3 size, glm::vec3 position, Quaternion q, glm::vec4 color, int textureID)
{
	glm::vec3 closeTopRight		= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f),		q);// *Game::main.zoom;
	glm::vec3 closeBottomRight	= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f),	q);// * Game::main.zoom;
	glm::vec3 closeBottomLeft	= Util::RotateRelative(	position,	position - (size / 2.0f),												q);// * Game::main.zoom;
	glm::vec3 closeTopLeft		= Util::RotateRelative(	position,	position + glm::vec3(-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f),	q);// * Game::main.zoom;

	glm::vec3 farTopRight		= Util::RotateRelative(	position,	position + (size / 2.0f),												q);// * Game::main.zoom;
	glm::vec3 farBottomRight	= Util::RotateRelative(	position,	position + glm::vec3(size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f),		q);// * Game::main.zoom;
	glm::vec3 farBottomLeft		= Util::RotateRelative(	position,	position + glm::vec3(-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f),	q);// * Game::main.zoom;
	glm::vec3 farTopLeft		= Util::RotateRelative(	position,	position + glm::vec3(-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f),		q);// * Game::main.zoom;

	float f = glm::length2(Game::main.cameraForward - Util::Rotate({ 0.0f, 0.0f, 1.0f }, q));
	float b = glm::length2(Game::main.cameraForward - Util::Rotate({ 0.0f, 0.0f, -1.0f }, q));
	float u = glm::length2(Game::main.cameraForward - Util::Rotate({ 0.0f, -1.0f, 0.0f }, q));
	float d = glm::length2(Game::main.cameraForward - Util::Rotate({ 0.0f, 1.0f, 0.0f }, q));
	float r = glm::length2(Game::main.cameraForward - Util::Rotate({ 1.0f, 0.0f, 0.0f }, q));
	float l = glm::length2(Game::main.cameraForward - Util::Rotate({ -1.0f, 0.0f, 0.0f }, q));

	float minDiff = 2.0f;

	// Front	- All the close verts.
	Quad front
	{
		{
			{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID },
			{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.0,	0.5,	(float)textureID },
			{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.0,	0.25,	(float)textureID },
		},
		{
			{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
			{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID },
			{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.0,	0.25,	(float)textureID },
		}
	};

	// Back		- All the far verts.
	Quad back
	{
		{
			{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID },
			{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.5,	(float)textureID },
			{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.25,	(float)textureID },

		},
		{
			{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID },
			{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID },
			{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.25,	(float)textureID },
		}
	};

	// Right		- Close left, top and bottom, and far left, top and bottom.		- The far verts will be treated as the quad's left, top and bottom.
	Quad right
	{
		{
			{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID },
			{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.75,	(float)textureID },
			{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.75,	(float)textureID },
		},
		{
			{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID },
			{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID },
			{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.75,	(float)textureID },
		}
	};

	// Left	- Close right, top and bottom, and far right, top and bottom.	- The close verts will be treated as the quad's left, top and bottom.
	Quad left
	{
		{
			{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
			{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.0,	(float)textureID },
			{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.0,	(float)textureID },
		},
		{
			{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID },
			{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
			{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.0,	(float)textureID },
		}
	};

	// Top		- Close top, left and right, and far top, left and right.		- The left verts, far and close, will be treated as the quad's left, top and bottom.
	Quad top
	{
		{
			{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID },
			{ closeTopLeft.x,		closeTopLeft.y,		closeTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.25,	(float)textureID },
			{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID },
		},
		{
			{ farTopLeft.x,			farTopLeft.y,		farTopLeft.z,		color.r,	color.g,	color.b,	color.a,	0.25,	0.5,	(float)textureID },
			{ farTopRight.x,		farTopRight.y,		farTopRight.z,		color.r,	color.g,	color.b,	color.a,	0.5,	0.5,	(float)textureID },
			{ closeTopRight.x,		closeTopRight.y,	closeTopRight.z,	color.r,	color.g,	color.b,	color.a,	0.5,	0.25,	(float)textureID },
		}
	};

	// Bottom		- Close bottom, left and right, and far bottom, left and right.	- The right verts, far and close, will be treated as the quad's left, top and bottom.
	Quad bottom
	{
		{
			{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.5,	(float)textureID },
			{ farBottomLeft.x,		farBottomLeft.y,	farBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.25,	(float)textureID },
			{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	1.0,	0.25,	(float)textureID },
		},
		{
			{ closeBottomLeft.x,	closeBottomLeft.y,	closeBottomLeft.z,	color.r,	color.g,	color.b,	color.a,	0.75,	0.5,	(float)textureID },
			{ closeBottomRight.x,	closeBottomRight.y,	closeBottomRight.z,	color.r,	color.g,	color.b,	color.a,	1.0,	0.5,	(float)textureID },
			{ farBottomRight.x,		farBottomRight.y,	farBottomRight.z,	color.r,	color.g,	color.b,	color.a,	1.0,	0.25,	(float)textureID },
		}
	};

	if (f > minDiff) PrepareQuad(front,		textureID);
	if (l > minDiff) PrepareQuad(left,		textureID);
	if (b > minDiff) PrepareQuad(back,		textureID);
	if (r > minDiff) PrepareQuad(right,		textureID);
	if (u > minDiff) PrepareQuad(top,		textureID);
	if (d > minDiff) PrepareQuad(bottom,	textureID);

	/*PrepareQuad(front, textureID);
	PrepareQuad(left, textureID);
	PrepareQuad(back, textureID);
	PrepareQuad(right, textureID);
	PrepareQuad(top, textureID);
	PrepareQuad(bottom, textureID);*/
}

void Renderer::PrepareQuad(Quad& input, int textureID)
{
	Bundle bundle = DetermineBatch(textureID);
	Batch& batch = batches[bundle.batch];

	// Triangle& t1 = batch.buffer[batch.index];
	input.left.topLeft.texture = bundle.location;
	input.left.bottomLeft.texture = bundle.location;
	input.left.bottomRight.texture = bundle.location;
	batch.buffer[batch.index] = input.left;
	batch.index++;

	// Triangle& t2 = batch.buffer[batch.index];
	input.right.topLeft.texture = bundle.location;
	input.right.bottomLeft.texture = bundle.location;
	input.right.bottomRight.texture = bundle.location;
	batch.buffer[batch.index] = input.right;
	batch.index++;
}

void Renderer::PrepareQuad(glm::vec2 size, glm::vec3 position, Quaternion q, glm::vec4 color, int textureID)
{
	/*Batch& batch = DetermineBatch(textureID);
	Quad& quad = batch.buffer[batch.index];
	batch.index++;*/

}

void Renderer::PrepareQuad(glm::vec2 size, glm::vec3 position, Quaternion q, glm::vec4 color, int animationID, int cellX, int cellY, int cols, int rows, bool flippedX, bool flippedY)
{
	glm::vec3 topRight		= Util::RotateRelative(	position,	position + glm::vec3(	size.x / 2.0f,	size.y / 2.0f,	0.0f),	q);//  * Game::main.zoom;
	glm::vec3 bottomRight	= Util::RotateRelative(	position,	position + glm::vec3(	size.x / 2.0f,	-size.y / 2.0f, 0.0f),	q);//  * Game::main.zoom;
	glm::vec3 bottomLeft	= Util::RotateRelative(	position,	position + glm::vec3(	-size.x / 2.0f,	-size.y / 2.0f, 0.0f),	q);//  * Game::main.zoom;
	glm::vec3 topLeft		= Util::RotateRelative(	position,	position + glm::vec3(	-size.x / 2.0f,	size.y / 2.0f, 0.0f),	q);//  * Game::main.zoom;

	float cellXMod = 1.0f / cols;
	float cellYMod = 1.0f / rows;

	float uvX0 = cellX * cellXMod;
	float uvY0 = cellY * cellYMod;
	float uvX1 = uvX0 + cellXMod;
	float uvY1 = uvY0 + cellYMod;

	if (flippedX)
	{
		float tempX0 = uvX0;

		uvX0 = uvX1;
		uvX1 = tempX0;
	}

	if (flippedY)
	{
		float tempY0 = uvY0;

		uvY0 = uvY1;
		uvY1 = tempY0;
	}

	Vertex tR = { topRight.x,			topRight.y,			topRight.z,		color.r,	color.g,	color.b,	color.a,	uvX0,	uvY0,	0 };
	Vertex bR = { bottomRight.x,		bottomRight.y,		bottomRight.z,	color.r,	color.g,	color.b,	color.a,	uvX0,	uvY1,	0 };
	Vertex bL = { bottomLeft.x,		bottomLeft.y,		bottomLeft.z,	color.r,	color.g,	color.b,	color.a,	uvX1,	uvY1,	0 };
	Vertex tL = { topLeft.x,			topLeft.y,			topLeft.z,		color.r,	color.g,	color.b,	color.a,	uvX1,	uvY0,	0 };

	Quad quad;
	quad.left =
	{
		tR,
		bR,
		bL
	};
	quad.right =
	{
		bR,
		tL,
		tR
	};

	PrepareQuad(quad, animationID);
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
	glBufferSubData(GL_ARRAY_BUFFER, 0, batch.index * sizeof(Triangle), &batch.buffer[0]);
	glDrawElements(GL_TRIANGLES, batch.index * 3, GL_UNSIGNED_INT, nullptr);
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