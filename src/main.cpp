// main.cpp
//

#include <iostream>
#include <string>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "game.h"
#include "ecs.h"
#include "util.h"

Game Game::main;
ECS ECS::main;

static int windowMoved = 0;
void WindowPosCallback(GLFWwindow* window, int xpos, int ypos)
{
	windowMoved = 1;
}

int main(void)
{
	// OpenGL Init
	GLFWwindow* window;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHintString(GLFW_X11_CLASS_NAME, "OpenGL");
	glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "OpenGL");

	window = glfwCreateWindow(Game::main.windowWidth, Game::main.windowHeight, "Unending", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		std::cout << "Failed to create Opengl Window." << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << '\n';
		return -1;
	}

	glfwSetWindowPosCallback(window, WindowPosCallback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	Game::main.window = window;

	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
	// \OpenGL Init

	// General Setup
	
	srand(time(NULL));

	ECS::main.Init();

	Game::main.UpdateProjection();

	Texture* whiteTexture = Texture::whiteTexture();

	Renderer renderer{ whiteTexture->ID };
	Game::main.renderer = &renderer;

	Texture test{ "assets/sprites/test2.png", true, GL_NEAREST };
	renderer.textureIDs.push_back(test.ID);
	Game::main.textureMap.emplace("test", &test);

	Texture block{ "assets/sprites/block.png", true, GL_NEAREST };
	renderer.textureIDs.push_back(block.ID);
	Game::main.textureMap.emplace("block", &block);

	Animation testIdle{ "assets/animations/test/test_idle.png", true, 2, 2, 0.5f, { 2, 2 }, true, GL_NEAREST };
	renderer.textureIDs.push_back(testIdle.ID);
	Game::main.animationMap.emplace("testIdle", &testIdle);

	// \General Setup

	// Main Loop
	float checkedTime = glfwGetTime();
	auto start = std::chrono::steady_clock::now();
	int frameCount = 0;

	while (!glfwWindowShouldClose(window))
	{
		// Delta Time
		float deltaTime = glfwGetTime() - checkedTime;
		checkedTime = glfwGetTime();

		// FPS Calculator
		frameCount++;
		auto now = std::chrono::steady_clock::now();
		auto diff = now - start;

		if (diff >= std::chrono::seconds(1))
		{
			/*std::cout << "Camera Position: " + std::to_string(Game::main.cameraPosition.x) + " / " + std::to_string(Game::main.cameraPosition.y) + " / " + std::to_string(Game::main.cameraPosition.z) << std::endl;
			std::cout << "Camera Rotation: " + std::to_string(Game::main.cameraRotation.x) + " / " + std::to_string(Game::main.cameraRotation.y) + " / " + std::to_string(Game::main.cameraRotation.z) << std::endl;
			std::cout << "Camera Zoom: " + std::to_string(Game::main.zoom) << std::endl;*/

			start = now;
			std::cout << "Frame Count: " + std::to_string(frameCount) << std::endl;

			frameCount = 0;
		}


		// Update View
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		if (w != Game::main.windowWidth || h != Game::main.windowHeight)
		{
			Game::main.windowWidth = w;
			Game::main.windowHeight = h;
		}

		Game::main.UpdateProjection();
		glm::vec3 cameraPosition = Game::main.cameraPosition;// *Game::main.zoom;
		glm::vec3 up = Util::Rotate(glm::vec3(0.0f, 1.0f, 0.0f), Game::main.cameraRotation);
		glm::vec3 center = cameraPosition + Util::Rotate(glm::vec3(0.0f, 0.0f, -1.0f), Game::main.cameraRotation);
		Game::main.view = glm::lookAt(cameraPosition, center, up);

		if (Game::main.projectionType == ProjectionType::perspective)
		{
			Game::main.view = glm::inverse(Game::main.view);
		}


		// Get Meta Input
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
		
		Util::NormalizeQuaternion(Game::main.cameraRotation);
		/*glm::vec3 cameraRotation = Util::QuaternionToEuler(Game::main.cameraRotation);
		cameraRotation.x = fmod(Game::main.cameraRotation.x, 6.2832f);
		cameraRotation.y = fmod(Game::main.cameraRotation.y, 6.2832f);
		cameraRotation.z = fmod(Game::main.cameraRotation.z, 6.2832f);
		Game::main.cameraRotation = Util::EulerToQuaternion(cameraRotation);*/

		// Game::main.cameraRotation = glm::vec3(fmod(Game::main.cameraRotation.x, 6.0f), fmod(Game::main.cameraRotation.y, 6.0f), fmod(Game::main.cameraRotation.z, 6.0f));

		// Update
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		int focus = glfwGetWindowAttrib(window, GLFW_FOCUSED);

		if (focus && !windowMoved)
		{
			float dT = deltaTime * Game::main.dTime;
			ECS::main.Update(dT);
		}

		Game::main.renderer->Display();
		Game::main.renderer->ResetBuffers();

		glfwSwapBuffers(window);
		windowMoved = 0;
		glfwPollEvents();
	}

	// \Main Loop

	// Shutdown
	glfwTerminate();
	return 0;
}