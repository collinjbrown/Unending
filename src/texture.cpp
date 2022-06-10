#include "texture.h"

#include "external/stb_image.h"

#include <iostream>
#include <climits>

Texture::Texture(const char* file, bool alpha, int filter) : width(0), height(0), internalFormat(GL_RGB), imageFormat(GL_RGB), wrapS(GL_REPEAT), wrapT(GL_REPEAT), filterMin(filter), filterMax(filter)
{
	glGenTextures(1, &this->ID);

	if (alpha)
	{
		this->internalFormat = GL_RGBA;
		this->imageFormat = GL_RGBA;
	}

	stbi_set_flip_vertically_on_load(true);

	int imageWidth, imageHeight, nrChannels;
	unsigned char* data = stbi_load(file, &imageWidth, &imageHeight, &nrChannels, 0);

	this->width = width;
	this->height = height;

	// Create
	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, imageWidth, imageHeight, 0, this->imageFormat, GL_UNSIGNED_BYTE, data);

	// Wrap & Filter Modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filterMax);

	// Unbind
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}

Texture::Texture() : width(0), height(0), internalFormat(GL_RGB), imageFormat(GL_RGB), wrapS(GL_REPEAT), wrapT(GL_REPEAT), filterMin(GL_LINEAR), filterMax(GL_LINEAR)
{
	glGenTextures(1, &this->ID);

	constexpr unsigned char data[] = { UCHAR_MAX, UCHAR_MAX, UCHAR_MAX };

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

Texture* Texture::whiteTexture()
{
	return new Texture();
}