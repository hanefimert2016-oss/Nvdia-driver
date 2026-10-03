#include "NVRMService.hpp"

#define super IOService
OSDefineMetaClassAndStructors(NVRMService, IOService)

static constexpr UInt16 kNvidiaVendor = 0x10de;
static constexpr UInt16 kRtx4060Laptop = 0x28e0;

bool NVRMService::start(IOService *provider)
{
    if (!super::start(provider))
        return false;

    pci_ = OSDynamicCast(IOPCIDevice, provider);
    if (!pci_) {
        IOLog("NVRM: provider is not IOPCIDevice\n");
        return false;
    }

    const UInt16 vendor = pci_->configRead16(kIOPCIConfigVendorID);
    const UInt16 device = pci_->configRead16(kIOPCIConfigDeviceID);
    IOLog("NVRM: PCI attach %04x:%04x\n", vendor, device);

    if (vendor != kNvidiaVendor || device != kRtx4060Laptop) {
        IOLog("NVRM: refusing non-AD107 RTX 4060 Laptop device\n");
        return false;
    }

    pci_->setMemoryEnable(true);
    pci_->setBusMasterEnable(true);

    bar0_ = pci_->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0);
    bar1_ = pci_->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress1);

    if (!bar0_) {
        IOLog("NVRM: BAR0 map failed\n");
        return false;
    }

    IOLog("NVRM: AD107 attached; BAR0 len=%llu",
          (unsigned long long)bar0_->getLength());
    if (bar1_)
        IOLog(" BAR1 len=%llu", (unsigned long long)bar1_->getLength());
    IOLog("\n");

    registerService();
    return true;
}

void NVRMService::stop(IOService *provider)
{
    if (bar1_) { bar1_->release(); bar1_ = nullptr; }
    if (bar0_) { bar0_->release(); bar0_ = nullptr; }
    pci_ = nullptr;
    super::stop(provider);
}
