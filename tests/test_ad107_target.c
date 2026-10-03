#include <assert.h>
#include <stdio.h>
#include "ad107_target.h"

int main(void)
{
    const nv_ad107_target_profile *p = nv_ad107_profile();

    assert(p != NULL);
    assert(nv_ad107_matches_pci(0x10de, 0x28e0));
    assert(!nv_ad107_matches_pci(0x10de, 0x0000));
    assert(!nv_ad107_matches_pci(0x1234, 0x28e0));

    assert(p->gpfifo_class_count >= 1);
    assert(p->gpfifo_classes[0] == 0x0000c56f);
    assert(p->gpfifo_classes[0] != 0x0000ca6f);
    assert(p->graphics_classes[0] == 0x0000c997);
    assert(p->compute_classes[0] == 0x0000c9c0);
    assert(p->max_channel_va == (1ull << 40));
    assert(p->max_host_va == (1ull << 40));

    puts("AD107 target tests: PASS");
    return 0;
}
