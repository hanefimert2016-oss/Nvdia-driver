# RTX 4060 Laptop (AD107M) macOS bring-up

Target: x86 macOS 15.7.x, NVIDIA GeForce RTX 4060 Laptop GPU (Ada AD107M, PCI 10de:28e0).

## Architecture

The intended stack mirrors the RTX 5060 experiment, but swaps the Blackwell hardware path for Ada:

1. **NVRM.kext** — XNU OS layer around NVIDIA open GSP-RM. The first checkpoint in this repo only attaches PCI and maps BARs.
2. **NVRMFB.kext** — NVKMS/nvidia-modeset port exposed as IOFramebuffer.
3. **libnvrm-xnu + Mesa NVK/NAK** — NVK uses an XNU shim instead of Linux DRM/dma-buf/syncobj.
4. **NVMTLDriver.bundle** — Metal driver plug-in translating Metal command/state to Vulkan and AIR to SPIR-V.
5. **AGDC + IOAccelDisplayPipeUserClient2 compatibility** — required before WindowServer presentation is considered complete.

## Ada-specific corrections

Do not copy Blackwell class IDs from an RTX 5060 implementation.

- RTX 4060 Laptop GPU: 10de:28e0, AD107M.
- Preferred GPFIFO candidate: AMPERE_CHANNEL_GPFIFO_A = 0xc56f.
- Fallback GPFIFO candidate to query at runtime: AMPERE_CHANNEL_GPFIFO_B = 0xc76f.
- Ada graphics class: ADA_A = 0xc997.
- Ada compute class: ADA_COMPUTE_A = 0xc9c0.
- Ada display family: AD102_DISP = 0xc770; core DMA = 0xc77d.
- Ada UVM constrains channel and host VA to 1 << 40.
- GSP firmware and the GSP-RM source/userspace ABI must remain version-matched.

Every class must still be validated by an RM/NVK supported-class query on the actual AD107 machine. Static constants are target hints, not permission to allocate blindly.

## Checkpoints

### C0 — PCI attach and BAR map
Status: source implemented; macOS hardware validation pending.

Success criteria:
- kext matches only 10de:28e0;
- BAR0 maps;
- BAR1 size is logged;
- bus mastering and memory decode are enabled;
- no GPU reset or panic.

### C1 — GSP-RM boot
Status: not implemented.

Success criteria:
- matching GSP firmware image accepted;
- GSP reaches ready state;
- RM client/device/subdevice allocations succeed;
- VRAM and BAR1 sizes match the Linux baseline.

### C2 — VA space + GPFIFO
Status: target constants implemented; transport not implemented.

Success criteria:
- create VA space inside Ada's channel VA limit;
- query supported classes and allocate GPFIFO (prefer 0xc56f when reported);
- create USERD/doorbell mapping;
- submit a no-op/fence and observe completion;
- zero refused RM calls in the minimal loop.

### C3 — NVK headless Vulkan
Status: not implemented.

Success criteria:
- NVK enumerates AD107;
- NAK compiles a shader;
- buffer-copy and offscreen clear/triangle tests pass;
- no llvmpipe fallback.

### C4 — display scanout
Status: not implemented.

Success criteria:
- NVKMS modesets the physical output;
- IOFramebuffer scanout and hardware cursor work;
- repeated page flips complete.

### C5 — Metal + WindowServer
Status: not implemented.

Success criteria:
- Metal enumerates the RTX 4060 Laptop from boot;
- WindowServer opens the display pipe;
- AGDC link-config and plane-capability requests return valid data;
- Finder/Dock/cursor remain accelerated under sustained presentation.

## Linux baseline capture

Run:

    chmod +x tools/capture-ad107-linux.sh
    ./tools/capture-ad107-linux.sh

Keep the resulting ad107-capture.txt. The GSP version, VBIOS, PCI/BAR layout and IOMMU grouping become the reference when the XNU port disagrees with Linux.

## Safety during bring-up

Keep an alternate boot path. Early GSP/NVKMS work can wedge the dGPU until a cold reboot. Do not add power-management, suspend/resume, or aggressive reset handling before C1-C3 are stable.
