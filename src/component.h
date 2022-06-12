#ifndef COMPONENT_H
#define COMPONENT_H

#include <map>
#include <glm/glm.hpp>

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

class PositionComponent : public Component
{
public:
	glm::vec3 position;
	glm::vec3 rotation;

	void SetRotation(glm::vec3 rotation);
	PositionComponent(Entity* entity, bool active, glm::vec3 position, glm::vec3 rotation);
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

	InputComponent(Entity* entity, bool active, bool acceptInput, float rollDelay);
};

class CameraFollowComponent : public Component
{
public:
	glm::vec3 offset;
	float distance;

	float speed;
	bool track;
	bool resetting;

	bool lockX;
	bool lockY;
	bool lockZ;

	CameraFollowComponent(Entity* entity, bool active, glm::vec3 offset, float distance, float speed, bool track, bool lockX, bool lockY, bool lockZ);
};

class BillboardingComponent : public Component
{
public:
	BillboardingComponent(Entity* entity, bool active);
};

enum class Face { front, back, left, right, top, bottom };
class ActorComponent : public Component
{
public:
	Face face;
	Entity* cube;

	float speed;

	glm::vec3 baseRotation;

	void SetRotation(glm::vec3 rotation);
	ActorComponent(Entity* entity, bool active, float speed, Face face, Entity* cube);
};

enum class MovementType { linear, curve, rotation };

struct Movement
{
	int ID;

	MovementType movementType;
	float speed;
	glm::vec3 target;

	float radius;

	glm::vec3 pivot;

	bool operator==(const Movement& rhs) const noexcept
	{
		return (this->ID == rhs.ID);
	}
};

class MovementComponent : public Component
{
public:
	bool moving;
	std::vector<Movement> queue;

	void RegisterMovement(float speed, glm::vec3 target, float radius, glm::vec3 pivot);
	void RegisterMovement(float speed, glm::vec3 target);
	void RegisterRotation(float speed, glm::vec3 target);
	MovementComponent(Entity* entity, bool active);
};

#endif