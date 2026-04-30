#include "Window.h"

Window::Window(const int aWidth, const int aHeight, const std::string& aWindowName)
	:width(aWidth)
	, height(aHeight)
	, windowName(aWindowName)
	, window(nullptr)
{
}

Window::~Window()
{
}

void Window::Init()
{
	if (!glfwInit())
	{
		std::cout << "Failed to Initialize GLFW" << std::endl;
		return;
	}

	window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
}

void Window::SwapBuffer()
{
	glfwSwapBuffers(window);
}
