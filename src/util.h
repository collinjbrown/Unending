#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>

enum class Face;

class Util
{
public:
	static glm::vec3 RotateRelative(glm::vec3 relative, glm::vec3 position, glm::vec3 rotation);
	static glm::vec3 Rotate(glm::vec3 position, glm::vec3 rotation);
	
	static glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, float step);

	static Face GetAbsoluteFace(Face relativeUp, Face pseudoForward);
	static glm::vec3 GetRelativeUp(Face face);
	static glm::vec3 GetLandingCoords(Face activeFace, Face rollDirection);

	static glm::vec3 GetFaceRotation(Face face);
	static glm::vec3 GetRollRotation(Face activeFace, Face rollDirection, glm::vec3 baseRotation);
};

#endif