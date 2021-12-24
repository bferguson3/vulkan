#define OK 0
#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>

#define _WIDTH 800
#define _HEIGHT 600

#define DEBUG 
//#define RASPI 

const std::vector<const char*> deviceExtensions = \
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME 
};

#ifdef DEBUG 
    const bool enableValidationLayers = true;
#else 
    const bool enableValidationLayers = false;
#endif

struct QueueFamilyIndices { 
    std::optional<uint32_t> graphicsFamily; // rendering hardware
    std::optional<uint32_t> presentFamily;  // displaying hardware
    
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails \
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

#ifdef RASPI
// No longer needed, legacy
    const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
#else 
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#endif

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
bool checkValidationLayerSupport(const std::vector<const char*> validationLayers);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
std::vector<const char*> getRequiredExtensions();
bool checkExtensions();
VkResult createInstance(VkInstance& instance);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        
static std::vector<char> readBinaryFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary); // ate = at end
    if(!file.is_open()) throw std::runtime_error("Couldn't open file!\n");
    size_t filesize = (size_t)file.tellg();
    std::vector<char> buffer(filesize);
    file.seekg(0); // back to start
    file.read(buffer.data(), filesize); // read all bytes
    file.close();
	#ifdef DEBUG 
		printf("DEBUG: Loaded binary file %s OK.\n", filename.data());
	#endif 
    return buffer;
}
//