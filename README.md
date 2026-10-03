# Nvdia-driver — RTX 4060 Laptop / AD107 macOS bring-up

Experimental NVIDIA-on-macOS driver work targeting the GeForce RTX 4060 Laptop GPU (Ada AD107M, PCI 10de:28e0).

This branch is intentionally milestone-driven. It does **not** claim a working Metal desktop yet.

## Current checkpoint

- [x] Ada/AD107 target profile and public class IDs
- [x] Blackwell RTX 5060 GPFIFO class guarded out of the AD107 path
- [x] Portable regression tests
- [x] Linux hardware/GSP/BAR capture helper
- [x] Minimal XNU IOPCIDevice attach + BAR mapping source for NVRM.kext
- [ ] GSP-RM boot on macOS
- [ ] RM client/device/subdevice + VA space
- [ ] GPFIFO/USERD submission
- [ ] Mesa NVK/NAK on macOS
- [ ] NVKMS / IOFramebuffer scanout
- [ ] NVMTLDriver.bundle
- [ ] AGDC + IOAccelDisplayPipeUserClient2
- [ ] Accelerated WindowServer desktop

## Why the RTX 5060 constants cannot be copied

The demonstrated RTX 5060 path uses Blackwell GPFIFO class 0xca6f. AD107 is Ada and reuses the Ampere channel ABI. This project therefore starts with a runtime-query-first candidate list headed by 0xc56f and uses Ada graphics/compute classes 0xc997 and 0xc9c0.

## Build the portable checkpoint

    make
    make test
    ./build/ad107-identify 0x10de 0x28e0

## Capture the Linux baseline on the target laptop

    chmod +x tools/capture-ad107-linux.sh
    ./tools/capture-ad107-linux.sh

The next implementation gate is GSP-RM boot under XNU. See docs/PORTING.md for the full checkpoint sequence and success criteria.

## Important

This is experimental kernel/graphics-driver work. Keep an alternate boot path and do not treat source-level completion as hardware validation.
