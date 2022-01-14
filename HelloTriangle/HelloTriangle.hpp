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

		GLFWwindow* window;         // window wrapper

		VkInstance instance;        // vulkan instance
        VkDebugUtilsMessengerEXT debugMessenger; // vulkan debugger
		VkSurfaceKHR surface;       // render surface
        VkDevice device;            // logical device
        VkPhysicalDevice physicalDevice;    // physical device
        VkQueue graphicsQueue;      // render queue
        VkQueue presentQueue;       // display queue
        VkSwapchainKHR swapChain;   // framebuffer contents
        std::vector<VkImage> swapChainImages;   // image data
        std::vector<VkImageView> swapChainImageViews; // 'views' are portions of an image
        VkFormat swapChainImageFormat;  // pixel format
        VkExtent2D swapChainExtent;     // display size
        VkPipelineLayout pipelineLayout;    // shader configuration
        VkRenderPass renderPass;        // rendering subpass definitions
        VkPipeline graphicsPipeline;    // container
        std::vector<VkFramebuffer> swapChainFramebuffers;   // swapchain + pipeline = framebuffer
        VkCommandPool commandPool;      // set command pool to graphics or present family (graphics)
        std::vector<VkCommandBuffer> commandBuffers; // allocates and records swapchain draw commands
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;

        // Functions
        void initWindow();
        int initVulkan();
        void createLogicalDevice();
        //VkResult createInstance();
        void createSurface();
        void createImageViews();
        void createSwapChain();
        void createRenderPass();
        void createGraphicsPipeline();
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void createCommandPool();
        void createCommandBuffers();
        void createFramebuffers();
        void createSemaphores();

        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        
        void mainLoop();
        void drawFrame();

        void cleanup();

        void setupDebugMessenger();
};
