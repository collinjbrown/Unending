#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture
{
private:
	Texture();

public:
	static Texture* whiteTexture();

	GLuint			ID;
	unsigned int	width;
	unsigned int	height;
	GLuint			internalFormat;
	GLuint			imageFormat;
	GLuint			wrapS;
	GLuint			wrapT;
	GLuint			filterMin;
	GLuint			filterMax;

	Texture(const char* file, bool alpha, int filter = GL_LINEAR);

	void Bind() const;
};

#endif