#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp>

#include <map>
#include <string>

#include "renderer.h"

enum class ProjectionType { perspective, orthographic };

enum class InputType { button, trigger, stickPos, stickNeg };

class Game
{
public:
	static Game main;
	GLFWwindow* window;

	Renderer* renderer;
	std::map<std::string, Texture*> textureMap;

	int windowWidth = 1280;
	int windowHeight = 720;

	glm::vec2 mousePosition = glm::vec2(0.0f, 0.0f);
	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	float cameraSpeed = 0.01f;

	glm::mat4 view;
	glm::mat4 projection;

	float nearClip = 0.1f;
	float farClip = 1500.0f;

	float fieldOfView = 180.0f;

	ProjectionType projectionType = ProjectionType::perspective;

	void UpdateProjection();

	// Keyboard and Mouse Mappings
	int clickKey = GLFW_MOUSE_BUTTON_1;

	int moveForwardKey = GLFW_KEY_KP_ADD;
	int moveBackKey = GLFW_KEY_KP_SUBTRACT;

	int moveRightKey = GLFW_KEY_D;
	int moveLeftKey = GLFW_KEY_A;

	int moveUpKey = GLFW_KEY_W;
	int moveDownKey = GLFW_KEY_S;
};

#endif