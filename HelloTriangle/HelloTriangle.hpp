#include <cstdlib> 
#include <vector>
#include <cstring>
#include <stdexcept>

//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>

// App defines
#define WIDTH 800
#define HEIGHT 600

#define DEBUG 

#define OK 0

class HelloTriangleApplication
{
	public:
                void run();

	private:
		GLFWwindow* window;
		VkInstance instance;
		std::vector<const char*> gl_extensions;

                void mainLoop();
                void initWindow();
                int initVulkan();
                VkResult createInstance();
                bool checkExtensions();
                void cleanup();		
};
