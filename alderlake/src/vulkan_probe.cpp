#include <vulkan/vulkan.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
constexpr std::uint32_t kIntelVendor = 0x8086;
constexpr std::uint32_t kAlderLakePDevice = 0x46A6;
}

int main() {
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = "AlderLakeVulkanProbe";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "AlderBridge";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    VkInstance instance = VK_NULL_HANDLE;
    const VkResult create_result = vkCreateInstance(&create_info, nullptr, &instance);
    if (create_result != VK_SUCCESS) {
        std::cerr << "vkCreateInstance failed: " << create_result << "\n";
        return 2;
    }

    std::uint32_t count = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (result != VK_SUCCESS || count == 0) {
        std::cerr << "No Vulkan physical devices found.\n";
        vkDestroyInstance(instance, nullptr);
        return 3;
    }

    std::vector<VkPhysicalDevice> devices(count);
    result = vkEnumeratePhysicalDevices(instance, &count, devices.data());
    if (result != VK_SUCCESS) {
        std::cerr << "vkEnumeratePhysicalDevices failed: " << result << "\n";
        vkDestroyInstance(instance, nullptr);
        return 4;
    }

    bool found_intel = false;
    bool found_exact = false;

    for (const auto device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);

        std::cout
            << props.deviceName
            << " vendor=0x" << std::hex << std::setw(4) << std::setfill('0') << props.vendorID
            << " device=0x" << std::setw(4) << props.deviceID
            << std::dec << "\n";

        if (props.vendorID == kIntelVendor) {
            found_intel = true;
            if (props.deviceID == kAlderLakePDevice) {
                found_exact = true;
                std::cout << "TARGET: Intel Alder Lake-P GT2 [8086:46A6] found.\n";

                std::uint32_t queue_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);
                std::vector<VkQueueFamilyProperties> queues(queue_count);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queues.data());

                for (std::uint32_t i = 0; i < queue_count; ++i) {
                    std::cout << "  queue[" << i << "]"
                              << " graphics=" << ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                              << " compute=" << ((queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
                              << " transfer=" << ((queues[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
                              << " count=" << queues[i].queueCount
                              << "\n";
                }
            }
        }
    }

    vkDestroyInstance(instance, nullptr);

    if (!found_intel) {
        std::cerr << "No Intel Vulkan device found.\n";
        return 5;
    }
    if (!found_exact) {
        std::cerr << "Intel Vulkan device found, but not PCI device 0x46A6.\n";
        return 6;
    }

    return 0;
}
