# AlderLakeFB display path

The target laptop's Linux DRM topology shows that the Intel iGPU owns:

- built-in `eDP-1`,
- connected `HDMI-A-1`,
- another HDMI-capable connector entry.

Therefore the requested physical HDMI output must ultimately be handled by the Intel display engine.

Planned bring-up order:

1. read connector/port topology,
2. obtain EDID from HDMI,
3. enumerate a known-safe mode,
4. allocate a linear scanout buffer,
5. program a single display pipe/transcoder,
6. enable scanout,
7. add vblank/page-flip,
8. add hotplug,
9. add cursor,
10. add eDP/backlight later.

The framebuffer driver is separate from the 3D Vulkan path so HDMI can be debugged independently.
