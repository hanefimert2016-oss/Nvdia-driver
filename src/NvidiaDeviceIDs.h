/*
 * NvidiaDeviceIDs.h
 * NvidiaHackintoshDriver
 *
 * PCI device IDs for supported NVIDIA GPUs.
 * Covers: GTX 10xx, RTX 20xx, RTX 30xx, RTX 40xx series.
 */

#ifndef NVIDIA_DEVICE_IDS_H
#define NVIDIA_DEVICE_IDS_H

#include <stdint.h>

#define NVIDIA_VENDOR_ID 0x10DE

/* -----------------------------------------------------------------------
 * GTX 10xx (Pascal) – selected models
 * --------------------------------------------------------------------- */
#define NVIDIA_GTX_1050       0x1C81
#define NVIDIA_GTX_1050_TI    0x1C82
#define NVIDIA_GTX_1060_3GB   0x1C02
#define NVIDIA_GTX_1060_6GB   0x1C03
#define NVIDIA_GTX_1070       0x1B81
#define NVIDIA_GTX_1070_TI    0x1B82
#define NVIDIA_GTX_1080       0x1B80
#define NVIDIA_GTX_1080_TI    0x1B06

/* -----------------------------------------------------------------------
 * RTX 20xx (Turing)
 * --------------------------------------------------------------------- */
#define NVIDIA_RTX_2060       0x1F08
#define NVIDIA_RTX_2060_SUPER 0x1F06
#define NVIDIA_RTX_2070       0x1F02
#define NVIDIA_RTX_2070_SUPER 0x1E84
#define NVIDIA_RTX_2080       0x1E82
#define NVIDIA_RTX_2080_SUPER 0x1E81
#define NVIDIA_RTX_2080_TI    0x1E04

/* -----------------------------------------------------------------------
 * RTX 30xx (Ampere)
 * --------------------------------------------------------------------- */
#define NVIDIA_RTX_3050       0x2584
#define NVIDIA_RTX_3060       0x2503
#define NVIDIA_RTX_3060_TI    0x2486
#define NVIDIA_RTX_3070       0x2484
#define NVIDIA_RTX_3070_TI    0x2482
#define NVIDIA_RTX_3080       0x2206
#define NVIDIA_RTX_3080_TI    0x2208
#define NVIDIA_RTX_3090       0x2204
#define NVIDIA_RTX_3090_TI    0x2203

/* -----------------------------------------------------------------------
 * RTX 40xx (Ada Lovelace)
 * --------------------------------------------------------------------- */
#define NVIDIA_RTX_4060       0x2882
#define NVIDIA_RTX_4060_TI    0x2803
#define NVIDIA_RTX_4070       0x2786
#define NVIDIA_RTX_4070_SUPER 0x2783
#define NVIDIA_RTX_4070_TI    0x2782
#define NVIDIA_RTX_4080       0x2704
#define NVIDIA_RTX_4090       0x2684

/* -----------------------------------------------------------------------
 * Table entry
 * --------------------------------------------------------------------- */
typedef struct {
    uint16_t    deviceID;
    const char *name;
    uint8_t     maxDisplays;   /* maximum simultaneous displays */
    uint8_t     generation;    /* 10=Pascal, 20=Turing, 30=Ampere, 40=Ada */
} NvidiaDeviceEntry;

static const NvidiaDeviceEntry kNvidiaDeviceTable[] = {
    /* GTX 10xx */
    { NVIDIA_GTX_1050,       "NVIDIA GeForce GTX 1050",       4, 10 },
    { NVIDIA_GTX_1050_TI,    "NVIDIA GeForce GTX 1050 Ti",    4, 10 },
    { NVIDIA_GTX_1060_3GB,   "NVIDIA GeForce GTX 1060 3GB",   4, 10 },
    { NVIDIA_GTX_1060_6GB,   "NVIDIA GeForce GTX 1060 6GB",   4, 10 },
    { NVIDIA_GTX_1070,       "NVIDIA GeForce GTX 1070",       4, 10 },
    { NVIDIA_GTX_1070_TI,    "NVIDIA GeForce GTX 1070 Ti",    4, 10 },
    { NVIDIA_GTX_1080,       "NVIDIA GeForce GTX 1080",       4, 10 },
    { NVIDIA_GTX_1080_TI,    "NVIDIA GeForce GTX 1080 Ti",    4, 10 },
    /* RTX 20xx */
    { NVIDIA_RTX_2060,       "NVIDIA GeForce RTX 2060",       4, 20 },
    { NVIDIA_RTX_2060_SUPER, "NVIDIA GeForce RTX 2060 Super", 4, 20 },
    { NVIDIA_RTX_2070,       "NVIDIA GeForce RTX 2070",       4, 20 },
    { NVIDIA_RTX_2070_SUPER, "NVIDIA GeForce RTX 2070 Super", 4, 20 },
    { NVIDIA_RTX_2080,       "NVIDIA GeForce RTX 2080",       4, 20 },
    { NVIDIA_RTX_2080_SUPER, "NVIDIA GeForce RTX 2080 Super", 4, 20 },
    { NVIDIA_RTX_2080_TI,    "NVIDIA GeForce RTX 2080 Ti",    4, 20 },
    /* RTX 30xx */
    { NVIDIA_RTX_3050,       "NVIDIA GeForce RTX 3050",       4, 30 },
    { NVIDIA_RTX_3060,       "NVIDIA GeForce RTX 3060",       4, 30 },
    { NVIDIA_RTX_3060_TI,    "NVIDIA GeForce RTX 3060 Ti",    4, 30 },
    { NVIDIA_RTX_3070,       "NVIDIA GeForce RTX 3070",       4, 30 },
    { NVIDIA_RTX_3070_TI,    "NVIDIA GeForce RTX 3070 Ti",    4, 30 },
    { NVIDIA_RTX_3080,       "NVIDIA GeForce RTX 3080",       4, 30 },
    { NVIDIA_RTX_3080_TI,    "NVIDIA GeForce RTX 3080 Ti",    4, 30 },
    { NVIDIA_RTX_3090,       "NVIDIA GeForce RTX 3090",       4, 30 },
    { NVIDIA_RTX_3090_TI,    "NVIDIA GeForce RTX 3090 Ti",    4, 30 },
    /* RTX 40xx */
    { NVIDIA_RTX_4060,       "NVIDIA GeForce RTX 4060",       4, 40 },
    { NVIDIA_RTX_4060_TI,    "NVIDIA GeForce RTX 4060 Ti",    4, 40 },
    { NVIDIA_RTX_4070,       "NVIDIA GeForce RTX 4070",       4, 40 },
    { NVIDIA_RTX_4070_SUPER, "NVIDIA GeForce RTX 4070 Super", 4, 40 },
    { NVIDIA_RTX_4070_TI,    "NVIDIA GeForce RTX 4070 Ti",    4, 40 },
    { NVIDIA_RTX_4080,       "NVIDIA GeForce RTX 4080",       4, 40 },
    { NVIDIA_RTX_4090,       "NVIDIA GeForce RTX 4090",       4, 40 },
    /* sentinel */
    { 0x0000, NULL, 0, 0 }
};

/* Look up a device entry by PCI device ID. Returns NULL when not found. */
static inline const NvidiaDeviceEntry *
NvidiaFindDevice(uint16_t deviceID)
{
    for (int i = 0; kNvidiaDeviceTable[i].deviceID != 0; i++) {
        if (kNvidiaDeviceTable[i].deviceID == deviceID)
            return &kNvidiaDeviceTable[i];
    }
    return NULL;
}

#endif /* NVIDIA_DEVICE_IDS_H */
