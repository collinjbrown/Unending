#ifndef ECS_H
#define ECS_H

#include <string>
#include <vector>
#include <map>

class Entity;
class System;
class Component;

class ComponentBlock
{
public:
	System* system;
	int componentID;

	void Update(int activeScene, float deltaTime);
	void AddComponent(Component* c);
	void PurgeEntity(Entity* e);
	ComponentBlock(System* system, int componentID);
};

class ECS
{
private:
	uint32_t entityIDCounter = 0;
	int round = 0;

public:
	static ECS main;
	int activeScene = 0;

	Entity* player;

	std::vector<Entity*> entities;
	std::vector<Entity*> dyingEntities;

	std::vector<ComponentBlock*> componentBlocks;

	uint32_t GetID();
	void Init();
	void Update(float deltaTime);

	Entity* CreateEntity(int scene, std::string name);
	void DeleteEntity(Entity* e);
	void AddDeadEntity(Entity* e);
	void PurgeDeadEntities();

	void RegisterComponent(Component* component, Entity* entity);
};

#endif