# Linux game HDR integration — 2026-10-03

Owner: RFC 0016 `render.output.v1`; presentation owns the negotiated format
and colour space, and VideoGraphicsSettingsService owns the settings transaction.

## Implementation

- Native game scene and managed scene targets use linear RGBA16F.
- HDR10 uses RGB10A2 Rec.2020/PQ; extended linear scRGB is the fallback HDR
  encoding. Unsupported presentation selects SDR with the same BT.2390 mapper.
- SDR screenshots are mapped through the core output pass before readback.
- Video > HDR shares Video's staged/applied state, backend commit and save
  retry. Back cancels only the HDR child draft; parent cancellation covers all
  video settings. Exact calibrated peaks remain selectable between presets.
- This machine's calibration is automatic HDR, 617 cd/m² peak and exposure 1.0.
  Reference white is 203 cd/m²; desktop scRGB presentation uses 80 cd/m² units.

## Evidence

Isolated product build: `build-hdr-game` (Portal 2, SDL3, native Vulkan, FSR).
Full product build and subsequent client/render builds passed.

`python3 tools/quality/conformance.py check --suite vgui.hdr_settings_menu
--suite vgui.graphics_settings --suite vgui.hdr_menu_layout
--suite vgui.hdr_menu_layout.overlap --suite render.output --config release`
passed: 42 settings-menu checks, 50 shared transaction checks, 25 production
resource-layout checks, 25 seeded overlapping-row checks (expected failure),
and 30 GPU output checks including HDR10, SDR mapping and scRGB scaling.
Evidence: `/tmp/source-hdr-work/verified.json`.

Native Wayland run negotiated Rec.2020/PQ 10-bit and RGBA16F scene storage.
An independent RenderDoc readback of a game frame found maximum RGB 4.68359375
and 146495 channel samples above 1.0, proving unclipped internal range.
The saved configuration preserved peak 617.

## Remaining acceptance

Native menu visual verification is in progress. Required full frame budgets,
all scene cohorts and Apple/device package acceptance remain unverified.
Wayland RenderDoc injection refused VK_KHR_wayland_surface on this host;
offscreen GPU captures work. These runs do not certify full platform parity.

Frozen-path: explicitly requested native game HDR integration and presentation
plumbing through the retained material API.
