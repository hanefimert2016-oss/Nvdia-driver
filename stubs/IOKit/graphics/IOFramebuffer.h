/*
 * stubs/IOKit/graphics/IOFramebuffer.h
 * Minimal IOFramebuffer stubs for syntax-checking on Linux CI.
 */
#ifndef _IOKIT_IOFRAMEBUFFER_H_STUB
#define _IOKIT_IOFRAMEBUFFER_H_STUB

#include <stdint.h>
#include "../IOService.h"
#include "../pci/IOPCIDevice.h"

typedef uint32_t  IODisplayModeID;
typedef int32_t   IOIndex;
typedef uint32_t  IOPixelAperture;

#define kIORGBDirectPixels  0x10
#define IO32BitDirectPixels "--------RRRRRRRRGGGGGGGGBBBBBBBB"

/* IODisplayModeInformation */
struct IODisplayModeInformation {
    uint32_t maxDepthIndex;
    uint32_t nominalWidth;
    uint32_t nominalHeight;
    uint32_t refreshRate;   /* 16.16 fixed-point Hz */
    uint8_t  _pad[64];
};

/* IOPixelInformation */
struct IOPixelInformation {
    uint32_t bytesPerRow;
    uint32_t bytesPerPlane;
    uint32_t bitsPerPixel;
    uint32_t pixelType;
    uint32_t componentMasks[4];
    uint8_t  bitsPerComponent;
    uint8_t  componentCount;
    char     pixelFormat[64];
    uint8_t  _pad[128];
};

class IOFramebuffer : public IOService {
public:
    virtual IOReturn enableController() { return kIOReturnSuccess; }
    virtual IOReturn getDisplayModes(IODisplayModeID *, uint32_t *) { return kIOReturnSuccess; }
    virtual IOReturn getDisplayModeInformation(IODisplayModeID, IOIndex,
                                               IODisplayModeInformation *) { return kIOReturnSuccess; }
    virtual IOReturn setDisplayMode(IODisplayModeID, IOIndex) { return kIOReturnSuccess; }
    virtual IOReturn getApertureRange(IOPixelAperture, IODeviceMemory **) { return kIOReturnSuccess; }
    virtual IOReturn getPixelInformation(IODisplayModeID, IOIndex, IOPixelAperture,
                                         IOPixelInformation *) { return kIOReturnSuccess; }
    virtual IOReturn getCurrentDisplayMode(IODisplayModeID *, IOIndex *) { return kIOReturnSuccess; }
};

#endif /* _IOKIT_IOFRAMEBUFFER_H_STUB */
