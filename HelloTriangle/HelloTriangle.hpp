#include <cstdlib> 
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>

// main app defines here:
#include "benvulkan.hpp"

#define WINDOW_TITLE "Bent Vulkan"

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
		VkSurfaceKHR surface;

        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews; // 'views' are portions of an image
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        // Functions
        void initWindow();
        int initVulkan();
        void createLogicalDevice();
        //VkResult createInstance();
        void createSurface();
        void createImageViews();
        void createSwapChain();
        void createGraphicsPipeline();
        VkShaderModule createShaderModule(const std::vector<char>& code);

        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        
        void mainLoop();
        
        void cleanup();

        void setupDebugMessenger();
};
