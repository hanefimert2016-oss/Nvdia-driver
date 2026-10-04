#pragma once

#include "alderbridge/ir.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace alderbridge {

// This is deliberately NOT Apple's private Metal ABI.
// It is a stable research packet format used to develop and test the
// translation core before a real Metal-facing plugin/shim is connected.
enum class MetalLikeOp : std::uint32_t {
    BeginRenderPass,
    EndRenderPass,
    SetRenderPipeline,
    SetVertexBuffer,
    DrawPrimitives,
    DispatchThreadgroups,
    CopyBuffer,
    Present
};

struct MetalLikePacket {
    MetalLikeOp op{};
    std::uint64_t a{};
    std::uint64_t b{};
    std::uint64_t c{};
    std::uint64_t d{};
};

enum class VulkanPlanOp : std::uint32_t {
    BeginDynamicRendering,
    EndDynamicRendering,
    BindGraphicsPipeline,
    BindVertexBuffer,
    Draw,
    Dispatch,
    CopyBuffer,
    QueuePresent
};

struct VulkanPlanStep {
    VulkanPlanOp op{};
    std::uint64_t a{};
    std::uint64_t b{};
    std::uint64_t c{};
    std::uint64_t d{};
};

struct TranslationResult {
    bool ok{true};
    std::string error;
    CommandBuffer ir;
    std::vector<VulkanPlanStep> vulkan_plan;
};

TranslationResult translate(const std::vector<MetalLikePacket>& packets);

} // namespace alderbridge
