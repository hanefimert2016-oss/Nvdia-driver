/*
 * NvidiaPCIDevice.cpp
 * NvidiaHackintoshDriver
 *
 * Implementation of NvidiaPCIDevice – PCI discovery and MMIO mapping.
 */

#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "NvidiaPCIDevice.h"

#define super IOService
OSDefineMetaClassAndStructors(NvidiaPCIDevice, IOService)

/* -------------------------------------------------------------------------
 * init
 * --------------------------------------------------------------------- */
bool NvidiaPCIDevice::init(OSDictionary *props)
{
    if (!super::init(props)) return false;
    IOLog("NvidiaPCIDevice::init\n");
    return true;
}

/* -------------------------------------------------------------------------
 * probe  – verify vendor / device before committing to start()
 * --------------------------------------------------------------------- */
IOService *NvidiaPCIDevice::probe(IOService *provider, SInt32 *score)
{
    IOPCIDevice *pci = OSDynamicCast(IOPCIDevice, provider);
    if (!pci) return nullptr;

    fVendorID = pci->configRead16(kIOPCIConfigVendorID);
    fDeviceID = pci->configRead16(kIOPCIConfigDeviceID);

    if (fVendorID != NVIDIA_VENDOR_ID) {
        IOLog("NvidiaPCIDevice::probe – not an NVIDIA device (vendor 0x%04X)\n",
              fVendorID);
        return nullptr;
    }

    fEntry = NvidiaFindDevice(fDeviceID);
    if (!fEntry) {
        IOLog("NvidiaPCIDevice::probe – unsupported device ID 0x%04X\n", fDeviceID);
        return nullptr;
    }

    IOLog("NvidiaPCIDevice::probe – found %s (0x%04X)\n",
          fEntry->name, fDeviceID);
    *score = 2000;
    return this;
}

/* -------------------------------------------------------------------------
 * start
 * --------------------------------------------------------------------- */
bool NvidiaPCIDevice::start(IOService *provider)
{
    if (!super::start(provider)) return false;

    fPCIDevice = OSDynamicCast(IOPCIDevice, provider);
    if (!fPCIDevice) return false;

    fPCIDevice->setMemoryEnable(true);

    if (!mapBARs()) {
        IOLog("NvidiaPCIDevice::start – failed to map BARs\n");
        return false;
    }

    /* Verify the chip ID register */
    uint32_t bootID = readMMIO(NV_PMC_BOOT_0);
    IOLog("NvidiaPCIDevice::start – NV_PMC_BOOT_0 = 0x%08X  GPU: %s\n",
          bootID, fEntry->name);

    registerService();
    return true;
}

/* -------------------------------------------------------------------------
 * stop
 * --------------------------------------------------------------------- */
void NvidiaPCIDevice::stop(IOService *provider)
{
    unmapBARs();
    super::stop(provider);
}

/* -------------------------------------------------------------------------
 * free
 * --------------------------------------------------------------------- */
void NvidiaPCIDevice::free()
{
    unmapBARs();
    super::free();
}

/* -------------------------------------------------------------------------
 * mapBARs  – map MMIO (BAR0) and framebuffer (BAR1)
 * --------------------------------------------------------------------- */
bool NvidiaPCIDevice::mapBARs()
{
    /* BAR 0 – MMIO registers */
    fMMIOMap = fPCIDevice->mapDeviceMemoryWithRegister(
                   kIOPCIConfigBaseAddress0 + NVIDIA_BAR_MMIO * 4);
    if (!fMMIOMap) {
        IOLog("NvidiaPCIDevice::mapBARs – BAR0 (MMIO) mapping failed\n");
        return false;
    }
    fMMIOBase = reinterpret_cast<volatile uint8_t *>(fMMIOMap->getVirtualAddress());

    /* BAR 1 – framebuffer memory */
    fFBMap = fPCIDevice->mapDeviceMemoryWithRegister(
                 kIOPCIConfigBaseAddress0 + NVIDIA_BAR_FB * 4);
    if (!fFBMap) {
        IOLog("NvidiaPCIDevice::mapBARs – BAR1 (framebuffer) mapping failed; "
              "continuing without direct FB access\n");
        /* Non-fatal: some display modes do not need direct FB writes */
    } else {
        fFBBase = reinterpret_cast<volatile uint8_t *>(fFBMap->getVirtualAddress());
        fFBSize = fFBMap->getLength();
        IOLog("NvidiaPCIDevice::mapBARs – framebuffer mapped, size = %llu MB\n",
              (unsigned long long)(fFBSize / (1024 * 1024)));
    }

    return true;
}

/* -------------------------------------------------------------------------
 * unmapBARs
 * --------------------------------------------------------------------- */
void NvidiaPCIDevice::unmapBARs()
{
    if (fFBMap)   { fFBMap->release();   fFBMap   = nullptr; fFBBase = nullptr; }
    if (fMMIOMap) { fMMIOMap->release(); fMMIOMap = nullptr; fMMIOBase = nullptr; }
}

/* -------------------------------------------------------------------------
 * readMMIO / writeMMIO
 * --------------------------------------------------------------------- */
uint32_t NvidiaPCIDevice::readMMIO(uint32_t offset) const
{
    if (!fMMIOBase) return 0xFFFFFFFF;
    __asm__ volatile ("" ::: "memory");   /* prevent reordering */
    return *reinterpret_cast<volatile uint32_t *>(fMMIOBase + offset);
}

void NvidiaPCIDevice::writeMMIO(uint32_t offset, uint32_t value)
{
    if (!fMMIOBase) return;
    *reinterpret_cast<volatile uint32_t *>(fMMIOBase + offset) = value;
    __asm__ volatile ("" ::: "memory");
}
