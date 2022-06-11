#include "ecs.h"

#include <algorithm>
#include <iostream>

#include "game.h"
#include "system.h"
#include "component.h"
#include "entity.h"
#include "util.h"

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
	componentBlocks.push_back(new ComponentBlock(new CubeSystem(), cubeComponentID));
	componentBlocks.push_back(new ComponentBlock(new AnimationControllerSystem(), animationControllerComponentID));
	componentBlocks.push_back(new ComponentBlock(new AnimationSystem(), animationComponentID));
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
				ECS::main.RegisterComponent(new CubeComponent(cube, true, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["block"]), cube);
			}
		}

		player = CreateEntity(0, "Player");
		Animation* testIdle = Game::main.animationMap["testIdle"];

		ECS::main.RegisterComponent(new PositionComponent(player, true, glm::vec3((10.0f * (mapWidth - 1)) - mapMidX, cubeSize / 2.0f, (-10.f * (mapDepth - 1)) + mapMidZ), glm::vec3(0.0f, 0.0f, 0.0f)), player);
		ECS::main.RegisterComponent(new AnimationComponent(player, true, glm::vec3(0.0f, (testIdle->height / testIdle->rows) * 0.5f * 0.4f, 0.0f), testIdle, "idle", 0.2f, 0.2f, false, false, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), player);
		AnimationComponent* a = (AnimationComponent*)player->componentIDMap[animationComponentID];
		ECS::main.RegisterComponent(new PlayerAnimationControllerComponent(player, true, a), player);
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

CubeComponent::CubeComponent(Entity* entity, bool active, glm::vec3 size, glm::vec4 color, Texture* texture)
{
	this->ID = cubeComponentID;
	this->entity = entity;
	this->active = active;

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


InputComponent::InputComponent(Entity* entity, bool active, bool acceptInput)
{
	this->ID = inputComponentID;
	this->entity = entity;
	this->active = active;

	this->acceptInput = acceptInput;
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
			
			Game::main.renderer->PrepareQuad(glm::vec2(activeAnimation->width * a->scaleX, activeAnimation->height * a->scaleY), a->offset + pos->position, Game::main.cameraRotation, a->color, activeAnimation->ID, cellX, cellY, activeAnimation->columns, activeAnimation->rows, a->flippedX, a->flippedY);
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

#pragma endregion