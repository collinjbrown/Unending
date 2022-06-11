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

	PositionComponent(Entity* entity, bool active, glm::vec3 position, glm::vec3 rotation);
};

class CubeComponent : public Component
{
public:
	glm::vec3 size;
	glm::vec4 color;

	Texture* texture;

	CubeComponent(Entity* entity, bool active, glm::vec3 size, glm::vec4 color, Texture* texture);
};

class AnimationComponent : public Component
{
public:
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

	AnimationComponent(Entity* entity, bool active, glm::vec3 offset, Animation* idleAnimation, std::string animationName, float scaleX, float scaleY, bool flippedX, bool flippedY, glm::vec4 color);
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

	InputComponent(Entity* entity, bool active, bool acceptInput);
};

class CameraFollowComponent : public Component
{
public:
	glm::vec3 offset;
	float distance;

	float speed;
	bool track;

	bool lockX;
	bool lockY;
	bool lockZ;

	CameraFollowComponent(Entity* entity, bool active, glm::vec3 offset, float distance, float speed, bool track, bool lockX, bool lockY, bool lockZ);
};

#endif