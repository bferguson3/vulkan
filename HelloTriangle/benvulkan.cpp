//#define GLFW_INCLUDE_VULKAN 
//#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <optional>
#include <set>

#include "benvulkan.hpp"

#define APPLICATION_NAME "Hello Triangle"
#define ENGINE_NAME "Bentgine"

std::vector<const char*> gl_extensions;

VkResult createInstance(VkInstance& instance)
{
	// validation layer check - only if debug mode
	if(enableValidationLayers && !checkValidationLayerSupport(validationLayers))
	{
		throw std::runtime_error("Validation layers required. Exiting.\n");
	}
	// appInfo
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	
	appInfo.pApplicationName = APPLICATION_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// createInfo (pg 58)
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	// get string array and number of glfw required vulkan extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;
	
	// store required gl extensions globally
	gl_extensions = std::vector<const char*>(glfwExtensionCount);
	for(uint8_t c = 0; c < glfwExtensionCount; c++){
		gl_extensions[c] = glfwExtensions[c];
		#ifdef DEBUG
			printf("DEBUG: glfw required extension: %s\n", gl_extensions[c]);
		#endif
	}
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	
	//pg58 cont'd
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// store validation layers
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		// allot debugger for instance create/destroy:
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// finally create vulkan instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	return result;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,\
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, \
    const VkAllocationCallbacks* pAllocator, \
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else 
        return VK_ERROR_EXTENSION_NOT_PRESENT;

}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,\
    const VkDebugUtilsMessengerEXT debugMessenger, \
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr)
    {
        return func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(\
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,\
	VkDebugUtilsMessageTypeFlagsEXT messageType,\
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,\
	void* pUserData)
{
	std::cerr << "DEBUG: validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | \
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | \
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | \
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | \
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

bool checkValidationLayerSupport(const std::vector<const char*> validationLayers)
{
	// Get num of layers and layer properties 
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	printf("Layers: %d\n", layerCount);
    
    for(const char* layerName : validationLayers) 
	{
		printf("validation layers required: %s\n", layerName);
		bool layerFound = false;
		for(const auto& layerProperties : availableLayers)
		{
			printf("found layers: %s\n", layerProperties.layerName);
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				#ifdef DEBUG
					printf("DEBUG: Validation layer found.\n");
				#endif 

				layerFound = true;
				break;
			}
		}
		if(!layerFound) 
			return false;
	}
	
	return true;
}


// PIXEL FORMAT AND COLOR DEPTH
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for(const auto& availableFormat : availableFormats)
	{
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && \
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			#ifdef DEBUG 
				printf("DEBUG: Selected priority format and colorspace: B8G8R8A8_SRGB and SRGB_NONLINEAR\n");
			#endif
			return availableFormat;
		}
	}
	#ifdef DEBUG 
		printf("DEBUG: Selected DEFAULT format and colorspace: %d, %d\n", availableFormats[0].format, availableFormats[0].colorSpace);
	#endif
	return availableFormats[0];
}

// VSYNC / RELAXED VSYNC / TRIPLE BUFFERING
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for(const auto& availablePresentMode : availablePresentModes )
	{
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			#ifdef DEBUG 
				printf("DEBUG: Selecting Mailbox presentation mode (triple buf)\n");
			#endif 
			return availablePresentMode;
		}
	}

	#ifdef DEBUG 
		printf("DEBUG: Selecting FIFO presentation mode (vsync)\n");
	#endif 
	return VK_PRESENT_MODE_FIFO_KHR;
}

// RESOLUTION OF SURFACE 
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if(capabilities.currentExtent.width != UINT32_MAX)
	{
		#ifdef DEBUG 
			printf("DEBUG: Selected default surface resolution: %d x %d\n", capabilities.currentExtent.width, capabilities.currentExtent.height);
		#endif 
		return capabilities.currentExtent;
	} 
	else {
		VkExtent2D actualExtent = { _WIDTH, _HEIGHT };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		#ifdef DEBUG 
			printf("DEBUG: Selected NEW surface resolution: %d x %d\n", actualExtent.width, actualExtent.height);
		#endif 
		return actualExtent;
	}
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	// get the capabilities of the framebuffer, pixel formats, and presentation modes.
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if(formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if(presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


// gets string[] of extensions required based on validation/debug enabled
std::vector<const char*> getRequiredExtensions() 
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if(enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}


bool checkExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	// make 'extensions' array
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	
	// Make sure every required GLFW extension exists in our supported extensions
	uint8_t found = 0;
	for(uint8_t c = 0; c < gl_extensions.size(); c++)
	{
		bool ok = false;
		for(const auto& extension : extensions) 
		{
			const char* key = gl_extensions[c];
			const char* abv = extension.extensionName;
			if(strcmp(key,abv) == 0) {
				found++;
				ok = true;
			}
		}
		if(!ok)
		{
			printf("Error: Required extension %s not found.\n",gl_extensions[c]);
			return false;
		}
	}
	if(found < gl_extensions.size()){
		printf("Error: All extensions not found.\n");
		return false;
	}

	#ifdef DEBUG
		printf("DEBUG: Vulkan init OK: %d extensions detected\n", extensionCount);
	#endif 

	return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	
	// how many dvc extensions?
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	// put the data into a new vector
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
	// make a string array of their names, and erase the ones we find...
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for(const auto& extension:availableExtensions)
	{
		#ifdef DEBUG 
			printf("DEBUG: Available extension: %s\n", extension.extensionName);
		#endif 
		requiredExtensions.erase(extension.extensionName);
	}
	// if its empty, we are good
	return requiredExtensions.empty();
}
