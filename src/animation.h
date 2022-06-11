#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Animation
{
public:
	GLuint				ID;
	unsigned int		width;
	unsigned int		height;
	unsigned int		rows;
	unsigned int		columns;
	float				speed;
	bool				loop;
	std::vector<int>	layout;
	GLuint				internalFormat;
	GLuint				imageFormat;
	GLuint				wrapS;
	GLuint				wrapT;
	GLuint				filterMin;
	GLuint				filterMax;

	Animation(const char* file, bool alpha, int columns, int rows, float speed, std::vector<int> layout, bool loop, int filter = GL_LINEAR);

	void Bind() const;
};

#endif