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
		drawFrame();
	}
	// cleanup and end
	vkDeviceWaitIdle(device);
}

void HelloTriangleApplication::drawFrame()
{
	uint32_t imageIndex;
	// logical device and swapchain from which we get the image
	// timeout in nanoseconds, or max to disable timeout
	// signaled sempahore and signaled fence
	// finally output variable of swapchain image array index that is now available
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	// configure queue to wait for color writing on imageavailable
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	// there are 1 cmd buffer
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	// configure which semaphore to signal when render is finished
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)!=VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer!\n");
	
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores; // just like submitinfo
	VkSwapchainKHR swapChains[] = {swapChain}; // one swapchain
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex; // almost always 1
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(presentQueue, &presentInfo);
	vkQueueWaitIdle(presentQueue);
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
	createRenderPass();
	createGraphicsPipeline(); // < exciting!
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSemaphores();
	return OK;
}

void HelloTriangleApplication::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || \
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphores!\n");
}

// create buffer for drawing commands
void HelloTriangleApplication::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // secondary means it will be called from another primary command buffer
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data())!=VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!\n");
	for(size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // contains timing for submitting command buffer usage
		beginInfo.pInheritanceInfo = nullptr; // for secondary command buffers
		if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo)!=VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer!\n");
		
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapChainExtent;
		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f}; // clear color = black
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		// render pass cmds are in primary command buffer
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); 
		// configure pipline bind point as graphics pipline
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		
		// now draw!
		// vertex count, instance count (for instanced rendering), 
		//   first vertex (gl_VertexIndex), first instance offset (for instanced rendering) (gl_InstanceIndex)
		
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		
		vkCmdEndRenderPass(commandBuffers[i]);

		if(vkEndCommandBuffer(commandBuffers[i])!=VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer!\n");
	}
	
}

// create pool to hold draw command buffers
void HelloTriangleApplication::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0; // if you want to change cmd buffers at runtime need flags
	if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)!=VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan command pool!\n");
}

void HelloTriangleApplication::createFramebuffers()
{
	// resize framebuffer to size of swapchain image views
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for(size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = { swapChainImageViews[i] };
		// set the framebuffer to have a single swapchain image view and single layer
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;
		if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer!");
	}
	
	#ifdef DEBUG 
		printf("DEBUG: All framebuffers created successfully!\n");
	#endif 
}

void HelloTriangleApplication::createRenderPass()
{
	// we have a single color buffer attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// clear framebuffer and enable write
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// set stencil buffer (if it exists) to the same
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	// this attachment does not care about initial fb format, and we want it in swapchain format
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// enable a subpass with color buffer optimization
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics subpass, not compute.
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // in the frag shader, this is layout location 0
	// finally, assemble the renderpass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	// 
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0; // dependency and subpass index
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0; // operations to wait on and what stage
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // operations that will wait
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)!= VK_SUCCESS)
		throw std::runtime_error("Could not create render pass!\n");

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

	// Create a pipeline that is only vertex and fragment
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// how are we loading the vertices? (we aren't for now)
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// how are we going to draw the vertices?
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// set viewport to min/max bounds of framebuffer
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f; // framebuffer depth

	// scissor/crop set to entire image
	VkRect2D scissor{};
	scissor.offset = {0, 0}; // start
	scissor.extent = swapChainExtent; // end

	// viewport + scissor = viewport state
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// rasterizer = depth test, culling, scissor test, and finally create fragment
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // requires gpu feature, for shadowmap test
	rasterizer.rasterizerDiscardEnable = VK_FALSE; // enable output to framebuffer
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // FILL, LINE, POINT. requires gpu feature.
	rasterizer.lineWidth = 1.0f; // lines are 1 fragment in width
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // cull back faces
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // clockwise face order
	rasterizer.depthBiasEnable = VK_FALSE; // for shadowmapping
	//rasterizer.depthBiasConastantFactor, BiasClamp, BiasSlopeFactor

	// multisampling (eg for AA) - requires gpu feature, disable for now
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	// minSampleShading, pSampleMask, alphaToCoverageEnable, alphaToOneEnable

	// depth/stencil test
	VkPipelineDepthStencilStateCreateInfo depthtest{};
	//depthtest = nullptr;

	// color blending - additive or bitwise operation on framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | \
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	//srcColorBlendFactor, dstColorBlendFactor, colorBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp
	// 1, 0, add, 1, 0, add
	// alpha blending example on page 115. this struct is PER FRAMEBUFFER.

	// this is for all framebuffers
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; // if this is true, blendEnable is set to false on all fbs
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	// logicOp, blendConstants[4]

	// Configure what states can be reconfigured at runtime:
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// create pipeline layout for shaders
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//setLayoutCount, pSetLayouts, pushConstantRangeCount, pPushConstantRanges
	if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)!=VK_SUCCESS)
		throw std::runtime_error("Could not create pipeline layout!\n");
	
	#ifdef DEBUG 
		printf("DEBUG: Pipeline layout created successfully.\n");
	#endif 

	// assemble pipeline from layout
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages; // just vert and frag
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0; // subpass index
	// pipeline derivitive - optional:
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	// the null is pipline cache which can be reused to make more pipelines
	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)!=VK_SUCCESS)
		throw std::runtime_error("Couldn't create graphics pipeline!\n");

	#ifdef DEBUG 
		printf("DEBUG: Graphics pipeline assembled OK!\n");
	#endif 

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
	//vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	//vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);

	for(auto framebuffer:swapChainFramebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

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
