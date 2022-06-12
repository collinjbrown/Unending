#include "util.h"
#include "component.h"

#include <corecrt_math_defines.h>
#include <iostream>

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

glm::vec3 Util::Lerp(glm::vec3 a, glm::vec3 b, float step)
{
	return (a * (1.0f - step) + (b * step));
}

glm::vec3 Util::GetLandingCoords(Face activeFace, Face rollDirection)
{
	glm::vec3 difference = glm::vec3(0, 1, 1);

	if (activeFace == Face::top && rollDirection == Face::back)				difference = { 0,	-1,	-1 };
	else if (activeFace == Face::top && rollDirection == Face::front)		difference = { 0,	-1,	1 };
	else if (activeFace == Face::top && rollDirection == Face::left)		difference = { -1,	-1,	0 };
	else if (activeFace == Face::top && rollDirection == Face::right)		difference = { 1,	-1,	0 };
	else if (activeFace == Face::bottom && rollDirection == Face::back)		difference = { 0,	1,	-1 };
	else if (activeFace == Face::bottom && rollDirection == Face::front)	difference = { 0,	1,	1 };
	else if (activeFace == Face::bottom && rollDirection == Face::left)		difference = { -1,	1,	0 };
	else if (activeFace == Face::bottom && rollDirection == Face::right)	difference = { 1,	1,	0 };
	else if (activeFace == Face::right && rollDirection == Face::back)		difference = { -1,	0,	1 };
	else if (activeFace == Face::right && rollDirection == Face::front)		difference = { -1,	0,	1 };
	else if (activeFace == Face::right && rollDirection == Face::top)		difference = { -1,	1,	0 };
	else if (activeFace == Face::right && rollDirection == Face::bottom)	difference = { -1,	-1,	0 };
	else if (activeFace == Face::left && rollDirection == Face::back)		difference = { 1,	0,	1 };
	else if (activeFace == Face::left && rollDirection == Face::front)		difference = { 1,	0,	1 };
	else if (activeFace == Face::left && rollDirection == Face::top)		difference = { 1,	1,	0 };
	else if (activeFace == Face::left && rollDirection == Face::bottom)		difference = { 1,	-1,	0 };
	else if (activeFace == Face::front && rollDirection == Face::left)		difference = { -1,	0,	1 };
	else if (activeFace == Face::front && rollDirection == Face::right)		difference = { 1,	0,	1 };
	else if (activeFace == Face::front && rollDirection == Face::top)		difference = { 0,	1,	1 };
	else if (activeFace == Face::front && rollDirection == Face::bottom)	difference = { 0,	-1,	1 };
	else if (activeFace == Face::back && rollDirection == Face::left)		difference = { -1,	0,	-1 };
	else if (activeFace == Face::back && rollDirection == Face::right)		difference = { 1,	0,	-1 };
	else if (activeFace == Face::back && rollDirection == Face::top)		difference = { 0,	1,	-1 };
	else if (activeFace == Face::back && rollDirection == Face::bottom)		difference = { 0,	-1,	-1 };

	return difference;
}

Face Util::GetAbsoluteFace(Face relativeUp, Face pseudoForward)
{
	if (relativeUp == Face::top) return pseudoForward;
	else if (relativeUp == Face::bottom)
	{
		if (pseudoForward == Face::front) return Face::back;
		else if (pseudoForward == Face::back) return Face::front;
		else if (pseudoForward == Face::right) return Face::left;
		else if (pseudoForward == Face::left) return Face::right;
	}
	else if (relativeUp == Face::front)
	{
		if (pseudoForward == Face::front) return Face::top;
		else if (pseudoForward == Face::back) return Face::bottom;
		else if (pseudoForward == Face::right ||
			pseudoForward == Face::left) return pseudoForward;
	}
	else if (relativeUp == Face::back)
	{
		if (pseudoForward == Face::front) return Face::bottom;
		else if (pseudoForward == Face::back) return Face::top;
		else if (pseudoForward == Face::right ||
			pseudoForward == Face::left) return pseudoForward;
	}
	else if (relativeUp == Face::right)
	{
		if (pseudoForward == Face::left) return Face::top;
		else if (pseudoForward == Face::right) return Face::bottom;
		else if (pseudoForward == Face::front ||
			pseudoForward == Face::back) return pseudoForward;
	}
	else if (relativeUp == Face::left)
	{
		if (pseudoForward == Face::left) return Face::bottom;
		else if (pseudoForward == Face::right) return Face::top;
		else if (pseudoForward == Face::front ||
			pseudoForward == Face::back) return pseudoForward;
	}
}

glm::vec3 Util::GetRelativeUp(Face face)
{
	glm::vec3 relativeUp = glm::vec3(0.0f, 1.0f, 0.0f);

	if (face == Face::front) relativeUp = { 0.0f, 0.0f, -1.0f };
	if (face == Face::back) relativeUp = { 0.0f, 0.0f, 1.0f };
	if (face == Face::left) relativeUp = { -1.0f, 0.0f, 0.0f };
	if (face == Face::right) relativeUp = { 1.0f, 0.0f, 0.0f };
	if (face == Face::bottom) relativeUp = { 0.0f, -1.0f, 0.0f };

	return relativeUp;
}

glm::vec3 Util::GetFaceRotation(Face face)
{
	float rn = 1.5708f;

	if (face == Face::top) return		{	0.0f,	0.0f,		0.0f	};
	if (face == Face::bottom) return	{	0.0f,	-rn * 2.0f,	0.0f	};
	if (face == Face::front) return		{	-rn,	0.0f,		0.0f	};
	if (face == Face::back) return		{	rn,		0.0f,		0.0f	};
	if (face == Face::right) return		{	0.0f,	0.0f,		rn		};
	if (face == Face::left) return		{	0.0f,	0.0f,		-rn		};
}

glm::vec3 Util::GetRollRotation(Face activeFace, Face rollDirection, glm::vec3 baseRotation)
{
	glm::vec3 relativeUp = GetRelativeUp(activeFace);
	glm::vec3 relativeForward = GetRelativeUp(rollDirection);
	glm::vec3 relativeRight = glm::cross(relativeForward, relativeUp);

	return relativeRight * 1.5708f;
}