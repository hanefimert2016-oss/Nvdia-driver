# AlderVulkan userspace

This will become the Darwin Vulkan implementation used by the Metal translator.

The intended strategy is to keep the Intel-specific Mesa ANV compiler/command generation code and replace its Linux DRM/i915-facing winsys with calls to `AlderLakeGPU.kext` through an `IOUserClient`.

First userspace milestones:

1. open the IOService,
2. query ABI version,
3. query PCI/device information,
4. query BAR/resource diagnostics,
5. create a buffer,
6. map it into CPU space,
7. bind a GPU VA,
8. create a GPU context,
9. submit a no-op/simple batch,
10. wait for a fence.

Only after those operations work should ANV be wired to the Darwin winsys.
