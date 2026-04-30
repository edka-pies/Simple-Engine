#pragma once


#include "GLFW/Include/glfw3.h"
#include <string>
#include <iostream>

class Window
{

public:
	Window(const int aWidth, const int aHeight, const std::string& aWindowName);
	~Window();

	void Init();

	inline bool ShouldClose() { return glfwWindowShouldClose(window); }

	inline GLFWwindow& GetWindow() const { return *window; }

	void SwapBuffer();

private:

	const int width;
	const int height;

	std::string windowName;
	GLFWwindow* window;
};

