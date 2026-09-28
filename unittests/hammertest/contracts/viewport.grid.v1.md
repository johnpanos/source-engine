# Contract: `viewport.grid.v1`

Module: `hammer.viewport`
Header: `public/hammer/viewport/grid.h` · Impl: `hammer/core/viewport/grid.cpp`
Conformance: `unittests/hammertest/viewport/test_grid.cpp`
Migration: R08-DOMAIN (supersedes `EditorController::Snap` and the GTK shell's
`Renderer::BuildGrid` once they are hooked to `hammer.viewport`)

The one snapping policy tools share and the grid lines a 2D view shows. RFC
0002 requires shared snapping to have one policy owner; this is it.

## 1. Purpose, consumers, scope

- `GridPolicy`: grid size, enabled flag, `Snap`, `SnapPoint`, finer/coarser
  steps.
- `VisibleLines( Camera2D, GridPolicy, GridDisplay )`: the classified lines a
  2D view draws; `DisplayedStep` exposes the spacing rule.
- Consumers: `hammer.tools` (every snapped gesture), presenters (the status
  bar's grid size), the GTK host (grid drawing). Required.
- Out of scope: drawing, colors, dotted-grid rendering, 3D grids.

## 2. Accepted inputs

- Sizes: powers of two in [1, 1024] (legacy range); anything else is rejected.
- Snap inputs: any double; non-finite values pass through unchanged.
- `AxisMask` selects the axes `SnapPoint` snaps.
- `GridDisplay`: `show`, `minSpacingPixels` (default 4; non-finite = no
  minimum), `majorEvery` (64), `blockEvery` (1024), `axisLines`,
  `maxLinesPerAxis` (512; 0 behaves as 1), `worldExtent` (16384; 0 = unbounded).

## 3. Results and guarantees

- `Snap` returns the nearest multiple of the size, ties away from zero (legacy
  `V_rint`, in double precision); disabled snapping is the identity.
- `StepFiner`/`StepCoarser` halve/double and clamp to [1, 1024], reporting
  whether the size changed.
- `VisibleLines` lists vertical lines (constant u) in increasing u, then
  horizontal lines (constant v) in increasing v, one per multiple of the
  displayed step inside the visible rectangle clipped to +/- `worldExtent`.
  The step starts at the grid size and doubles until lines are at least
  `minSpacingPixels` apart (legacy hide-small-grid at 4 px), then keeps
  doubling while either axis would exceed `maxLinesPerAxis`.
- Classification by world coordinate, first match wins: `Axis` at 0 (when
  `axisLines`), `Block` on multiples of `blockEvery` (legacy highlight 1024),
  `Major` on multiples of `majorEvery` (legacy highlight 64), else `Minor`.
- Each line carries its world coordinate and exact screen position.
- Display is independent of snapping (the legacy toggles are separate).
- Nothing is returned when `show` is false or the camera has no viewport.

## 4. Ownership, threading

- `GridPolicy` is a value owned by the editing session; `VisibleLines` is a
  pure function returning an owned vector. No shared state.

## 5. Invariants

- Size is always a power of two in [1, 1024].
- At most `max(maxLinesPerAxis, 1)` lines per orientation; lines are at least
  `minSpacingPixels` apart unless the cap forced a coarser step (which only
  widens spacing).

## 6. Side effects and performance

- `VisibleLines` allocates one vector of at most 2 x cap lines; the step
  search is bounded (zoom >= 1/64 and a 1e18 step ceiling).

## 7. Conformance suite and providers

- `test_grid.cpp` (46 checks): size validation and clamps, tie rounding,
  per-axis masks, disabled identity; line counts, spacing, positions and
  classification at 1 px/unit and zoomed out; `blockEvery`/`axisLines`
  options; display independent of snapping; hidden display and empty viewport;
  world-extent clipping; the per-axis cap on a two-million-pixel viewport;
  bounded minimum-zoom output.
- Negative control: a seeded `minSpacingPixels = 0` display fails the spacing
  oracle.
- Runs headlessly with g++ and clang++ (`-std=c++20 -Wall -Wextra -Werror`).
