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
	uint32_t otherIDCounter = 0;
	int round = 0;

public:
	static ECS main;
	int activeScene = 0;

	static const int cubeSize = 10;

	static const int maxWidth = 100;
	static const int maxHeight = 100;
	static const int maxDepth = 100;

	Entity* player;
	Entity* cubes[maxWidth][maxHeight][maxDepth];

	std::vector<Entity*> entities;
	std::vector<Entity*> dyingEntities;

	std::vector<ComponentBlock*> componentBlocks;

	uint32_t GetID();
	uint32_t GetOtherID();
	void Init();
	void Update(float deltaTime);

	Entity* CreateEntity(int scene, std::string name);
	void DeleteEntity(Entity* e);
	void AddDeadEntity(Entity* e);
	void PurgeDeadEntities();

	void RegisterComponent(Component* component, Entity* entity);

	glm::vec3 CubeToWorldSpace(int x, int y, int z);
	glm::vec3 WorldToCubeSpace(glm::vec3 position);

	Entity* GetCube(int x, int y, int z);
	void MoveCube(CubeComponent* cube, int x, int y, int z);
	void PositionCube(CubeComponent* cube, int x, int y, int z);
	void PositionActor(ActorComponent* actor);

	void MoveActor(ActorComponent* actor, int dX, int dY, int dZ);

	// bool CheckAutofrendation(CubeComponent* activeCube, Face activeFace, Face rollDirection); // Self-Crushing

	void FloodFill(std::vector<CubeComponent*>& inside, CubeComponent* cube, CubeComponent* activeCube, CubeComponent* fulcrum);
	std::vector<CubeComponent*> DetermineStructure(CubeComponent* cube, CubeComponent* fulcrum, Face direction);

	std::pair<Face, bool> FindFulcrum(CubeComponent* activeCube, Face activeFace, Face rollDirection);
	Face DetermineRollDirection(Face fulcrum, Face activeFace, Face rollDirection);

	void QuarterRoll(ActorComponent* actor, Face standingFace, Face rollDirection, Entity* landingTarget, Face landingFace, std::vector<CubeComponent*> affectedCubes);
	void HalfRoll(ActorComponent* actor, Face standingFace, Face oppFulcrum, Face rollDirection, Entity* landingTarget, Face landingFace, std::vector<CubeComponent*> affectedCubes);

	void RollCube(ActorComponent* actor, Face rollDirection);

	void RollActor(ActorComponent* actor, Face rollDirection, Face landingFace, Face standingFace, bool half);
};

#endif