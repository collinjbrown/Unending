#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>

class Util
{
public:
	static glm::vec3 RotateRelative(glm::vec3 relative, glm::vec3 position, glm::vec3 rotation);
	static glm::vec3 Rotate(glm::vec3 position, glm::vec3 rotation);
	
	static glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, float step);
};

#endif