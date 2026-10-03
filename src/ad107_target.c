#include "ad107_target.h"

static const uint32_t k_gpfifo[] = {
    NV_CLASS_AMPERE_CHANNEL_GPFIFO_A,
    NV_CLASS_AMPERE_CHANNEL_GPFIFO_B,
};

static const uint32_t k_graphics[] = {
    NV_CLASS_ADA_3D_A,
};

static const uint32_t k_compute[] = {
    NV_CLASS_ADA_COMPUTE_A,
};

static const uint32_t k_copy[] = {
    NV_CLASS_AMPERE_DMA_COPY_B,
};

static const nv_ad107_target_profile k_profile = {
    .vendor_id = NV_PCI_VENDOR_ID,
    .device_id = NV_RTX4060_LAPTOP_DEVICE_ID,
    .marketing_name = "NVIDIA GeForce RTX 4060 Laptop GPU",
    .chip = "AD107M (Ada Lovelace)",
    /*
     * NVIDIA's Ada UVM path limits channel/host VA to 40 bits even though
     * the architecture can expose wider virtual addressing elsewhere.
     */
    .max_channel_va = 1ull << 40,
    .max_host_va = 1ull << 40,
    .gpfifo_classes = k_gpfifo,
    .gpfifo_class_count = sizeof(k_gpfifo) / sizeof(k_gpfifo[0]),
    .graphics_classes = k_graphics,
    .graphics_class_count = sizeof(k_graphics) / sizeof(k_graphics[0]),
    .compute_classes = k_compute,
    .compute_class_count = sizeof(k_compute) / sizeof(k_compute[0]),
    .copy_classes = k_copy,
    .copy_class_count = sizeof(k_copy) / sizeof(k_copy[0]),
};

int nv_ad107_matches_pci(uint16_t vendor_id, uint16_t device_id)
{
    return vendor_id == k_profile.vendor_id && device_id == k_profile.device_id;
}

const nv_ad107_target_profile *nv_ad107_profile(void)
{
    return &k_profile;
}

int nv_ad107_is_blackwell_class(uint32_t class_id)
{
    return class_id == NV_CLASS_BLACKWELL_GPFIFO_B;
}
