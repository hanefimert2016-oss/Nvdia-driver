#!/bin/zsh
set -euo pipefail

cd "$(dirname "$0")"

if ! command -v xcodegen >/dev/null 2>&1; then
  echo "xcodegen is required" >&2
  exit 2
fi

if [[ ! -d MacKernelSDK ]]; then
  git clone --depth=1 https://github.com/acidanthera/MacKernelSDK.git MacKernelSDK
fi

rm -rf AlderLakeGPU.xcodeproj build
xcodegen generate --spec project.yml --project .

python3 - <<'PY'
from pathlib import Path
p = Path("AlderLakeGPU.xcodeproj/project.pbxproj")
s = p.read_text()
needle = 'productType = "com.apple.product-type.bundle";'
if s.count(needle) != 1:
    raise SystemExit("Expected exactly one bundle target")
s = s.replace(needle, 'productType = "com.apple.product-type.kernel-extension";')
s = s.replace("wrapper.cfbundle", "wrapper.kext")
s = s.replace("AlderLakeGPU.bundle", "AlderLakeGPU.kext")
p.write_text(s)
PY

xcodebuild \
  -project AlderLakeGPU.xcodeproj \
  -scheme AlderLakeGPU \
  -configuration Release \
  -derivedDataPath "$PWD/build" \
  SYMROOT="$PWD/build/Products" \
  OBJROOT="$PWD/build/Intermediates" \
  CLANG_MODULE_CACHE_PATH="$PWD/build/ModuleCache.noindex" \
  SDK_STAT_CACHE_DIR="$PWD/build/SDKStatCaches.noindex" \
  CODE_SIGNING_ALLOWED=NO \
  build

KEXT="$PWD/build/Products/Release/AlderLakeGPU.kext"
test -f "$KEXT/Contents/MacOS/AlderLakeGPU"
/usr/bin/plutil -lint "$KEXT/Contents/Info.plist"
/usr/bin/file "$KEXT/Contents/MacOS/AlderLakeGPU"
/usr/bin/nm -g "$KEXT/Contents/MacOS/AlderLakeGPU" | grep -E '(_kmod_info|AlderLakeGPU)' || true

echo "Built: $KEXT"
