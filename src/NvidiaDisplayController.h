/*
 * NvidiaDisplayController.h
 * NvidiaHackintoshDriver
 *
 * IOFramebuffer subclass.
 *
 * Phase 1 – "single display" mode:
 *   Exposes exactly one display connector (HDMI/DP) and drives it at the
 *   requested resolution using the GPU's display engine registers.
 *
 * Phase 2 – "accelerated" mode:
 *   Enables the 2-D / 3-D acceleration engine via IOAccelerator,
 *   registers surfaces, and hands off to the GPU command processor.
 */

#ifndef NVIDIA_DISPLAY_CONTROLLER_H
#define NVIDIA_DISPLAY_CONTROLLER_H

#include <IOKit/graphics/IOFramebuffer.h>
#include "NvidiaPCIDevice.h"

/* Display engine register offsets relative to NV_PDISPLAY_BASE */
#define NV_DISP_CTRL            0x0000   /* display control          */
#define NV_DISP_STATUS          0x0004   /* status / vsync flag      */
#define NV_DISP_HTOTAL          0x0100   /* horizontal total         */
#define NV_DISP_HBLANK          0x0104   /* horizontal blank         */
#define NV_DISP_HSYNC           0x0108   /* horizontal sync          */
#define NV_DISP_VTOTAL          0x0140   /* vertical total           */
#define NV_DISP_VBLANK          0x0144   /* vertical blank           */
#define NV_DISP_VSYNC           0x0148   /* vertical sync            */
#define NV_DISP_FB_OFFSET       0x0200   /* framebuffer start offset */
#define NV_DISP_FB_PITCH        0x0204   /* framebuffer pitch        */
#define NV_DISP_OUTPUT_FORMAT   0x0300   /* output colour format     */

/* Acceleration engine registers */
#define NV_PGRAPH_BASE          0x00400000
#define NV_PGRAPH_CTX_CONTROL   0x00000144
#define NV_PGRAPH_FIFO          0x00000500

/* Supported pixel formats */
typedef enum {
    kNvidiaPixelFormat32BGRA = 0,
    kNvidiaPixelFormatCount
} NvidiaPixelFormat;

/* A single display timing descriptor */
typedef struct {
    uint32_t hActive;
    uint32_t hFrontPorch;
    uint32_t hSyncWidth;
    uint32_t hBackPorch;
    uint32_t vActive;
    uint32_t vFrontPorch;
    uint32_t vSyncWidth;
    uint32_t vBackPorch;
    uint32_t pixelClockKHz;
    const char *name;
} NvidiaDisplayTiming;

class NvidiaDisplayController : public IOFramebuffer
{
    OSDeclareDefaultStructors(NvidiaDisplayController)

public:
    /* IOFramebuffer overrides */
    virtual IOReturn    enableController() override;
    virtual IOReturn    getDisplayModes(IODisplayModeID *allDisplayModes,
                                        uint32_t *count) override;
    virtual IOReturn    getDisplayModeInformation(IODisplayModeID displayMode,
                                                  IOIndex depth,
                                                  IODisplayModeInformation *info) override;
    virtual IOReturn    setDisplayMode(IODisplayModeID displayMode,
                                       IOIndex depth) override;
    virtual IOReturn    getApertureRange(IOPixelAperture aperture,
                                         IODeviceMemory **range) override;
    virtual IOReturn    getPixelInformation(IODisplayModeID displayMode,
                                            IOIndex depth,
                                            IOPixelAperture aperture,
                                            IOPixelInformation *pixelInfo) override;
    virtual IOReturn    getCurrentDisplayMode(IODisplayModeID *displayMode,
                                              IOIndex *depth) override;

    /* Lifecycle */
    virtual bool        start(IOService *provider) override;
    virtual void        stop(IOService *provider) override;

private:
    NvidiaPCIDevice    *fGPU            = nullptr;
    IODisplayModeID     fCurrentMode    = 1;
    IOIndex             fCurrentDepth   = 0;

    /* Built-in timing table (one per supported mode) */
    static const NvidiaDisplayTiming kTimings[];
    static const uint32_t kTimingCount;

    IOReturn programTiming(const NvidiaDisplayTiming &t);
    void     enableAcceleration();
    void     writeDisplayReg(uint32_t offset, uint32_t value);
    uint32_t readDisplayReg(uint32_t offset) const;
};

#endif /* NVIDIA_DISPLAY_CONTROLLER_H */
