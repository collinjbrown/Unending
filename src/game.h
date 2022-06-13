#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp>

#include <map>
#include <string>

#include "component.h"
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
	std::map<std::string, Animation*> animationMap;

	int windowWidth = 1280;
	int windowHeight = 720;

	glm::vec2 mousePosition = glm::vec2(0.0f, 0.0f);
	glm::vec3 cameraPosition = glm::vec3(900.0f, 500.0f, 300.0f);
	Quaternion cameraRotation = { 1, 0, 0, 0 };
	glm::vec3 baseCameraRotation = glm::vec3(-0.77f, -0.77f, 0.0f);

	float orthographicSpeedModifier = 100.0f;

	float zoom = 8.5f;
	float cameraSpeed = 80.0f;
	float rotationSpeed = 20.0f;
	float zoomSpeed = 10.0f;

	float dTime = 1.0f;

	glm::mat4 view;
	glm::mat4 projection;

	float nearClip = 0.1f;
	float farClip = 15000.0f;

	float fieldOfView = 180.0f;

	ProjectionType projectionType = ProjectionType::orthographic;

	void UpdateProjection();

	// Keyboard and Mouse Mappings
	int clickKey = GLFW_MOUSE_BUTTON_1;

	int moveForwardKey = GLFW_KEY_W;
	int moveBackKey = GLFW_KEY_S;

	int moveRightKey = GLFW_KEY_D;
	int moveLeftKey = GLFW_KEY_A;

	int cubeControlKey = GLFW_KEY_LEFT_CONTROL;

	/*int moveUpKey = GLFW_KEY_W;
	int moveDownKey = GLFW_KEY_S;*/

	int camForwardKey = GLFW_KEY_O;
	int camBackKey = GLFW_KEY_P;

	int camRightKey = GLFW_KEY_RIGHT;
	int camLeftKey = GLFW_KEY_LEFT;

	int camUpKey = GLFW_KEY_UP;
	int camDownKey = GLFW_KEY_DOWN;

	int freeCamKey = GLFW_KEY_RIGHT_CONTROL;

	int rotateXKey = GLFW_KEY_KP_2;
	int unrotateXKey = GLFW_KEY_KP_8;

	int rotateYKey = GLFW_KEY_KP_6;
	int unrotateYKey = GLFW_KEY_KP_4;

	int rotateZKey = GLFW_KEY_KP_9;
	int unrotateZKey = GLFW_KEY_KP_7;

	int zoomInKey = GLFW_KEY_KP_ADD;
	int zoomOutKey = GLFW_KEY_KP_SUBTRACT;

	int resetRotationKey = GLFW_KEY_R;
};

#endif