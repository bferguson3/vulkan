#include "HelloTriangle.hpp"

void HelloTriangleApplication::run()
{
	initWindow(); // for GLFW

	if (initVulkan() != OK) {
		throw std::runtime_error("Vulkan init failed.\n");
	}
	else 
	{
		#ifdef DEBUG 
			printf("DEBUG: Extensions enabled! Starting application.\n");
		#endif 

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
	// cleanup and end
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

	createSurface();

	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();

	return OK;
}

void HelloTriangleApplication::createSurface()
{
	if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Could not create glfw window surface!");
	// else OK.
}

void HelloTriangleApplication::createLogicalDevice()
{
	/*
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	
	VkDeviceQueueCreateInfo queueCreateInfo{}; // creates empty struct
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f; // < REQUIRED!!
	queueCreateInfo.pQueuePriorities = &queuePriority;
	*/
	// create presentation queue:
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // create array of structs
	std::set<uint32_t> uniqueQueueFamilies = { 
		indices.graphicsFamily.value(), indices.presentFamily.value()
	}; // < make a new set variable type that contains the queue families
	// 
	float queuePriority = 1.0f;
	for(uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority; // dont forget to set this!
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	//createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	// legacy support:
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if(enableValidationLayers){
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}
	// finally create device
	if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical Vulkan device.");
	// otherwise OK!
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	#ifdef DEBUG 
		printf("DEBUG: Graphics family queue index: %d\n", indices.graphicsFamily.value());
		printf("DEBUG: Presentation family queue index: %d\n", indices.presentFamily.value());
	#endif
}


void HelloTriangleApplication::pickPhysicalDevice()
{
	physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCt = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCt, nullptr); // native class member VkInstance
	// Throw if no good GPUs found
	if(deviceCt == 0) throw std::runtime_error("No Vulkan-supported GPU detected.");
	// otherwise, put them in a new vector
	std::vector<VkPhysicalDevice> devices(deviceCt);
	vkEnumeratePhysicalDevices(instance, &deviceCt, devices.data());
	for (const auto& device : devices)
	{
		if(isDeviceSuitable(device)){
			physicalDevice = device;
			break;
		}
	}
	if(physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("No Vulkan-supported GPU found.");
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
	// TODO: Add logic to explicitly prefer a device with draw/present in the same queue
	QueueFamilyIndices indices;
	// find graphics queue family
	uint32_t queueFamilyCount = 0;
	// run once to get count, then again to get details
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
		// Ensure graphics queue family and physical device support Khronos Surface rendering
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if(presentSupport) indices.presentFamily = i;

		if(indices.isComplete()) {
			#ifdef DEBUG 
				printf("DEBUG: KHR surface support in graphics queue family found!\n");
			#endif 
			break;
		}
		i++;
	}

	return indices;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
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

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	
	// swap chain support test - 1 format and 1 present mode OK
	bool swapChainAdequate = false;
	if(extensionsSupported){
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		#ifdef DEBUG 
			printf("DEBUG: At least one framebuffer format and display mode found. Continuing...\n");
		#endif 
	}
	
	return indices.isComplete() && extensionsSupported && swapChainAdequate; //graphicsFamily.has_value();
	//return true;
}

// PIXEL FORMAT AND COLOR DEPTH
VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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
VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

void HelloTriangleApplication::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// try to get 1 extra framebuffer image, for optimization sake
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount){
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	// tie swapchain to app's parent VkSurfaceKHR surface
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	// populate all the good stuff 
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // 2 if you are making stereoscopic 3D 
	// direct render:
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// render to separate image(post-process):
	//createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { 
		indices.graphicsFamily.value(), 
		indices.presentFamily.value()
	};
	if(indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else { // almost always this:
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // optional
		createInfo.pQueueFamilyIndices = nullptr; //optional 
	}
	 // do not apply an additional transform
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	// ignore alpha channel
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // unless you need to read clipped pixels for some reason!
	createInfo.oldSwapchain = VK_NULL_HANDLE; // for later

	if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("Couldn't create swapchain (aka framebuffer)!\n");
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

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
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
	vkDestroySwapchainKHR(device, swapChain, nullptr); //  before device
	vkDestroyDevice(device, nullptr); 
	vkDestroySurfaceKHR(instance, surface, nullptr); // before instance
	if(enableValidationLayers){
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroyInstance(instance, nullptr); // after surface
	
	glfwDestroyWindow(window); // after vulkan
	glfwTerminate();
	#ifdef DEBUG 
		printf("DEBUG: Process cleaned up OK.\n");
	#endif 
}
