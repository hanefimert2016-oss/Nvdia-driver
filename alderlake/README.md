# Alder Lake iGPU: Metal → Vulkan experiment

Target machine: Intel Alder Lake-P GT2 / Iris Xe (PCI 8086:46A6), with the laptop's eDP and HDMI routed through the Intel display engine.

This branch is an **experimental driver research branch**, not a working macOS GPU driver yet.

## Goal

Build a macOS graphics stack where Apple-side rendering commands are normalized into a small internal IR and then emitted through Vulkan. The intended long-term path is:

```
Apple graphics / Metal-facing shim
        ↓
AlderBridge IR
        ↓
Vulkan command backend
        ↓
Mesa ANV-derived Intel Vulkan userspace
        ↓
XNU kernel memory/queue/interrupt shim
        ↓
Alder Lake Xe-LP iGPU
        ↓
Intel display engine → eDP / HDMI
```

The important constraint is that translating Metal commands is only half of the job: macOS does not ship a Vulkan driver for this Alder Lake iGPU. We therefore also need a Darwin-capable Intel Vulkan userspace backend plus the kernel/display plumbing that ANV normally gets from Linux DRM/i915.

## What is implemented in this first branch

- Portable command IR for render/compute/copy/present operations.
- A Metal-like packet translator into that IR.
- Mapping of the IR into explicit Vulkan operation plans.
- A Vulkan probe that can locate Intel vendor `0x8086` and prefer device `0x46A6`.
- Unit tests for command translation.
- Linux CI for the portable pieces and Vulkan probe build.

This lets us validate the translator independently before touching XNU or private Apple graphics ABI.

## Build on Arch Linux

```bash
sudo pacman -S --needed cmake ninja vulkan-headers vulkan-icd-loader
cmake -S alderlake -B build/alderlake -G Ninja
cmake --build build/alderlake
ctest --test-dir build/alderlake --output-on-failure
./build/alderlake/alderlake_vulkan_probe
```

On the G5 KF Linux install, the probe should identify the Intel GPU as vendor `0x8086`, device `0x46A6`.

## Next milestones

1. Add a Darwin/XNU PCI probe for `8086:46A6` with safe BAR discovery only.
2. Define a minimal memory-object and command-submission ABI between userspace and the XNU driver.
3. Port the smallest ANV dependencies needed to create a Vulkan device on Darwin.
4. Bring up buffer allocation, VM mappings and a no-op batch.
5. Bring up a simple compute/clear operation.
6. Add display scanout for the Intel-connected HDMI/eDP paths.
7. Only after the Vulkan backend is stable, connect a Metal-facing plugin/shim.

Do not attempt arbitrary MMIO writes until BAR layout, power state and forcewake handling have been verified for this exact device.
