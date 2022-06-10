#include "game.h"
#include <glm/gtc/matrix_transform.hpp>

void Game::UpdateProjection()
{
	if (projectionType == ProjectionType::orthographic)
	{
		float halfWidth = windowWidth / 2.0f;
		float halfHeight = windowHeight / 2.0f;

		this->projection = glm::ortho(-halfWidth, halfWidth, -halfWidth, halfHeight, nearClip, farClip);

	}
	else if (projectionType == ProjectionType::perspective)
	{
		float aspectRatio = windowWidth / (float)windowHeight;

		this->projection = glm::perspective(fieldOfView, aspectRatio, nearClip, farClip);
	}
}