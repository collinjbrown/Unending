#include "util.h"
#include "component.h"

#include <corecrt_math_defines.h>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include <algorithm>

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

	if (isnan(q.x) || isnan(q.y) || isnan(q.z)) return r;

	if (theta > 2.0) return -q;
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

	NormalizeQuaternion(q);

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

Quaternion Util::GetFaceRotation(Face face)
{
	if (face == Face::top) return { 0.0f, 0.0f, 1.0f, 0.0f };
	if (face == Face::bottom) return { 0.0f, 0.0f, -1.0f, 0.0f };
	if (face == Face::right) return { 0.0f, 1.0f, 0.0f, 0.0f };
	if (face == Face::left) return { 0.0f, -1.0f, 0.0f, 0.0f };
	if (face == Face::back) return { 0.0f, 0.0f, 0.0f, 1.0f };
	if (face == Face::front) return { 0.0f, 0.0f, 0.0f, -1.0f };
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
	glm::vec3 relativeRight = GetForward(activeFace, rollDirection, (turns == 2)) * ((float)(M_PI / 2.0f) * turns);

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

	glm::vec3 relativeRight = glm::cross(relativeUp, relativeForward);

	return relativeRight;
}

Quaternion Util::GetCameraOrientation(Face face, Face lastFace, Corner corner)
{
	Quaternion xzRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

	if (face == Face::bottom)		xzRotation = EulerToQuaternion({ M_PI, 0.0f, 0.0f });
	else if (face == Face::right)	xzRotation = EulerToQuaternion({ 0.0f, 0.0f, -M_PI / 2.0f });
	else if (face == Face::left)	xzRotation = EulerToQuaternion({ 0.0f, 0.0f, M_PI / 2.0f });
	else if (face == Face::front)	xzRotation = EulerToQuaternion({ M_PI / 2.0f, 0.0f, 0.0f });
	else if (face == Face::back)	xzRotation = EulerToQuaternion({ -M_PI / 2.0f, 0.0f, 0.0f });

	float mod = 0.0f;
	// && (corner == Corner::top || corner == Corner::bottom)
	if (face == Face::bottom && (lastFace == Face::right || lastFace == Face::left)) mod = M_PI;

	Quaternion xyRotation = EulerToQuaternion({ -M_PI / 4.0f, (M_PI / 4.0f) + mod, 0.0f});

	if (corner == Corner::top)			xyRotation = EulerToQuaternion({ -M_PI / 4.0f, (M_PI * 1.25f) + mod, 0.0f });
	else if (corner == Corner::right)	xyRotation = EulerToQuaternion({ -M_PI / 4.0f, (M_PI * 0.75f) + mod, 0.0f});
	else if (corner == Corner::left)	xyRotation = EulerToQuaternion({ -M_PI / 4.0f, (-M_PI * 0.25f) + mod, 0.0f});

	Quaternion total = xzRotation * xyRotation;
	NormalizeQuaternion(total);
	return total;
}

Face Util::GetFaceFromQuaternion(Quaternion q)
{
	std::vector<std::pair<Face, float>> faceDistances;

	faceDistances.push_back(std::pair<Face, float>(Face::top, Util::QuaternionDistance(q, GetQuaternionFromFace(Face::top))));
	faceDistances.push_back(std::pair<Face, float>(Face::bottom, Util::QuaternionDistance(q, GetQuaternionFromFace(Face::bottom))));
	faceDistances.push_back(std::pair<Face, float>(Face::right, Util::QuaternionDistance(q, GetQuaternionFromFace(Face::right))));
	faceDistances.push_back(std::pair<Face, float>(Face::left, Util::QuaternionDistance(q, GetQuaternionFromFace(Face::left))));
	faceDistances.push_back(std::pair<Face, float>(Face::front, Util::QuaternionDistance(q, GetQuaternionFromFace(Face::front))));
	faceDistances.push_back(std::pair<Face, float>(Face::back, Util::QuaternionDistance(q, GetQuaternionFromFace(Face::back))));

	std::sort(faceDistances.begin(), faceDistances.end(), [](const std::pair<Face, float>& a, const std::pair<Face, float>& b) -> bool
		{
			return a.second < b.second;
		});

	return faceDistances[0].first;
}

Quaternion Util::GetQuaternionFromFace(Face f)
{
	if (f == Face::top) return EulerToQuaternion({ 0.0f, 0.0f, 0.0f });
	if (f == Face::bottom) return EulerToQuaternion({ M_PI, 0.0f, 0.0f });
	if (f == Face::right) return EulerToQuaternion({ 0.0f, 0.0f, -M_PI / 2.0f });
	if (f == Face::left) return EulerToQuaternion({ 0.0f, 0.0f, M_PI / 2.0f });
	if (f == Face::front) return EulerToQuaternion({ M_PI / 2.0f, 0.0f, 0.0f });
	if (f == Face::back) return EulerToQuaternion({ -M_PI / 2.0f, 0.0f, 0.0f });
}


Face Util::GetCameraFace(glm::vec3 forward)
{
	std::vector<std::pair<Face, float>> faceDistances;

	faceDistances.push_back(std::pair<Face, float>(Face::top, glm::length2(forward - glm::vec3(0.0f, 1.0f, 0.0f))));
	faceDistances.push_back(std::pair<Face, float>(Face::bottom, glm::length2(forward - glm::vec3(0.0f, -1.0f, 0.0f))));
	faceDistances.push_back(std::pair<Face, float>(Face::right, glm::length2(forward - glm::vec3(1.0f, 0.0f, 0.0f))));
	faceDistances.push_back(std::pair<Face, float>(Face::left, glm::length2(forward - glm::vec3(-1.0f, 0.0f, 0.0f))));
	faceDistances.push_back(std::pair<Face, float>(Face::front, glm::length2(forward - glm::vec3(0.0f, 0.0f, 1.0f))));
	faceDistances.push_back(std::pair<Face, float>(Face::back, glm::length2(forward - glm::vec3(0.0f, 0.0f, -1.0f))));

	std::sort(faceDistances.begin(), faceDistances.end(), [](const std::pair<Face, float>& a, const std::pair<Face, float>& b) -> bool
		{
			return a.second > b.second;
		});

	return faceDistances[0].first;
}

Face Util::GetFaceChangeVertical(Face face, Corner corner, int dX)
{
	Corner c;

	if (corner == Corner::bottom)		c = Corner::right;
	else if (corner == Corner::right)	c = Corner::top;
	else if (corner == Corner::top)		c = Corner::left;
	else								c = Corner::bottom;

	return GetFaceChangeHorizontal(face, c, dX);
}

Face Util::GetFaceChangeHorizontal(Face face, Corner corner, int dZ)
{
	// My brain is too smooth, sorry.

	if (face == Face::top && corner == Corner::bottom && dZ > 0) return Face::right;
	if (face == Face::top && corner == Corner::right && dZ > 0) return Face::back;
	if (face == Face::top && corner == Corner::top && dZ > 0) return Face::left;
	if (face == Face::top && corner == Corner::left && dZ > 0) return Face::front;

	if (face == Face::top && corner == Corner::bottom && dZ < 0) return Face::left;
	if (face == Face::top && corner == Corner::right && dZ < 0) return Face::front;
	if (face == Face::top && corner == Corner::top && dZ < 0) return Face::right;
	if (face == Face::top && corner == Corner::left && dZ < 0) return Face::back;


	if (face == Face::bottom && corner == Corner::bottom && dZ > 0) return Face::left;
	if (face == Face::bottom && corner == Corner::right && dZ > 0) return Face::front;
	if (face == Face::bottom && corner == Corner::top && dZ > 0) return Face::right;
	if (face == Face::bottom && corner == Corner::left && dZ > 0) return Face::back;

	if (face == Face::bottom && corner == Corner::bottom && dZ < 0) return Face::right;
	if (face == Face::bottom && corner == Corner::right && dZ < 0) return Face::back;
	if (face == Face::bottom && corner == Corner::top && dZ < 0) return Face::left;
	if (face == Face::bottom && corner == Corner::left && dZ < 0) return Face::front;


	if (face == Face::right && corner == Corner::bottom && dZ > 0) return Face::bottom;
	if (face == Face::right && corner == Corner::right && dZ > 0) return Face::back;
	if (face == Face::right && corner == Corner::top && dZ > 0) return Face::top;
	if (face == Face::right && corner == Corner::left && dZ > 0) return Face::front;

	if (face == Face::right && corner == Corner::bottom && dZ < 0) return Face::top;
	if (face == Face::right && corner == Corner::right && dZ < 0) return Face::front;
	if (face == Face::right && corner == Corner::top && dZ < 0) return Face::bottom;
	if (face == Face::right && corner == Corner::left && dZ < 0) return Face::back;


	if (face == Face::left && corner == Corner::bottom && dZ > 0) return Face::top;
	if (face == Face::left && corner == Corner::right && dZ > 0) return Face::front;
	if (face == Face::left && corner == Corner::top && dZ > 0) return Face::bottom;
	if (face == Face::left && corner == Corner::left && dZ > 0) return Face::back;

	if (face == Face::left && corner == Corner::bottom && dZ < 0) return Face::bottom;
	if (face == Face::left && corner == Corner::right && dZ < 0) return Face::back;
	if (face == Face::left && corner == Corner::top && dZ < 0) return Face::top;
	if (face == Face::left && corner == Corner::left && dZ < 0) return Face::front;


	if (face == Face::front && corner == Corner::bottom && dZ > 0) return Face::left;
	if (face == Face::front && corner == Corner::right && dZ > 0) return Face::top;
	if (face == Face::front && corner == Corner::top && dZ > 0) return Face::right;
	if (face == Face::front && corner == Corner::left && dZ > 0) return Face::bottom;

	if (face == Face::front && corner == Corner::bottom && dZ < 0) return Face::right;
	if (face == Face::front && corner == Corner::right && dZ < 0) return Face::bottom;
	if (face == Face::front && corner == Corner::top && dZ < 0) return Face::left;
	if (face == Face::front && corner == Corner::left && dZ < 0) return Face::top;


	if (face == Face::back && corner == Corner::bottom && dZ > 0) return Face::right;
	if (face == Face::back && corner == Corner::right && dZ > 0) return Face::bottom;
	if (face == Face::back && corner == Corner::top && dZ > 0) return Face::left;
	if (face == Face::back && corner == Corner::left && dZ > 0) return Face::top;

	if (face == Face::back && corner == Corner::bottom && dZ < 0) return Face::left;
	if (face == Face::back && corner == Corner::right && dZ < 0) return Face::top;
	if (face == Face::back && corner == Corner::top && dZ < 0) return Face::right;
	if (face == Face::back && corner == Corner::left && dZ < 0) return Face::bottom;
}