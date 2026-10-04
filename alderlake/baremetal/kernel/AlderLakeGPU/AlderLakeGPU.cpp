#include "AlderLakeGPU.hpp"

#include <IOKit/IOLib.h>

#define super IOService
OSDefineMetaClassAndStructors(AlderLakeGPU, IOService)

namespace {
constexpr UInt16 kIntelVendor = 0x8086;
constexpr UInt16 kAlderLakeP46A6 = 0x46A6;
}

bool AlderLakeGPU::verifyTarget() const {
    if (!pci_) {
        return false;
    }

    const UInt16 vendor = pci_->configRead16(kIOPCIConfigVendorID);
    const UInt16 device = pci_->configRead16(kIOPCIConfigDeviceID);
    const UInt8 revision = pci_->configRead8(kIOPCIConfigRevisionID);

    IOLog("AlderLakeGPU: PCI %04x:%04x revision=%02x\n", vendor, device, revision);
    return vendor == kIntelVendor && device == kAlderLakeP46A6;
}

bool AlderLakeGPU::mapDiagnosticsBar() {
    // Bring-up stage A: map the first PCI memory resource but never write MMIO.
    // This is intentionally diagnostics-only.
    bar0_map_ = pci_->mapDeviceMemoryWithIndex(0);
    if (!bar0_map_) {
        IOLog("AlderLakeGPU: BAR index 0 could not be mapped\n");
        return false;
    }

    IOLog(
        "AlderLakeGPU: BAR0 phys=0x%llx len=0x%llx virt=0x%llx diagnostics-only\n",
        static_cast<unsigned long long>(bar0_map_->getPhysicalAddress()),
        static_cast<unsigned long long>(bar0_map_->getLength()),
        static_cast<unsigned long long>(bar0_map_->getVirtualAddress())
    );

    return true;
}

bool AlderLakeGPU::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    pci_ = OSDynamicCast(IOPCIDevice, provider);
    if (!pci_) {
        IOLog("AlderLakeGPU: provider is not IOPCIDevice\n");
        return false;
    }

    pci_->retain();

    if (!verifyTarget()) {
        IOLog("AlderLakeGPU: refusing non-target PCI device\n");
        releaseResources();
        return false;
    }

    // Only enable PCI memory decoding for BAR discovery.
    // Bus mastering, interrupts, forcewake and GPU command submission remain off.
    pci_->setMemoryEnable(true);

    if (!mapDiagnosticsBar()) {
        releaseResources();
        return false;
    }

    setProperty("AlderBridgeStage", "PCI-diagnostics");
    setProperty("AlderBridgeTarget", "8086:46A6");

    IOLog("AlderLakeGPU: matched physical Alder Lake-P GT2 8086:46A6\n");
    registerService();
    return true;
}

void AlderLakeGPU::releaseResources() {
    if (bar0_map_) {
        bar0_map_->release();
        bar0_map_ = nullptr;
    }

    if (pci_) {
        pci_->release();
        pci_ = nullptr;
    }
}

void AlderLakeGPU::stop(IOService* provider) {
    IOLog("AlderLakeGPU: stop\n");
    releaseResources();
    super::stop(provider);
}
