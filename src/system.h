#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>

class Component;
class PositionComponent;
class CubeComponent;
class AnimationComponent;
class AnimationControllerComponent;
class InputComponent;
class CameraFollowComponent;
class BillboardingComponent;
class ActorComponent;
class MovementComponent;

class System
{
public:
	virtual void Update(int activeScene, float deltaTime) = 0;
	virtual void AddComponent(Component* component) = 0;
	virtual void PurgeEntity(Entity* e) = 0;
};

struct Chunk
{
	int count;

	int cols;	// x
	int rows;	// y
	int aisles;	// z

	glm::vec3 size;
	glm::vec3 position;
	Quaternion quaternion;
	glm::vec4 color;
	int textureID;

	Chunk()
	{
		count = 1;

		cols = 1;
		rows = 1;
		aisles = 1;

		size = { 0.0f, 0.0f, 0.0f };
		position = { 0.0f, 0.0f, 0.0f };
		quaternion = { 1.0f, 0.0f, 0.0f, 0.0f };
		color = { 0.0f, 0.0f, 0.0f, 0.0f };

		textureID = -1;
	}
};

class CubeSystem : public System
{
public:
	std::vector<CubeComponent*> cubes;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class AnimationControllerSystem : public System
{
public:
	std::vector<AnimationControllerComponent*> controllers;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class AnimationSystem : public System
{
public:
	std::vector<AnimationComponent*> anims;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class CameraFollowSystem : public System
{
public:
	std::vector<CameraFollowComponent*> cams;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class InputSystem : public System
{
public:
	std::vector<InputComponent*> inputs;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class BillboardingSystem : public System
{
public:
	std::vector<BillboardingComponent*> boards;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class TurnSystem : public System
{
	std::vector<ActorComponent*> actors;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

class MovementSystem : public System
{
	std::vector<MovementComponent*> movers;

	void Update(int activeScene, float deltaTime);

	void AddComponent(Component* component);

	void PurgeEntity(Entity* e);
};

#endif