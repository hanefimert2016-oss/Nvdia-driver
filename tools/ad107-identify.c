#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ad107_target.h"

static unsigned long parse_hex(const char *s)
{
    char *end = NULL;
    errno = 0;
    unsigned long v = strtoul(s, &end, 0);
    if (errno || end == s || *end != '\0') {
        fprintf(stderr, "invalid number: %s\n", s);
        exit(2);
    }
    return v;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <vendor-id> <device-id>\n", argv[0]);
        fprintf(stderr, "example: %s 0x10de 0x28e0\n", argv[0]);
        return 2;
    }

    uint16_t vendor = (uint16_t)parse_hex(argv[1]);
    uint16_t device = (uint16_t)parse_hex(argv[2]);

    if (!nv_ad107_matches_pci(vendor, device)) {
        printf("unsupported PCI device %04x:%04x\n", vendor, device);
        return 1;
    }

    const nv_ad107_target_profile *p = nv_ad107_profile();
    printf("%s\n", p->marketing_name);
    printf("chip: %s\n", p->chip);
    printf("PCI: %04x:%04x\n", p->vendor_id, p->device_id);
    printf("channel VA limit: 0x%" PRIx64 "\n", p->max_channel_va);
    printf("primary GPFIFO: 0x%08x\n", p->gpfifo_classes[0]);
    printf("Ada 3D: 0x%08x\n", p->graphics_classes[0]);
    printf("Ada compute: 0x%08x\n", p->compute_classes[0]);
    printf("copy engine candidate: 0x%08x\n", p->copy_classes[0]);
    return 0;
}
