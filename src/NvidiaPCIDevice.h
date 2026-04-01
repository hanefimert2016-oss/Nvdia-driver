/*
 * NvidiaPCIDevice.h
 * NvidiaHackintoshDriver
 *
 * IOPCIDevice wrapper: discovers the GPU on the PCI bus, maps MMIO bars,
 * and exposes register-level read/write helpers.
 */

#ifndef NVIDIA_PCI_DEVICE_H
#define NVIDIA_PCI_DEVICE_H

#include <IOKit/pci/IOPCIDevice.h>
#include "NvidiaDeviceIDs.h"

/* NVIDIA MMIO register offsets (subset used by this driver) */
#define NV_PMC_BOOT_0           0x00000000  /* chip identification  */
#define NV_PMC_ENABLE           0x00000200  /* sub-unit power gates */
#define NV_PBUS_DEBUG_DUALHEAD  0x00001084  /* dual-head control    */
#define NV_PDISPLAY_BASE        0x00610000  /* display engine base  */

#define NVIDIA_BAR_MMIO         0           /* BAR 0 – MMIO         */
#define NVIDIA_BAR_FB           1           /* BAR 1 – framebuffer  */

class NvidiaPCIDevice : public IOService
{
    OSDeclareDefaultStructors(NvidiaPCIDevice)

public:
    /* Lifecycle */
    virtual bool        init(OSDictionary *props = nullptr) override;
    virtual IOService  *probe(IOService *provider, SInt32 *score) override;
    virtual bool        start(IOService *provider) override;
    virtual void        stop(IOService *provider) override;
    virtual void        free() override;

    /* Register access */
    uint32_t            readMMIO(uint32_t offset) const;
    void                writeMMIO(uint32_t offset, uint32_t value);

    /* Framebuffer access */
    volatile uint8_t   *getFramebufferBase() const { return fFBBase; }
    uint64_t            getFramebufferSize() const  { return fFBSize; }

    /* Device information */
    uint16_t            getDeviceID()   const { return fDeviceID; }
    uint16_t            getVendorID()   const { return fVendorID; }
    const NvidiaDeviceEntry *getDeviceEntry() const { return fEntry; }

private:
    IOPCIDevice        *fPCIDevice  = nullptr;
    IOMemoryMap        *fMMIOMap    = nullptr;
    IOMemoryMap        *fFBMap      = nullptr;
    volatile uint8_t   *fMMIOBase   = nullptr;
    volatile uint8_t   *fFBBase     = nullptr;
    uint64_t            fFBSize     = 0;
    uint16_t            fVendorID   = 0;
    uint16_t            fDeviceID   = 0;
    const NvidiaDeviceEntry *fEntry = nullptr;

    bool mapBARs();
    void unmapBARs();
};

#endif /* NVIDIA_PCI_DEVICE_H */
