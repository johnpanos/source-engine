# Contract: `render.scene.v1` and `render.visibility.v1`

Module: `render.scene` (RFC 0016 layer 4)
Headers: `public/render/scene/`
Suites: `unittests/rendertest/core/scene/test_scene.cpp` (`render.scene.v1`),
`test_scene_publication.cpp` (`render.scene.publication`, its TSan lane and the
lane's seeded negative control)
Rows: R89 (RFC 0016 K5)

| Clause | Obligation |
| --- | --- |
| C1 | A commit applies all of its change set or none of it, and an invalid, unknown or duplicate id names the offending change |
| C2 | A published snapshot never changes; the next commit publishes a new revision |
| C3 | Frustum culling is conservative: on seeded random scenes no instance that a clip-space corner test proves visible is culled, and every instance wholly outside one clip plane is. Draws sort by material, mesh, then front to back |
| C4 | A visibility provider can only remove candidates |
| C5 | Scenes are independent, and a held snapshot outlives its scene |
| C6 | Pooled culling (`BuildDrawListPooled`, frustum chunks as `jobs.graph` jobs merged in index order) gives the serial draw list exactly, items, depths and counts, on 1,000 seeded scenes, views and chunk sizes, with and without a provider; the serial function is the oracle |
| P1–P4 | Publication: while the owner commits, readers on other threads only ever see complete snapshots, in revision order, and `Revision()` never runs ahead of the snapshot a reader can take next. The TSan lane runs this clean; a scene built with `RENDER_SCENE_SEEDED_UNSYNCHRONIZED_PUBLICATION` (no release/acquire edge) must fail it |

Engine-facing: the headers carry no dual-ABI library type, because the engine
builds with the legacy libstdc++ ABI and the core does not.
Open for K5: GPU-resident instance storage (snapshots still copy the table,
O(instances) per commit), the engine's BSP visibility provider and the legacy
culling comparison on the K0 views, world and prop drawing from the scene, two
scenes rendering in one process, and the submission-cost target.
