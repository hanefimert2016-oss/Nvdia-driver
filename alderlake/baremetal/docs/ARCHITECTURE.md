# Bare-metal architecture

## 1. Why this is different from a VM

A VM can expose a virtual GPU with a host-side Vulkan implementation. A real Hackintosh cannot rely on a Linux host once macOS has booted. Every layer required to access the Alder Lake iGPU must therefore exist on Darwin itself.

That changes the architecture substantially:

- there is no host Vulkan process,
- there is no shared-memory transport to Linux,
- there is no PCI passthrough layer,
- the macOS kernel driver owns the physical Intel PCI function,
- the Vulkan backend must run on macOS,
- HDMI/eDP scanout must be driven by the Intel display engine on macOS.

## 2. Kernel component: AlderLakeGPU.kext

Provider: `IOPCIDevice`

Initial PCI match:

```
vendor = 0x8086
device = 0x46A6
```

Responsibilities, introduced in stages:

- claim the PCI device,
- enable memory access,
- inspect and map BAR resources,
- establish power-state handling,
- allocate and pin GPU-visible memory,
- manage GPU virtual address spaces,
- expose queue/context creation,
- submit command buffers,
- expose fences/timeline completion,
- handle interrupts and reset/error paths,
- provide an `IOUserClient` ABI to userspace.

The first milestone intentionally excludes command submission. It is read-only diagnostics so hardware assumptions can be checked before any risky register programming.

## 3. Userspace Vulkan component: AlderVulkan

Linux Mesa ANV already knows how to generate Alder Lake Xe-LP command streams and shaders, but it normally talks to Linux DRM/i915.

For macOS we need a **Darwin winsys** underneath ANV.

Conceptually:

```
ANV core
  │
  ├─ shader compiler / pipeline code
  ├─ descriptor handling
  ├─ command emission
  └─ synchronization model
         │
         ▼
Darwin winsys
         │
         ├─ create_buffer()
         ├─ map_buffer()
         ├─ bind_gpu_va()
         ├─ create_context()
         ├─ submit_batch()
         ├─ wait_fence()
         └─ query_device()
                 │
                 ▼
AlderLakeGPU IOUserClient
```

The port should minimize changes inside ANV. Linux-specific DRM calls should be isolated behind the winsys boundary.

## 4. Metal-facing component: AlderMetal.plugin

The first prototype translator in this repository uses a synthetic `MetalLikePacket` format. That is only a test harness.

For a real Hackintosh, the target is a driver/plugin loaded beneath Apple's Metal framework rather than application-level hooking.

Responsibilities:

- advertise an Intel GPU device to the Apple graphics stack,
- map Metal resource objects to Vulkan resources,
- compile/translate Metal shader representations into a form the Vulkan backend can consume,
- translate render/compute/blit encoders into AlderBridge/Vulkan command buffers,
- implement fences/events/resource residency,
- report unsupported operations as errors rather than silently returning incorrect output.

Metal integration is intentionally late in the schedule. Before this layer is attempted, the Darwin Vulkan backend should be able to allocate memory and execute simple workloads on the physical GPU.

## 5. Display component: AlderLakeFB.kext

The target laptop routes both the built-in panel and HDMI through the Intel display engine, so display support is not optional for the requested setup.

Responsibilities:

- connector discovery,
- EDID/DDC,
- eDP panel detection,
- HDMI hotplug,
- mode enumeration,
- PLL/pipe/transcoder setup,
- framebuffer scanout,
- page flip/vblank,
- hardware cursor,
- backlight later.

Render and display are kept separate. Early framebuffer bring-up may use a simple linear scanout buffer before accelerated WindowServer rendering works.

## 6. Boot sequence

Expected long-term boot flow:

```
OpenCore
  ↓
macOS kernel
  ↓
AlderLakeGPU.kext attaches to 00:02.0 / 8086:46A6
  ↓
AlderLakeFB discovers eDP + HDMI
  ↓
userspace starts
  ↓
AlderVulkan opens IOUserClient
  ↓
Vulkan device becomes available
  ↓
AlderMetal.plugin publishes Metal device
  ↓
WindowServer selects the device
  ↓
GPU-rendered desktop
```

During early development OpenCore/UEFI GOP can remain the boot framebuffer. The native framebuffer driver should replace it only after modesetting is proven stable.

## 7. First bare-metal success criteria

Milestone A:

- kext matches only `8086:46A6`,
- logs PCI vendor/device,
- maps a BAR without writing to it,
- unload/boot cycle does not hang.

Milestone B:

- userspace diagnostics can open the driver,
- memory object can be allocated/mapped,
- no-op batch completes,
- fence signals,
- GPU remains responsive.

Milestone C:

- a Vulkan clear/copy works on macOS,
- output can be verified by CPU readback.

Milestone D:

- HDMI modeset works,
- a test framebuffer is scanned out.

Only after A-D should the Metal plugin be connected to WindowServer.
