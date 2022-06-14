#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>

enum class Face;
struct Quaternion;

class Util
{
public:
	static Quaternion Slerp(Quaternion q, Quaternion r, float step);
	static float QuaternionDistance(Quaternion l, Quaternion r);

	static void NormalizeQuaternion(Quaternion& q);
	static Quaternion EulerToQuaternion(glm::vec3 e);
	static glm::vec3 QuaternionToEuler(Quaternion q);

	static glm::vec3 RotateRelative(glm::vec3 relative, glm::vec3 position, Quaternion q);
	static glm::vec3 Rotate(glm::vec3 position, Quaternion q);
	
	static glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, float step);

	static Face GetAbsoluteFace(Face relativeUp, Face pseudoForward);
	static glm::vec3 GetRelativeUp(Face face);
	static glm::vec3 GetForward(Face up, Face right, bool corner);
	static glm::vec3 GetLandingCoords(Face fulcrum, Face rollDirection);

	static Face OppositeFace(Face face);
	static glm::vec3 GetFaceRotation(Face face);
	static Face GetFaceFromDifference(glm::vec3 difference);
	static Quaternion GetRollRotation(Face activeFace, Face rollDirection, Quaternion baseQuaternion, int turns);
};

#endif