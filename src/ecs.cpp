#include "ecs.h"

#include <algorithm>
#include <iostream>
#include <glm/gtx/norm.hpp>

#include "game.h"
#include "system.h"
#include "entity.h"
#include "puzzle.h"

#pragma region Map

glm::vec3 ECS::CubeToWorldSpace(int x, int y, int z)
{
	return glm::vec3((float)cubeSize * x, (float)cubeSize * y, (float)cubeSize * -z);
}

glm::vec3 ECS::WorldToCubeSpace(glm::vec3 position)
{
	return glm::vec3((int)(position.x / cubeSize), (int)(position.y / cubeSize), (int)(-position.z / cubeSize));
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
	
	Quaternion r = Util::GetQuaternionFromFace(actor->face);
	pos->position = CubeToWorldSpace(cube->x, cube->y, cube->z) + Util::Rotate(glm::vec3(0.0f, (float)cubeSize, 0.0f), r);
	pos->quaternion = r;

	/*glm::vec3 rotation = Util::GetRelativeUp(actor->face);
	actor->baseQuaternion = { 0.0f, rotation.x, rotation.y, rotation.z };
	Util::NormalizeQuaternion(actor->baseQuaternion);*/
}

void ECS::MoveCube(CubeComponent* cube, int x, int y, int z)
{
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

			Quaternion r = Util::GetQuaternionFromFace(actor->face);
			glm::vec3 t = targetPos->position + Util::Rotate(glm::vec3(0.0f, (float)cubeSize, 0.0f), r);

			MovementComponent* mover = (MovementComponent*)actor->entity->componentIDMap[movementComponentID];
			mover->RegisterMovement(actor->speed, t);
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

void ECS::FloodFill(std::vector<CubeComponent*> &inside, CubeComponent* cube, CubeComponent* activeCube, CubeComponent* fulcrum)
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
		float distToCube = glm::length2(glm::vec3(activeCube->x, activeCube->y, activeCube->z) - glm::vec3(cube->x, cube->y, cube->z));
		float distToFulcrum = glm::length2(glm::vec3(fulcrum->x, fulcrum->y, fulcrum->z) - glm::vec3(cube->x, cube->y, cube->z));

		if (distToCube < distToFulcrum)
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
								FloodFill(inside, nextCube, activeCube, fulcrum);
							}
						}
					}
				}
			}
		}
	}
}

std::vector<CubeComponent*> ECS::DetermineStructure(CubeComponent* cube, CubeComponent* fulcrum, Face direction)
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

	FloodFill(retCubes, activeCube, cube, fulcrum);

	if (retCubes.size() < 4)
	{
		return retCubes;
	}

	int touchingSidesFulcrum = 0;
	bool tf = false,
		tb = false,
		tr = false,
		tl = false,
		tu = false,
		td = false;

	for (int i = 0; i < retCubes.size(); i++)
	{
		CubeComponent* c = retCubes[i];
		if (c != cube)
		{
			int diffX = c->x - cube->x;
			int diffY = c->y - cube->y;
			int diffZ = c->z - cube->z;

			if (diffX == 1 && diffY == 0 && diffZ == 0) tr = true;
			else if (diffX == -1 && diffY == 0 && diffZ == 0) tl = true;
			else if (diffX == 0 && diffY == 1 && diffZ == 0) tu = true;
			else if (diffX == 0 && diffY == -1 && diffZ == 0) td = true;
			else if (diffX == 0 && diffY == 0 && diffZ == 1) tb = true;
			else if (diffX == 0 && diffY == 0 && diffZ == -1) tf = true;

			int diff = abs(fulcrum->x - c->x) + abs(fulcrum->y - c->y) + abs(fulcrum->z - c->z);

			if (diff == 1)
			{
				touchingSidesFulcrum++;
			}
		}
	}

	if (tf && direction == Face::back ||
		tb && direction == Face::front ||
		tr && direction == Face::left ||
		tl && direction == Face::right ||
		tu && direction == Face::bottom ||
		td && direction == Face::top)
	{
		retCubes.clear();
		return retCubes;
	}

	if (touchingSidesFulcrum > 2)
	{
		retCubes.clear();
		return retCubes;
	}
	else
	{
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

	// And then we can figure out where it is going to land in cube and world space.
	glm::vec3 landingUp = Util::GetRelativeUp(landingFace);
	glm::vec3 landingCubePosition = glm::vec3(landingCube->x + (int)landingUp.x, landingCube->y + (int)landingUp.y, landingCube->z + (int)landingUp.z);
	glm::vec3 landingWorldPosition = CubeToWorldSpace(landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// This is going to be the smallest t value at which a collision occurs.
	float minT = 1.0f;

	// First, we need to sweep the whole cube structure over its whole course to see if it collides with anything.
	// There might be a more elegant way to do this, but we're just gonna go ahead and do it the slow(er) way.
	for (int i = 0; i < affectedCubes.size(); i++)
	{
		// We need to grab the cube and position components, as well as their movement components.
		CubeComponent* c = affectedCubes[i];
		PositionComponent* pc = (PositionComponent*)c->entity->componentIDMap[positionComponentID];
		MovementComponent* mc = (MovementComponent*)c->entity->componentIDMap[movementComponentID];

		int dX = pivotPos.x - c->x;
		int dY = pivotPos.y - c->y;
		int dZ = pivotPos.z - c->z;

		Quaternion diffRot = Util::GetRollRotation(landingFace, roll, { 1, 0, 0, 0 }, 1);
		glm::vec3 newDifference = Util::Rotate(glm::vec3(-dX, dY, dZ), diffRot);

		// And then we need to convert that to world space, instead of cube space.
		glm::vec3 worldDifference = CubeToWorldSpace(landingCubePosition.x + (int)newDifference.x, landingCubePosition.y + (int)newDifference.y, landingCubePosition.z + (int)newDifference.z);

		glm::vec3 forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pc->position - worldDifference);
		float length = dist * cos(45) * (2.0 / 3.0f);
		glm::vec3 p1 = pc->position + (forward * length);
		glm::vec3 p2 = worldDifference + (up * length);

		BezierCurve b = { { pc->position, p1, p2, worldDifference } };

		float lastSafeT = 0.0f;
		for (float j = 0.0f; j < 1.0f; j += 0.01f)
		{
			glm::vec3 cubeSpace = WorldToCubeSpace(b.GetPoint(j));
			Entity* e = GetCube(cubeSpace.x, cubeSpace.y, cubeSpace.z);
			if (e != nullptr && e != c->entity)
			{
				bool isAffected = false;

				for (int m = 0; m < affectedCubes.size(); m++)
				{
					if (e == affectedCubes[m]->entity)
					{
						isAffected = true;
					}
				}
				
				if (!isAffected)
				{
					minT = std::min(lastSafeT, minT);
					break;
				}

				lastSafeT = j;
			}
			else
			{
				lastSafeT = j;
			}
		}
	}

	if (minT == 0.0f) return;

	// Now, we need to quickly unassign all the affected cubes from their old positions.
	// So that we don't run into any bugs when we add them to a new spot.

	for (int i = 0; i < affectedCubes.size(); i++)
	{
		CubeComponent* cube = affectedCubes[i];
		cubes[cube->x][cube->y][cube->z] = nullptr;
	}

	// Now we need to figure out how the cube is gonna roll.
	Quaternion newRotation = Util::GetRollRotation(landingFace, roll, pos->quaternion, std::max(2, (int)(2 * (minT))));
	glm::vec3 activeFinalPoint = landingWorldPosition;
	glm::vec3 activeCubeSpace = landingCubePosition;

	// Then, we register the movements to the landing position with the active cube's movement component.
	// The bezier curve is a little complicated.
	if (affectedCubes.size() > 1)
	{
		glm::vec3 forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pos->position - landingWorldPosition);
		float length = dist * cos(45) * (2.0 / 3.0f);
		glm::vec3 p1 = pos->position + (forward * length);
		glm::vec3 p2 = landingWorldPosition + (up * length);

		BezierCurve b = { { pos->position, p1, p2, landingWorldPosition } };
		activeCubeSpace = WorldToCubeSpace(b.GetPoint(minT));
		activeFinalPoint = CubeToWorldSpace(activeCubeSpace.x, activeCubeSpace.y, activeCubeSpace.z);
		dist = glm::length(pos->position - activeFinalPoint);
		length = dist * cos(45) * (2.0 / 3.0f);
		p1 = pos->position + (forward * length);
		p2 = activeFinalPoint + (up * length);

		mover->RegisterMovement(3.5f, { { pos->position, p1, p2, activeFinalPoint } }, 1.0f);
	}
	else
	{
		glm::vec3 leap = (glm::normalize(landingUp) * 5.0f);
		if (landingUp.z != 0) leap *= -1.0f;
		glm::vec3 zenith = ((pos->position + landingWorldPosition) / 2.0f) + leap;

		mover->RegisterMovement(3.5f, { { pos->position, zenith, landingWorldPosition } }, 1.0f);
	}

	mover->RegisterMovement(3.5f, {{ pos->quaternion, newRotation }}, 1.0f);

	// And finally position the active cube in the right position.
	MoveCube(activeCube, activeCubeSpace.x, activeCubeSpace.y, activeCubeSpace.z);

	// Oh, and we roll the actor onto the right face.
	RollActor(actor, roll, landingFace, standingFace, false);

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
		Quaternion affRot = Util::GetRollRotation(landingFace, roll, pc->quaternion, std::max(2, (int)(2 * (minT))));

		// And the second is the rotation in space around the initial cube.
		Quaternion diffRot = Util::GetRollRotation(landingFace, roll, { 1, 0, 0, 0 }, 1);

		// We need to record the initial difference in position between this cube
		// and the cube we rolled earlier.
		// We could've waited to finalize the movement of the "active" cube,
		// but we'll instead make use of that handy pivot point we saved earlier.
		int dX = pivotPos.x - c->x;
		int dY = pivotPos.y - c->y;
		int dZ = pivotPos.z - c->z;

		// if (dY != 0 && dZ != 0) dX *= -1.0f;

		// Now, we need to figure out what the difference should be after the rotation of the structure.
		glm::vec3 newDifference = Util::Rotate(glm::vec3(-dX, dY, dZ), diffRot);

		// And then we need to convert that to world space, instead of cube space.
		glm::vec3 worldDifference = CubeToWorldSpace(landingCubePosition.x + (int)newDifference.x, landingCubePosition.y + (int)newDifference.y, landingCubePosition.z + (int)newDifference.z);

		// Finally, we can register the necessary movements with the movement component and actually finalize the movement of the cube in cube space.
		glm::vec3 forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pc->position - worldDifference);
		float length = dist * cos(45) * (2.0 / 3.0f);
		glm::vec3 p1 = pc->position + (forward * length);
		glm::vec3 p2 = worldDifference + (up * length);

		BezierCurve b = { { pc->position, p1, p2, worldDifference } };
		glm::vec3 cubeSpace = WorldToCubeSpace(b.GetPoint(minT));
		glm::vec3 finalPoint = CubeToWorldSpace(cubeSpace.x, cubeSpace.y, cubeSpace.z);
		dist = glm::length(pc->position - finalPoint);
		length = dist * cos(45) * (2.0 / 3.0f);
		p1 = pc->position + (forward * length);
		p2 = finalPoint + (up * length);

		/*CubeComponent* lastCube = affectedCubes[j - 1];
		if (abs(cubeSpace.x - lastCube->x) + abs(cubeSpace.y - lastCube->y) + abs(cubeSpace.z - lastCube->z) > 1)
		{
			cubeSpace = (cubeSpace + glm::vec3(lastCube->x, lastCube->y, lastCube->z)) / 2.0f;
		}*/
		finalPoint = CubeToWorldSpace(cubeSpace.x, cubeSpace.y, cubeSpace.z);

		mc->RegisterMovement(3.5f, { { pc->position, p1, p2, finalPoint } }, 1.0f);
		mc->RegisterMovement(3.5f, { { pc->quaternion, affRot } }, 1.0f);

		MoveCube(c, cubeSpace.x, cubeSpace.y, cubeSpace.z);
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

	// And then we can figure out where it is going to land in cube and world space.
	glm::vec3 landingUp = Util::GetRelativeUp(landingFace);
	glm::vec3 landingCubePosition = glm::vec3(landingCube->x + landingUp.x, landingCube->y + landingUp.y, landingCube->z + landingUp.z);
	glm::vec3 landingWorldPosition = CubeToWorldSpace(landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// This is going to be the smallest t value at which a collision occurs.
	float minT = 1.0f;

	// First, we need to sweep the whole cube structure over its whole course to see if it collides with anything.
	// There might be a more elegant way to do this, but we're just gonna go ahead and do it the slow(er) way.
	for (int i = 0; i < affectedCubes.size(); i++)
	{
		// We need to grab the cube and position components, as well as their movement components.
		CubeComponent* c = affectedCubes[i];
		PositionComponent* pc = (PositionComponent*)c->entity->componentIDMap[positionComponentID];
		MovementComponent* mc = (MovementComponent*)c->entity->componentIDMap[movementComponentID];

		int dX = pivotPos.x - c->x;
		int dY = pivotPos.y - c->y;
		int dZ = pivotPos.z - c->z;

		Quaternion diffRot = Util::GetRollRotation(oppFulcrum, roll, { 1, 0, 0, 0 }, 1);
		glm::vec3 newDifference = Util::Rotate(glm::vec3(dX, dY, dZ), diffRot);

		// And then we need to convert that to world space, instead of cube space.
		glm::vec3 worldDifference = CubeToWorldSpace(landingCubePosition.x + (int)newDifference.x, landingCubePosition.y + (int)newDifference.y, landingCubePosition.z + (int)newDifference.z);

		glm::vec3 forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		glm::vec3 up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		float dist = glm::length(pc->position - worldDifference);
		float length = dist * cos(45) * (2.0 / 3.0f);
		glm::vec3 p1 = pc->position + (forward * length);
		glm::vec3 p2 = worldDifference + (up * length);

		BezierCurve b = { { pc->position, p1, p2, worldDifference } };

		float lastSafeT = 0.0f;
		for (float j = 0.0f; j < 1.0f; j += 0.01f)
		{
			glm::vec3 cubeSpace = WorldToCubeSpace(b.GetPoint(j));
			Entity* e = GetCube(cubeSpace.x, cubeSpace.y, cubeSpace.z);

			if (e != nullptr && e != c->entity)
			{
				bool isAffected = false;

				for (int m = 0; m < affectedCubes.size(); m++)
				{
					if (e == affectedCubes[m]->entity || e == landingTarget)
					{
						isAffected = true;
					}
				}

				if (!isAffected)
				{
					minT = std::min(lastSafeT, minT);
					break;
				}

				lastSafeT = j;
			}
			else
			{
				lastSafeT = j;
			}
		}
	}

	if (minT == 0.0f) return;

	for (int i = 0; i < affectedCubes.size(); i++)
	{
		CubeComponent* cube = affectedCubes[i];
		cubes[cube->x][cube->y][cube->z] = nullptr;
	}

	// Now we need to figure out how the cube is gonna roll.
	Quaternion newRotation = Util::GetRollRotation(oppFulcrum, roll, pos->quaternion, std::max(2, (int)(2 * (minT))));
	Quaternion newRotation2 = Util::GetRollRotation(oppFulcrum, roll, newRotation, std::max(2, (int)(2 * (minT))));

	// Then, we register the movements to the landing position with the active cube's movement component.
	// The bezier curve is a little complicated.
	glm::vec3 forward = Util::GetRelativeUp(roll);
	if (forward.z != 0) forward *= -1.0f;
	glm::vec3 up = Util::GetRelativeUp(oppFulcrum);
	if (up.z != 0) up *= -1.0f;
	float dist = glm::length(pos->position - landingWorldPosition);
	float length = dist * cos(45) * (2.0 / 3.0f);
	glm::vec3 p1 = pos->position + (forward * length);
	glm::vec3 p2 = landingWorldPosition + (up * length);

	mover->RegisterMovement(3.5f, { { pos->position, p1, p2, landingWorldPosition } }, 1.0f);
	mover->RegisterMovement(3.5f, { { pos->quaternion, newRotation, newRotation2 } }, 1.0f);

	// And finally position the active cube in the right position.
	MoveCube(activeCube, landingCubePosition.x, landingCubePosition.y, landingCubePosition.z);

	// Oh, and we roll the actor onto the right face.
	RollActor(actor, roll, oppFulcrum, standingFace, true);
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
		Quaternion affRot = Util::GetRollRotation(oppFulcrum, roll, pc->quaternion, std::max(2, (int)(2 * (minT))));
		Quaternion affRot2 = Util::GetRollRotation(oppFulcrum, roll, affRot, std::max(2, (int)(2 * (minT))));

		// And the second is the rotation in space around the initial cube.
		Quaternion diffRot = Util::GetRollRotation(oppFulcrum, roll, { 1, 0, 0, 0 }, 1);

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
		forward = Util::GetRelativeUp(roll);
		if (forward.z != 0) forward *= -1.0f;
		up = Util::GetRelativeUp(landingFace);
		if (up.z != 0) up *= -1.0f;
		dist = glm::length(pc->position - worldDifference);
		length = dist * cos(45) * (2.0 / 3.0f);
		p1 = pc->position + (forward * length);
		p2 = worldDifference + (up * length);

		BezierCurve b = { { pc->position, p1, p2, worldDifference } };
		glm::vec3 cubeSpace = WorldToCubeSpace(b.GetPoint(minT));
		glm::vec3 finalPoint = CubeToWorldSpace(cubeSpace.x, cubeSpace.y, cubeSpace.z);
		dist = glm::length(pc->position - finalPoint);
		length = dist * cos(45) * (2.0 / 3.0f);
		p1 = pc->position + (forward * length);
		p2 = finalPoint + (up * length);

		/*CubeComponent* lastCube = affectedCubes[j - 1];
		if (abs(cubeSpace.x - lastCube->x) + abs(cubeSpace.y - lastCube->y) + abs(cubeSpace.z - lastCube->z) > 1)
		{
			cubeSpace = (cubeSpace + glm::vec3(lastCube->x, lastCube->y, lastCube->z)) / 2.0f;
		}*/
		finalPoint = CubeToWorldSpace(cubeSpace.x, cubeSpace.y, cubeSpace.z);

		mc->RegisterMovement(3.5f, { { pc->position, p1, p2, finalPoint } }, 1.0f);
		mc->RegisterMovement(3.5f, { { pc->quaternion, affRot, affRot2 } }, 1.0f);
		MoveCube(c, cubeSpace.x, cubeSpace.y, cubeSpace.z);
	}
}

void ECS::RollCube(ActorComponent* actor, Face rollDirection)
{
	CubeComponent* activeCube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];

	// There are cubes to roll.

	// We're (maybe) going to want to check possible landing points from non-initial circumstances, so
	// we copy them into more variables so that we can change these while preserving the inputs.
	Face activeFace = actor->face;
	Face roll = rollDirection;

	// All rotations expect a fulcrum.
	std::pair<Face, bool> fulcrum = FindFulcrum(activeCube, activeFace, roll);
	if (fulcrum.second == false) return;

	glm::vec3 fulcrumPos = Util::GetRelativeUp(fulcrum.first) + glm::vec3(activeCube->x, activeCube->y, activeCube->z);
	Entity* fulcrumEntity = cubes[(int)fulcrumPos.x][(int)fulcrumPos.y][(int)fulcrumPos.z];
	CubeComponent* fulcrumCube = (CubeComponent*)fulcrumEntity->componentIDMap[cubeComponentID];
	std::vector<CubeComponent*> affectedCubes = DetermineStructure(activeCube, fulcrumCube, rollDirection);

	if (affectedCubes.size() > 0)
	{
		// Now that we know we have a fulcrum, we want to figure out the axis upon which the cube is rotating.
		// This is a function of the position of the fulcrum, the face the player is standing on, and the roll direction.
		roll = DetermineRollDirection(fulcrum.first, activeFace, roll);

		// We need to make sure that the active cube *can* roll in that direction.
		// If another cube is blocking it, the whole roll should be stopped.
		// Let's check that. We don't need any fancy function for this, just:
		glm::vec3 rollUp = Util::GetRelativeUp(roll);
		Entity* possibleBlockerEntity = GetCube(activeCube->x + (int)rollUp.x, activeCube->y + (int)rollUp.y, activeCube->z + (int)rollUp.z);

		/*bool blocked = false;

		if (possibleBlockerEntity != nullptr && affectedCubes.size() > 1)
		{
			CubeComponent* possibleBlocker = (CubeComponent*)possibleBlockerEntity->componentIDMap[cubeComponentID];

			auto result = std::find(affectedCubes.begin(), affectedCubes.end(), possibleBlocker);

			if (result == affectedCubes.end())
			{
				blocked = true;
			}
		}*/

		if (possibleBlockerEntity != nullptr) return;

		// Next we need to see if the block right above that is blocked.

		glm::vec3 standUp = Util::GetRelativeUp(actor->face);
		Entity* possibleBlockerEntity2 = GetCube(activeCube->x + (int)rollUp.x + (int)standUp.x,
			activeCube->y + (int)rollUp.y + (int)standUp.y,
			activeCube->z + (int)rollUp.z + (int)standUp.z);

		bool blocked = false;

		if (possibleBlockerEntity2 != nullptr && affectedCubes.size() > 1)
		{
			CubeComponent* possibleBlocker = (CubeComponent*)possibleBlockerEntity2->componentIDMap[cubeComponentID];

			auto result = std::find(affectedCubes.begin(), affectedCubes.end(), possibleBlocker);

			if (result == affectedCubes.end())
			{
				blocked = true;
			}
		}

		if (possibleBlockerEntity2 != nullptr && blocked) return;

		// Now, we have the "real" direction we're going to be rolling.
		// We want to figure out where we're going to land.
		// This is a function of the fulcrum and the roll direction.
		// We always want to roll "towards" the fulcrum.
		glm::vec3 landingCoords = Util::GetLandingCoords(fulcrum.first, roll) + glm::vec3(activeCube->x, activeCube->y, activeCube->z);

		// Now we need to see if there is a cube there for us to land on.
		Entity* landingTarget = GetCube(landingCoords.x, landingCoords.y, landingCoords.z);
		Face landingFace;

		// If there is a cube there, we can stop now and (check something, then) initiate the actual roll.
		if (landingTarget != nullptr)
		{
			// The face we're landing on is the opposite of the fulcrum.
			// At least so long as we're doing only a 90 degree rotation.
			landingFace = Util::OppositeFace(fulcrum.first);

			// We need to make sure that the player won't get stuck on any cubes while rolling.
			CubeComponent* landingCube = (CubeComponent*)landingTarget->componentIDMap[cubeComponentID];
			glm::vec3 landingUp = Util::GetRelativeUp(landingFace);
			glm::vec3 landingCubePosition = glm::vec3(landingCube->x + (int)landingUp.x, landingCube->y + (int)landingUp.y, landingCube->z + (int)landingUp.z);

			standUp = Util::GetRelativeUp(rollDirection);
			Entity* possibleBlockerEntity3 = GetCube(landingCubePosition.x + (int)standUp.x, landingCubePosition.y + (int)standUp.y, landingCubePosition.z + (int)standUp.z);

			if (possibleBlockerEntity3 != nullptr) return;

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
		if (landingTarget != nullptr && affectedCubes.size() == 1)
		{
			// We need to make sure that the player won't get stuck on any cubes while rolling.
			CubeComponent* landingCube = (CubeComponent*)landingTarget->componentIDMap[cubeComponentID];
			glm::vec3 landingUp = Util::GetRelativeUp(roll);
			glm::vec3 landingCubePosition = glm::vec3(landingCube->x + (int)landingUp.x, landingCube->y + (int)landingUp.y, landingCube->z + (int)landingUp.z);

			standUp = Util::GetRelativeUp(Util::OppositeFace(actor->face));
			Entity* possibleBlockerEntity3 = GetCube(landingCubePosition.x + (int)standUp.x, landingCubePosition.y + (int)standUp.y, landingCubePosition.z + (int)standUp.z);

			if (possibleBlockerEntity3 != nullptr) return;

			// We can assume that the landing face is the same as our roll direction.

			HalfRoll(actor, Util::OppositeFace(actor->face), Util::OppositeFace(fulcrum.first), roll, landingTarget, roll, affectedCubes);
			return;
		}
	}

	// This means we couldn't find a suitable landing position and thus shouldn't roll the cubes at all.
	// Oh well, better luck next time.
	return;
}

void ECS::RollActor(ActorComponent* actor, Face rollDirection, Face landingFace, Face standingFace, bool half)
{
	PositionComponent* pos = (PositionComponent*)actor->entity->componentIDMap[positionComponentID];
	MovementComponent* mover = (MovementComponent*)actor->entity->componentIDMap[movementComponentID];
	CubeComponent* cube = (CubeComponent*)actor->cube->componentIDMap[cubeComponentID];

	Quaternion r = Util::GetRollRotation(landingFace, rollDirection, pos->quaternion, 2);
	glm::vec3 endPos = CubeToWorldSpace(cube->x, cube->y, cube->z) + Util::Rotate(glm::vec3(0.0f, (float)cubeSize, 0.0f), Util::GetQuaternionFromFace(standingFace));

	glm::vec3 forward = Util::GetRelativeUp(rollDirection);
	if (forward.z != 0) forward *= -1.0f;
	glm::vec3 up = Util::GetRelativeUp(landingFace);
	if (up.z != 0) up *= -1.0f;
	float dist = glm::length(pos->position - endPos);
	float length = dist * cos(45) * (2.0 / 3.0f);
	glm::vec3 p1 = pos->position + (forward * length);
	glm::vec3 p2 = endPos + (up * length);

	mover->RegisterMovement(3.5f, { { pos->position, p1, p2, endPos } }, 1.0f);

	if (half)
	{
		Quaternion r2 = Util::GetRollRotation(landingFace, rollDirection, r, 2);
		mover->RegisterMovement(3.5f, { {pos->quaternion, r, r2 } }, 1.0f);
	}
	else
	{
		mover->RegisterMovement(3.5f, { { pos->quaternion, r } }, 1.0f);
	}

	actor->face = standingFace;
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
	componentBlocks.push_back(new ComponentBlock(new ModelSystem(), modelComponentID));
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
		int mapHeight = 5;
		int mapDepth = 5;

		int midMaxX = (ECS::main.maxWidth / 2) - (mapWidth / 2);
		int midMaxY = (ECS::main.maxHeight / 2) - (mapHeight / 2);
		int midMaxZ = (ECS::main.maxDepth / 2) - (mapDepth / 2);

		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
			{
				for (int z = 0; z < mapDepth; z++)
				{
					Entity* cube = CreateEntity(0, "Cube: " + std::to_string(x + midMaxX) + std::to_string(y + midMaxY) + " / " + std::to_string(z + midMaxZ));
					ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), cube);
					ECS::main.RegisterComponent(new CubeComponent(cube, true, x + midMaxX, y + midMaxY, z + midMaxZ, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["test"]), cube);
					ECS::main.PositionCube((CubeComponent*)cube->componentIDMap[cubeComponentID], x + midMaxX, y + midMaxY, z + midMaxZ);
					ECS::main.RegisterComponent(new MovementComponent(cube, true), cube);
					ECS::main.cubes[x + midMaxX][y + midMaxY][z + midMaxZ] = cube;
				}
			}
		}

		for (int x = 0; x < mapWidth; x++)
		{
			for (int i = 1; i < 10; i++)
			{
				Entity* cube = CreateEntity(0, "Cube: " + std::to_string(x + midMaxX) + " / " + std::to_string(-i + midMaxY) + " / " + std::to_string(mapDepth - 1 + midMaxZ));
				ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), cube);
				ECS::main.RegisterComponent(new CubeComponent(cube, true, x + midMaxX, -i + midMaxY, mapDepth - 1 + midMaxZ, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["block"]), cube);
				ECS::main.PositionCube((CubeComponent*)cube->componentIDMap[cubeComponentID], x + midMaxX, -i + midMaxY, mapDepth - 1 + midMaxZ);
				ECS::main.RegisterComponent(new MovementComponent(cube, true), cube);
				ECS::main.cubes[x + + midMaxX][-i + midMaxY][mapDepth - 1 + midMaxZ] = cube;
			}
		}

		/*Entity* cube = CreateEntity(0, "Cube: " + std::to_string(1 + midMaxX) + std::to_string(midMaxY + mapHeight - 3) + " / " + std::to_string(midMaxZ - 1));
		ECS::main.RegisterComponent(new PositionComponent(cube, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), cube);
		ECS::main.RegisterComponent(new CubeComponent(cube, true, 1 + midMaxX, midMaxY + mapHeight - 3, midMaxZ - 1, glm::vec3(cubeSize, cubeSize, cubeSize), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Game::main.textureMap["test"]), cube);
		ECS::main.PositionCube((CubeComponent*)cube->componentIDMap[cubeComponentID], 1 + midMaxX, midMaxY + mapHeight - 3, midMaxZ - 1);
		ECS::main.RegisterComponent(new MovementComponent(cube, true), cube);
		ECS::main.cubes[1 + midMaxX][midMaxY + mapHeight - 3][midMaxZ - 1] = cube;*/

		player = CreateEntity(0, "Player");
		Animation* testIdle = Game::main.animationMap["testIdle"];

		ECS::main.RegisterComponent(new PositionComponent(player, true, glm::vec3(0.0f, 0.0f, 0.0f), { 1, 0, 0, 0 }), player);
		ECS::main.RegisterComponent(new CameraFollowComponent(player, true, { -1.0f, 0.0f, 0.0f, 0.0f }, 500.0f, 40.0f, true, false, false, false), player);
		ECS::main.RegisterComponent(new InputComponent(player, true, true, 0.5f, 0.5f), player);
		ECS::main.RegisterComponent(new ActorComponent(player, true, 10.0f, Face::back, ECS::main.cubes[(mapWidth / 2) + midMaxX][midMaxY - 1][mapDepth - 1 + midMaxZ]), player);
		ECS::main.RegisterComponent(new MovementComponent(player, true), player);
		ECS::main.RegisterComponent(new ModelComponent(player, true, Game::main.modelMap["test"], { 0.0f, 2.0f, 0.0f }, {0.5f, 0.5f, 0.5f, 1.0f}, {5.0f, 5.0f, 5.0f}), player);

		PositionActor((ActorComponent*)player->componentIDMap[actorComponentID]);

		glm::vec3 possPos = ECS::main.CubeToWorldSpace((mapWidth / 2) + midMaxX, midMaxY - 1, mapDepth - 1 + midMaxZ);

		Game::main.cameraPosition += possPos;
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
		size_t n = dyingEntities.size();

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

InputComponent::InputComponent(Entity* entity, bool active, bool acceptInput, float rollDelay, float turnDelay)
{
	this->ID = inputComponentID;
	this->entity = entity;
	this->active = active;

	this->acceptInput = acceptInput;

	this->rollDelay = rollDelay;
	this->lastRoll = 0.0f;

	this->turnDelay = turnDelay;
	this->lastTurn = 0.0f;
}

#pragma endregion

#pragma region Camera Follow Component

CameraFollowComponent::CameraFollowComponent(Entity* entity, bool active, Quaternion rotation, float distance, float speed, bool track, bool lockX, bool lockY, bool lockZ)
{
	this->ID = cameraFollowComponentID;
	this->entity = entity;
	this->active = active;

	this->rotation = rotation;
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
}

#pragma endregion

#pragma region Movement Component

MovementComponent::MovementComponent(Entity* entity, bool active)
{
	this->ID = movementComponentID;
	this->entity = entity;
	this->active = active;

	this->moving = false;
}

void MovementComponent::RegisterMovement(float speed, BezierCurve curve, float targetT)
{
	this->moving = true;

	BezierMovement* m = new BezierMovement();
	m->ID = ECS::main.GetOtherID();
	m->movementType = MovementType::bezier;
	m->speed = speed;
	m->curve = curve;
	m->t = 0.0f;
	m->targetT = targetT;
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

void MovementComponent::RegisterMovement(float speed, BezierQuaternion curve, float targetT)
{
	this->moving = true;

	BezierRotatingMovement* m = new BezierRotatingMovement();
	m->ID = ECS::main.GetOtherID();
	m->movementType = MovementType::bezierRotation;
	m->speed = speed;
	m->curve = curve;
	m->targetT = targetT;
	this->queue.push_back(m);
}

#pragma endregion

#pragma region Model Component

ModelComponent::ModelComponent(Entity* entity, bool active, Model* model, glm::vec3 offset, glm::vec4 color, glm::vec3 scale)
{
	this->ID = modelComponentID;
	this->entity = entity;
	this->active = active;
	
	this->offset = offset;
	this->baseOffset = offset;

	this->model = model;
	this->color = color;
	this->scale = scale;
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

			Entity* up = nullptr;
			if (cube->y + 1 < ECS::main.maxHeight)
			{
				up = ECS::main.cubes[cube->x + 0][cube->y + 1][cube->z + 0];
				if (up != nullptr) { MovementComponent* m = (MovementComponent*)up->componentIDMap[movementComponentID]; if (m->moving) { up = nullptr; } }
			}

			Entity* down = nullptr;
			if (cube->y - 1 > 0)
			{
				down = ECS::main.cubes[cube->x + 0][cube->y - 1][cube->z + 0];
				if (down != nullptr) { MovementComponent* m = (MovementComponent*)down->componentIDMap[movementComponentID]; if (m->moving) { down = nullptr; } }
			}

			Entity* right = nullptr;
			if (cube->x + 1 < ECS::main.maxWidth)
			{
				right = ECS::main.cubes[cube->x + 1][cube->y + 0][cube->z + 0];
				if (right != nullptr) { MovementComponent* m = (MovementComponent*)right->componentIDMap[movementComponentID]; if (m->moving) { right = nullptr; } }
			}

			Entity* left = nullptr;
			if (cube->x - 1 > 0)
			{
				left = ECS::main.cubes[cube->x - 1][cube->y + 0][cube->z + 0];
				if (left != nullptr) { MovementComponent* m = (MovementComponent*)left->componentIDMap[movementComponentID]; if (m->moving) { left = nullptr; } }
			}

			Entity* back = nullptr;
			if (cube->z + 1 < ECS::main.maxDepth)
			{
				back = ECS::main.cubes[cube->x + 0][cube->y + 0][cube->z + 1];
				if (back != nullptr) { MovementComponent* m = (MovementComponent*)back->componentIDMap[movementComponentID]; if (m->moving) { back = nullptr; } }
			}

			Entity* front = nullptr;
			if (cube->z - 1 > 0)
			{
				front = ECS::main.cubes[cube->x + 0][cube->y + 0][cube->z - 1];
				if (front != nullptr) { MovementComponent* m = (MovementComponent*)front->componentIDMap[movementComponentID]; if (m->moving) { front = nullptr; } }
			}

			if (up == nullptr || down == nullptr || right == nullptr || left == nullptr || back == nullptr || front == nullptr)
			{
				Game::main.renderer->PrepareCube(cube->size, pos->position, pos->quaternion, cube->color, cube->texture->ID);
			}
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
			
			Game::main.renderer->PrepareQuad(glm::vec2(activeAnimation->width * a->scaleX, activeAnimation->height * a->scaleY), pos->position, pos->quaternion, a->color, activeAnimation->ID, cellX, cellY, activeAnimation->columns, activeAnimation->rows, a->flippedX, a->flippedY);
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
				// We want the camera to move along a sphere around the followed object
				// maintaining a constant distance from that object.
				// To do this, we need to have a few things:
				// - The position of the object.
				// - The desired point on the sphere as described by a quaternion.
				// - The distance we want the object to be along that vector.
				// Thus, what the camera follow component needs isn't an offset in space
				// but a quaternion defining the desired rotation along that sphere.

				PositionComponent* pos = (PositionComponent*)c->entity->componentIDMap[positionComponentID];
				glm::vec3 position = pos->position;
				Quaternion rotation = c->rotation;
				float d = c->distance;

				glm::vec3 p = glm::normalize(Util::Rotate({ 0.0f, 0.0f, 1.0f }, rotation));
				glm::vec3 point = position + (p * d);

				Game::main.cameraPosition = point;
				Game::main.cameraRotation = rotation;
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
				ECS::main.MoveActor(actor, (int)movement.x, (int)movement.y, (int)movement.z);
			}
			else if (!cubeControl && moveBack && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::front));
				ECS::main.MoveActor(actor, (int)movement.x, (int)movement.y, (int)movement.z);
			}
			else if (!cubeControl && moveRight && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::right));
				ECS::main.MoveActor(actor, (int)movement.x, (int)movement.y, (int)movement.z);
			}
			else if (!cubeControl && moveLeft && !mover->moving && !cube->moving)
			{
				movement = Util::GetRelativeUp(Util::GetAbsoluteFace(actor->face, Face::left));
				ECS::main.MoveActor(actor, (int)movement.x, (int)movement.y, (int)movement.z);
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

			// glm::vec3 camRot = Util::QuaternionToEuler(camFollower->rotation);
			Quaternion camRot = camFollower->rotation;

			float deltaTheta = Game::main.rotationSpeed * deltaTime * (1 / Game::main.zoom);

			bool canTurn = (input->lastTurn > input->turnDelay);
			input->lastTurn += deltaTime;

			if (rotX && canTurn)
			{
				camFollower->resetting = false;
				input->lastTurn = 0.0f;
				Game::main.lastFace = Game::main.face;
				Game::main.face = Util::GetFaceChangeVertical(Game::main.face, Game::main.corner, 1.0f);
			}
			else if (unrotX && canTurn)
			{
				camFollower->resetting = false;
				input->lastTurn = 0.0f;
				Game::main.lastFace = Game::main.face;
				Game::main.face = Util::GetFaceChangeVertical(Game::main.face, Game::main.corner, -1.0f);
			}

			if (rotY && canTurn)
			{
				camFollower->resetting = false;
				input->lastTurn = 0.0f;
				Game::main.corner = (Corner)(((int)Game::main.corner + 1) % 4);
			}
			else if (unrotY && canTurn)
			{
				camFollower->resetting = false;
				input->lastTurn = 0.0f;
				int c = (int)(Game::main.corner) - 1;
				if (c < 0) c = 3;
				Game::main.corner = (Corner)(c);
			}

			if (rotZ && canTurn)
			{
				camFollower->resetting = false;
				input->lastTurn = 0.0f;
				Game::main.lastFace = Game::main.face;
				Game::main.face = Util::GetFaceChangeHorizontal(Game::main.face, Game::main.corner, 1.0f);
			}
			else if (unrotZ && canTurn)
			{
				camFollower->resetting = false;
				input->lastTurn = 0.0f;
				Game::main.lastFace = Game::main.face;
				Game::main.face = Util::GetFaceChangeHorizontal(Game::main.face, Game::main.corner, -1.0f);
			}

			if (zoomOut)
			{
				Game::main.zoom += Game::main.zoomSpeed * deltaTime;
			}
			else if (zoomIn)
			{
				Game::main.zoom -= Game::main.zoomSpeed * deltaTime;

				if (Game::main.zoom < 0.001f)
				{
					Game::main.zoom = 0.001f;
				}
			}

			camFollower->rotation = Util::Slerp(camFollower->rotation, Util::GetCameraOrientation(Game::main.face, Game::main.lastFace, Game::main.corner), deltaTime * 8.0f);

			if (resetRotation || camFollower->resetting)
			{
				Game::main.face = Face::top;
				Game::main.lastFace = Face::top;
				Game::main.corner = Corner::bottom;
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
			pos->quaternion = Util::Slerp(pos->quaternion, Game::main.cameraRotation, 20.0f * deltaTime);
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
			/*PositionComponent* pos = (PositionComponent*)actor->entity->componentIDMap[positionComponentID];
			AnimationComponent* anim = (AnimationComponent*)actor->entity->componentIDMap[animationComponentID];
			Quaternion r = Util::GetQuaternionFromFace(actor->face);
			anim->offset = Util::Rotate(anim->baseOffset, r);*/
			// pos->quaternion = r;
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

					if (curving->t >= curving->targetT)
					{
						pos->position = curving->curve.GetPoint(curving->targetT);
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
				else if (m->movementType == MovementType::bezierRotation)
				{
					PositionComponent* pos = (PositionComponent*)move->entity->componentIDMap[positionComponentID];

					BezierRotatingMovement* curving = (BezierRotatingMovement*)m;

					curving->t += deltaTime * curving->speed;
					pos->quaternion = curving->curve.GetQuaternion(curving->t);

					if (curving->t >= curving->targetT)
					{
						pos->quaternion = curving->curve.GetQuaternion(curving->targetT);
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

#pragma region Model System

void ModelSystem::Update(int activeScene, float deltaTime)
{
	for (int i = 0; i < models.size(); i++)
	{
		ModelComponent* model = models[i];

		if (model->active && model->entity->GetScene() == activeScene ||
			model->active && model->entity->GetScene() == 0)
		{
			PositionComponent* pos = (PositionComponent*)model->entity->componentIDMap[positionComponentID];
			glm::vec3 offset = Util::Rotate(model->offset, pos->quaternion);

			Game::main.renderer->PrepareModel(model->scale, pos->position + offset, pos->quaternion, model->color, model->model);
		}
	}
}

void ModelSystem::AddComponent(Component* component)
{
	models.push_back((ModelComponent*)component);
}

void ModelSystem::PurgeEntity(Entity* e)
{
	for (int i = 0; i < models.size(); i++)
	{
		if (models[i]->entity == e)
		{
			ModelComponent* s = models[i];
			models.erase(std::remove(models.begin(), models.end(), s), models.end());
			delete s;
		}
	}
}

#pragma endregion

#pragma endregion