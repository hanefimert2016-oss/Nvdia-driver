#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-}"
if [[ -z "$ROOT" ]]; then
  echo "Usage: $0 /path/to/OPENCORE-mount-or-OC-directory" >&2
  exit 2
fi

ROOT="$(realpath "$ROOT")"

if [[ -f "$ROOT/EFI/OC/config.plist" ]]; then
  OC="$ROOT/EFI/OC"
elif [[ -f "$ROOT/OC/config.plist" ]]; then
  OC="$ROOT/OC"
elif [[ -f "$ROOT/config.plist" ]]; then
  OC="$ROOT"
else
  echo "Could not find EFI/OC/config.plist below: $ROOT" >&2
  exit 3
fi

HERE="$(cd "$(dirname "$0")" && pwd)"
KEXT_SRC="$HERE/AlderLakeGPU.kext"
CONFIG="$OC/config.plist"

if [[ ! -d "$KEXT_SRC" ]]; then
  echo "Missing $KEXT_SRC. Build/unpack AlderLakeGPU.kext first." >&2
  exit 4
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP="$OC/config.plist.before-AlderLakeGPU-$STAMP"

cp "$CONFIG" "$BACKUP"
rm -rf "$OC/Kexts/AlderLakeGPU.kext"
cp -r "$KEXT_SRC" "$OC/Kexts/AlderLakeGPU.kext"

python3 - "$CONFIG" <<'PY'
import plistlib, sys

path = sys.argv[1]
with open(path, "rb") as f:
    c = plistlib.load(f)

entry = {
    "Arch": "x86_64",
    "BundlePath": "AlderLakeGPU.kext",
    "Comment": "Alder Lake-P GT2 8086:46A6 bare-metal PCI diagnostics (Stage A)",
    "Enabled": True,
    "ExecutablePath": "Contents/MacOS/AlderLakeGPU",
    "MaxKernel": "24.99.99",
    "MinKernel": "24.0.0",
    "PlistPath": "Contents/Info.plist",
}

adds = c.setdefault("Kernel", {}).setdefault("Add", [])
adds = [x for x in adds if x.get("BundlePath") != "AlderLakeGPU.kext"]
idx = next((i for i, x in enumerate(adds)
            if x.get("BundlePath") == "WhateverGreen.kext"), len(adds))
adds.insert(idx, entry)
c["Kernel"]["Add"] = adds

with open(path, "wb") as f:
    plistlib.dump(c, f, fmt=plistlib.FMT_XML, sort_keys=False)
PY

sync
echo "Installed: $OC/Kexts/AlderLakeGPU.kext"
echo "Backup:    $BACKUP"
