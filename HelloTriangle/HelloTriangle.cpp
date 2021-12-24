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

	window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);
}

int HelloTriangleApplication::initVulkan()
{
	if(createInstance(instance) != VK_SUCCESS)
		throw std::runtime_error("failed to make Vulkan instance.\n");
	
	if(!checkExtensions())
		throw std::runtime_error("Vulkan requirements not met.\n");

	setupDebugMessenger();

	createSurface();

	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createGraphicsPipeline(); // < exciting!

	return OK;
}

void HelloTriangleApplication::createGraphicsPipeline()
{
	auto vertShaderCode = readBinaryFile("shaders/hello.vert.spv");
	auto fragShaderCode = readBinaryFile("shaders/hello.frag.spv");
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule; // point to the code wrapper
	vertShaderStageInfo.pName = "main"; // entrypoint of the shader

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// destroy the shader modules after the pipeline is done
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code)
{
	// need to use special function to ensure size is maintained properly when recast to uint32_t*
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	
	VkShaderModule shaderModule;
	if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Could not create shader module!\n");
	
	return shaderModule;
}

void HelloTriangleApplication::createImageViews()
{
	// obviously should be the same size:
	swapChainImageViews.resize(swapChainImages.size());
	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // standard type as 2d texture
		createInfo.format = swapChainImageFormat;
		// here we can swizzle the color channels if we want, but not yet
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		// set framebuffer image as a color target with no mipmaps or layers
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		// if this were stereoscopic, the swapchain would have multiple layers. then 
		// you would make multiple image views for each image as R/L eyes via layers.
		if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image views!\n");
	}
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
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		#ifdef DEBUG 
			printf("DEBUG: At least one framebuffer format and display mode found. Continuing...\n");
		#endif 
	}
	
	return indices.isComplete() && extensionsSupported && swapChainAdequate; //graphicsFamily.has_value();
	//return true;
}

void HelloTriangleApplication::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
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

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	#ifdef DEBUG 
		printf("DEBUG: Framebuffer image count: %d\n", imageCount);
	#endif 
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
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



// * APP CLEANUP * // 
void HelloTriangleApplication::cleanup()
{
	for (auto imageView:swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

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
