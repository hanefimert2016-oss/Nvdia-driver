/*
 * NvidiaController.h
 * NvidiaHackintoshDriver
 *
 * Top-level IOService that IOKit matches against the PCI personality declared
 * in Info.plist.  It allocates the NvidiaPCIDevice nub and attaches the
 * NvidiaDisplayController to it.
 */

#ifndef NVIDIA_CONTROLLER_H
#define NVIDIA_CONTROLLER_H

#include <IOKit/IOService.h>
#include "NvidiaPCIDevice.h"
#include "NvidiaDisplayController.h"

class NvidiaController : public IOService
{
    OSDeclareDefaultStructors(NvidiaController)

public:
    virtual bool        init(OSDictionary *props = nullptr) override;
    virtual IOService  *probe(IOService *provider, SInt32 *score) override;
    virtual bool        start(IOService *provider) override;
    virtual void        stop(IOService *provider) override;
    virtual void        free() override;

private:
    NvidiaPCIDevice          *fPCIDevice  = nullptr;
    NvidiaDisplayController  *fDisplay    = nullptr;

    bool attachDisplay();
    void detachDisplay();
};

#endif /* NVIDIA_CONTROLLER_H */
