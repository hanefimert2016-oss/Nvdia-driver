#pragma once

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>

class NVRMService final : public IOService
{
    OSDeclareDefaultStructors(NVRMService)

private:
    IOPCIDevice *pci_ = nullptr;
    IOMemoryMap *bar0_ = nullptr;
    IOMemoryMap *bar1_ = nullptr;

public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
};
