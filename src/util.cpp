#include "util.h"
#include "component.h"

#include <corecrt_math_defines.h>
#include <iostream>
#include <glm/gtx/norm.hpp>

Quaternion Util::Slerp(Quaternion l, Quaternion r, float step)
{
	Quaternion q = { 0, 0, 0, 0 };

	float dot = l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
	float theta, st, sut, sout, coeff1, coeff2;

	step /= 2.0f;

	theta = (float)acos(dot);
	if (theta < 0.0) theta = -theta;

	st = (float)sin(theta);
	sut = (float)sin(step * theta);
	sout = (float)sin((1 - step) * theta);
	coeff1 = sout / st;
	coeff2 = sut / st;

	q.x = coeff1 * l.x + coeff2 * r.x;
	q.y = coeff1 * l.y + coeff2 * r.y;
	q.z = coeff1 * l.z + coeff2 * r.z;
	q.w = coeff1 * l.w + coeff2 * r.w;

	NormalizeQuaternion(q);
	return q;
}

float Util::QuaternionDistance(Quaternion l, Quaternion r)
{
	glm::vec3 ref = { 1.0f, 1.0f, 1.0f };
	glm::vec3 ori = { 0.0f, 0.0f, 0.0f };
	glm::vec3 refL = Rotate(ref, l);
	glm::vec3 refR = Rotate(ref, r);

	return glm::length2(refL - refR);
}

void Util::NormalizeQuaternion(Quaternion& q)
{
	float mag = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	q.w /= mag;
	q.x /= mag;
	q.y /= mag;
	q.z /= mag;
}

Quaternion Util::EulerToQuaternion(glm::vec3 e)
{
	Quaternion q = { 0, 0, 0, 0 };
	double cy = cos(e.z * 0.5);
	double sy = sin(e.z * 0.5);
	double cp = cos(e.y * 0.5);
	double sp = sin(e.y * 0.5);
	double cr = cos(e.x * 0.5);
	double sr = sin(e.x * 0.5);

	q.w = (float)(cr * cp * cy + sr * sp * sy);
	q.x = (float)(sr * cp * cy - cr * sp * sy);
	q.y = (float)(cr * sp * cy + sr * cp * sy);
	q.z = (float)(cr * cp * sy - sr * sp * cy);

	return q;
}

glm::vec3 Util::QuaternionToEuler(Quaternion q)
{
	glm::vec3 e = { 0, 0, 0 };
	double sinrCosp = 2 * (q.w * q.x + q.y * q.z);
	double cosrCosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	e.x = (float)std::atan2(sinrCosp, cosrCosp);

	double sinp = 2 * (q.w * q.y - q.z * q.x);
	if (std::abs(sinp) >= 1)
	{
		e.y = (float)std::copysign(M_PI / 2, sinp);
	}
	else
	{
		e.y = (float)std::asin(sinp);
	}

	double sinyCosp = 2 * (q.w * q.z + q.x * q.y);
	double cosyCosp = 1 - 2 * (q.y, q.y + q.z * q.z);
	e.z = (float)std::atan2(sinyCosp, cosyCosp);

	return e;
}

glm::vec3 Util::Rotate(glm::vec3 position, Quaternion q)
{
	glm::mat3x3 mat = glm::mat3x3(	1 - 2 * (q.y * q.y + q.z * q.z),	2 * (q.x * q.y - q.z * q.w),		2 * (q.x * q.z + q.y * q.w),
									2 * (q.x * q.y + q.z * q.w),		1 - 2 * (q.x * q.x + q.z * q.z),	2 * (q.y * q.z - q.x * q.w),
									2 * (q.x * q.z - q.y * q.w),		2 * (q.y * q.z + q.x * q.w),		1 - 2 * (q.x * q.x + q.y * q.y));

		/*glm::mat3x3 matX = glm::mat3x3(1, 0, 0, 0, cos(rotation.x), sin(rotation.x), 0, -sin(rotation.x), cos(rotation.x));
		glm::mat3x3 matY = glm::mat3x3(cos(rotation.y), 0, -sin(rotation.y), 0, 1, 0, sin(rotation.y), 0, cos(rotation.y));
		glm::mat3x3 matZ = glm::mat3x3(cos(rotation.z), sin(rotation.z), 0, -sin(rotation.z), cos(rotation.z), 0, 0, 0, 1);*/

		return position * mat;// X* matY* matZ;
}

glm::vec3 Util::RotateRelative(glm::vec3 relative, glm::vec3 position, Quaternion q)
{
	return Rotate(relative - position, q) + relative;
}

glm::vec3 Util::Lerp(glm::vec3 a, glm::vec3 b, float step)
{
	return (a * (1.0f - step) + (b * step));
}

glm::vec3 Util::GetLandingCoords(Face fulcrum, Face rollDirection)
{
	glm::vec3 forward = Util::GetRelativeUp(rollDirection);
	glm::vec3 down = Util::GetRelativeUp(fulcrum);

	return forward + down;
}

Face Util::GetAbsoluteFace(Face relativeUp, Face pseudoForward)
{
	if (relativeUp == Face::top) return pseudoForward;
	else if (relativeUp == Face::bottom)
	{
		if (pseudoForward == Face::front) return Face::back;
		else if (pseudoForward == Face::back) return Face::front;
		else if (pseudoForward == Face::right) return Face::right;
		else if (pseudoForward == Face::left) return Face::left;
	}
	else if (relativeUp == Face::front)
	{
		if (pseudoForward == Face::front) return Face::bottom;
		else if (pseudoForward == Face::back) return Face::top;
		else if (pseudoForward == Face::right ||
			pseudoForward == Face::left) return pseudoForward;
	}
	else if (relativeUp == Face::back)
	{
		if (pseudoForward == Face::front) return Face::top;
		else if (pseudoForward == Face::back) return Face::bottom;
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


Face Util::OppositeFace(Face face)
{
	if (face == Face::front) return Face::back;
	if (face == Face::back) return Face::front;
	if (face == Face::top) return Face::bottom;
	if (face == Face::bottom) return Face::top;
	if (face == Face::left) return Face::right;
	if (face == Face::right) return Face::left;
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

Face Util::GetFaceFromDifference(glm::vec3 difference)
{
	if (difference.x == 0 && difference.y == 1 && difference.z == 0) return Face::top;
	if (difference.x == 0 && difference.y == -1 && difference.z == 0) return Face::bottom;
	if (difference.x == 0 && difference.y == 0 && difference.z == -1) return Face::front;
	if (difference.x == 0 && difference.y == 0 && difference.z == 1) return Face::back;
	if (difference.x == 1 && difference.y == 0 && difference.z == 0) return Face::right;
	if (difference.x == -1 && difference.y == 0 && difference.z == 0) return Face::left;
}

Quaternion Util::GetRollRotation(Face activeFace, Face rollDirection, Quaternion baseQuaternion, int turns)
{
	glm::vec3 relativeRight = GetForward(activeFace, rollDirection, (turns == 2)) * (1.5708f * turns);

	Quaternion rightQuat = EulerToQuaternion(relativeRight);

	Quaternion ret = rightQuat * baseQuaternion;

	return ret;
}

glm::vec3 Util::GetForward(Face up, Face right, bool corner)
{
	glm::vec3 relativeUp = GetRelativeUp(up);
	glm::vec3 relativeForward = GetRelativeUp(right);

	if (relativeForward.z != 0) relativeForward *= -1.0f;
	if ((up == Face::front || up == Face::back)) relativeForward *= -1.0f;
	// if (corner && (up == Face::front || up == Face::back) && (right != Face::right && right != Face::left)) relativeForward *= -1.0f;

	glm::vec3 relativeRight = glm::cross(relativeUp, relativeForward);

	return relativeRight;
}