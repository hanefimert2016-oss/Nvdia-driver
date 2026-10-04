#pragma once

#include <IOKit/IOService.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/pci/IOPCIDevice.h>

class AlderLakeGPU final : public IOService {
    OSDeclareDefaultStructors(AlderLakeGPU)

public:
    bool start(IOService* provider) override;
    void stop(IOService* provider) override;

private:
    IOPCIDevice* pci_{nullptr};
    IOMemoryMap* bar0_map_{nullptr};

    bool verifyTarget() const;
    bool mapDiagnosticsBar();
    void releaseResources();
};
