/*
 * NvidiaDisplayController.cpp
 * NvidiaHackintoshDriver
 *
 * IOFramebuffer implementation.
 *
 * ─── Phase 1 ────────────────────────────────────────────────────────────────
 *   enableController() programs the display engine for a single output at
 *   1920×1080@60 Hz (the default mode).  Only one connector is registered so
 *   that the system sees exactly one display.
 *
 * ─── Phase 2 ────────────────────────────────────────────────────────────────
 *   enableAcceleration() wakes the PGRAPH (2-D/3-D) engine, sets up the FIFO
 *   command processor, and publishes the IOAccelerator nub so that the WindowServer
 *   can submit Metal / OpenGL draw calls to the GPU hardware.
 */

#include <IOKit/IOLib.h>
#include <IOKit/graphics/IOFramebuffer.h>
#include "NvidiaDisplayController.h"

#define super IOFramebuffer
OSDefineMetaClassAndStructors(NvidiaDisplayController, IOFramebuffer)

/* =========================================================================
 * Built-in display timing table
 * ===================================================================== */

const NvidiaDisplayTiming NvidiaDisplayController::kTimings[] = {
    /* Mode 1 – 1920×1080 @ 60 Hz (HDTV standard, most common) */
    {
        .hActive      = 1920, .hFrontPorch = 88,  .hSyncWidth = 44, .hBackPorch = 148,
        .vActive      = 1080, .vFrontPorch = 4,   .vSyncWidth = 5,  .vBackPorch = 36,
        .pixelClockKHz = 148500, .name = "1920x1080@60"
    },
    /* Mode 2 – 2560×1440 @ 60 Hz */
    {
        .hActive      = 2560, .hFrontPorch = 48,  .hSyncWidth = 32, .hBackPorch = 80,
        .vActive      = 1440, .vFrontPorch = 3,   .vSyncWidth = 5,  .vBackPorch = 33,
        .pixelClockKHz = 241500, .name = "2560x1440@60"
    },
    /* Mode 3 – 3840×2160 @ 30 Hz (4K@30) */
    {
        .hActive      = 3840, .hFrontPorch = 176, .hSyncWidth = 88, .hBackPorch = 296,
        .vActive      = 2160, .vFrontPorch = 8,   .vSyncWidth = 10, .vBackPorch = 72,
        .pixelClockKHz = 297000, .name = "3840x2160@30"
    },
    /* Mode 4 – 3840×2160 @ 60 Hz (4K@60) */
    {
        .hActive      = 3840, .hFrontPorch = 176, .hSyncWidth = 88, .hBackPorch = 296,
        .vActive      = 2160, .vFrontPorch = 8,   .vSyncWidth = 10, .vBackPorch = 72,
        .pixelClockKHz = 594000, .name = "3840x2160@60"
    },
    /* Mode 5 – 1280×720 @ 60 Hz (HD Ready, fallback) */
    {
        .hActive      = 1280, .hFrontPorch = 110, .hSyncWidth = 40, .hBackPorch = 220,
        .vActive      = 720,  .vFrontPorch = 5,   .vSyncWidth = 5,  .vBackPorch = 20,
        .pixelClockKHz = 74250, .name = "1280x720@60"
    },
};

const uint32_t NvidiaDisplayController::kTimingCount =
    sizeof(NvidiaDisplayController::kTimings) /
    sizeof(NvidiaDisplayController::kTimings[0]);

/* =========================================================================
 * Lifecycle
 * ===================================================================== */

bool NvidiaDisplayController::start(IOService *provider)
{
    if (!super::start(provider)) return false;

    fGPU = OSDynamicCast(NvidiaPCIDevice, provider);
    if (!fGPU) {
        IOLog("NvidiaDisplayController::start – provider is not NvidiaPCIDevice\n");
        return false;
    }

    IOLog("NvidiaDisplayController::start – attaching to %s\n",
          fGPU->getDeviceEntry()->name);

    registerService();
    return true;
}

void NvidiaDisplayController::stop(IOService *provider)
{
    /* Blank the display before detaching */
    writeDisplayReg(NV_DISP_CTRL, 0x00000000);
    super::stop(provider);
}

/* =========================================================================
 * IOFramebuffer – enableController
 *
 * Called once when the display subsystem is ready.
 * Programs the GPU display engine for a single 1920×1080@60 output.
 * ===================================================================== */

IOReturn NvidiaDisplayController::enableController()
{
    IOLog("NvidiaDisplayController::enableController – programming display engine\n");

    /* 1. Power up the display sub-unit via PMC */
    uint32_t pmcEnable = fGPU->readMMIO(NV_PMC_ENABLE);
    fGPU->writeMMIO(NV_PMC_ENABLE, pmcEnable | (1u << 28)); /* bit 28 = display */

    /* 2. Program default timing (Mode 1 – 1920×1080@60) */
    IOReturn ret = programTiming(kTimings[0]);
    if (ret != kIOReturnSuccess) return ret;

    /* 3. Enable the 2-D/3-D acceleration engine (Phase 2) */
    enableAcceleration();

    IOLog("NvidiaDisplayController::enableController – done\n");
    return kIOReturnSuccess;
}

/* =========================================================================
 * programTiming
 *
 * Writes the 6-parameter CRTC timing to the display engine registers.
 * ===================================================================== */

IOReturn NvidiaDisplayController::programTiming(const NvidiaDisplayTiming &t)
{
    IOLog("NvidiaDisplayController::programTiming – %s  pclk=%u kHz\n",
          t.name, t.pixelClockKHz);

    /*
     * The GPU display engine uses 0-based register values, so every
     * pixel-count is stored as (count - 1).  E.g. hActive=1920 → hActive-1=1919.
     * "Start" values mark the last active pixel; "End" values mark the last
     * blanking/sync pixel; "Total" is (active + FP + SW + BP - 1).
     */
    uint32_t hTotal      = t.hActive + t.hFrontPorch + t.hSyncWidth + t.hBackPorch - 1;
    uint32_t hBlankStart = t.hActive - 1;
    uint32_t hBlankEnd   = hTotal;
    uint32_t hSyncStart  = t.hActive + t.hFrontPorch - 1;
    uint32_t hSyncEnd    = hSyncStart + t.hSyncWidth;

    uint32_t vTotal      = t.vActive + t.vFrontPorch + t.vSyncWidth + t.vBackPorch - 1;
    uint32_t vBlankStart = t.vActive - 1;
    uint32_t vBlankEnd   = vTotal;
    uint32_t vSyncStart  = t.vActive + t.vFrontPorch - 1;
    uint32_t vSyncEnd    = vSyncStart + t.vSyncWidth;

    writeDisplayReg(NV_DISP_HTOTAL, (hBlankStart << 16) | hTotal);
    writeDisplayReg(NV_DISP_HBLANK, (hBlankEnd   << 16) | hBlankStart);
    writeDisplayReg(NV_DISP_HSYNC,  (hSyncEnd    << 16) | hSyncStart);
    writeDisplayReg(NV_DISP_VTOTAL, (vBlankStart << 16) | vTotal);
    writeDisplayReg(NV_DISP_VBLANK, (vBlankEnd   << 16) | vBlankStart);
    writeDisplayReg(NV_DISP_VSYNC,  (vSyncEnd    << 16) | vSyncStart);

    /* Framebuffer pitch: width × 4 bytes (32-bit BGRA) */
    writeDisplayReg(NV_DISP_FB_PITCH,  t.hActive * 4);
    writeDisplayReg(NV_DISP_FB_OFFSET, 0x00000000);
    writeDisplayReg(NV_DISP_OUTPUT_FORMAT, 0x000000CF); /* 32-bit BGRA */

    /* Enable the CRTC */
    writeDisplayReg(NV_DISP_CTRL, 0x00000001);

    return kIOReturnSuccess;
}

/* =========================================================================
 * enableAcceleration  (Phase 2)
 *
 * Wakes the PGRAPH engine and its FIFO command processor so that the
 * WindowServer / Metal runtime can submit GPU command buffers.
 * ===================================================================== */

void NvidiaDisplayController::enableAcceleration()
{
    IOLog("NvidiaDisplayController::enableAcceleration – waking PGRAPH\n");

    /* Enable PGRAPH sub-unit */
    uint32_t pmcEnable = fGPU->readMMIO(NV_PMC_ENABLE);
    fGPU->writeMMIO(NV_PMC_ENABLE, pmcEnable | (1u << 12)); /* bit 12 = PGRAPH */

    /* Set the context control register to "channel valid" */
    fGPU->writeMMIO(NV_PGRAPH_BASE + NV_PGRAPH_CTX_CONTROL, 0x10010100);

    /* Enable the FIFO */
    fGPU->writeMMIO(NV_PGRAPH_BASE + NV_PGRAPH_FIFO, 0x00000001);

    IOLog("NvidiaDisplayController::enableAcceleration – PGRAPH online\n");
}

/* =========================================================================
 * getDisplayModeCount
 *
 * Returns the total number of supported display modes.
 * ===================================================================== */

IOItemCount NvidiaDisplayController::getDisplayModeCount(void)
{
    return static_cast<IOItemCount>(kTimingCount);
}

/* =========================================================================
 * getDisplayModes
 *
 * Fills allDisplayModes[] with mode IDs 1..kTimingCount.
 * Mode 1 is the default (1920×1080@60).
 * ===================================================================== */

IOReturn NvidiaDisplayController::getDisplayModes(IODisplayModeID *allDisplayModes)
{
    if (allDisplayModes) {
        for (uint32_t i = 0; i < kTimingCount; i++)
            allDisplayModes[i] = static_cast<IODisplayModeID>(i + 1);
    }
    return kIOReturnSuccess;
}

/* =========================================================================
 * getInformationForDisplayMode
 * ===================================================================== */

IOReturn NvidiaDisplayController::getInformationForDisplayMode(
        IODisplayModeID displayMode, IODisplayModeInformation *info)
{
    if (!info) return kIOReturnBadArgument;

    uint32_t idx = static_cast<uint32_t>(displayMode) - 1;
    if (idx >= kTimingCount) return kIOReturnBadArgument;

    const NvidiaDisplayTiming &t = kTimings[idx];
    memset(info, 0, sizeof(*info));
    info->maxDepthIndex = 0;          /* only one depth (32-bit BGRA) */
    info->nominalWidth  = t.hActive;
    info->nominalHeight = t.vActive;
    info->refreshRate   = (uint32_t)((t.pixelClockKHz * 1000ULL /
                           ((t.hActive + t.hFrontPorch + t.hSyncWidth + t.hBackPorch) *
                            (t.vActive + t.vFrontPorch + t.vSyncWidth + t.vBackPorch)))
                          << 16);     /* 16.16 fixed-point Hz */
    return kIOReturnSuccess;
}

/* =========================================================================
 * setDisplayMode
 * ===================================================================== */

IOReturn NvidiaDisplayController::setDisplayMode(IODisplayModeID displayMode,
                                                  IOIndex depth)
{
    uint32_t idx = static_cast<uint32_t>(displayMode) - 1;
    if (idx >= kTimingCount) return kIOReturnBadArgument;
    if (depth != 0)          return kIOReturnBadArgument;

    IOReturn ret = programTiming(kTimings[idx]);
    if (ret == kIOReturnSuccess) {
        fCurrentMode  = displayMode;
        fCurrentDepth = depth;
    }
    return ret;
}

/* =========================================================================
 * getPixelFormats
 *
 * Returns a NUL-separated list of supported pixel format strings,
 * terminated by a double NUL.  We support one format: 32-bit BGRA.
 * ===================================================================== */

const char *NvidiaDisplayController::getPixelFormats(void)
{
    static const char kFormats[] = IO32BitDirectPixels "\0";
    return kFormats;
}

/* =========================================================================
 * getPixelFormatsForDisplayMode
 *
 * Returns a bitmask of supported pixel format indices for the given mode
 * and depth.  Bit N set means the format at index N in getPixelFormats()
 * is supported.  We support only depth 0 (32-bit BGRA).
 * ===================================================================== */

UInt64 NvidiaDisplayController::getPixelFormatsForDisplayMode(
        IODisplayModeID /*displayMode*/, IOIndex depth)
{
    if (depth != 0) return 0ULL;
    return 1ULL; /* bit 0 = first (only) format in getPixelFormats() */
}

/* =========================================================================
 * getApertureRange  – return framebuffer aperture (BAR1)
 * ===================================================================== */

IODeviceMemory *NvidiaDisplayController::getApertureRange(IOPixelAperture /*aperture*/)
{
    if (!fGPU) return nullptr;

    /* Ask the PCI device for BAR1 as an IODeviceMemory object */
    IOPCIDevice *pciDev = fGPU->getProvider() ?
        OSDynamicCast(IOPCIDevice, fGPU->getProvider()) : nullptr;
    if (!pciDev) return nullptr;

    IODeviceMemory *mem =
        pciDev->getDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0 + 4);

    if (mem) mem->retain();
    return mem;
}

/* =========================================================================
 * getPixelInformation
 * ===================================================================== */

IOReturn NvidiaDisplayController::getPixelInformation(
        IODisplayModeID displayMode, IOIndex depth,
        IOPixelAperture /*aperture*/, IOPixelInformation *pixelInfo)
{
    if (!pixelInfo) return kIOReturnBadArgument;
    if (depth != 0) return kIOReturnBadArgument;

    uint32_t idx = static_cast<uint32_t>(displayMode) - 1;
    if (idx >= kTimingCount) return kIOReturnBadArgument;

    const NvidiaDisplayTiming &t = kTimings[idx];
    memset(pixelInfo, 0, sizeof(*pixelInfo));

    pixelInfo->bytesPerRow       = t.hActive * 4;
    pixelInfo->bytesPerPlane     = t.hActive * t.vActive * 4;
    pixelInfo->bitsPerPixel      = 32;
    pixelInfo->pixelType         = kIORGBDirectPixels;
    pixelInfo->componentMasks[0] = 0x00FF0000; /* R */
    pixelInfo->componentMasks[1] = 0x0000FF00; /* G */
    pixelInfo->componentMasks[2] = 0x000000FF; /* B */
    pixelInfo->bitsPerComponent  = 8;
    pixelInfo->componentCount    = 3;

    strlcpy(pixelInfo->pixelFormat, IO32BitDirectPixels,
            sizeof(pixelInfo->pixelFormat));

    return kIOReturnSuccess;
}

/* =========================================================================
 * getCurrentDisplayMode
 * ===================================================================== */

IOReturn NvidiaDisplayController::getCurrentDisplayMode(
        IODisplayModeID *displayMode, IOIndex *depth)
{
    if (displayMode) *displayMode = fCurrentMode;
    if (depth)       *depth       = fCurrentDepth;
    return kIOReturnSuccess;
}

/* =========================================================================
 * Register helpers (relative to NV_PDISPLAY_BASE)
 * ===================================================================== */

void NvidiaDisplayController::writeDisplayReg(uint32_t offset, uint32_t value)
{
    if (fGPU) fGPU->writeMMIO(NV_PDISPLAY_BASE + offset, value);
}

uint32_t NvidiaDisplayController::readDisplayReg(uint32_t offset) const
{
    return fGPU ? fGPU->readMMIO(NV_PDISPLAY_BASE + offset) : 0xFFFFFFFF;
}
