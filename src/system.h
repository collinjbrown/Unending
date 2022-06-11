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

class System
{
public:
	virtual void Update(int activeScene, float deltaTime) = 0;
	virtual void AddComponent(Component* component) = 0;
	virtual void PurgeEntity(Entity* e) = 0;
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

#endif