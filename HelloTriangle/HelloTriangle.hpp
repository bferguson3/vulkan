#include <cstdlib> 
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>

//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>

// App defines

#define _WIDTH 800
#define _HEIGHT 600

#define DEBUG 
//#define RASPI 

#define OK 0

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(\
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,\
	VkDebugUtilsMessageTypeFlagsEXT messageType,\
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,\
	void* pUserData);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,\
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, \
        const VkAllocationCallbacks* pAllocator, \
        VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,\
        const VkDebugUtilsMessengerEXT debugMessenger, \
        const VkAllocationCallbacks* pAllocator);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


class HelloTriangleApplication
{
	public:
                void run();

	private:
                const uint32_t WIDTH = _WIDTH;
                const uint32_t HEIGHT = _HEIGHT;

		GLFWwindow* window;
		VkInstance instance;
                VkDebugUtilsMessengerEXT debugMessenger;
		std::vector<const char*> gl_extensions;

                #ifdef RASPI
                const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
                #else 
                const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
                #endif

                #ifdef DEBUG 
                const bool enableValidationLayers = true;
                #else 
                const bool enableValidationLayers = false;
                #endif

                // Functions
                void initWindow();
                int initVulkan();
                VkResult createInstance();
                
                bool checkExtensions();
                std::vector<const char*> getRequiredExtensions();
                
                bool checkValidationLayerSupport();
                
                void mainLoop();
                
                void cleanup();

                static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(\
                        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,\
                        VkDebugUtilsMessageTypeFlagsEXT messageType,\
                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,\
                        void* pUserData);
                void setupDebugMessenger();
};
