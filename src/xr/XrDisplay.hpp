#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

class VulkanInstance;
class Viewport;

struct RendererRequirements
{
    uint32_t minApiVersion;
    uint32_t maxApiVersion;

    std::vector<std::string> instanceExtensions;

    VkPhysicalDevice physicalDevice;

    std::vector<std::string> deviceExtensions;
};

class XrDisplay
{
public:
    ~XrDisplay();

    bool initialize();
    bool getRequirements(RendererRequirements*);
    bool getVulkanDevice(VkInstance, VkPhysicalDevice*);
    bool createSession(class VulkanInstance*);
    void pollEvents(bool*, bool*);
    void beginFrame(double*, bool*);
    void endFrame();
    void destroySession();

    void enumerateSwapchainFormats(std::vector<VkFormat>*);
    bool createViewports(VulkanInstance*, std::vector<Viewport>*, VkFormat);

    bool enableValidationLayers = true;

    XrInstance instance = XR_NULL_HANDLE;
    XrDebugUtilsMessengerEXT debugMessenger = XR_NULL_HANDLE;
    XrSystemId systemId = XR_NULL_SYSTEM_ID;
    XrSession session = XR_NULL_HANDLE;

    XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;
    XrFrameState currentFrameState;
private:
    void populateDebugMessengerCreateInfo(XrDebugUtilsMessengerCreateInfoEXT*);
    bool createInstance();
    bool setupDebugMessenger();
    bool findSystem();

    PFN_xrCreateDebugUtilsMessengerEXT ext_xrCreateDebugUtilsMessengerEXT = nullptr;
    PFN_xrDestroyDebugUtilsMessengerEXT ext_xrDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_xrGetVulkanGraphicsRequirementsKHR ext_xrGetVulkanGraphicsRequirementsKHR = nullptr;
    PFN_xrGetVulkanInstanceExtensionsKHR ext_xrGetVulkanInstanceExtensionsKHR = nullptr;
    PFN_xrGetVulkanGraphicsDeviceKHR ext_xrGetVulkanGraphicsDeviceKHR = nullptr;
    PFN_xrGetVulkanDeviceExtensionsKHR ext_xrGetVulkanDeviceExtensionsKHR = nullptr;
};
