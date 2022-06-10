#ifndef GAME_H
#define GAME_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum class ProjectionType { perspective, orthographic };

class Game
{
public:
	static Game main;
	GLFWwindow* window;

	int windowWidth = 1280;
	int windowHeight = 720;

	glm::vec2 mousePosition = glm::vec2(0.0f, 0.0f);
	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 view;
	glm::mat4 projection;

	float farClip = 0.0f;
	float nearClip = 0.0f;

	float fieldOfView = 90.0f;

	ProjectionType projectionType = ProjectionType::perspective;

	void UpdateProjection();
};

#endif