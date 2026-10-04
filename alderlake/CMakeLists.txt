cmake_minimum_required(VERSION 3.20)
project(AlderLakeMetalVulkanBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_library(alderbridge
    src/translator.cpp
)
target_include_directories(alderbridge PUBLIC include)

add_executable(alderlake_vulkan_probe
    src/vulkan_probe.cpp
)
target_link_libraries(alderlake_vulkan_probe PRIVATE Vulkan::Vulkan)

enable_testing()

add_executable(alderbridge_tests
    tests/translator_tests.cpp
)
target_link_libraries(alderbridge_tests PRIVATE alderbridge)
add_test(NAME alderbridge_tests COMMAND alderbridge_tests)

add_executable(alderbridge_abi_tests
    baremetal/tests/abi_layout_test.cpp
)
target_include_directories(alderbridge_abi_tests PRIVATE baremetal/shared)
add_test(NAME alderbridge_abi_tests COMMAND alderbridge_abi_tests)
