#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

SDK_DIR="$PWD/MacKernelSDK"
OUT="$PWD/cross-build"
rm -rf "$OUT"
mkdir -p "$OUT/AlderLakeGPU.kext/Contents/MacOS"

if [[ ! -d "$SDK_DIR" ]]; then
  git clone --depth=1 https://github.com/acidanthera/MacKernelSDK.git "$SDK_DIR"
fi

CXX="${CXX:-clang++}"
CC="${CC:-clang}"

COMMON=(
  -target x86_64-apple-macos10.15
  -DKERNEL=1
  -D__KERNEL__=1
  -fapple-kext
  -fno-builtin
  -fno-stack-protector
  -fno-exceptions
  -fno-rtti
  -fno-threadsafe-statics
  -mno-red-zone
  -ffreestanding
  -fvisibility=hidden
  -Wno-deprecated-declarations
  -I"$SDK_DIR/Headers"
)

"$CXX" "${COMMON[@]}" -std=gnu++17 -c AlderLakeGPU.cpp -o "$OUT/AlderLakeGPU.o"
"$CC" "${COMMON[@]}" -c kmod_info.c -o "$OUT/kmod_info.o"

"$CXX"   -target x86_64-apple-macos10.15   -fuse-ld=lld   -nostdlib   -Wl,-bundle   -Wl,-undefined,dynamic_lookup   -Wl,-dead_strip   -Wl,-no_adhoc_codesign   -o "$OUT/AlderLakeGPU.kext/Contents/MacOS/AlderLakeGPU"   "$OUT/AlderLakeGPU.o"   "$OUT/kmod_info.o"   "$SDK_DIR/Library/x86_64/libkmod.a"

cp Info.plist "$OUT/AlderLakeGPU.kext/Contents/Info.plist"

file "$OUT/AlderLakeGPU.kext/Contents/MacOS/AlderLakeGPU"
llvm-nm "$OUT/AlderLakeGPU.kext/Contents/MacOS/AlderLakeGPU" | grep _kmod_info
echo "Cross-built: $OUT/AlderLakeGPU.kext"
