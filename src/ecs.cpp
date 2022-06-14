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

glm::vec3 ECS::CubeToWorldSpace(int x, int y, int z)
{
	return glm::vec3((float)cubeSize * x, (float)cubeSize * y, (float)cubeSize * -z);
}

void ECS::PositionCube(CubeComponent* cube, int x, int y, int z)
{
	PositionComponent* pos = (PositionComponent*)cube->entity->componentIDMap[positionComponentID];
	pos->position = CubeToWorldSpace(x, y, z);
}

void ECS::PositionActor(ActorComponent* actor)
{
	PositionComponent* pos = (PositionComponent*)actor->entity->componentIDMap[positionComponentID];
	CubeComponent* cube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];
	
	pos->position = CubeToWorldSpace(cube->x, cube->y, cube->z) + ((Util::GetRelativeUp(actor->face) * (cubeSize / 2.0f)));
}

void ECS::MoveCube(CubeComponent* cube, int x, int y, int z)
{
	cubes[cube->x][cube->y][cube->z] = nullptr;

	cube->x = x;
	cube->y = y;
	cube->z = z;

	cubes[x][y][z] = cube->entity;
}

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

void ECS::MoveActor(ActorComponent* actor, int dX, int dY, int dZ)
{
	CubeComponent* currentCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];
	Entity* target = GetCube(currentCube->x + dX, currentCube->y + dY, currentCube->z + dZ);
	if (target != nullptr)
	{
		glm::vec3 relativeUp = Util::GetRelativeUp(actor->face);

		Entity* targetUp = GetCube(currentCube->x + dX + (int)relativeUp.x, currentCube->y + dY + (int)relativeUp.y, currentCube->z + dZ + (int)relativeUp.z);

		if (targetUp == nullptr && target != nullptr)
		{
			CubeComponent* targetCube = (CubeComponent*)target->componentIDMap[cubeComponentID];
			PositionComponent* targetPos = (PositionComponent*)target->componentIDMap[positionComponentID];

			actor->cube = target;

			MovementComponent* mover = (MovementComponent*)actor->entity->componentIDMap[movementComponentID];
			mover->RegisterMovement(actor->speed, targetPos->position + ((targetCube->size.y / 2.0f) * relativeUp));
		}
	}
}

std::pair<Face, bool> ECS::FindFulcrum(CubeComponent* activeCube, Face activeFace, Face rollDirection)
{
	// This serves to find the cube off which any rotating cube should intuitively push.
	// This will usually be right below the cube, but this should also account for when
	// the player is pushing a cube perpendicular to the surface it should land on
	// rather than parallel to it.

	// First we'll check right below the active cube (relative to the direction being pushed).
	
	glm::vec3 expectedDirection = -Util::GetRelativeUp(activeFace);
	Entity* possibleFulcrum = GetCube(	activeCube->x + expectedDirection.x,
										activeCube->y + expectedDirection.y,
										activeCube->z + expectedDirection.z);

	if (possibleFulcrum != nullptr)
	{
		return std::pair<Face, bool>(Util::GetFaceFromDifference(expectedDirection), true);
	}

	// In this case, the fulcrum is not where we expected it to be.
	// Now, we should check to see if the fulcrum is instead opposite the rolling direction.
	// This will warrant the same reaction as the above fulcrum.

	expectedDirection = -Util::GetRelativeUp(rollDirection);
	possibleFulcrum = GetCube(	activeCube->x + expectedDirection.x,
								activeCube->y + expectedDirection.y,
								activeCube->z + expectedDirection.z);

	if (possibleFulcrum != nullptr)
	{
		return std::pair<Face, bool>(Util::GetFaceFromDifference(expectedDirection), true);
	}

	// In this case, neither of the allowed fulcrum points can be found.
	return std::pair<Face, bool>(Util::GetFaceFromDifference(expectedDirection), false);
}

Face ECS::DetermineRollDirection(Face fulcrum, Face activeFace, Face rollDirection)
{
	glm::vec3 fulcrumUp = Util::GetRelativeUp(fulcrum);
	glm::vec3 activeDown = -Util::GetRelativeUp(activeFace);

	// If the fulcrum is right below the face the actor is standing on.
	if (fulcrumUp == activeDown)
	{
		// Then we want to roll in the direction specified by rollDirection.
		return rollDirection;
	}

	// If the fulcrum is below the roll direction, then we want to move opposite the activeFace.
	if (fulcrumUp == -Util::GetRelativeUp(rollDirection))
	{
		return Util::GetFaceFromDifference(activeDown);
	}

	// These are technically, I believe, the only inputs we expect to receive, but just in case:
	return rollDirection;
}

//bool ECS::CheckAutofrendation(CubeComponent* activeCube, Face activeFace, Face rollDirection)
//{
//	// True = Autofrendated.
//	// False = Not autofrendated.
//
//	// I'm not actually sure we need this function....
//}

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
					if ((x != 0 || y != 0 || z != 0) &&		// Ensures that you're not looking at the center cube.
						(abs(x) + abs(y) + abs(z) == 1))	// I believe this is a more elegant way to solve this than my previous method.
					{
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

	glm::vec3 startDirection = Util::GetRelativeUp(direction);

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

void ECS::QuarterRoll(ActorComponent* actor, Face standingFace, Face rollDirection, Entity* landingTarget, Face landingFace, std::vector<CubeComponent*> affectedCubes)
{
	// We need to grab some components first.
	CubeComponent* activeCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];
	PositionComponent* pos = (PositionComponent*)actor->cube->componentIDMap[positionComponentID];
	MovementComponent* mover = (MovementComponent*)actor->cube->componentIDMap[movementComponentID];
	CubeComponent* landingCube = (CubeComponent*)landingTarget->componentIDMap[cubeComponentID];

	// And just as a precaution.
	Face activeFace = actor->face;
	Face roll = rollDirection;

	// This is a secret tool that will help us later.
	glm::vec3 pivotPos = glm::vec3(activeCube->x, activeCube->y, activeCube->z);

	// Now we need to figure out how the cube is gonna roll.
	Quaternion newRotation = Util::GetRollRotation(landingFace, roll, pos->quaternion, 1);

	// And then we can figure out where it is going to land in cube and world space.
	glm::vec3 landingUp = Util::GetRelativeUp(landingFace);
	glm::vec3 landingCubePosition = glm::vec3(landingCube->x + (int)landingUp.x, landingCube->y + (int)landingUp.y, landingCube->z + (int)landingUp.z);
	glm::vec3 landingWorldPosition = CubeToWorldSpace(landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// Then, we register the movements to the landing position with the active cube's movement component.
	// The bezier curve is a little complicated.
	if (affectedCubes.size() > 1)
	{
		glm::vec3 forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pos->position - landingWorldPosition);
		float length = dist * sin(45) * sin(90) * (2.0 / 3.0f);
		glm::vec3 p1 = pos->position + (forward * length);
		glm::vec3 p2 = landingWorldPosition + (up * length);
		mover->RegisterMovement(3.5f, { { pos->position, p1, p2, landingWorldPosition } });
	}
	else
	{
		glm::vec3 leap = (glm::normalize(landingUp) * 5.0f);
		if (landingUp.z != 0) leap *= -1.0f;
		glm::vec3 zenith = ((pos->position + landingWorldPosition) / 2.0f) + leap;
		mover->RegisterMovement(3.5f, { { pos->position, zenith, landingWorldPosition } });
	}
	mover->RegisterMovement(19.0f, newRotation);

	// And finally position the active cube in the right position.
	MoveCube(activeCube, landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// Oh, and we roll the actor onto the right face.
	RollActor(actor, standingFace);
	/*actor->face = standingFace;
	PositionActor(actor);*/
	// actor->face = landingFace;

	// But we can't forget that this might involve a larger structure of cubes.
	// This is where things get a little complicated.
	for (int j = 1; j < affectedCubes.size(); j++)
	{
		// We need to grab the cube and position components, as well as their movement components.
		CubeComponent* c = affectedCubes[j];
		PositionComponent* pc = (PositionComponent*)c->entity->componentIDMap[positionComponentID];
		MovementComponent* mc = (MovementComponent*)c->entity->componentIDMap[movementComponentID];

		// There are two rotations necessary here.
		// The first is the rotation of the actual position component.
		Quaternion affRot = Util::GetRollRotation(landingFace, roll, pc->quaternion, 1);

		// And the second is the rotation in space around the initial cube.
		Quaternion diffRot = Util::GetRollRotation(landingFace, roll, { 1, 0, 0, 0 }, 1);

		// We need to record the initial difference in position between this cube
		// and the cube we rolled earlier.
		// We could've waited to finalize the movement of the "active" cube,
		// but we'll instead make use of that handy pivot point we saved earlier.
		int dX = pivotPos.x - c->x;
		int dY = pivotPos.y - c->y;
		int dZ = pivotPos.z - c->z;

		// Now, we need to figure out what the difference should be after the rotation of the structure.
		glm::vec3 newDifference = Util::Rotate(glm::vec3(dX, dY, dZ), diffRot);

		// And then we need to convert that to world space, instead of cube space.
		glm::vec3 worldDifference = CubeToWorldSpace(activeCube->x + (int)newDifference.x, activeCube->y + (int)newDifference.y, activeCube->z + (int)newDifference.z);

		// Finally, we can register the necessary movements with the movement component and actually finalize the movement of the cube in cube space.
		glm::vec3 forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pc->position - worldDifference);
		float length = dist * sin(45) * sin(90) * (2.0 / 3.0f);
		glm::vec3 p1 = pc->position + (forward * length);
		glm::vec3 p2 = worldDifference + (up * length);

		mc->RegisterMovement(3.5f, { { pc->position, p1, p2, worldDifference } });
		mc->RegisterMovement(19.0f, affRot);
		MoveCube(c, activeCube->x + (int)newDifference.x, activeCube->y + (int)newDifference.y, activeCube->z + (int)newDifference.z);
	}
}

void ECS::HalfRoll(ActorComponent* actor, Face standingFace, Face oppFulcrum, Face rollDirection, Entity* landingTarget, Face landingFace, std::vector<CubeComponent*> affectedCubes)
{
	// We need to grab some components first.
	CubeComponent* activeCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];
	PositionComponent* pos = (PositionComponent*)actor->cube->componentIDMap[positionComponentID];
	MovementComponent* mover = (MovementComponent*)actor->cube->componentIDMap[movementComponentID];
	CubeComponent* landingCube = (CubeComponent*)landingTarget->componentIDMap[cubeComponentID];

	// And just as a precaution.
	Face activeFace = actor->face;
	Face roll = rollDirection;

	// This is a secret tool that will help us later.
	glm::vec3 pivotPos = glm::vec3(activeCube->x, activeCube->y, activeCube->z);

	// Now we need to figure out how the cube is gonna roll.
	// std::cout << "Roll Faces: " + std::to_string((int)standingFace) + " / " + std::to_string((int)roll) << std::endl;
	Quaternion newRotation = Util::GetRollRotation(oppFulcrum, roll, pos->quaternion, 2);

	// And then we can figure out where it is going to land in cube and world space.
	glm::vec3 landingUp = Util::GetRelativeUp(landingFace);
	glm::vec3 landingCubePosition = glm::vec3(landingCube->x + landingUp.x, landingCube->y + landingUp.y, landingCube->z + landingUp.z);
	glm::vec3 landingWorldPosition = CubeToWorldSpace(landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// Then, we register the movements to the landing position with the active cube's movement component.
	// The bezier curve is a little complicated.
	glm::vec3 forward = Util::GetRelativeUp(landingFace);
	if (forward.z != 0) forward *= -1.0f;
	glm::vec3 up = Util::GetRelativeUp(oppFulcrum);
	if (up.z != 0) up *= -1.0f;
	float dist = glm::length(pos->position - landingWorldPosition);
	float length = dist * sin(45) * sin(90) * (2.0 / 3.0f);
	glm::vec3 p1 = pos->position + (forward * length);
	glm::vec3 p2 = landingWorldPosition + (up * length);

	mover->RegisterMovement(3.5f, { { pos->position, p1, p2, landingWorldPosition } });
	mover->RegisterMovement(19.0f, newRotation);

	// And finally position the active cube in the right position.
	MoveCube(activeCube, landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// Oh, and we roll the actor onto the right face.
	RollActor(actor, standingFace);
	/*actor->face = standingFace;
	PositionActor(actor);*/
	// actor->face = landingFace;

	// But we can't forget that this might involve a larger structure of cubes.
	// This is where things get a little complicated.
	for (int j = 1; j < affectedCubes.size(); j++)
	{
		// We need to grab the cube and position components, as well as their movement components.
		CubeComponent* c = affectedCubes[j];
		PositionComponent* pc = (PositionComponent*)c->entity->componentIDMap[positionComponentID];
		MovementComponent* mc = (MovementComponent*)c->entity->componentIDMap[movementComponentID];

		// There are two rotations necessary here.
		// The first is the rotation of the actual position component.
		Quaternion affRot = Util::GetRollRotation(activeFace, roll, pc->quaternion, 1);

		// And the second is the rotation in space around the initial cube.
		Quaternion diffRot = Util::GetRollRotation(activeFace, roll, { 1, 0, 0, 0 }, 1);

		// We need to record the initial difference in position between this cube
		// and the cube we rolled earlier.
		// We could've waited to finalize the movement of the "active" cube,
		// but we'll instead make use of that handy pivot point we saved earlier.
		int dX = c->x - pivotPos.x;
		int dY = c->y - pivotPos.y;
		int dZ = c->z - pivotPos.z;

		// Now, we need to figure out what the difference should be after the rotation of the structure.
		glm::vec3 newDifference = Util::Rotate(glm::vec3(dX, dY, dZ), diffRot);

		// And then we need to convert that to world space, instead of cube space.
		glm::vec3 worldDifference = CubeToWorldSpace(activeCube->x + (int)newDifference.x, activeCube->y + (int)newDifference.y, activeCube->z + (int)newDifference.z);

		// Finally, we can register the necessary movements with the movement component and actually finalize the movement of the cube in cube space.
		glm::vec3 forward = Util::GetRelativeUp(landingFace);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(oppFulcrum);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pc->position - worldDifference);
		float length = dist * sin(45) * sin(90) * (2.0 / 3.0f);
		glm::vec3 p1 = pc->position + (forward * length);
		glm::vec3 p2 = worldDifference + (up * length);

		mc->RegisterMovement(3.5f, { { pc->position, p1, p2, worldDifference } });
		mc->RegisterMovement(19.0f, affRot);
		MoveCube(c, activeCube->x + (int)newDifference.x, activeCube->y + (int)newDifference.y, activeCube->z + (int)newDifference.z);
	}
}

void ECS::RollCube(ActorComponent* actor, Face rollDirection)
{
	CubeComponent* activeCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];
	std::vector<CubeComponent*> affectedCubes = DetermineStructure(activeCube, rollDirection);

	if (affectedCubes.size() > 0)
	{
		// There are cubes to roll.

		// We're (maybe) going to want to check possible landing points from non-initial circumstances, so
		// we copy them into more variables so that we can change these while preserving the inputs.
		Face activeFace = actor->face;
		Face roll = rollDirection;

		// All rotations expect a fulcrum.
		std::pair<Face, bool> fulcrum = FindFulcrum(activeCube, activeFace, roll);
		if (fulcrum.second == false) return;

		// Now that we know we have a fulcrum, we want to figure out the axis upon which the cube is rotating.
		// This is a function of the position of the fulcrum, the face the player is standing on, and the roll direction.
		roll = DetermineRollDirection(fulcrum.first, activeFace, roll);

		// We need to make sure that the active cube *can* roll in that direction.
		// If another cube is blocking it, the whole roll should be stopped.
		// Let's check that. We don't need any fancy function for this, just:
		glm::vec3 rollUp = Util::GetRelativeUp(roll);
		Entity* possibleBlockerEntity = GetCube(activeCube->x + rollUp.x, activeCube->y + rollUp.y, activeCube->z + rollUp.z);

		bool blocked = false;

		if (possibleBlockerEntity != nullptr)
		{
			CubeComponent* possibleBlocker = (CubeComponent*)possibleBlockerEntity->componentIDMap[cubeComponentID];

			auto result = std::find(affectedCubes.begin(), affectedCubes.end(), possibleBlocker);
			int location;

			if (result == affectedCubes.end())
			{
				blocked = true;
			}
		}

		if (blocked) return;

		// Now, we have the "real" direction we're going to be rolling.
		// We want to figure out where we're going to land.
		// This is a function of the fulcrum and the roll direction.
		// We always want to roll "towards" the fulcrum.
		glm::vec3 landingCoords = Util::GetLandingCoords(fulcrum.first, roll) + glm::vec3(activeCube->x, activeCube->y, activeCube->z);

		// Now we need to see if there is a cube there for us to land on.
		Entity* landingTarget = GetCube(landingCoords.x, landingCoords.y, landingCoords.z);
		Face landingFace;

		// If there is a cube there, we can stop now and initiate the actual roll.
		if (landingTarget != nullptr)
		{
			// The face we're landing on is the opposite of the fulcrum.
			// At least so long as we're doing only a 90 degree rotation.
			landingFace = Util::OppositeFace(fulcrum.first);

			QuarterRoll(actor, rollDirection, roll, landingTarget, landingFace, affectedCubes);
			return;
		}

		// It is a real shame that our rotation couldn't be that simple.
		// If we've gotten here, it means there isn't a cube for ours to land on.
		// That means we need to rotate another 90 degrees.
		// Luckily, a cube can only rotate 180 degrees, no more, so we only need
		// to do one more check.
		
		// Well, we actually know that if we have gotten this far there is somewhere for
		// the cube to land: the fulcrum itself. The landing face is just going to be
		// the rolling direction.
		landingCoords = glm::vec3(activeCube->x, activeCube->y, activeCube->z) + Util::GetRelativeUp(fulcrum.first);
		landingTarget = GetCube(landingCoords.x, landingCoords.y, landingCoords.z);

		// Voila, easy as that.
		// And if there is something for us to land on there, let's do it.
		if (landingTarget != nullptr)
		{
			// We can assume that the landing face is the sane as our roll direction.

			HalfRoll(actor, rollDirection, Util::OppositeFace(fulcrum.first), roll, landingTarget, roll, affectedCubes);
			return;
		}

		// This means we couldn't find a suitable landing position and thus shouldn't roll the cubes at all.
		// Oh well, better luck next time.
		return;
	}
}

void ECS::RollCube(CubeComponent* cube, Face rollDirection)
{

}

void ECS::RollActor(ActorComponent* actor, Face rollDirection)
{
	actor->face = rollDirection;
	PositionActor(actor);
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

uint32_t ECS::GetOtherID()
{
	return ++otherIDCounter;
}

void ECS::Init()
{
	componentBlocks.push_back(new ComponentBlock(new InputSystem(), inputComponentID));
	componentBlocks.push_back(new ComponentBlock(new MovementSystem(), movementComponentID));
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
		float cubeSize = (float)this->cubeSize;
		int mapWidth = 5;
		int mapHeight = 3;
		int mapDepth = 5;

		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
			{
				for (int z = 0; z < mapDepth; z++)
				{
					Entity* cube = CreateEntity(0, "Cube: " + std::to_string(x) + std::to_string(y) + " / " + std::to_string(z));
					ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), cube);
					ECS::main.RegisterComponent(new CubeComponent(cube, true, x, y, z, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["test"]), cube);
					ECS::main.PositionCube((CubeComponent*)cube->componentIDMap[cubeComponentID], x, y, z);
					ECS::main.RegisterComponent(new MovementComponent(cube, true), cube);
					ECS::main.cubes[x][y][z] = cube;
				}
			}
		}

		Entity* cube = CreateEntity(0, "Cube: " + std::to_string(mapWidth - 2) + " / " + std::to_string(mapHeight - 1) + " / " + std::to_string(mapDepth));
		ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), cube);
		ECS::main.RegisterComponent(new CubeComponent(cube, true, mapWidth - 2, mapHeight - 1, mapDepth, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["test"]), cube);
		ECS::main.PositionCube((CubeComponent*)cube->componentIDMap[cubeComponentID], mapWidth - 2, mapHeight - 1, mapDepth);
		ECS::main.RegisterComponent(new MovementComponent(cube, true), cube);
		ECS::main.cubes[mapWidth - 2][mapHeight - 1][mapDepth] = cube;


		cube = CreateEntity(0, "Cube: " + std::to_string(mapWidth - 2) + " / " + std::to_string(mapHeight - 1) + " / " + std::to_string(mapDepth + 1));
		ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), cube);
		ECS::main.RegisterComponent(new CubeComponent(cube, true, mapWidth - 2, mapHeight - 1, mapDepth + 1, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["test"]), cube);
		ECS::main.PositionCube((CubeComponent*)cube->componentIDMap[cubeComponentID], mapWidth - 2, mapHeight - 1, mapDepth + 1);
		ECS::main.RegisterComponent(new MovementComponent(cube, true), cube);
		ECS::main.cubes[mapWidth - 2][mapHeight - 1][mapDepth + 1] = cube;

		player = CreateEntity(0, "Player");
		Animation* testIdle = Game::main.animationMap["testIdle"];

		ECS::main.RegisterComponent(new PositionComponent(player, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), player);
		ECS::main.RegisterComponent(new AnimationComponent(player, true, glm::vec3(0.0f, (testIdle->height / testIdle->rows) * 0.5f * 0.4f, 0.0f), testIdle, "idle", 0.2f, 0.2f, false, false, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), player);
		AnimationComponent* a = (AnimationComponent*)player->componentIDMap[animationComponentID];
		ECS::main.RegisterComponent(new PlayerAnimationControllerComponent(player, true, a), player);
		ECS::main.RegisterComponent(new BillboardingComponent(player, true), player);
		ECS::main.RegisterComponent(new CameraFollowComponent(player, true, glm::vec3(-295.0f, 615.0f, 1760.0f), 750.0f, 10.0f, true, false, false, false), player);
		ECS::main.RegisterComponent(new InputComponent(player, true, true, 0.5f), player);
		ECS::main.RegisterComponent(new ActorComponent(player, true, 10.0f, Face::top, ECS::main.cubes[mapWidth - 2][mapHeight - 1][mapDepth]), player);
		ECS::main.RegisterComponent(new MovementComponent(player, true), player);

		PositionActor((ActorComponent*)player->componentIDMap[actorComponentID]);
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

void PositionComponent::SetRotation(Quaternion rotation)
{
	
	this->quaternion = rotation;
	Util::NormalizeQuaternion(this->quaternion);
}

PositionComponent::PositionComponent(Entity* entity, bool active, glm::vec3 position, Quaternion quaternion)
{
	this->ID = positionComponentID;
	this->entity = entity;
	this->active = active;

	this->position = position;
	this->quaternion = quaternion;
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

	this->checked = false;
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

AnimationComponent::AnimationComponent(Entity* entity, bool active, glm::vec3 baseOffset, Animation* idleAnimation, std::string animationName, float scaleX, float scaleY, bool flippedX, bool flippedY, glm::vec4 color)
{
	this->ID = animationComponentID;
	this->entity = entity;
	this->active = active;

	this->baseOffset = baseOffset;
	this->offset = baseOffset;

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

void ActorComponent::SetRotation(glm::vec3 rotation)
{
	rotation.x = fmod(rotation.x, 6.2832f);
	rotation.y = fmod(rotation.y, 6.2832f);
	rotation.z = fmod(rotation.z, 6.2832f);

	this->baseQuaternion = Util::EulerToQuaternion(rotation);
	Util::NormalizeQuaternion(this->baseQuaternion);

	AnimationComponent* anim = (AnimationComponent*)this->entity->componentIDMap[animationComponentID];
	if (anim != nullptr)
	{
		anim->offset = Util::Rotate(anim->baseOffset, this->baseQuaternion);
	}
}

ActorComponent::ActorComponent(Entity * entity, bool active, float speed, Face face, Entity * cube)
{
	this->ID = actorComponentID;
	this->entity = entity;
	this->active = active;

	this->speed = speed;

	this->face = face;
	this->cube = cube;

	this->baseQuaternion = { 1, 0, 0, 0 };

	if (this->face == Face::front)		this->baseQuaternion = Util::EulerToQuaternion({ 3.2f,	0,		0		});
	if (this->face == Face::back)		this->baseQuaternion = Util::EulerToQuaternion({ -3.2f,	0,		0		});
	if (this->face == Face::bottom)		this->baseQuaternion = Util::EulerToQuaternion({ 0,		3.2f,	0		});
	if (this->face == Face::right)		this->baseQuaternion = Util::EulerToQuaternion({ 0,		0,		3.2f	});
	if (this->face == Face::left)		this->baseQuaternion = Util::EulerToQuaternion({ 0,		0,		-3.2f	});

	Util::NormalizeQuaternion(this->baseQuaternion);
}

#pragma endregion

#pragma region

MovementComponent::MovementComponent(Entity* entity, bool active)
{
	this->ID = movementComponentID;
	this->entity = entity;
	this->active = active;

	this->moving = false;
}

void MovementComponent::RegisterMovement(float speed, BezierCurve curve)
{
	this->moving = true;

	BezierMovement* m = new BezierMovement();
	m->ID = ECS::main.GetOtherID();
	m->movementType = MovementType::bezier;
	m->speed = speed;
	m->curve = curve;
	m->t = 0.0f;
	this->queue.push_back(m);
}

void MovementComponent::RegisterMovement(float speed, glm::vec3 target)
{
	this->moving = true;

	LinearMovement* m = new LinearMovement();
	m->ID = ECS::main.GetOtherID();
	m->movementType = MovementType::linear;
	m->speed = speed;
	m->target = target;
	this->queue.push_back(m);
}

void MovementComponent::RegisterMovement(float speed, Quaternion target)
{
	this->moving = true;

	RotatingMovement* m = new RotatingMovement();
	m->ID = ECS::main.GetOtherID();
	m->movementType = MovementType::rotation;
	m->speed = speed;
	m->targetRotation = target;
	this->queue.push_back(m);
}

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
			Game::main.renderer->PrepareCube(cube->size, pos->position, pos->quaternion, cube->color, cube->texture->ID);
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
			
			Game::main.renderer->PrepareQuad(glm::vec2(activeAnimation->width * a->scaleX, activeAnimation->height * a->scaleY), a->offset + pos->position, pos->quaternion, a->color, activeAnimation->ID, cellX, cellY, activeAnimation->columns, activeAnimation->rows, a->flippedX, a->flippedY);
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

				glm::vec3 normalizedOffset = glm::normalize(Util::Rotate(c->offset, pos->quaternion));

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
			ActorComponent* actor = (ActorComponent*)input->entity->componentIDMap[actorComponentID];

			// Testing
			if (glfwGetKey(Game::main.window, GLFW_KEY_0) == GLFW_PRESS) actor->face = Face::front;
			if (glfwGetKey(Game::main.window, GLFW_KEY_1) == GLFW_PRESS) actor->face = Face::back;
			if (glfwGetKey(Game::main.window, GLFW_KEY_2) == GLFW_PRESS) actor->face = Face::left;
			if (glfwGetKey(Game::main.window, GLFW_KEY_3) == GLFW_PRESS) actor->face = Face::right;
			if (glfwGetKey(Game::main.window, GLFW_KEY_4) == GLFW_PRESS) actor->face = Face::top;
			if (glfwGetKey(Game::main.window, GLFW_KEY_5) == GLFW_PRESS) actor->face = Face::bottom;

			// Player Controlling
			// ActorComponent* actor = (ActorComponent*)input->entity->componentIDMap[actorComponentID];
			MovementComponent* mover = (MovementComponent*)input->entity->componentIDMap[movementComponentID];
			MovementComponent* cube = (MovementComponent*)actor->cube->componentIDMap[movementComponentID];

			bool moveForward = ((glfwGetKey(Game::main.window, Game::main.moveForwardKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveForwardKey) == GLFW_PRESS));
			bool moveBack = ((glfwGetKey(Game::main.window, Game::main.moveBackKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveBackKey) == GLFW_PRESS));
			bool moveRight = ((glfwGetKey(Game::main.window, Game::main.moveRightKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveRightKey) == GLFW_PRESS));
			bool moveLeft = ((glfwGetKey(Game::main.window, Game::main.moveLeftKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.moveLeftKey) == GLFW_PRESS));
			bool cubeControl = ((glfwGetKey(Game::main.window, Game::main.cubeControlKey) == GLFW_PRESS) || (glfwGetMouseButton(Game::main.window, Game::main.cubeControlKey) == GLFW_PRESS));

			glm::vec3 movement;
			if (!cubeControl && moveForward && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::back));
				ECS::main.MoveActor(actor, movement.x, movement.y, movement.z);
			}
			else if (!cubeControl && moveBack && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::front));
				ECS::main.MoveActor(actor, movement.x, movement.y, movement.z);
			}
			else if (!cubeControl && moveRight && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::right));
				ECS::main.MoveActor(actor, movement.x, movement.y, movement.z);
			}
			else if (!cubeControl && moveLeft && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::left));
				ECS::main.MoveActor(actor, movement.x, movement.y, movement.z);
			}

			// Cube Controlling

			if (cubeControl && moveForward && !mover->moving && input->lastRoll > input->rollDelay)
			{
				input->lastRoll = 0.0f;
				ECS::main.RollCube(actor, Util::GetAbsoluteFace(actor->face, Face::back));
			}
			else if (cubeControl && moveBack && !mover->moving && input->lastRoll > input->rollDelay)
			{
				input->lastRoll = 0.0f;
				ECS::main.RollCube(actor, Util::GetAbsoluteFace(actor->face, Face::front));
			}

			if (cubeControl && moveRight && !mover->moving && input->lastRoll > input->rollDelay)
			{
				input->lastRoll = 0.0f;
				ECS::main.RollCube(actor, Util::GetAbsoluteFace(actor->face, Face::right));
			}
			else if (cubeControl && moveLeft && !mover->moving && input->lastRoll > input->rollDelay)
			{
				input->lastRoll = 0.0f;
				ECS::main.RollCube(actor, Util::GetAbsoluteFace(actor->face, Face::left));
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

			glm::vec3 camRot = Util::QuaternionToEuler(Game::main.cameraRotation);

			float deltaTheta = Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);

			if (rotX)
			{
				camFollower->resetting = false;
				Quaternion locQ = Util::EulerToQuaternion({ deltaTheta, 0.0f, 0.0f });
				Util::NormalizeQuaternion(locQ);
				Game::main.cameraRotation = locQ * Game::main.cameraRotation;
			}
			else if (unrotX)
			{
				camFollower->resetting = false;
				Quaternion locQ = Util::EulerToQuaternion({ -deltaTheta, 0.0f, 0.0f });
				Util::NormalizeQuaternion(locQ);
				Game::main.cameraRotation = locQ * Game::main.cameraRotation;
			}

			if (rotY)
			{
				camFollower->resetting = false;
				Quaternion locQ = Util::EulerToQuaternion({ 0.0f, deltaTheta, 0.0f });
				Util::NormalizeQuaternion(locQ);
				Game::main.cameraRotation = locQ * Game::main.cameraRotation;
			}
			else if (unrotY)
			{
				camFollower->resetting = false;
				Quaternion locQ = Util::EulerToQuaternion({ 0.0f, -deltaTheta, 0.0f });
				Util::NormalizeQuaternion(locQ);
				Game::main.cameraRotation = locQ * Game::main.cameraRotation;
			}

			if (rotZ)
			{
				camFollower->resetting = false;
				Quaternion locQ = Util::EulerToQuaternion({ 0.0f, 0.0f, deltaTheta });
				Util::NormalizeQuaternion(locQ);
				Game::main.cameraRotation = locQ * Game::main.cameraRotation;
			}
			else if (unrotZ)
			{
				camFollower->resetting = false;
				Quaternion locQ = Util::EulerToQuaternion({ 0.0f, 0.0f, -deltaTheta });
				Util::NormalizeQuaternion(locQ);
				Game::main.cameraRotation = locQ * Game::main.cameraRotation;
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

				Quaternion target = Util::EulerToQuaternion(Game::main.baseCameraRotation);
				Quaternion slerp = Util::Slerp(Game::main.cameraRotation, target, deltaTime * 8.0f);

				Game::main.cameraRotation = slerp;

				if (Util::QuaternionDistance(Game::main.cameraRotation, target) < 0.000005f)
				{
					Game::main.cameraRotation = target;
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
			pos->quaternion = Game::main.cameraRotation;
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

			BillboardingComponent* bill = (BillboardingComponent*)actor->entity->componentIDMap[billboardingComponentID];

			if (bill != nullptr)
			{
				pos->quaternion = Util::EulerToQuaternion(Util::QuaternionToEuler(actor->baseQuaternion) + Util::QuaternionToEuler(Game::main.cameraRotation));
			}
			else
			{
				pos->quaternion = actor->baseQuaternion;
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

#pragma region Movement System

void MovementSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < movers.size(); i++)
	{
		MovementComponent* move = movers[i];

		if (move->active && move->entity->GetScene() == activeScene ||
			move->active && move->entity->GetScene() == 0)
		{
			std::vector<Movement*> finishedMoves;

			for (int j = 0; j < move->queue.size(); j++)
			{
				Movement* m = move->queue[j];

				if (m->movementType == MovementType::linear)
				{
					PositionComponent* pos = (PositionComponent*)move->entity->componentIDMap[positionComponentID];

					LinearMovement* line = (LinearMovement*)m;

					glm::vec3 nextStep = Util::Lerp(pos->position, line->target, deltaTime * line->speed);

					pos->position = nextStep;

					float dist = glm::length2(pos->position - line->target);
					if (dist < 0.5f)
					{
						pos->position = line->target;
						finishedMoves.push_back(m);
					}
				}
				else if (m->movementType == MovementType::bezier)
				{
					PositionComponent* pos = (PositionComponent*)move->entity->componentIDMap[positionComponentID];

					BezierMovement* curving = (BezierMovement*)m;

					curving->t += deltaTime * curving->speed;
					pos->position = curving->curve.GetPoint(curving->t);

					if (curving->t >= 1.0f)
					{
						pos->position = curving->curve.GetPoint(1.0f);
						finishedMoves.push_back(m);
					}
				}
				else if (m->movementType == MovementType::rotation)
				{
					PositionComponent* pos = (PositionComponent*)move->entity->componentIDMap[positionComponentID];

					RotatingMovement* rotation = (RotatingMovement*)m;

					Quaternion slerp = Util::Slerp(pos->quaternion, rotation->targetRotation, deltaTime * rotation->speed);

					pos->SetRotation(slerp);

					if (Util::QuaternionDistance(pos->quaternion, rotation->targetRotation) < 0.0025f)
					{
						pos->SetRotation(rotation->targetRotation);
						finishedMoves.push_back(m);
					}
				}
			}

			for (int j = 0; j < finishedMoves.size(); j++)
			{
				move->queue.erase(std::remove(move->queue.begin(), move->queue.end(), finishedMoves[j]), move->queue.end());
			}

			finishedMoves.clear();

			if (move->queue.size() == 0)
			{
				move->moving = false;
			}
		}
	}
}

void MovementSystem::AddComponent(Component* component)
{
	movers.push_back((MovementComponent*)component);
}

void MovementSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < movers.size(); i++)
	{
		if (movers[i]->entity == e)
		{
			MovementComponent* s = movers[i];
			movers.erase(std::remove(movers.begin(), movers.end(), s), movers.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma endregion