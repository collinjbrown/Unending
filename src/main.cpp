// main.cpp
//

#include <iostream>
#include <string>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "game.h"

Game Game::main;

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

	Game::main.window = window;

	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
	// \OpenGL Init

	// General Setup
	
	srand(time(NULL));

	Game::main.UpdateProjection();

	// \General Setup

	// Main Loop
	auto start = std::chrono::steady_clock::now();
	int frameCount = 0;

	while (!glfwWindowShouldClose(window))
	{
		// FPS Calculator
		frameCount++;
		auto now = std::chrono::steady_clock::now();
		auto diff = now - start;

		if (diff >= std::chrono::seconds(1))
		{
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
			Game::main.UpdateProjection();
		}


		// Get Meta Input
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}


		// Update
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		int focus = glfwGetWindowAttrib(window, GLFW_FOCUSED);

		if (focus && !windowMoved)
		{
			// Simulate the world and whatnot.
		}

		glfwSwapBuffers(window);
		windowMoved = 0;
		glfwPollEvents();
	}

	// \Main Loop

	// Shutdown
	glfwTerminate();
	return 0;
}