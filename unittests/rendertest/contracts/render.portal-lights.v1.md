# Contract: `render.portal-lights.v1`

Header: `public/render/portal_lights.h` (render.contracts; header-only, shared
by the engine, the client and the render core)
Engine consumer: `engine/portal_dlights.{h,cpp}`
Suites: `unittests/rendertest/test_portal_lights.cpp` (`render.portal-lights`,
headless, with two seeded rows) and `tools/quality/portal_dlight_lab.py check`
(`render.portal-lights.lab`, in game on the `portal_dlight_lab` test map, with a
seeded row; `render.portal-lights.lab.portal2` runs it in Portal 2 with
`--game portal2`, with a seeded row for P6)
Rows: R90 (RFC 0016 K7), with RFC 0011 G10's portal set

| Clause | Obligation |
| --- | --- |
| P1 | A light makes an image through an open portal when it is in front of the portal and its radius reaches the portal's rectangle (`EntersPortal`); the image is the light moved by the pair's transform (`ImagePoint`, `ImageVector`), with the same color, radius and falloff |
| P2 | A receiver on the linked side is lit by the image only where the path from the light passes through the entry portal's opening (`ReachesThroughPortal`, decided in the entry's frame); this agrees exactly with a reference built from both frames and decided in the exit's frame, away from the rectangle's edges |
| P3 | Images are not imaged again (one hop), are released every frame before new ones are made, never take a slot another light is using, and an image with no free slot is counted, not silently lost |
| P4 | Every consumer of dlight slots either clips images (world lightmaps per luxel, bumped and flat; models at their lighting origin) or leaves them out (displacements; the per-pixel light set of WMSH PBR), so light never leaks behind a wall |
| P5 | In game: the floor inside the through-portal light brightens; a floor point within the light's reach whose path misses the opening, and the wall around the exit portal, do not; nothing brightens through the pair with `r_portal_dlights 0` or with a portal closed. With the clip removed (`r_portal_dlights_seed_noclip`, a cheat) the out-of-cone point brightens and the check fails |
| P6 | A light flagged `DLIGHT_NO_PORTAL_IMAGE` (`public/dlight.h`) makes no image. A Portal 2 portal's own glow (`r_portal_use_dlights`) sets it: the glow stands for light leaving the portal's rim, so it lights its own side only. In game, with only the glows lit the engine reports no image through the two open portals; with the flag ignored (`r_portal_dlights_seed_glowimage`, a cheat) each glow is imaged and the check fails |
| P7 | Both games' clients publish their open portals (`VIndirectLightPortals001`, every frame before rendering): Portal 1 from `C_Prop_Portal`, Portal 2 from `C_Portal_Base2D`. P5 holds in both games |

Not claimed: images for elights (model-only lights), displacements and the
WMSH PBR per-pixel path (they leave images out), recursive portal paths, and
shadowing of the path through the portal by geometry on either side (dynamic
lights cast no shadows in the lightmap path).
