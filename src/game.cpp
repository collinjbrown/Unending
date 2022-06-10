#include "game.h"
#include <glm/gtc/matrix_transform.hpp>
#include <corecrt_math_defines.h>

void Game::UpdateProjection()
{
	if (projectionType == ProjectionType::orthographic)
	{
		float halfWidth = windowWidth / 2.0f;
		float halfHeight = windowHeight / 2.0f;

		this->projection = glm::ortho(-halfWidth * zoom, halfWidth * zoom, -halfWidth * zoom, halfHeight * zoom, nearClip, farClip);

	}
	else if (projectionType == ProjectionType::perspective)
	{
		float aspectRatio = windowWidth / (float)windowHeight;

		this->projection = glm::perspective(fieldOfView, aspectRatio, nearClip, farClip);
	}
}