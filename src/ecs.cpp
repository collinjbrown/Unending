#include "ecs.h"

#include <algorithm>
#include <iostream>

#include "game.h"
#include "system.h"
#include "component.h"
#include "entity.h"
#include "util.h"
#include <glm/gtx/norm.hpp>

#pragma region Map

Entity* ECS::GetCube(int x, int y, int z)
{
	if (x >= 0 && x < maxWidth &&
		y >= 0 && y < maxHeight &&
		z >= 0 && z < maxDepth)
	{
		return ECS::main.cubes[x][y][z];
	}
	else
	{
		return nullptr;
	}
}

glm::vec3 ECS::GetLandingCoords(Face activeFace, Face rollDirection)
{
	glm::vec3 difference = glm::vec3(0, 1, 1);

	if (activeFace == Face::top && rollDirection == Face::back)				difference	= {	0,	-1,	-1	};
	else if (activeFace == Face::top && rollDirection == Face::front)		difference	= { 0,	-1,	1	};
	else if (activeFace == Face::top && rollDirection == Face::left)		difference	= { -1,	-1,	0	};
	else if (activeFace == Face::top && rollDirection == Face::right)		difference	= { 1,	-1,	0	};
	else if (activeFace == Face::bottom && rollDirection == Face::back)		difference	= { 0,	1,	-1	};
	else if (activeFace == Face::bottom && rollDirection == Face::front)	difference	= { 0,	1,	1	};
	else if (activeFace == Face::bottom && rollDirection == Face::left)		difference	= { -1,	1,	0	};
	else if (activeFace == Face::bottom && rollDirection == Face::right)	difference	= { 1,	1,	0	};
	else if (activeFace == Face::right && rollDirection == Face::back)		difference	= { -1,	0,	1	};
	else if (activeFace == Face::right && rollDirection == Face::front)		difference	= { -1,	0,	1	};
	else if (activeFace == Face::right && rollDirection == Face::top)		difference	= { -1,	1,	0	};
	else if (activeFace == Face::right && rollDirection == Face::bottom)	difference	= { -1,	-1,	0	};
	else if (activeFace == Face::left && rollDirection == Face::back)		difference	= { 1,	0,	1	};
	else if (activeFace == Face::left && rollDirection == Face::front)		difference	= { 1,	0,	1	};
	else if (activeFace == Face::left && rollDirection == Face::top)		difference	= { 1,	1,	0	};
	else if (activeFace == Face::left && rollDirection == Face::bottom)		difference	= { 1,	-1,	0	};

	else if (activeFace == Face::front && rollDirection == Face::left)		difference	= { -1,	0,	1	};
	else if (activeFace == Face::front && rollDirection == Face::right)		difference	= { 1,	0,	1	};
	else if (activeFace == Face::front && rollDirection == Face::top)		difference	= { 0,	1,	1	};
	else if (activeFace == Face::front && rollDirection == Face::bottom)	difference	= { 0,	-1,	1	};

	else if (activeFace == Face::back && rollDirection == Face::left)		difference	= { -1,	0,	-1	};
	else if (activeFace == Face::back && rollDirection == Face::right)		difference	= { 1,	0,	-1	};
	else if (activeFace == Face::back && rollDirection == Face::top)		difference	= { 0,	1,	-1	};
	else if (activeFace == Face::back && rollDirection == Face::bottom)		difference	= { 0,	-1,	-1	};

	return difference;
}

glm::vec3 ECS::GetRelativeUp(Face face)
{
	glm::vec3 relativeUp = glm::vec3(0, 1, 0);

	if (face == Face::front) relativeUp = { 0, 0, -1 };
	if (face == Face::back) relativeUp = { 0, 0, 1 };
	if (face == Face::left) relativeUp = { -1, 0, 0 };
	if (face == Face::right) relativeUp = { 1, 0, 0 };
	if (face == Face::bottom) relativeUp = { 0, -1, 0 };

	return relativeUp;
}

void ECS::MoveActor(ActorComponent* actor, int dX, int dY, int dZ)
{
	CubeComponent* currentCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];

	Entity* target = GetCube(currentCube->x + dX, currentCube->y + dY, currentCube->z + dZ);
	if (target != nullptr)
	{
		glm::vec3 relativeUp = GetRelativeUp(actor->face);

		Entity* targetUp = GetCube(currentCube->x + dX + (int)relativeUp.x, currentCube->y + dY + (int)relativeUp.y, currentCube->z + dZ + (int)relativeUp.z);

		if (targetUp == nullptr && target != nullptr)
		{
			CubeComponent* targetCube = (CubeComponent*)target->componentIDMap[cubeComponentID];
			PositionComponent* targetPos = (PositionComponent*)target->componentIDMap[positionComponentID];

			actor->cube = target;

			actor->target = targetPos->position + ((targetCube->size.y / 2.0f) * relativeUp);
			actor->moving = true;
		}
	}
}

void ECS::FloodFill(std::vector<CubeComponent*> &inside, CubeComponent* cube)
{
	auto result = std::find(inside.rbegin(), inside.rend(), cube);
	int location;

	if (result != inside.rend())
	{
		location = inside.rend() - result;
	}
	else
	{
		location = -1;
	}

	if (location != -1)
	{
		return;
	}
	else
	{
		inside.push_back(cube);

		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				for (int z = -1; z <= 1; z++)
				{
					if ((x != 0 || y != 0 || z != 0) &&
						(abs(x) + abs(y) + abs(z) == 1))
					{
						// There has to be a better way to do that. Like, almost certainly.

						Entity* activeCubeEntity = GetCube(cube->x + x, cube->y + y, cube->z + z);

						if (activeCubeEntity != nullptr)
						{
							CubeComponent* nextCube = (CubeComponent*)activeCubeEntity->componentIDMap[cubeComponentID];
							FloodFill(inside, nextCube);
						}
					}
				}
			}
		}
	}
}

std::vector<CubeComponent*> ECS::DetermineStructure(CubeComponent* cube, Face direction)
{
	std::vector<CubeComponent*> retCubes;
	retCubes.push_back(cube);

	glm::vec3 startDirection = GetRelativeUp(direction);

	Entity* activeCubeEntity = GetCube(cube->x + (int)startDirection.x, cube->y + (int)startDirection.y, cube->z + (int)startDirection.z);

	if (activeCubeEntity == nullptr)
	{
		return retCubes;
	}

	CubeComponent* activeCube = (CubeComponent*)activeCubeEntity->componentIDMap[cubeComponentID];

	FloodFill(retCubes, activeCube);

	if (retCubes.size() < 4)
	{
		return retCubes;
	}

	int touchingSides = 0;

	for (int i = 0; i < retCubes.size(); i++)
	{
		CubeComponent* c = retCubes[i];
		if (c != cube)
		{
			int diff = abs((cube->x - c->x) + (cube->y - c->y) + (cube->z - c->z));

			if (diff < 3)
			{
				touchingSides++;
			}
		}
	}

	if (touchingSides > 2)
	{
		retCubes.clear();
		return retCubes;
	}
}

void ECS::RollCube(ActorComponent* actor, Face rollDirection)
{
	CubeComponent* activeCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];
	std::vector<CubeComponent*> affectedCubes = DetermineStructure(activeCube, rollDirection);

	if (affectedCubes.size() > 0)
	{
		// There are cubes to roll.
		std::cout << "Flood Fill: " + std::to_string(affectedCubes.size()) << std::endl;

		CubeComponent* landingCube = GetLandingCube(activeCube->x, activeCube->y, activeCube->z, actor->face, rollDirection);
		// if (landingCube)
	}
}

void ECS::RollCube(CubeComponent* cube, Face rollDirection)
{

}

#pragma endregion

#pragma region Entities

int Entity::GetID() { return ID; }
int Entity::GetScene() { return scene; }
std::string Entity::GetName() { return name;  }

void Entity::SetID(int ID) { this->ID = ID; }
void Entity::SetScene(int scene) { this->scene = scene; }
void Entity::SetName(std::string name) { this->name = name; }

Entity::Entity(int ID, int scene, std::string name)
{
	this->ID = ID;
	this->scene = scene;
	this->name = name;
};

#pragma endregion

#pragma region Component Blocks

void ComponentBlock::Update(int activeScene, float deltaTime)
{
	system->Update(activeScene, deltaTime);
}
void ComponentBlock::AddComponent(Component* c)
{
	system->AddComponent(c);
}
void ComponentBlock::PurgeEntity(Entity* e)
{
	system->PurgeEntity(e);
}
ComponentBlock::ComponentBlock(System* system, int componentID)
{
	this->system = system;
	this->componentID = componentID;
}

#pragma endregion

#pragma region ECS

uint32_t ECS::GetID()
{
	return ++entityIDCounter;
}

void ECS::Init()
{
	componentBlocks.push_back(new ComponentBlock(new InputSystem(), inputComponentID));
	componentBlocks.push_back(new ComponentBlock(new CubeSystem(), cubeComponentID));
	componentBlocks.push_back(new ComponentBlock(new AnimationControllerSystem(), animationControllerComponentID));
	componentBlocks.push_back(new ComponentBlock(new AnimationSystem(), animationComponentID));
	componentBlocks.push_back(new ComponentBlock(new BillboardingSystem(), billboardingComponentID));
	componentBlocks.push_back(new ComponentBlock(new CameraFollowSystem(), cameraFollowComponentID));
	componentBlocks.push_back(new ComponentBlock(new TurnSystem(), actorComponentID));
}

void ECS::Update(float deltaTime)
{
	if (round < 2)
	{
		round++;
	}

	if (round == 1)
	{
		float cubeSize = 10.0f;
		int mapWidth = 5;
		int mapDepth = 5;
		float mapMidX = (mapWidth / 2.0f) * cubeSize;
		float mapMidZ = (mapDepth / 2.0f) * cubeSize;

		for (int x = 0; x < mapWidth; x++)
		{
			for (int z = 0; z < mapDepth; z++)
			{
				Entity* cube = CreateEntity(0, "Cube: " + std::to_string(x) + " / " + std::to_string(z));
				ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3((10.0f * x) - mapMidX, 0.0f, (-10.f * z) + mapMidZ), glm::vec3(0.0f, 0.0f, 0.0f)), cube);
				ECS::main.RegisterComponent(new CubeComponent(cube, true, x, 0, z, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["block"]), cube);

				ECS::main.cubes[x][0][z] = cube;
			}
		}

		Entity* cube = CreateEntity(0, "Cube: " + std::to_string(mapWidth - 1) + " / " + std::to_string(mapDepth));
		ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3((10.0f * (mapWidth - 1)) - mapMidX, 0.0f, (-10.f * (mapDepth)) + mapMidZ), glm::vec3(0.0f, 0.0f, 0.0f)), cube);
		ECS::main.RegisterComponent(new CubeComponent(cube, true, mapWidth - 1, 0, mapDepth, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["block"]), cube);

		ECS::main.cubes[mapWidth - 1][0][mapDepth] = cube;

		player = CreateEntity(0, "Player");
		Animation* testIdle = Game::main.animationMap["testIdle"];

		ECS::main.RegisterComponent(new PositionComponent(player, true, glm::vec3((10.0f * (mapWidth - 1)) - mapMidX, cubeSize / 2.0f, (-10.f * (mapDepth - 1)) + mapMidZ), glm::vec3(0.0f, 0.0f, 0.0f)), player);
		ECS::main.RegisterComponent(new AnimationComponent(player, true, glm::vec3(0.0f, (testIdle->height / testIdle->rows) * 0.5f * 0.4f, 0.0f), testIdle, "idle", 0.2f, 0.2f, false, false, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), player);
		AnimationComponent* a = (AnimationComponent*)player->componentIDMap[animationComponentID];
		ECS::main.RegisterComponent(new PlayerAnimationControllerComponent(player, true, a), player);
		ECS::main.RegisterComponent(new BillboardingComponent(player, true), player);
		ECS::main.RegisterComponent(new CameraFollowComponent(player, true, glm::vec3(-295.0f, 615.0f, 1760.0f), 500.0f, 10.0f, true, false, false, false), player);
		ECS::main.RegisterComponent(new InputComponent(player, true, true, 0.5f), player);
		ECS::main.RegisterComponent(new ActorComponent(player, true, 10.0f, Face::top, ECS::main.cubes[mapWidth - 1][0][mapDepth - 1]), player);
	}

	for (int i = 0; i < componentBlocks.size(); i++)
	{
		componentBlocks[i]->Update(activeScene, deltaTime);
	}

	PurgeDeadEntities();
}

void ECS::AddDeadEntity(Entity* e)
{
	if (std::find(dyingEntities.begin(), dyingEntities.end(), e) == dyingEntities.end())
	{
		dyingEntities.push_back(e);
	}
}

void ECS::PurgeDeadEntities()
{
	if (dyingEntities.size() > 0)
	{
		int n = dyingEntities.size();

		for (int i = 0; i < n; i++)
		{
			DeleteEntity(dyingEntities[i]);
		}

		dyingEntities.clear();
	}
}

Entity* ECS::CreateEntity(int scene, std::string name)
{
	Entity* e = new Entity(GetID(), scene, name);
	return e;
}

void ECS::DeleteEntity(Entity* e)
{
	for (int i = 0; i < componentBlocks.size(); i++)
	{
		componentBlocks[i]->PurgeEntity(e);
	}

	delete e;
}

void ECS::RegisterComponent(Component* component, Entity* entity)
{
	entity->components.push_back(component);
	entity->componentIDMap.emplace(component->ID, component);

	for (int i = 0; i < componentBlocks.size(); i++)
	{
		if (componentBlocks[i]->componentID == component->ID)
		{
			componentBlocks[i]->AddComponent(component);
			return;
		}
	}
}

#pragma endregion

#pragma region Components

#pragma region Position Component

PositionComponent::PositionComponent(Entity* entity, bool active, glm::vec3 position, glm::vec3 rotation)
{
	this->ID = positionComponentID;
	this->entity = entity;
	this->active = active;

	this->position = position;
	this->rotation = rotation;
}

#pragma endregion

#pragma region Cube Component

CubeComponent::CubeComponent(Entity* entity, bool active, int x, int y, int z, glm::vec3 size, glm::vec4 color, Texture* texture)
{
	this->ID = cubeComponentID;
	this->entity = entity;
	this->active = active;

	this->x = x;
	this->y = y;
	this->z = z;

	this->size = size;
	this->color = color;
	this->texture = texture;
}

#pragma endregion

#pragma region Animation Component


void AnimationComponent::SetAnimation(std::string s)
{
	if (animations[s] != NULL)
	{
		activeAnimation = s;
		activeX = 0;
		activeY = animations[s]->rows - 1;
		lastTick = 0;
	}
}

void AnimationComponent::AddAnimation(std::string s, Animation* anim)
{
	animations.emplace(s, anim);
}

AnimationComponent::AnimationComponent(Entity* entity, bool active, glm::vec3 offset, Animation* idleAnimation, std::string animationName, float scaleX, float scaleY, bool flippedX, bool flippedY, glm::vec4 color)
{
	this->ID = animationComponentID;
	this->entity = entity;
	this->active = active;

	this->offset = offset;

	lastTick = 0;
	activeX = 0;
	activeY = 0;

	this->scaleX = scaleX;
	this->scaleY = scaleY;

	this->flippedX = flippedX;
	this->flippedY = flippedY;

	this->color = color;

	activeAnimation = animationName;
	animations.emplace(animationName, idleAnimation);
	activeY = animations[activeAnimation]->rows - 1;
}

#pragma endregion

#pragma region Player Animation Controller Component

PlayerAnimationControllerComponent::PlayerAnimationControllerComponent(Entity* entity, bool active, AnimationComponent* animator)
{
	this->ID = animationControllerComponentID;
	this->subID = playerAnimControllerSubID;
	this->entity = entity;
	this->active = active;

	this->animator = animator;
}

#pragma endregion

#pragma region Input Component

InputComponent::InputComponent(Entity* entity, bool active, bool acceptInput, float rollDelay)
{
	this->ID = inputComponentID;
	this->entity = entity;
	this->active = active;

	this->acceptInput = acceptInput;
	this->rollDelay = rollDelay;
	this->lastRoll = 0.0f;
}

#pragma endregion

#pragma region Camera Follow Component

CameraFollowComponent::CameraFollowComponent(Entity* entity, bool active, glm::vec3 offset, float distance, float speed, bool track, bool lockX, bool lockY, bool lockZ)
{
	this->ID = cameraFollowComponentID;
	this->entity = entity;
	this->active = active;

	this->offset = offset;
	this->distance = distance;

	this->resetting = false;

	this->speed = speed;
	this->track = track;

	this->lockX = lockX;
	this->lockY = lockY;
	this->lockZ = lockZ;
}

#pragma endregion

#pragma region Billboarding Component

BillboardingComponent::BillboardingComponent(Entity* entity, bool active)
{
	this->ID = billboardingComponentID;
	this->entity = entity;
	this->active = active;
}

#pragma endregion

#pragma region Actor Component

ActorComponent::ActorComponent(Entity * entity, bool active, float speed, Face face, Entity * cube)
{
	this->ID = actorComponentID;
	this->entity = entity;
	this->active = active;

	this->speed = speed;

	this->face = face;
	this->cube = cube;

	this->moving = false;
	this->target = glm::vec3(0, 0, 0);

	this->baseRotation = glm::vec3(0, 0, 0);

	if (this->face == Face::front)		this->baseRotation = { 3.2f,	0,		0		};
	if (this->face == Face::back)		this->baseRotation = { -3.2f,	0,		0		};
	if (this->face == Face::bottom)		this->baseRotation = { 0,		3.2f,	0		};
	if (this->face == Face::right)		this->baseRotation = { 0,		0,		3.2f	};
	if (this->face == Face::left)		this->baseRotation = { 0,		0,		-3.2f	};
}

#pragma endregion

#pragma endregion

#pragma region Systems

#pragma region Cube System

void CubeSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < cubes.size(); i++)
	{
		CubeComponent* cube = cubes[i];

		if (cube->active && cube->entity->GetScene() == activeScene ||
			cube->active && cube->entity->GetScene() == 0)
		{
			PositionComponent* pos = (PositionComponent*)cube->entity->componentIDMap[positionComponentID];

			Game::main.renderer->PrepareCube(cube->size, pos->position, pos->rotation, cube->color, cube->texture->ID);
		}
	}
}

void CubeSystem::AddComponent(Component* component)
{
	cubes.push_back((CubeComponent*)component);
}

void CubeSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < cubes.size(); i++)
	{
		if (cubes[i]->entity == e)
		{
			CubeComponent* s = cubes[i];
			cubes.erase(std::remove(cubes.begin(), cubes.end(), s), cubes.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Animation System
void AnimationSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < anims.size(); i++)
	{
		AnimationComponent* a = anims[i];

		if (a->active && a->entity->GetScene() == activeScene ||
			a->active && a->entity->GetScene() == 0)
		{
			a->lastTick += deltaTime;

			Animation* activeAnimation = a->animations[a->activeAnimation];

			int cellX = a->activeX, cellY = a->activeY;

			if (activeAnimation->speed < a->lastTick)
			{
				a->lastTick = 0;

				if (a->activeX + 1 < activeAnimation->layout[cellY])
				{
					cellX = a->activeX += 1;
				}
				else
				{
					if (activeAnimation->loop ||
						a->activeY > 0)
					{
						cellX = a->activeX = 0;
					}

					if (a->activeY - 1 >= 0)
					{
						cellY = a->activeY -= 1;
					}
					else if (activeAnimation->loop)
					{
						cellX = a->activeX = 0;
						cellY = a->activeY = activeAnimation->rows - 1;
					}
				}
			}

			PositionComponent* pos = (PositionComponent*)a->entity->componentIDMap[positionComponentID];
			
			Game::main.renderer->PrepareQuad(glm::vec2(activeAnimation->width * a->scaleX, activeAnimation->height * a->scaleY), a->offset + pos->position, pos->rotation, a->color, activeAnimation->ID, cellX, cellY, activeAnimation->columns, activeAnimation->rows, a->flippedX, a->flippedY);
		}
	}
}
void AnimationSystem::AddComponent(Component* component)
{
	anims.push_back((AnimationComponent*)component);
}

void AnimationSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < anims.size(); i++)
	{
		if (anims[i]->entity == e)
		{
			AnimationComponent* s = anims[i];
			anims.erase(std::remove(anims.begin(), anims.end(), s), anims.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Animation Controller System

void AnimationControllerSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < controllers.size(); i++)
	{
		AnimationControllerComponent* c = controllers[i];

		if (c->active && c->entity->GetScene() == activeScene ||
			c->active && c->entity->GetScene() == 0)
		{
			if (c->subID == playerAnimControllerSubID)
			{
				// Baba booey.
			}
		}
	}
}

void AnimationControllerSystem::AddComponent(Component* component)
{
	controllers.push_back((AnimationControllerComponent*)component);
}

void AnimationControllerSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < controllers.size(); i++)
	{
		if (controllers[i]->entity == e)
		{
			AnimationControllerComponent* s = controllers[i];
			controllers.erase(std::remove(controllers.begin(), controllers.end(), s), controllers.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Camera Follow System

void CameraFollowSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < cams.size(); i++)
	{
		CameraFollowComponent* c = cams[i];

		if (c->track)
		{
			if (c->active && c->entity->GetScene() == activeScene ||
				c->active && c->entity->GetScene() == 0)
			{
				PositionComponent* pos = (PositionComponent*)c->entity->componentIDMap[positionComponentID];

				glm::vec3 normalizedOffset = glm::normalize(Util::Rotate(c->offset, pos->rotation));

				Game::main.cameraPosition = Util::Lerp(Game::main.cameraPosition, pos->position + (normalizedOffset * c->distance), deltaTime * c->speed);
			}
		}
	}
}

void CameraFollowSystem::AddComponent(Component* component)
{
	cams.push_back((CameraFollowComponent*)component);
}

void CameraFollowSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < cams.size(); i++)
	{
		if (cams[i]->entity == e)
		{
			CameraFollowComponent* s = cams[i];
			cams.erase(std::remove(cams.begin(), cams.end(), s), cams.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Input System

void InputSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < inputs.size(); i++)
	{
		InputComponent* input = inputs[i];

		if (input->active && input->entity->GetScene() == activeScene ||
			input->active && input->entity->GetScene() == 0)
		{
			// Player Controlling
			ActorComponent* actor = (ActorComponent*)input->entity->componentIDMap[actorComponentID];

			bool moveForward = ((glfwGetKey(Game::main.window, Game::main.moveForwardKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveForwardKey) == GLFW_PRESS));
			bool moveBack = ((glfwGetKey(Game::main.window, Game::main.moveBackKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveBackKey) == GLFW_PRESS));
			bool moveRight = ((glfwGetKey(Game::main.window, Game::main.moveRightKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveRightKey) == GLFW_PRESS));
			bool moveLeft = ((glfwGetKey(Game::main.window, Game::main.moveLeftKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveLeftKey) == GLFW_PRESS));
			bool cubeControl = ((glfwGetKey(Game::main.window, Game::main.cubeControlKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.cubeControlKey) == GLFW_PRESS));

			if (!cubeControl && moveForward && !actor->moving)
			{
				ECS::main.MoveActor(actor, 0, 0, 1);
			}
			else if (!cubeControl && moveBack && !actor->moving)
			{
				ECS::main.MoveActor(actor, 0, 0, -1);
			}

			if (!cubeControl && moveRight && !actor->moving)
			{
				ECS::main.MoveActor(actor, 1, 0, 0);
			}
			else if (!cubeControl && moveLeft && !actor->moving)
			{
				ECS::main.MoveActor(actor, -1, 0, 0);
			}

			// Cube Controlling

			if (cubeControl && moveForward && !actor->moving && input->lastRoll > input->rollDelay)
			{
				input->lastRoll = 0.0f;
				ECS::main.RollCube(actor, Face::back);
			}

			if (input->lastRoll < input->rollDelay)
			{
				input->lastRoll += deltaTime;
			}

			// Camera Controlling

			bool freeCam = ((glfwGetKey(Game::main.window, Game::main.freeCamKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.freeCamKey) == GLFW_PRESS));

			CameraFollowComponent* camFollower = (CameraFollowComponent*)ECS::main.player->componentIDMap[cameraFollowComponentID];

			bool rotX = ((glfwGetKey(Game::main.window, Game::main.rotateXKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.rotateXKey) == GLFW_PRESS));
			bool unrotX = ((glfwGetKey(Game::main.window, Game::main.unrotateXKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.unrotateXKey) == GLFW_PRESS));
			bool rotY = ((glfwGetKey(Game::main.window, Game::main.rotateYKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.rotateYKey) == GLFW_PRESS));
			bool unrotY = ((glfwGetKey(Game::main.window, Game::main.unrotateYKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.unrotateYKey) == GLFW_PRESS));
			bool rotZ = ((glfwGetKey(Game::main.window, Game::main.rotateZKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.rotateZKey) == GLFW_PRESS));
			bool unrotZ = ((glfwGetKey(Game::main.window, Game::main.unrotateZKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.unrotateZKey) == GLFW_PRESS));

			bool zoomIn = ((glfwGetKey(Game::main.window, Game::main.zoomInKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.zoomInKey) == GLFW_PRESS));
			bool zoomOut = ((glfwGetKey(Game::main.window, Game::main.zoomOutKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.zoomOutKey) == GLFW_PRESS));

			bool resetRotation = ((glfwGetKey(Game::main.window, Game::main.resetRotationKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.resetRotationKey) == GLFW_PRESS));

			if (rotX)
			{
				camFollower->resetting = false;
				Game::main.cameraRotation.x += Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);
			}
			else if (unrotX)
			{
				camFollower->resetting = false;
				Game::main.cameraRotation.x -= Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);
			}

			if (rotY)
			{
				camFollower->resetting = false;
				Game::main.cameraRotation.y += Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);
			}
			else if (unrotY)
			{
				camFollower->resetting = false;
				Game::main.cameraRotation.y -= Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);
			}

			if (rotZ)
			{
				camFollower->resetting = false;
				Game::main.cameraRotation.z += Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);
			}
			else if (unrotZ)
			{
				camFollower->resetting = false;
				Game::main.cameraRotation.z -= Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);
			}

			if (zoomIn)
			{
				Game::main.zoom += Game::main.zoomSpeed * deltaTime;
			}
			else if (zoomOut)
			{
				Game::main.zoom -= Game::main.zoomSpeed * deltaTime;
			}

			if (resetRotation || camFollower->resetting)
			{
				camFollower->resetting = true;
				Game::main.cameraRotation = Util::Lerp(Game::main.cameraRotation, Game::main.baseCameraRotation + actor->baseRotation, deltaTime * 2.5f);
				float d = glm::length2(Game::main.cameraRotation - Game::main.baseCameraRotation + actor->baseRotation);

				if (d <= 0.00001f)
				{
					Game::main.cameraRotation = Game::main.baseCameraRotation + actor->baseRotation;
					camFollower->resetting = false;
				}
			}

			if (freeCam)
			{
				camFollower->track = false;

				bool camRight = ((glfwGetKey(Game::main.window, Game::main.camRightKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.camRightKey) == GLFW_PRESS));
				bool camLeft = ((glfwGetKey(Game::main.window, Game::main.camLeftKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.camLeftKey) == GLFW_PRESS));
				bool camUp = ((glfwGetKey(Game::main.window, Game::main.camUpKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.camUpKey) == GLFW_PRESS));
				bool camDown = ((glfwGetKey(Game::main.window, Game::main.camDownKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.camDownKey) == GLFW_PRESS));
				// bool camForward = ((glfwGetKey(Game::main.window, Game::main.camForwardKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.camForwardKey) == GLFW_PRESS));
				// bool camBack = ((glfwGetKey(Game::main.window, Game::main.camBackKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.camBackKey) == GLFW_PRESS));

				float oMod = Game::main.orthographicSpeedModifier;
				if (Game::main.projectionType == ProjectionType::perspective) oMod = 1.0f;

				glm::vec3 relRight = Util::Rotate(glm::vec3(1.0f, 0.0f, 0.0f), Game::main.cameraRotation);
				glm::vec3 relForward = Util::Rotate(glm::vec3(0.0f, 0.0f, 1.0f), Game::main.cameraRotation);
				glm::vec3 relUp = Util::Rotate(glm::vec3(0.0f, 1.0f, 0.0f), Game::main.cameraRotation);

				if (camRight)
				{
					Game::main.cameraPosition += relRight * Game::main.cameraSpeed * deltaTime * oMod * (1.5f / Game::main.zoom);
				}
				else if (camLeft)
				{
					Game::main.cameraPosition -= relRight * Game::main.cameraSpeed * deltaTime * oMod * (1.5f / Game::main.zoom);
				}

				if (camUp)
				{
					Game::main.cameraPosition += relUp * Game::main.cameraSpeed * deltaTime * oMod * (1.5f / Game::main.zoom);
				}
				else if (camDown)
				{
					Game::main.cameraPosition -= relUp * Game::main.cameraSpeed * deltaTime * oMod * (1.5f / Game::main.zoom);
				}

				/*if (camForward)
				{
					Game::main.cameraPosition += relForward * Game::main.cameraSpeed * deltaTime * oMod * (1.5f / Game::main.zoom);
				}
				else if (camBack)
				{
					Game::main.cameraPosition -= relForward * Game::main.cameraSpeed * deltaTime * oMod * (1.5f / Game::main.zoom);
				}*/
			}
			else
			{
				camFollower->track = true;
			}
		}
	}
}

void InputSystem::AddComponent(Component* component)
{
	inputs.push_back((InputComponent*)component);
}

void InputSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < inputs.size(); i++)
	{
		if (inputs[i]->entity == e)
		{
			InputComponent* s = inputs[i];
			inputs.erase(std::remove(inputs.begin(), inputs.end(), s), inputs.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Billboarding System

void BillboardingSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < boards.size(); i++)
	{
		BillboardingComponent* board = boards[i];

		if (board->active && board->entity->GetScene() == activeScene ||
			board->active && board->entity->GetScene() == 0)
		{
			PositionComponent* pos = (PositionComponent*)board->entity->componentIDMap[positionComponentID];
			pos->rotation = Game::main.cameraRotation;
		}
	}
}

void BillboardingSystem::AddComponent(Component* component)
{
	boards.push_back((BillboardingComponent*)component);
}

void BillboardingSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < boards.size(); i++)
	{
		if (boards[i]->entity == e)
		{
			BillboardingComponent* s = boards[i];
			boards.erase(std::remove(boards.begin(), boards.end(), s), boards.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma region Actor System

void TurnSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < actors.size(); i++)
	{
		ActorComponent* actor = actors[i];

		if (actor->active && actor->entity->GetScene() == activeScene ||
			actor->active && actor->entity->GetScene() == 0)
		{
			PositionComponent* pos = (PositionComponent*)actor->entity->componentIDMap[positionComponentID];

			if (actor->moving)
			{
				glm::vec3 nextStep = Util::Lerp(pos->position, actor->target, deltaTime * actor->speed);

				pos->position = nextStep;

				float dist = glm::length2(pos->position - actor->target);
				if (dist < 0.5f)
				{
					pos->position = actor->target;
					actor->moving = false;
				}
			}

			actor->baseRotation = glm::vec3(0, 0, 0);

			if (actor->face == Face::front)		actor->baseRotation		= { 3.2f,	0,		0 };
			if (actor->face == Face::back)		actor->baseRotation		= { -3.2f,	0,		0 };
			if (actor->face == Face::bottom)	actor->baseRotation		= { 0,		3.2f,	0 };
			if (actor->face == Face::right)		actor->baseRotation		= { 0,		0,		3.2f };
			if (actor->face == Face::left)		actor->baseRotation		= { 0,		0,		-3.2f };

			BillboardingComponent* bill = (BillboardingComponent*)actor->entity->componentIDMap[billboardingComponentID];

			if (bill != nullptr)
			{
				pos->rotation = actor->baseRotation + Game::main.cameraRotation;
			}
			else
			{
				pos->rotation = actor->baseRotation;
			}
		}
	}
}

void TurnSystem::AddComponent(Component* component)
{
	actors.push_back((ActorComponent*)component);
}

void TurnSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < actors.size(); i++)
	{
		if (actors[i]->entity == e)
		{
			ActorComponent* s = actors[i];
			actors.erase(std::remove(actors.begin(), actors.end(), s), actors.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma endregion