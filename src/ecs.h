#ifndef ECS_H
#define ECS_H

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <map>

class Entity;
class System;
class Component;

class ActorComponent;
class CubeComponent;
enum class Face;

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

	static const int maxWidth = 50;
	static const int maxHeight = 50;
	static const int maxDepth = 50;

	Entity* player;
	Entity* cubes[maxWidth][maxHeight][maxDepth];

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

	Entity* GetCube(int x, int y, int z);
	void MoveCube(CubeComponent* cube, int x, int y, int z);
	void PositionCube(CubeComponent* cube, int x, int y, int z);

	Face GetAbsoluteFace(Face relativeUp, Face pseudoForward);
	glm::vec3 GetRelativeUp(Face face);

	void MoveActor(ActorComponent* actor, int dX, int dY, int dZ);

	void FloodFill(std::vector<CubeComponent*>& inside, CubeComponent* cube);
	std::vector<CubeComponent*> DetermineStructure(CubeComponent* cube, Face direction);

	void RollCube(ActorComponent* actor, Face rollDirection);
	void RollCube(CubeComponent* cube, Face rollDirection);
	glm::vec3 GetLandingCoords(Face activeFace, Face rollDirection);

	void RollActor(ActorComponent* actor, Face rollDirection);
};

#endif