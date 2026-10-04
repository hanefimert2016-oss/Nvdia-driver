#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace alderbridge {

using ResourceId = std::uint64_t;

struct BeginRenderPass {
    ResourceId color_target{};
};

struct EndRenderPass {};

struct BindPipeline {
    ResourceId pipeline{};
};

struct BindVertexBuffer {
    ResourceId buffer{};
    std::uint32_t slot{};
    std::uint64_t offset{};
};

struct Draw {
    std::uint32_t vertex_count{};
    std::uint32_t instance_count{1};
    std::uint32_t first_vertex{};
    std::uint32_t first_instance{};
};

struct Dispatch {
    std::uint32_t groups_x{1};
    std::uint32_t groups_y{1};
    std::uint32_t groups_z{1};
};

struct CopyBuffer {
    ResourceId src{};
    ResourceId dst{};
    std::uint64_t src_offset{};
    std::uint64_t dst_offset{};
    std::uint64_t size{};
};

struct Present {
    ResourceId drawable{};
};

using CommandPayload = std::variant<
    BeginRenderPass,
    EndRenderPass,
    BindPipeline,
    BindVertexBuffer,
    Draw,
    Dispatch,
    CopyBuffer,
    Present
>;

struct Command {
    CommandPayload payload;
};

struct CommandBuffer {
    std::vector<Command> commands;
};

} // namespace alderbridge
