#pragma once

#include <cstdint>

namespace alderbridge::abi {

constexpr std::uint32_t kAbiVersion = 1;
constexpr std::uint16_t kIntelVendorId = 0x8086;
constexpr std::uint16_t kAlderLakePDeviceId = 0x46A6;

enum class Method : std::uint32_t {
    GetAbiVersion = 0,
    GetDeviceInfo = 1,
    GetBarInfo = 2,

    // Reserved for later milestones.
    CreateBuffer = 16,
    DestroyBuffer = 17,
    MapBuffer = 18,
    BindGpuVa = 19,
    CreateContext = 24,
    DestroyContext = 25,
    Submit = 26,
    WaitFence = 27,
};

struct DeviceInfo {
    std::uint32_t abi_version{kAbiVersion};
    std::uint16_t vendor_id{};
    std::uint16_t device_id{};
    std::uint8_t revision_id{};
    std::uint8_t reserved0{};
    std::uint16_t reserved1{};
};

struct BarInfo {
    std::uint32_t index{};
    std::uint32_t reserved{};
    std::uint64_t physical_address{};
    std::uint64_t length{};
};

struct BufferCreate {
    std::uint64_t size{};
    std::uint64_t alignment{};
    std::uint32_t flags{};
    std::uint32_t reserved{};
};

struct BufferHandle {
    std::uint64_t handle{};
    std::uint64_t gpu_va{};
    std::uint64_t size{};
};

struct SubmitRequest {
    std::uint64_t context{};
    std::uint64_t command_buffer_handle{};
    std::uint64_t command_offset{};
    std::uint64_t command_length{};
    std::uint64_t signal_fence{};
};

static_assert(sizeof(DeviceInfo) == 12);
static_assert(sizeof(BarInfo) == 24);

} // namespace alderbridge::abi
