#ifndef NVRM_AD107_TARGET_H
#define NVRM_AD107_TARGET_H

#include <stddef.h>
#include <stdint.h>

#define NV_PCI_VENDOR_ID                 0x10deu
#define NV_RTX4060_LAPTOP_DEVICE_ID      0x28e0u

/* Public NVIDIA/Nouveau class IDs used by Ada AD10x. */
#define NV_CLASS_AMPERE_CHANNEL_GPFIFO_A 0x0000c56fu
#define NV_CLASS_AMPERE_CHANNEL_GPFIFO_B 0x0000c76fu
#define NV_CLASS_AMPERE_USERMODE_A       0x0000c561u
#define NV_CLASS_AMPERE_DMA_COPY_B       0x0000c7b5u
#define NV_CLASS_ADA_3D_A                0x0000c997u
#define NV_CLASS_ADA_COMPUTE_A           0x0000c9c0u
#define NV_CLASS_ADA_DISPLAY             0x0000c770u
#define NV_CLASS_ADA_DISP_CORE_DMA       0x0000c77du

/* Blackwell value from the RTX 5060 bring-up. It is intentionally NOT
 * selected for AD107 and is kept here only as a regression sentinel. */
#define NV_CLASS_BLACKWELL_GPFIFO_B      0x0000ca6fu

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    const char *marketing_name;
    const char *chip;
    uint64_t max_channel_va;
    uint64_t max_host_va;
    const uint32_t *gpfifo_classes;
    size_t gpfifo_class_count;
    const uint32_t *graphics_classes;
    size_t graphics_class_count;
    const uint32_t *compute_classes;
    size_t compute_class_count;
    const uint32_t *copy_classes;
    size_t copy_class_count;
} nv_ad107_target_profile;

int nv_ad107_matches_pci(uint16_t vendor_id, uint16_t device_id);
const nv_ad107_target_profile *nv_ad107_profile(void);
int nv_ad107_is_blackwell_class(uint32_t class_id);

#endif
