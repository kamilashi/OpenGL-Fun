
#include <GLFW/glfw3.h>

#include "application.h"
#include "graphics.h"
#include "simulation.h"
#include "scene.h"

namespace Application
{
	WindowParams startupParams;
	ViewportParams viewportParams;
}

void onWindowResize(GLFWwindow* pWin, int width, int height)
{
	if (height == 0)
	{
		return;
	}

	Graphics::pViewPortCamera->createPerspectiveProjection(*Graphics::pViewPortCameraParams, width, height);
	Application::viewportParams.set(width, height);
	glViewport(0, 0, width, height);
}

int Application::runWindow()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	viewportParams.recalculate(static_cast<int>(mode->width / 2.0f), startupParams.aspectRatio);

	window = glfwCreateWindow(viewportParams.width, viewportParams.height, "OpenGLApp", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// VSync
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		return -1;
	}

	Scene scene = Scene();
	scene.create(viewportParams);

	Graphics::prepare(&scene);

	glfwSetFramebufferSizeCallback(window, onWindowResize);

	while (!glfwWindowShouldClose(window))
	{
		float totalTime = static_cast<float>(glfwGetTime());

		Simulation::run(&scene, totalTime);
		Graphics::render(&scene, viewportParams, totalTime);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}