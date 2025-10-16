
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "application.h"
#include "graphics.h"
#include "simulation.h"
#include "scene.h"
#include "uicontroller.h"

#include "profiler.h"

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

void setupImGUI(GLFWwindow* pWindow, GLFWmonitor* pPrimaryMonitor)
{
	float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(pPrimaryMonitor);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        
	style.FontScaleDpi = main_scale;        

	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);

	const char* glsl_version = "#version 330 core";
	ImGui_ImplOpenGL3_Init(glsl_version);
}

int Application::runWindow()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();


	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	viewportParams.recalculate(static_cast<int>(mode->width / 2.0f), startupParams.aspectRatio);

	window = glfwCreateWindow(viewportParams.width, viewportParams.height, "Jet Loop", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// VSync
	glfwSwapInterval(1);

	// GUI
	setupImGUI(window, primaryMonitor);

	PROFILER_INIT();

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		return -1;
	}

	UI::SceneControlData sceneControlData;
	sceneControlData.fillPreset0();

	Scene scene = Scene();
	scene.create(viewportParams);

	Graphics::prepare(&scene);

	glfwSetFramebufferSizeCallback(window, onWindowResize);

	while (!glfwWindowShouldClose(window))
	{
		float totalTime = static_cast<float>(glfwGetTime());

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		PROFILER_START_FRAME();

		UI::showUiWidget(&sceneControlData);

		{
		PROFILE_SCOPE("Frame");
		scene.update(sceneControlData);

		Simulation::run(&scene, totalTime);
		Graphics::render(&scene, viewportParams, sceneControlData.showShadowDepthMap, totalTime);
		}

		PROFILER_END_FRAME();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	PROFILER_END();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}