# RFC 0026 progress: Box3D Beyond IVP, with IVP as the Fallback

Roadmap row: R98 (`active`). Design: [RFC 0026](0026-box3d-beyond-ivp.md).

## Scope and owner (2026-10-07)

User direction: extend Box3D past IVP while keeping IVP as the fallback,
starting with the Box3D features IVP lacks; write the RFC, update the docs,
give agents a done checklist and ratchets, and publish test maps that set up
physics scenarios for the user.

Delivered in this change:

- RFC 0026 with the fallback guarantee, the scoreboard, phases B0–B9, the
  done checklist and six ratchets.
- Doc updates: AGENTS.md (RFC table, row R98 at rank 74, R44 narrowed to keep
  IVP selectable, R67 pointer, tracking note), RFC 0013 (program pointer,
  catalog rows for capsule shapes, surface motion and contact events, phase
  mapping), RFC 0004 (IVP kept as the fallback; Phase F amended) and RFC 0004
  progress (corrected provider defaults; F3 paused).
- Scenario maps: `tools/quality/physics_lab_maps.py`, its wiring test
  `tools/quality/tests/test_physics_lab_maps.py` (in CI,
  `conformance.yml`), and an optional `spawn_angles` argument on
  `gyro_lab_map.room` (default unchanged).

Not delivered: every phase gate. B0 (parity verdict, ratchet tool,
scoreboard section, `physics_lab.py` runner) and B1 are next.

## Scenario maps installed (2026-10-07)

`python3 tools/quality/physics_lab_maps.py` compiles the five maps with the
pinned legacy vbsp/vvis/vrad (`build/toolchains/pbrt-map-toolchain.json`,
full lighting) and publishes them to `run/maps/<map>`. Every compile passed
leak-free. Play them with `./play <map>` (Box3D) and
`PHYSICS=vphysics ./play <map>` (IVP).

Each scene:

- spawns once at map load and respawns from a button in front of it (a
  `point_template` over its bodies, killed and spawned afresh);
- shows what it tests and what to look for when the player walks up to it
  (`game_text` lines from a `trigger_multiple`);
- where it counts something, prints one `physlab ...` console line per
  counted body: a filtered physics trigger renames each body once counted, so
  nothing is counted twice.

Wiring test (`python3 -m unittest tools/quality/tests/test_physics_lab_maps.py`,
3 tests): every map is deterministic and brace-balanced; every output target,
template, thruster, constraint and filter resolves (trailing-`*` wildcards
must match an entity); every scene is started at load and has a button;
every counted name is killed by its scene's restart; only declared models are
used; the cylinder generator's faces point outward. Three seeded faults (a
wrong thruster target, a wrong template name, an undeclared model) are each
detected.

### Headless results (Linux desktop, current `build/`)

Command per map and provider (`fps_max` 60, about 15 s of game time, one
screenshot at the spawn view):

```sh
python3 tools/quality/portal_boot.py --runtime run/runtime --build build \
  --content-root quality-results/physics-lab-maps/<map>/content --map <map> \
  --physics <vphysics_box3d|vphysics> --headless --capture-wait 900 --out <dir>
grep physlab <dir>/runtime/portal/console.log | sort | uniq -c
```

| Map | Box3D | IVP |
| --- | --- | --- |
| `phys_tunnel` | pass; lane C 8 of 8 through, lanes A and B 0 | pass; lane C 4 of 8 through, lanes A and B 0 |
| `phys_stack` | pass; tower 0 of 16 fell, pyramid 0 of 55 | pass; tower 16 of 16 fell, pyramid 0 of 55 |
| `phys_joints` | pass; chain links joined, weight at its rest height (screenshot) | pass; chain links pulled apart and twisted (screenshot) |
| `phys_impacts` | pass | pass |
| `phys_rolling` | pass | pass |

- `phys_tunnel` reproduces the benchmark's tunneling result in game (RFC
  0013: `panes-64` IVP 24/64, Box3D 56/64). It is B1's in-game oracle.
- `phys_stack`'s tower reproduces the benchmark's stack result in game (RFC
  0013: `stack-20` IVP 38 collapsed, Box3D 0). In the bin, Box3D stacked the
  64 cubes in columns while IVP's heap scattered (spawn-view screenshots).
- `phys_joints`' chain (12 links, a weight of 50 link masses) shows the
  benchmark's joint-error result (RFC 0013: `ragdolls-128` IVP 57 units,
  Box3D 0.08) in game; B5 adds the stretch readout.
- The 10-wide pyramid stands on both providers; the benchmark's collapse is
  on its 20-wide pyramid, which this map does not reproduce.

### Findings

- **Stale staged binaries.** `portal_boot.py` without `--build` boots the
  binaries staged in `run/runtime`, whose `libvphysics_box3d.so` dates from
  2026-09-22. That provider predates the constraint-detach code
  (`CPhysicsEnvironmentBox3D::DetachObject`), and `phys_tunnel` segfaulted
  at level shutdown in `CPhysHinge::~CPhysHinge → CPhysConstraint::Deactivate`
  (a hinge reading its destroyed attached pane). gdb showed the field was
  never cleared, and disassembly showed a `DestroyObject` without the detach
  loop. With `--build build` the same map exits cleanly on both providers.
  This is a harness trap, not a current provider bug; the RFC and AGENTS.md
  tell agents to pass `--build`.
- **Double counting fixed before recording.** The first stack counter used
  four separate trigger entities around each footprint, so a cube landing on
  a corner counted twice (23 "fell" events for a 16-cube tower). One trigger
  entity with four brushes counts each body once (16 of 16).
- **Mass ratio fixed.** `massScale` multiplies a body's own mass, and the
  first weight had about 19 times a link's volume, so with `massScale` 50 the
  chain carried roughly 960 link masses instead of the labelled 50. The
  generator now derives the scale from the volumes (same material), and both
  providers were rerun.
- **Edge false positives fixed.** The fall strips started 0.15 units from the
  pyramid's base row, so an edge cube that only settled was counted; the
  strips now start 16 units out.
- `func_conveyor` moves players but not VPhysics props on either provider
  (`FL_CONVEYOR` is read only by player and step movement), which is what B3's
  surface velocity is for. The rolling map labels say so.

## Unverified and open

- Every phase gate (B0–B9), the ratchet tool, the scoreboard section and the
  map runner.
- The scenario maps have no Android or Apple run.
- `phys_joints`, `phys_impacts` and `phys_rolling` have no console readout
  yet; their phases add one.
- No user play-through of the maps on a desktop session yet; the evidence is
  headless runs and spawn-view screenshots.
