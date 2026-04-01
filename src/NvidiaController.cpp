/*
 * NvidiaController.cpp
 * NvidiaHackintoshDriver
 *
 * Top-level controller: owns the PCI device nub and the display controller.
 */

#include <IOKit/IOLib.h>
#include "NvidiaController.h"

#define super IOService
OSDefineMetaClassAndStructors(NvidiaController, IOService)

/* -------------------------------------------------------------------------
 * init
 * --------------------------------------------------------------------- */
bool NvidiaController::init(OSDictionary *props)
{
    if (!super::init(props)) return false;
    IOLog("NvidiaController::init\n");
    return true;
}

/* -------------------------------------------------------------------------
 * probe
 * --------------------------------------------------------------------- */
IOService *NvidiaController::probe(IOService *provider, SInt32 *score)
{
    IOService *ret = super::probe(provider, score);
    IOLog("NvidiaController::probe\n");
    return ret;
}

/* -------------------------------------------------------------------------
 * start
 * --------------------------------------------------------------------- */
bool NvidiaController::start(IOService *provider)
{
    if (!super::start(provider)) return false;

    IOLog("NvidiaController::start – initialising NVIDIA Hackintosh driver\n");

    /* 1. Create and start the PCI device wrapper */
    fPCIDevice = new NvidiaPCIDevice;
    if (!fPCIDevice || !fPCIDevice->init()) {
        IOLog("NvidiaController::start – failed to create NvidiaPCIDevice\n");
        OSSafeReleaseNULL(fPCIDevice);
        return false;
    }

    if (!fPCIDevice->attach(provider) || !fPCIDevice->start(provider)) {
        IOLog("NvidiaController::start – NvidiaPCIDevice failed to start\n");
        fPCIDevice->detach(provider);
        OSSafeReleaseNULL(fPCIDevice);
        return false;
    }

    /* 2. Create and attach the display controller */
    if (!attachDisplay()) {
        IOLog("NvidiaController::start – display attach failed\n");
        fPCIDevice->stop(provider);
        fPCIDevice->detach(provider);
        OSSafeReleaseNULL(fPCIDevice);
        return false;
    }

    IOLog("NvidiaController::start – driver online\n");
    return true;
}

/* -------------------------------------------------------------------------
 * stop
 * --------------------------------------------------------------------- */
void NvidiaController::stop(IOService *provider)
{
    detachDisplay();

    if (fPCIDevice) {
        fPCIDevice->stop(provider);
        fPCIDevice->detach(provider);
        OSSafeReleaseNULL(fPCIDevice);
    }

    super::stop(provider);
}

/* -------------------------------------------------------------------------
 * free
 * --------------------------------------------------------------------- */
void NvidiaController::free()
{
    OSSafeReleaseNULL(fDisplay);
    OSSafeReleaseNULL(fPCIDevice);
    super::free();
}

/* -------------------------------------------------------------------------
 * attachDisplay
 * --------------------------------------------------------------------- */
bool NvidiaController::attachDisplay()
{
    fDisplay = new NvidiaDisplayController;
    if (!fDisplay || !fDisplay->init()) {
        OSSafeReleaseNULL(fDisplay);
        return false;
    }

    if (!fDisplay->attach(fPCIDevice) || !fDisplay->start(fPCIDevice)) {
        IOLog("NvidiaController::attachDisplay – NvidiaDisplayController failed to start\n");
        fDisplay->detach(fPCIDevice);
        OSSafeReleaseNULL(fDisplay);
        return false;
    }

    IOLog("NvidiaController::attachDisplay – display controller online\n");
    return true;
}

/* -------------------------------------------------------------------------
 * detachDisplay
 * --------------------------------------------------------------------- */
void NvidiaController::detachDisplay()
{
    if (fDisplay) {
        fDisplay->stop(fPCIDevice);
        fDisplay->detach(fPCIDevice);
        OSSafeReleaseNULL(fDisplay);
    }
}
