# Contract: `render.math-builders.v1`

Module: `render.math` (RFC 0016 layer 0)
Header: `public/render/math/matrix.h` (`LookBasis`, `PixelToClip`)
Suite: `unittests/rendertest/core/math/test_math_builders.cpp` (`render.math.builders`, headless)
Rows: R86 (RFC 0016 decision "cameras": convention-free builders for callers that own their
camera conventions, first the Hammer editor's viewports)

| Clause | Obligation |
| --- | --- |
| B1 | `LookBasis(eye, forward, right, up)` maps the eye to the view origin, forward to view -Z, right to +X and up to +Y; a basis derived as `LookAt` derives it gives `LookAt`'s matrix |
| B2 | `PixelToClip(w, h)` maps logical pixels (origin top-left, y down, the device's row order) to clip space: the top-left corner to (-1, +1), the bottom-right to (+1, -1), the centre to the origin; z and w pass through |
| B3 | Each clause's checks reject a seeded wrong builder (swapped basis rows; an unflipped y) |

The builders name no camera convention: yaw, pitch, Source's axes and the
2D view axes stay with their owners (`hammer.viewport`, `mapgeometry`).
