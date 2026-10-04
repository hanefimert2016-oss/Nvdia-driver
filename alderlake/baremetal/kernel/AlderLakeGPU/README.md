# AlderLakeGPU.kext

First bare-metal kernel milestone for Intel `8086:46A6`.

Current code is intentionally minimal:

- matches only the target Alder Lake-P GT2 PCI ID,
- confirms vendor/device through PCI config space,
- enables PCI memory-space decoding,
- maps BAR resource index 0,
- logs physical/virtual BAR information,
- performs **no MMIO writes**,
- performs **no command submission**,
- performs **no interrupt setup**.

This is the correct first test for a real Hackintosh because it verifies that XNU/IOPCIFamily can attach to and map the physical iGPU before GPU initialization code is attempted.

The source requires the macOS/Xcode kernel extension SDK environment and is not expected to compile on Linux.
