#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main()
{
	// GLFW to make window context
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Test", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << extensionCount << " extensions supported.\n";

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	// Until exit:
	glfwDestroyWindow(window); // destroy window
	glfwTerminate(); // destroy context

	return 0;
}