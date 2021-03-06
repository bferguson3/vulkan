
#include <iostream>
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>

// App
#include "HelloTriangle.hpp"


int main()
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}