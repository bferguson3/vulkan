#include <cstdlib> 
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>

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

struct QueueFamilyIndices { 
        std::optional<uint32_t> graphicsFamily; // rendering hardware
        std::optional<uint32_t> presentFamily;  // displaying hardware
        
        bool isComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
        }
};

const std::vector<const char*> deviceExtensions = \
{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME 
};

struct SwapChainSupportDetails \
{
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
};

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
                VkSurfaceKHR surface;

                VkDevice device;
                VkPhysicalDevice physicalDevice;
                VkQueue graphicsQueue;
                VkQueue presentQueue;
                VkSwapchainKHR swapChain;

                #ifdef RASPI
                // No longer needed, legacy
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
                void createLogicalDevice();
                VkResult createInstance();
                void createSurface();
                
                bool checkExtensions();
                std::vector<const char*> getRequiredExtensions();
                bool checkDeviceExtensionSupport(VkPhysicalDevice device);
                SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
                VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

                bool checkValidationLayerSupport();
                void pickPhysicalDevice();
                bool isDeviceSuitable(VkPhysicalDevice device);
                QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
                VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
                void createSwapChain();

                void mainLoop();
                
                void cleanup();

                static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(\
                        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,\
                        VkDebugUtilsMessageTypeFlagsEXT messageType,\
                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,\
                        void* pUserData);
                void setupDebugMessenger();
};
