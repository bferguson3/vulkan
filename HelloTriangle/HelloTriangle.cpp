#include "HelloTriangle.hpp"

void HelloTriangleApplication::run()
{
		initWindow(); // for GLFW

		if (initVulkan() != OK) {
				throw std::runtime_error("Vulkan init failed.\n");
		}
		else {
				mainLoop();
		}
		
		cleanup();
} 

// * MAIN * // 		
void HelloTriangleApplication::mainLoop()
{
		while (!glfwWindowShouldClose(window))
		{
				glfwPollEvents();
		}
}

// * GLFW / VULKAN INIT * // 
void HelloTriangleApplication::initWindow() 
{
		glfwInit();

		// Disable GLFW api and resizeable
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Ben Vulkan", nullptr, nullptr);
}

int HelloTriangleApplication::initVulkan()
{
		if(createInstance() != VK_SUCCESS)
				throw std::runtime_error("failed to make Vulkan instance.\n");
		
		if(!checkExtensions())
				throw std::runtime_error("Vulkan requirements not met.\n");

		setupDebugMessenger();

		return OK;
}



void HelloTriangleApplication::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);
	/*
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | \
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | \
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | \
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | \
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; //opt
	*/
	if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to setup debug messenger.");
	}
}


VkResult HelloTriangleApplication::createInstance()
{
	// validation layer check - only if debug mode
	if(enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers required. Exiting.\n");
	}
	// appInfo
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	
	appInfo.pEngineName = "Bentgine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// createInfo
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
	for(uint8_t c = 0; c < glfwExtensionCount; c++)
		gl_extensions[c] = glfwExtensions[c];

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	
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

bool HelloTriangleApplication::checkExtensions()
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

// gets string[] of extensions required based on validation/debug enabled
std::vector<const char*> HelloTriangleApplication::getRequiredExtensions() 
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if(enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}


bool HelloTriangleApplication::checkValidationLayerSupport()
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


// * APP CLEANUP * // 
void HelloTriangleApplication::cleanup()
{
		if(enableValidationLayers){
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
		#ifdef DEBUG 
		printf("DEBUG: Process cleaned up OK.\n");
		#endif 
}
