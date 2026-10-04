#include "alderbridge/translator.hpp"

#include <type_traits>

namespace alderbridge {
namespace {

void append_vulkan_plan(const Command& command, std::vector<VulkanPlanStep>& out) {
    std::visit([&](const auto& payload) {
        using T = std::decay_t<decltype(payload)>;

        if constexpr (std::is_same_v<T, BeginRenderPass>) {
            out.push_back({VulkanPlanOp::BeginDynamicRendering, payload.color_target, 0, 0, 0});
        } else if constexpr (std::is_same_v<T, EndRenderPass>) {
            out.push_back({VulkanPlanOp::EndDynamicRendering, 0, 0, 0, 0});
        } else if constexpr (std::is_same_v<T, BindPipeline>) {
            out.push_back({VulkanPlanOp::BindGraphicsPipeline, payload.pipeline, 0, 0, 0});
        } else if constexpr (std::is_same_v<T, BindVertexBuffer>) {
            out.push_back({
                VulkanPlanOp::BindVertexBuffer,
                payload.buffer,
                payload.slot,
                payload.offset,
                0
            });
        } else if constexpr (std::is_same_v<T, Draw>) {
            out.push_back({
                VulkanPlanOp::Draw,
                payload.vertex_count,
                payload.instance_count,
                payload.first_vertex,
                payload.first_instance
            });
        } else if constexpr (std::is_same_v<T, Dispatch>) {
            out.push_back({
                VulkanPlanOp::Dispatch,
                payload.groups_x,
                payload.groups_y,
                payload.groups_z,
                0
            });
        } else if constexpr (std::is_same_v<T, CopyBuffer>) {
            // Resource IDs are emitted here; offsets/size stay in IR until
            // resource binding is implemented in the Vulkan recorder.
            out.push_back({
                VulkanPlanOp::CopyBuffer,
                payload.src,
                payload.dst,
                payload.size,
                0
            });
        } else if constexpr (std::is_same_v<T, Present>) {
            out.push_back({VulkanPlanOp::QueuePresent, payload.drawable, 0, 0, 0});
        }
    }, command.payload);
}

} // namespace

TranslationResult translate(const std::vector<MetalLikePacket>& packets) {
    TranslationResult result;

    bool render_pass_open = false;

    for (const auto& packet : packets) {
        Command command{};

        switch (packet.op) {
        case MetalLikeOp::BeginRenderPass:
            if (render_pass_open) {
                result.ok = false;
                result.error = "nested render pass is not allowed";
                return result;
            }
            render_pass_open = true;
            command.payload = BeginRenderPass{packet.a};
            break;

        case MetalLikeOp::EndRenderPass:
            if (!render_pass_open) {
                result.ok = false;
                result.error = "EndRenderPass without BeginRenderPass";
                return result;
            }
            render_pass_open = false;
            command.payload = EndRenderPass{};
            break;

        case MetalLikeOp::SetRenderPipeline:
            command.payload = BindPipeline{packet.a};
            break;

        case MetalLikeOp::SetVertexBuffer:
            command.payload = BindVertexBuffer{
                packet.a,
                static_cast<std::uint32_t>(packet.b),
                packet.c
            };
            break;

        case MetalLikeOp::DrawPrimitives:
            if (!render_pass_open) {
                result.ok = false;
                result.error = "DrawPrimitives outside render pass";
                return result;
            }
            command.payload = Draw{
                static_cast<std::uint32_t>(packet.a),
                static_cast<std::uint32_t>(packet.b == 0 ? 1 : packet.b),
                static_cast<std::uint32_t>(packet.c),
                static_cast<std::uint32_t>(packet.d)
            };
            break;

        case MetalLikeOp::DispatchThreadgroups:
            command.payload = Dispatch{
                static_cast<std::uint32_t>(packet.a),
                static_cast<std::uint32_t>(packet.b),
                static_cast<std::uint32_t>(packet.c)
            };
            break;

        case MetalLikeOp::CopyBuffer:
            command.payload = CopyBuffer{
                packet.a,
                packet.b,
                0,
                0,
                packet.c
            };
            break;

        case MetalLikeOp::Present:
            if (render_pass_open) {
                result.ok = false;
                result.error = "Present while render pass is still open";
                return result;
            }
            command.payload = Present{packet.a};
            break;
        }

        result.ir.commands.push_back(command);
        append_vulkan_plan(command, result.vulkan_plan);
    }

    if (render_pass_open) {
        result.ok = false;
        result.error = "unterminated render pass";
    }

    return result;
}

} // namespace alderbridge
