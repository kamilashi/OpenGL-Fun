#include <glad.h>
#include <GLFW/glfw3.h>

const struct WindowParams
{
	 float aspectRatio = 1.5f;
};

int main(int argc, char* argv[])
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	WindowParams startupParams;
	int width = static_cast<int>(mode->width / 2.0f);
	int height = static_cast<int>(width/ startupParams.aspectRatio);

	window = glfwCreateWindow(static_cast<int>(mode->width/2.0f), static_cast<int>(mode->height / 2.0f), "OpenGLApp", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		// failed to load OpenGL via GLAD
		return -1;
	}

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
