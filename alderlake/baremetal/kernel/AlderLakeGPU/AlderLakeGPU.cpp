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

    IOLog("AlderLakeGPU: PCI %04x:%04x\n", vendor, device);
    return vendor == kIntelVendor && device == kAlderLakeP46A6;
}

bool AlderLakeGPU::mapDiagnosticsBar() {
    // Milestone A intentionally performs no MMIO writes.
    // Mapping is used only to validate the resource layout reported by IOPCIFamily.
    bar0_map_ = pci_->mapDeviceMemoryWithIndex(0);
    if (!bar0_map_) {
        IOLog("AlderLakeGPU: BAR index 0 could not be mapped\n");
        return false;
    }

    IOLog(
        "AlderLakeGPU: BAR0 phys=0x%llx len=0x%llx virt=%p (read-only diagnostics stage)\n",
        static_cast<unsigned long long>(bar0_map_->getPhysicalAddress()),
        static_cast<unsigned long long>(bar0_map_->getLength()),
        reinterpret_cast<void*>(bar0_map_->getVirtualAddress())
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

    // Enable PCI memory-space decoding. No bus mastering or register writes yet.
    pci_->setMemoryEnable(true);

    if (!mapDiagnosticsBar()) {
        releaseResources();
        return false;
    }

    IOLog("AlderLakeGPU: matched 8086:46A6; diagnostics-only attach successful\n");
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
