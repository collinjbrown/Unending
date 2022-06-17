#ifndef COMPONENT_H
#define COMPONENT_H

#include <iostream>
#include <map>

#include "util.h"
#include "texture.h"
#include "animation.h"

class Entity;

static int positionComponentID				= 1;
static int cubeComponentID					= 2;
static int animationComponentID				= 3;
static int animationControllerComponentID	= 4;
static int inputComponentID					= 5;
static int cameraFollowComponentID			= 6;
static int billboardingComponentID			= 7;
static int actorComponentID					= 8;
static int movementComponentID				= 9;

static int playerAnimControllerSubID		= 1;

class Component
{
public:
	bool active;
	Entity* entity;
	int ID;
};

struct Quaternion
{
	float w;
	float x;
	float y;
	float z;

	Quaternion operator*(const Quaternion& rhs) const noexcept
	{
		Quaternion q = { 0, 0, 0, 0 };
		q.w = (this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z);
		q.x = (this->w * rhs.x + this->x * rhs.w + this->y * rhs.z - this->z * rhs.y);
		q.y = (this->w * rhs.y - this->x * rhs.z + this->y * rhs.w + this->z * rhs.x);
		q.z = (this->w * rhs.z + this->x * rhs.y - this->y * rhs.x + this->z * rhs.w);
		return q;
	}

	bool operator==(const Quaternion& rhs) const noexcept
	{
		return ((this->w == rhs.w) && (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z));
	}
};

struct BezierCurve
{
	std::vector<glm::vec3> points;

	glm::vec3 GetPoint(float t)
	{
		std::vector<glm::vec3> ps = points;
		std::vector<glm::vec3> tmp;

		for (int i = 0; i < points.size(); i++)
		{
			for (int j = 0; j < ps.size() - 1; j++)
			{
				tmp.push_back(Util::Lerp(ps[j], ps[j + 1], t));
			}

			ps = tmp;
			tmp.clear();

			if (ps.size() == 1)
			{
				return ps[0];
			}
		}

		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
};

struct BezierQuaternion
{
	std::vector<Quaternion> rotations;

	Quaternion GetQuaternion(float t)
	{
		std::vector<Quaternion> qs = rotations;
		std::vector<Quaternion> tmp;

		for (int i = 0; i < rotations.size(); i++)
		{
			for (int j = 0; j < qs.size() - 1; j++)
			{
				tmp.push_back(Util::Slerp(qs[j], qs[j + 1], t));
			}

			qs = tmp;
			tmp.clear();

			if (qs.size() == 1)
			{
				return qs[0];
			}
		}

		return { 1.0f, 0.0f, 0.0f, 0.0f };
	}
};

class PositionComponent : public Component
{
public:
	glm::vec3 position;
	Quaternion quaternion;

	void SetRotation(Quaternion rotation);
	PositionComponent(Entity* entity, bool active, glm::vec3 position, Quaternion quaternion);
};

class CubeComponent : public Component
{
public:
	int x;
	int y;
	int z;

	glm::vec3 size;
	glm::vec4 color;

	Texture* texture;

	bool checked;

	CubeComponent(Entity* entity, bool active, int x, int y, int z, glm::vec3 size, glm::vec4 color, Texture* texture);
};

class AnimationComponent : public Component
{
public:
	glm::vec3 baseOffset;
	glm::vec3 offset;

	int activeX;
	int activeY;

	std::string activeAnimation;
	std::map<std::string, Animation*> animations;

	float lastTick;

	float scaleX;
	float scaleY;

	bool flippedX;
	bool flippedY;

	glm::vec4 color;

	void SetAnimation(std::string s);

	void AddAnimation(std::string s, Animation* anim);

	AnimationComponent(Entity* entity, bool active, glm::vec3 baseOffset, Animation* idleAnimation, std::string animationName, float scaleX, float scaleY, bool flippedX, bool flippedY, glm::vec4 color);
};

class AnimationControllerComponent : public Component
{
public:
	AnimationComponent* animator;
	int subID;
};

class PlayerAnimationControllerComponent : public AnimationControllerComponent
{
public:
	PlayerAnimationControllerComponent(Entity* entity, bool active, AnimationComponent* animator);
};

class InputComponent : public Component
{
public:
	bool acceptInput;

	float lastRoll;
	float rollDelay;

	float lastTurn;
	float turnDelay;

	InputComponent(Entity* entity, bool active, bool acceptInput, float rollDelay, float turnDelay);
};

class CameraFollowComponent : public Component
{
public:
	Quaternion rotation;
	float distance;

	float speed;
	bool track;
	bool resetting;

	bool lockX;
	bool lockY;
	bool lockZ;

	CameraFollowComponent(Entity* entity, bool active, Quaternion rotation, float distance, float speed, bool track, bool lockX, bool lockY, bool lockZ);
};

class BillboardingComponent : public Component
{
public:
	BillboardingComponent(Entity* entity, bool active);
};

enum class Face { front, back, left, right, top, bottom };
enum class Corner { left, bottom, right, top };
class ActorComponent : public Component
{
public:
	Face face;
	Entity* cube; 

	float speed;

	Quaternion baseQuaternion;

	ActorComponent(Entity* entity, bool active, float speed, Face face, Entity* cube);
}; 

enum class MovementType { linear, bezier, rotation, bezierRotation };

struct Movement
{
	int ID;

	MovementType movementType;
	float speed;

	bool operator==(const Movement& rhs) const noexcept
	{
		return (this->ID == rhs.ID);
	}

	bool operator==(const Movement* rhs) const noexcept
	{
		return (this->ID == rhs->ID);
	}
};

struct LinearMovement : public Movement
{
	glm::vec3 target;
};

struct BezierMovement : public Movement
{
	BezierCurve curve;
	float t = 0.0f;
	float targetT = 1.0f;
};

struct RotatingMovement : public Movement
{
	Quaternion targetRotation;
};

struct BezierRotatingMovement : public Movement
{
	BezierQuaternion curve;
	float t = 0.0f;
	float targetT = 1.0f;
};

class MovementComponent : public Component
{
public:
	bool moving;
	std::vector<Movement*> queue;

	void RegisterMovement(float speed, glm::vec3 target);
	void RegisterMovement(float speed, Quaternion target);
	void RegisterMovement(float speed, BezierCurve curve, float targetT);
	void RegisterMovement(float speed, BezierQuaternion curve, float targetT);
	MovementComponent(Entity* entity, bool active);
};

#endif