#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "glm/glm.hpp"

class Model
{
public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	void LoadObjFile(const char* path);
	Model(const char* path);
};

#endif