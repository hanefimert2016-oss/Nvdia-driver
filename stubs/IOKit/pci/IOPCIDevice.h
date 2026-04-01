/*
 * stubs/IOKit/pci/IOPCIDevice.h
 * Minimal IOKit PCI stubs for syntax-checking on Linux CI.
 */
#ifndef _IOKIT_PCI_IOPCIDEVICE_H_STUB
#define _IOKIT_PCI_IOPCIDEVICE_H_STUB

#include <stdint.h>
#include "../IOService.h"

/* PCI config-space register offsets */
#define kIOPCIConfigVendorID      0x00
#define kIOPCIConfigDeviceID      0x02
#define kIOPCIConfigBaseAddress0  0x10

/* IODeviceMemory stub */
class IODeviceMemory : public OSObject {
public:
    uintptr_t getVirtualAddress() const { return 0; }
    uint64_t  getLength()         const { return 0; }
};

/* IOMemoryMap stub */
class IOMemoryMap : public OSObject {
public:
    uintptr_t getVirtualAddress() const { return 0; }
    uint64_t  getLength()         const { return 0; }
};

/* IOPCIDevice stub */
class IOPCIDevice : public IOService {
public:
    uint16_t configRead16(uint8_t /*reg*/) const { return 0; }
    void     setMemoryEnable(bool) {}
    IOMemoryMap    *mapDeviceMemoryWithRegister(uint8_t /*reg*/) { return nullptr; }
    IODeviceMemory *getDeviceMemoryWithRegister(uint8_t /*reg*/) { return nullptr; }
};

#endif /* _IOKIT_PCI_IOPCIDEVICE_H_STUB */
