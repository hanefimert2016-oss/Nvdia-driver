# AlderMetal plugin

The final Metal-facing component should sit under Apple's graphics frameworks rather than patching each application.

The existing portable `MetalLikePacket` translator is only a development harness.

The production layer must eventually provide:

- device/resource objects expected by the Apple graphics stack,
- buffer/texture creation,
- render/compute/blit encoder translation,
- pipeline and shader handling,
- synchronization/events,
- drawable/present integration,
- explicit failure for unsupported features.

The desired data path is:

```
Metal framework
   ↓
AlderMetal
   ↓
AlderBridge IR
   ↓
AlderVulkan
   ↓
AlderLakeGPU.kext
   ↓
8086:46A6
```

Do not connect this layer to WindowServer until the native Darwin Vulkan backend passes buffer, submission and readback tests.
