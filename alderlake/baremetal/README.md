# Alder Lake iGPU bare-metal Hackintosh driver

This directory is for the **real-machine / bare-metal macOS path**. It does not target QEMU, PCI passthrough, virtio, Reims VGPU, or a virtual GPU.

Target hardware currently validated from Linux:

- CPU: Intel Core i5-12500H
- iGPU: Intel Alder Lake-P GT2 / Iris Xe
- PCI ID: `8086:46A6`
- Linux Vulkan device: `Intel(R) Iris(R) Xe Graphics (ADL GT2)`
- Vulkan queue 0: graphics + compute + transfer
- Internal eDP and the laptop HDMI output are routed through the Intel DRM device

The Linux probe proves the physical GPU and the host Vulkan stack work. It **does not** prove macOS can use Vulkan yet; macOS still needs its own kernel driver, Intel Vulkan userspace port and display driver.

## Bare-metal stack

```
macOS application
      │
      ▼
Metal.framework / Quartz / WindowServer
      │
      ▼
AlderMetal.plugin
Metal-driver-facing translation layer
      │
      ▼
AlderBridge IR
      │
      ▼
AlderVulkan.dylib
Mesa ANV-derived Vulkan implementation for Darwin
      │
      ▼
IOUserClient ABI
      │
      ▼
AlderLakeGPU.kext
PCI + VM + buffer objects + queues + fences + interrupts
      │
      ├──────────────► AlderLakeFB.kext
      │                 modeset / EDID / hotplug / scanout / cursor
      ▼
Intel Alder Lake-P GT2 [8086:46A6]
      │
      ├── eDP
      └── HDMI
```

The design deliberately keeps Apple's public frameworks in place. The goal is not to patch every application or intercept every Metal call globally. The long-term clean path is to provide the GPU driver/plugin underneath the framework and translate the resulting driver operations to Vulkan.

## Bring-up order

1. **PCI attach only** — bind to `8086:46A6`, confirm power state and map BARs read-only.
2. **Diagnostics user client** — allow safe userspace queries for PCI/BAR/device state.
3. **Memory manager** — GPU virtual addresses, buffer objects, CPU mappings.
4. **Queue submission** — no-op/simple batch, fence completion, hang detection.
5. **Darwin ANV winsys** — replace Linux DRM/i915 calls with our IOUserClient ABI.
6. **Vulkan device creation on macOS** — enumerate the real Alder Lake iGPU from Darwin.
7. **Simple GPU work** — clear/copy/compute before 3D.
8. **Display** — Intel HDMI/eDP modesetting and framebuffer scanout.
9. **Metal plugin** — connect Metal-facing objects to AlderBridge/Vulkan.
10. **WindowServer** — only after the lower layers are stable.

## Current status

The portable translator and Linux Vulkan probe already pass. Bare-metal work starts with the XNU PCI driver skeleton in this branch.

Do not perform undocumented MMIO writes during the first stages. The initial kernel target is discovery and read-only diagnostics only.
