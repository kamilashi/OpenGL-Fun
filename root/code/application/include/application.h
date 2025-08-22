#ifndef APPLICATION_H
#define APPLICATION_H

const struct ViewportParams
{
	int width = 0;
	int height = 0;

	void recalculate(int width, float aspectRatio)
	{
		this->width = width;
		height = static_cast<int>(width / aspectRatio);
	}

	void set(int width, int height)
	{
		this->width = width;
		this->height = height;
	}
};

namespace Application
{
	const struct WindowParams
	{
		float aspectRatio = 1.5f;
	};

	extern WindowParams startupParams;
	extern ViewportParams viewportParams;

	int runWindow();
}

#endif APPLICATION_H