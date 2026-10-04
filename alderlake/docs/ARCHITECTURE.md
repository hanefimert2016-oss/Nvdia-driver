# Architecture notes

## Why "Metal → Vulkan" is not enough by itself

The Alder Lake iGPU supports Vulkan on Linux through Mesa ANV, but macOS does not provide that Vulkan implementation. A Metal-facing translator therefore needs a Vulkan backend that can actually allocate Intel GPU memory, create contexts, submit batches and synchronize on Darwin.

The project is split into independent layers so each one can be validated separately.

### 1. Metal-facing layer

Future component. It will convert the calls/objects exposed by the Apple graphics stack into a stable internal representation.

The current `MetalLikePacket` format is only a test protocol. It is not presented as Apple's private Metal driver ABI.

### 2. AlderBridge IR

Portable representation for:

- render pass begin/end
- pipeline binding
- vertex buffer binding
- draw
- compute dispatch
- buffer copy
- present

The first translator maps this IR to a Vulkan operation plan such as `vkCmdBeginRendering`, `vkCmdDraw`, `vkCmdDispatch` and `vkQueuePresentKHR`.

### 3. Intel Vulkan backend

Long-term plan: reuse as much of Mesa ANV as practical. Linux-specific DRM/i915 dependencies must be isolated behind a Darwin winsys layer.

Needed primitives include:

- GPU virtual-address management
- GEM-like buffer objects
- CPU mappings
- exec/queue submission
- fences/timeline synchronization
- context reset/error reporting

### 4. XNU kernel component

The first kernel milestone should be intentionally small and safe:

- match PCI vendor/device `8086:46A6`
- verify BAR discovery
- report power state
- expose read-only diagnostics

Only after that should memory management, forcewake, interrupts and command submission be added.

### 5. Display engine

On the target laptop, Linux DRM reports eDP and HDMI on the Intel DRM card. That means a usable macOS desktop on the laptop panel or built-in HDMI ultimately needs Intel display-engine support even if another GPU renders.

Display bring-up is therefore a separate milestone from 3D acceleration.

## First real hardware success criteria

The first meaningful milestone is not "macOS desktop". It is:

1. Darwin userspace can enumerate the Alder Lake iGPU.
2. A buffer can be allocated and mapped.
3. A valid no-op/simple batch can be submitted.
4. Completion can be observed without a GPU hang.

After that, a compute clear or simple render target clear is the next step.
