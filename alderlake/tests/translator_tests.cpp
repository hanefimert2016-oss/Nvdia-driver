#include "alderbridge/translator.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace alderbridge;

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        std::exit(1);
    }
}

void test_render_sequence() {
    const std::vector<MetalLikePacket> packets{
        {MetalLikeOp::BeginRenderPass, 10, 0, 0, 0},
        {MetalLikeOp::SetRenderPipeline, 20, 0, 0, 0},
        {MetalLikeOp::SetVertexBuffer, 30, 0, 128, 0},
        {MetalLikeOp::DrawPrimitives, 3, 1, 0, 0},
        {MetalLikeOp::EndRenderPass, 0, 0, 0, 0},
        {MetalLikeOp::Present, 10, 0, 0, 0},
    };

    const auto result = translate(packets);
    require(result.ok, "valid render sequence should translate");
    require(result.ir.commands.size() == packets.size(), "IR command count mismatch");
    require(result.vulkan_plan.size() == packets.size(), "Vulkan plan count mismatch");
    require(result.vulkan_plan[0].op == VulkanPlanOp::BeginDynamicRendering,
            "render pass should map to dynamic rendering");
    require(result.vulkan_plan[3].op == VulkanPlanOp::Draw,
            "draw should map to Vulkan draw");
    require(result.vulkan_plan[5].op == VulkanPlanOp::QueuePresent,
            "present should map to queue present");
}

void test_draw_outside_render_pass_rejected() {
    const std::vector<MetalLikePacket> packets{
        {MetalLikeOp::DrawPrimitives, 3, 1, 0, 0},
    };

    const auto result = translate(packets);
    require(!result.ok, "draw outside render pass must be rejected");
}

void test_unterminated_render_pass_rejected() {
    const std::vector<MetalLikePacket> packets{
        {MetalLikeOp::BeginRenderPass, 1, 0, 0, 0},
    };

    const auto result = translate(packets);
    require(!result.ok, "unterminated render pass must be rejected");
}

} // namespace

int main() {
    test_render_sequence();
    test_draw_outside_render_pass_rejected();
    test_unterminated_render_pass_rejected();

    std::cout << "alderbridge tests: OK\n";
    return 0;
}
