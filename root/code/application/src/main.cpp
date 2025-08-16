#include <glad.h>
#include <GLFW/glfw3.h>
#include "types.h"

const struct WindowParams
{
	 float aspectRatio = 1.5f;
};

const float vertexPositions[] = {
	0.75f, 0.75f, 0.0f,
	0.75f, -0.75f, 0.0f,
	-0.75f, -0.75f, 0.0f,
	-0.75f, 0.75f, 0.0f,
}; 

unsigned int vertexIndices[] = {  
	0, 1, 3,  
	1, 2, 3  
};

uint VBO;
uint EBO;


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

	window = glfwCreateWindow(static_cast<int>(mode->width/2.0f), static_cast<int>(mode->height / 2.0f), "OpenGLApp", nullptr, nullptr);
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

	glGenBuffers(1, &VBO);
	glBindVertexArray(VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices), vertexIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
	glEnableVertexAttribArray(0);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		//glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(VBO);
		glDrawElements(GL_TRIANGLES, sizeof(vertexIndices) / sizeof(int), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
