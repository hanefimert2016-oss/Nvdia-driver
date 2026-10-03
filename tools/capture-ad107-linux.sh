#!/usr/bin/env bash
set -euo pipefail

OUT="${1:-ad107-capture.txt}"

{
  echo "=== timestamp ==="
  date -Is
  echo
  echo "=== kernel ==="
  uname -a
  echo
  echo "=== PCI NVIDIA ==="
  lspci -nnk -d 10de:
  echo
  echo "=== BARs ==="
  for dev in /sys/bus/pci/devices/*; do
    [[ -r "$dev/vendor" && -r "$dev/device" ]] || continue
    [[ "$(cat "$dev/vendor")" == "0x10de" ]] || continue
    echo "--- $(basename "$dev") vendor=$(cat "$dev/vendor") device=$(cat "$dev/device") ---"
    cat "$dev/resource" || true
  done
  echo
  echo "=== nvidia-smi basic ==="
  nvidia-smi -q 2>&1 | grep -E "Product Name|VBIOS Version|GSP Firmware Version|BAR1 Memory Usage|Bus Id" || true
  echo
  echo "=== iommu groups ==="
  for dev in /sys/bus/pci/devices/*; do
    [[ -r "$dev/vendor" ]] || continue
    [[ "$(cat "$dev/vendor")" == "0x10de" ]] || continue
    printf "%s -> " "$(basename "$dev")"
    readlink -f "$dev/iommu_group" || true
  done
} | tee "$OUT"

echo "saved: $OUT"
