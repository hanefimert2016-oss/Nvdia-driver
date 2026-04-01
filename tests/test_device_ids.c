/*
 * tests/test_device_ids.c
 * NvidiaHackintoshDriver – unit tests
 *
 * Tests that:
 *  1. NvidiaFindDevice returns the correct entry for every supported device.
 *  2. NvidiaFindDevice returns NULL for unknown device IDs.
 *  3. The device table sentinel is present and correct.
 *  4. Every entry in the table has a non-NULL name and a supported generation.
 *  5. Display timing arithmetic produces sane refresh rates.
 *
 * Compiled and run on any platform (Linux, macOS, Windows).
 * No IOKit headers required.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*
 * Pull in only the header we actually need; stub out IOKit types so that
 * the test binary can be compiled without a macOS SDK.
 */
#define __KERNEL__  /* suppress IOKit includes inside the header */
#define KERNEL
typedef unsigned int IOReturn;
#include "NvidiaDeviceIDs.h"

/* -------------------------------------------------------------------------
 * Tiny test framework
 * --------------------------------------------------------------------- */

static int g_tests_run    = 0;
static int g_tests_failed = 0;

#define TEST(name)  static void test_##name(void)
#define RUN(name)   do { \
    printf("  %-50s", #name); \
    g_tests_run++; \
    test_##name(); \
    printf("OK\n"); \
} while(0)

#define EXPECT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "\n  FAIL  %s:%d  (%s)\n", __FILE__, __LINE__, #cond); \
        g_tests_failed++; \
        return; \
    } \
} while(0)

/* -------------------------------------------------------------------------
 * Tests
 * --------------------------------------------------------------------- */

TEST(find_known_device_rtx4090)
{
    const NvidiaDeviceEntry *e = NvidiaFindDevice(NVIDIA_RTX_4090);
    EXPECT(e != NULL);
    EXPECT(e->deviceID == NVIDIA_RTX_4090);
    EXPECT(strcmp(e->name, "NVIDIA GeForce RTX 4090") == 0);
    EXPECT(e->generation == 40);
}

TEST(find_known_device_rtx3080)
{
    const NvidiaDeviceEntry *e = NvidiaFindDevice(NVIDIA_RTX_3080);
    EXPECT(e != NULL);
    EXPECT(e->deviceID == NVIDIA_RTX_3080);
    EXPECT(e->generation == 30);
}

TEST(find_known_device_rtx2060)
{
    const NvidiaDeviceEntry *e = NvidiaFindDevice(NVIDIA_RTX_2060);
    EXPECT(e != NULL);
    EXPECT(e->deviceID == NVIDIA_RTX_2060);
    EXPECT(e->generation == 20);
}

TEST(find_known_device_gtx1080ti)
{
    const NvidiaDeviceEntry *e = NvidiaFindDevice(NVIDIA_GTX_1080_TI);
    EXPECT(e != NULL);
    EXPECT(e->deviceID == NVIDIA_GTX_1080_TI);
    EXPECT(e->generation == 10);
}

TEST(find_unknown_device_returns_null)
{
    /* 0xDEAD is not a valid NVIDIA device ID in our table */
    const NvidiaDeviceEntry *e = NvidiaFindDevice(0xDEAD);
    EXPECT(e == NULL);
}

TEST(sentinel_is_last)
{
    int i = 0;
    while (kNvidiaDeviceTable[i].deviceID != 0) i++;
    /* sentinel must have NULL name */
    EXPECT(kNvidiaDeviceTable[i].name == NULL);
}

TEST(all_entries_have_valid_name_and_generation)
{
    for (int i = 0; kNvidiaDeviceTable[i].deviceID != 0; i++) {
        EXPECT(kNvidiaDeviceTable[i].name != NULL);
        EXPECT(strlen(kNvidiaDeviceTable[i].name) > 0);
        uint8_t gen = kNvidiaDeviceTable[i].generation;
        EXPECT(gen == 10 || gen == 20 || gen == 30 || gen == 40);
        EXPECT(kNvidiaDeviceTable[i].maxDisplays >= 1);
    }
}

TEST(all_rtx_entries_have_rtx_prefix)
{
    for (int i = 0; kNvidiaDeviceTable[i].deviceID != 0; i++) {
        uint8_t gen = kNvidiaDeviceTable[i].generation;
        if (gen >= 20) {
            /* RTX 20xx / 30xx / 40xx must have "RTX" in the name */
            EXPECT(strstr(kNvidiaDeviceTable[i].name, "RTX") != NULL);
        }
    }
}

TEST(gtx_entries_have_gtx_prefix)
{
    for (int i = 0; kNvidiaDeviceTable[i].deviceID != 0; i++) {
        if (kNvidiaDeviceTable[i].generation == 10) {
            EXPECT(strstr(kNvidiaDeviceTable[i].name, "GTX") != NULL);
        }
    }
}

TEST(unique_device_ids)
{
    /* No two entries should share the same device ID */
    for (int i = 0; kNvidiaDeviceTable[i].deviceID != 0; i++) {
        for (int j = i + 1; kNvidiaDeviceTable[j].deviceID != 0; j++) {
            if (kNvidiaDeviceTable[i].deviceID == kNvidiaDeviceTable[j].deviceID) {
                fprintf(stderr,
                    "\n  FAIL  duplicate deviceID 0x%04X at indices %d and %d\n",
                    kNvidiaDeviceTable[i].deviceID, i, j);
                g_tests_failed++;
                return;
            }
        }
    }
}

TEST(display_timing_1080p_refresh_rate)
{
    /*
     * 1920×1080@60 Hz
     *   pixel clock = 148 500 kHz
     *   htotal      = 1920 + 88 + 44 + 148 = 2200
     *   vtotal      = 1080 + 4  + 5  + 36  = 1125
     *   refresh     = 148 500 000 / (2200 × 1125) ≈ 60.00 Hz
     */
    uint32_t hActive = 1920, hFP = 88, hSW = 44, hBP = 148;
    uint32_t vActive = 1080, vFP = 4,  vSW = 5,  vBP = 36;
    uint32_t pclkHz  = 148500 * 1000UL;

    uint32_t hTotal = hActive + hFP + hSW + hBP;
    uint32_t vTotal = vActive + vFP + vSW + vBP;
    uint32_t refresh = pclkHz / (hTotal * vTotal);

    EXPECT(refresh >= 59 && refresh <= 61);
}

TEST(display_timing_4k60_refresh_rate)
{
    /*
     * 3840×2160@60 Hz
     *   pixel clock = 594 000 kHz
     *   htotal      = 3840 + 176 + 88 + 296 = 4400
     *   vtotal      = 2160 + 8   + 10 + 72  = 2250
     *   refresh     = 594 000 000 / (4400 × 2250) ≈ 60.00 Hz
     */
    uint32_t hActive = 3840, hFP = 176, hSW = 88, hBP = 296;
    uint32_t vActive = 2160, vFP = 8,   vSW = 10, vBP = 72;
    uint32_t pclkHz  = 594000UL * 1000UL;

    uint32_t hTotal = hActive + hFP + hSW + hBP;
    uint32_t vTotal = vActive + vFP + vSW + vBP;
    uint32_t refresh = pclkHz / (hTotal * vTotal);

    EXPECT(refresh >= 59 && refresh <= 61);
}

/* -------------------------------------------------------------------------
 * main
 * --------------------------------------------------------------------- */

int main(void)
{
    printf("\nNvidiaHackintoshDriver – unit tests\n");
    printf("=====================================\n");

    RUN(find_known_device_rtx4090);
    RUN(find_known_device_rtx3080);
    RUN(find_known_device_rtx2060);
    RUN(find_known_device_gtx1080ti);
    RUN(find_unknown_device_returns_null);
    RUN(sentinel_is_last);
    RUN(all_entries_have_valid_name_and_generation);
    RUN(all_rtx_entries_have_rtx_prefix);
    RUN(gtx_entries_have_gtx_prefix);
    RUN(unique_device_ids);
    RUN(display_timing_1080p_refresh_rate);
    RUN(display_timing_4k60_refresh_rate);

    printf("=====================================\n");
    printf("Results: %d/%d passed", g_tests_run - g_tests_failed, g_tests_run);
    if (g_tests_failed > 0) {
        printf("  (%d FAILED)\n", g_tests_failed);
        return 1;
    }
    printf("\n");
    return 0;
}
