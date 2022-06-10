#include "util.h"

glm::vec3 Util::Rotate(glm::vec3 position, glm::vec3 rotation)
{
	glm::mat3x3 matX = glm::mat3x3(1, 0, 0, 0, cos(rotation.x), sin(rotation.x), 0, -sin(rotation.x), cos(rotation.x));
	glm::mat3x3 matY = glm::mat3x3(cos(rotation.y), 0, -sin(rotation.y), 0, 1, 0, sin(rotation.y), 0, cos(rotation.y));
	glm::mat3x3 matZ = glm::mat3x3(cos(rotation.z), sin(rotation.z), 0, -sin(rotation.z), cos(rotation.z), 0, 0, 0, 1);

	return position * matX * matY * matZ;
}

glm::vec3 Util::RotateRelative(glm::vec3 relative, glm::vec3 position, glm::vec3 rotation)
{
	return Rotate(relative - position, rotation) + relative;
}
