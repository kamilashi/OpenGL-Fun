
#include <glm/gtc/matrix_transform.hpp>

const struct PerspCameraParams
{
	float fov = 45.0f;
	float nearPlane = 0.01f;
	float farPlane = 100.0f;
};

class Camera
{
public:

	glm::mat4 projMatrix; 
	glm::mat4 viewMatrix;
	glm::vec3 position;


	void createPerspectiveProjection(PerspCameraParams camParams, int winWidth, int winHeight)
	{
		projMatrix = glm::perspective(glm::radians(camParams.fov), (float)winWidth / (float)winHeight, camParams.nearPlane, camParams.farPlane);
	}

	void createOrthogonalProjection(float left, float right, float top, float bottom, float nearPlane, float farPlane, glm::vec3 viewVector)
	{
		projMatrix = glm::ortho(left, right, top, bottom, nearPlane, farPlane);
	}

	void createView(glm::vec3 position)
	{
		this->position = position;
		viewMatrix = glm::translate(glm::mat4(1.0f), -position);
	}

	void lookAt(glm::vec3 target)
	{
		glm::vec3 worldUp = { 0.f, 1.f, 0.f };

		viewMatrix = glm::lookAt(position, target, worldUp);
	}

};

