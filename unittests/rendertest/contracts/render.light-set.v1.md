# render.light-set.v1

`public/render/light_set.h` owns the runtime light set of RFC 0011: the
frame's lights as an immutable value snapshot. The engine builds it on the
main thread before render work (`engine/light_set_publisher.cpp`), and a
renderer that implements `ILightSetConsumer` borrows it for that frame. The
native Vulkan world path takes direct light for unbaked lights from it.

Obligations of a builder:

- **Stable identity.** A world light's ID is fixed by its map index for the
  whole map. A dynamic or entity light keeps its ID while it stays alive in the
  same slot with the same key. It gets a new ID when its slot is reused or it
  is absent for a frame. No ID is reused within a map, and IDs are unique
  within a snapshot. World and dynamic IDs never collide.
- **Style scalars.** A world light's color is scaled by its style's current
  scalar; a style the engine does not report is 1.
- **Baked state.** Every world light is `baked`. It `matchesBaked` exactly when
  its style scalar equals the baked value, 1 (±1e-4). Dynamic and entity
  lights are neither.
- **Epoch.** Each built frame's epoch is one more than the last; `BeginMap`
  restarts it and drops every dynamic identity.
- **Values.** A snapshot owns its data; later input changes do not alter it.

The suite runs a scripted frame sequence against these rules. It includes
slot reuse, a light going dark and returning, key changes, style changes and
a map change. The sensitivity builds replace the builder with one that
derives dynamic IDs from slot indices (reusing identities), one that ignores
style scalars, and one whose epoch does not advance. The oracle must reject
each.
