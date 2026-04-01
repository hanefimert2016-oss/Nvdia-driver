/*
 * stubs/IOKit/graphics/IOFramebuffer.h
 * Minimal IOFramebuffer stubs for syntax-checking on Linux CI.
 * Signatures mirror the real macOS SDK IOFramebuffer.h pure virtuals.
 */
#ifndef _IOKIT_IOFRAMEBUFFER_H_STUB
#define _IOKIT_IOFRAMEBUFFER_H_STUB

#include <stdint.h>
#include "../IOService.h"
#include "../pci/IOPCIDevice.h"

typedef uint32_t  IODisplayModeID;
typedef int32_t   IOIndex;
typedef uint32_t  IOPixelAperture;
typedef uint32_t  IOItemCount;
typedef uint64_t  UInt64;

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

/*
 * IOFramebuffer – pure-virtual interface matching the real macOS SDK.
 * All pure virtuals must be implemented by subclasses.
 */
class IOFramebuffer : public IOService {
public:
    virtual IOReturn         enableController()                                           { return 0; }
    virtual IOItemCount      getDisplayModeCount(void)                                    = 0;
    virtual IOReturn         getDisplayModes(IODisplayModeID *allDisplayModes)            = 0;
    virtual IOReturn         getInformationForDisplayMode(IODisplayModeID displayMode,
                                                          IODisplayModeInformation *info) = 0;
    virtual const char      *getPixelFormats(void)                                        = 0;
    virtual UInt64           getPixelFormatsForDisplayMode(IODisplayModeID displayMode,
                                                           IOIndex depth)                 = 0;
    virtual IOReturn         getPixelInformation(IODisplayModeID displayMode,
                                                 IOIndex depth,
                                                 IOPixelAperture aperture,
                                                 IOPixelInformation *pixelInfo)           = 0;
    virtual IODeviceMemory  *getApertureRange(IOPixelAperture aperture)                   = 0;
    virtual IOReturn         setDisplayMode(IODisplayModeID displayMode,
                                            IOIndex depth)                                { return 0; }
    virtual IOReturn         getCurrentDisplayMode(IODisplayModeID *displayMode,
                                                   IOIndex *depth)                        { return 0; }
};

#endif /* _IOKIT_IOFRAMEBUFFER_H_STUB */

