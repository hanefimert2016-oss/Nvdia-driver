#include "AlderBridgeABI.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    using namespace alderbridge::abi;

    if (kAbiVersion != 1) {
        std::cerr << "Unexpected ABI version\n";
        return EXIT_FAILURE;
    }

    DeviceInfo info{};
    info.vendor_id = kIntelVendorId;
    info.device_id = kAlderLakePDeviceId;

    if (info.vendor_id != 0x8086 || info.device_id != 0x46A6) {
        std::cerr << "Target PCI ID mismatch\n";
        return EXIT_FAILURE;
    }

    std::cout << "baremetal ABI layout: OK\n";
    return EXIT_SUCCESS;
}
