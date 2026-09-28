# RFC 0002 progress: Hammer responsibility factorization

Updated: 2026-09-26
Source revision at assessment: `2d7e01d5` (working tree; AGENTS.md portfolio row: R08).
Current-state review: `d6260d90` (2026-09-25); see
[Current state](#current-state-2026-09-25).

This file is the human-readable gate-decision record for RFC 0002. The machine
facts live in the versioned artifacts under `architecture/` and the conformance
tests under `unittests/hammertest/`; those artifacts are authoritative when they
disagree with prose here.

RFC 0002 is Proposed. Accepting it establishes the architecture and migration
protocol; it does not mark any delivery gate complete. This record opens Phase H0
and answers the open decisions the RFC deferred to measured H0/R1 results.

## Current state (2026-09-25)

Checked at `d6260d90` against the machine artifacts. The dated sections below
are history. Refreshed 2026-09-26 for the counts and states that the
map-building-loop slices changed (R08-CMD through R08-ASYNC-BUILD, below).

- AGENTS.md row R08 is `active`. No RFC 0002 delivery gate is complete.
  `tools/roadmap/roadmap.py show` reports R08 and R17 startable. R13, R22 and
  R23 are `partial`; R17, R24, R25, R33 and R43 are `planned`.
- **`archlint hammer --verify` passes again (2026-09-25).** From `7035c29e`
  (2026-09-22, the RFC 0007 R47 PBR schema) it rejected the `hammer.formats`
  → `render.contracts` edge, because the Hammer graph validator knew only
  Hammer modules. By user decision in the R01 re-audit, the validator now
  accepts a Hammer module's edge to a module registered in
  `capabilityModules`. It still rejects unknown targets and cycles, and it
  rejects a Hammer module that reuses a capability module's id (3 new tests
  in `tools/archlint/tests/test_hammer.py`). The inventory, ledger,
  compatibility and HAM003 checks pass.
- Strict sources exist for `hammer.scene`, `hammer.formats` and `hammer.app`
  (`hammer/core/`), plus the `hammer.ports` headers. `hammer.geometry` is
  gone: R08-LIBS moved it into the `mapgeometry` library, beside `kvtext` and
  `vmf`. `hammer.viewport`, `hammer.tools` and `hammer.presenters` are
  declared but empty; the interactive tools live in `hammer.app`
  (`EditorController`).
- Ledger: 28 migrations. 11 are `extracted`, 11 `characterized`, 2
  `substitutable`, 2 `isolated` and 2 `inventoried`; none is `cutover` or
  `retired`. Legacy Hammer is still the live authority for every legacy caller.
- Inventory: 46 authored records. `hammer --coverage` reports 38 of 531 files
  classified (2026-09-26), because 8 records were extracted into capability
  libraries and no longer count. The authored total is still 452.
- Conformance: 67 Q-EDITOR suites are registered on 2026-09-26 (63 headless,
  four corpus: loop, ui, mcp and glib-runner), all on `checks-v1`. There
  were 60 at `d6260d90`.
  The last recorded full run is the 2026-09-22 R02 evidence: g++ and clang++,
  default and release, and Wine PE 60/60
  ([RFC 0005 Q1](0005-progress.md#q1--r02-runner-shared-conformance-runner)).
  This review did not rerun them.
- 31 contract records exist under `unittests/hammertest/contracts/`.
  `unittests/hammertest/fixtures/` holds only its README; the sample maps are
  `hammer/gtk/samples/{room,wedge,displacement}.vmf`.
- Build: there is still no `hammer/wscript`, but `hammer/core/wscript` and
  `hammer/cli/wscript` build the strict core and the headless `hammer_cli`
  (R08-LOOP). The GTK shell is still built by `hammer/gtk/build.sh`.
- Work this record did not yet describe is summarized in
  [Additional work recorded 2026-09-25](#additional-work-recorded-2026-09-25).

## Map-building loop direction and R08-CMD (2026-09-25)

**User direction.** The new Hammer must always be able to build a real map.
Every slice keeps a fast author → save → compile → play loop. A
conformance suite checks it by driving the real UI the way a person does,
and the same commands run headlessly.

Behavior lives in shared headless domain and command libraries. The GTK UI is
a thin layer, and an MCP server can use the same commands. Later the same day
the user added more:

- format libraries are small, layered and enforced, with arrows pointing
  down;
- UI/UX takes inspiration from the Source 2 tools' ergonomics;
- the editor embeds the native renderer and a play/stop game preview.

All of these are recorded in AGENTS.md. The plan runs in slices:

1. **1a:** a command layer, VMF fixes and exact domain operations (below).
2. **1b:** layered per-format libraries, plus Waf targets for them and a
   headless `hammer_cli`.
3. **2:** an author → compile → boot loop suite with negative controls.
4. **3:** the GTK UI routed through the commands, with a UI-driven test in an
   isolated compositor and Source 2-informed ergonomics.
5. **4:** an MCP adapter.
6. **5:** play-in-editor through a dmabuf-export presentation pair.

A read-only survey found how far the loop is today:

- The headless `EditorController` can author and save a VMF.
- The GTK shell (`hammer/gtk`) can't place entities, set properties, or
  compile and run.
- There was no command layer.
- Saved VMFs gave every face the floor's texture axes and no `skyname`.
- No test went from input to a booted map.

### R08-CMD: shared command layer and exact authoring (slice 1a, done)

- **Command layer.** `public/hammer/app/editor_commands.h` and
  `hammer/core/app/editor_commands.cpp` add named, serializable commands
  over `EditorController`.
  - One table owns each command's name, required and optional arguments,
    summary and handler. `Catalog()` serves help text and tool listings from
    that same table.
  - Commands: `new_map`, `open`, `save` (through `SaveDocument`, atomic),
    `create_block`, `place_entity`, `set_entity_origin`,
    `set_entity_property`, `set_world_property`, `set_material`,
    `apply_material`, `delete_selection`, `set_grid`, `undo`, `redo` and
    `info`.
  - I/O goes through the `hammer.ports` file-store port.
  - Errors are structured `CommandError`s (status, command, detail, script
    line) in `foundation::Expected`.
  - Scripts have one command per line, `name key=value key="v w"`, with `#`
    comments.
  - `hammer.app` gains the `foundation` edge (HAM002).
- **Exact domain operations.** `CreateBlock`, `PlaceEntity` and
  `SetEntityOrigin` give each edit one undo unit.
  - The Block-tool commit and the Entity-tool click now call them, so
    gestures and commands share one authority.
  - Worldspawn properties (`SetWorldProperty`, `WorldProperties`) are
    undoable document state and round-trip through VMF.
  - A new map starts with `skyname sky_day01_01`.
- **Texture axes.** `hammer.geometry` gains a named owner,
  `WorldAlignedTextureAxes`, ported from legacy `CMapFace`'s `baseaxis`
  table with its tie order. The table matches vbsp's
  `TextureAxisFromPlane`. `ToVmf` writes each face's own axes.
- **Found and fixed.** Saving a reloaded map was not byte-identical: normals
  rebuilt from plane points printed `-0`. Plane and axis text now write
  zero as 0.
- **Found, not fixed:** brush entities are folded into world brushes on
  load, so saving turns a trigger into world geometry. This is R22 scope,
  owned by the persistence slice.
- **Oracles.**
  - `hammer.app.editor_commands` (54 checks):
    - a script builds a sealed room with a player start and a light, saves
      it, and reopens it byte-identically;
    - every error status is checked (unknown command, missing, invalid and
      unknown arguments, rejected edits, I/O failure, failed save without a
      partial file);
    - a failing script line stops the run and reports its line;
    - script syntax errors are caught.
  - `hammer.app.editor_controller` grows from 182 to 233 checks: exact
    operations and undo, world-property round-trip including byte-stable
    reload, per-face axes, and the axis policy with legacy tie order.
  - Ten mutants are each detected: the unknown-argument check, error line
    numbers, `MarkSaved`, the tie order, a swapped table row, world
    properties in snapshots, world-property loading, negative zero, and two
    parser paths.
- **Not claimed:**
  - no Waf target or CLI host yet (1b);
  - no compile or boot (2);
  - the GTK UI does not use the commands yet (3).

### R08-LIBS: layered format libraries, cohort 1 (slice 1b, done 2026-09-25)

User direction: format and domain code lives in small libraries that the
editor, compile tools and engine all compose, with enforced imports and
arrows pointing down. A library never depends on an application such as
Hammer. This cohort moves the geometry and VMF codec cores out of `hammer/`.

- **What moved.** 18 files, moved (not copied); the old paths no longer exist,
  and there are no forwarding headers or namespace aliases:
  - all of `hammer.geometry` (`aabb`, `angle`, `brush`, `displacement`,
    `rounding`, `texture_axes`) to `public/mapgeometry/` and `mapgeometry/`,
    namespace `mapgeometry`;
  - the keyvalues text codec (`keyvalues.h/.cpp`) to `public/kvtext/` and
    `kvtext/`, namespace `kvtext`;
  - the VMF decoders and the shared placement transform
    (`vmf_geometry`, `vmf_transform`) to `public/vmf/` and `vmf/`,
    namespace `vmf`.
- **Modules and edges** (`architecture/modules.json`, bottom to top):
  - `world.map-geometry`: no edges (standard library only);
  - `content.keyvalues-text`: no edges;
  - `content.vmf`: `content.keyvalues-text`, `world.map-geometry`.

  The `hammer.geometry` module is gone. Every Hammer edge to it now points
  to `world.map-geometry`. `hammer.formats` gains `content.keyvalues-text`
  and `content.vmf`, and so do the GTK adapter and composition;
  `hammer.adapters.mfc` gains `content.keyvalues-text`.
  `content.hammer-ktx2-preview` gains `content.keyvalues-text`, because its
  Waf target compiles the codec. `hammer.app` gains no codec edge. Its two
  R22 include exceptions are retargeted with the same removal condition
  (a `hammer.ports` persistence contract):
  - `editor_document.h` → `content.keyvalues-text`;
  - `editor_controller.cpp` → `content.vmf`.

  `cctype` joins the portable standard headers.
- **Upward dependencies.** None found: every moved file includes only the
  standard library and its own or allowed lower libraries. A seeded
  `hammer/app` and `kvtext` include in `mapgeometry/aabb.cpp` is rejected
  (CAP002).
- **Waf targets.** The static libraries `mapgeometry`, `kvtext` and `vmf`
  (`vmf` uses the other two) are built:
  - with the strict C++20 environment and `capability_strict`;
  - each declaring `arch_module`, with a cxx20 entry in
    `quality/toolchain/policy.json` and a `capabilityModules.targets` entry;
  - registered in the root wscript's `tests` and `tools` subproject lists
    beside `mapcontainer`.

  No product links them yet. Build scripts outside Waf compile the moved
  sources by their new paths: the GTK shell, `camera_nav_test.sh`, the
  adapter shell scripts, `tools/portal_pbr/workflow.py`, the
  texture-container test wscript and the conformance manifest.
- **Records.** The inventory, migration ledger and parity records name the
  new paths and owners, and migration IDs and history are unchanged:
  - `HAM-GEOMETRY-001` and `HAM-DISP-001` now point to
    `world.map-geometry`;
  - the four affected migrations carry a `relocation` note;
  - the moved files' inventory records keep their provenance.

  `archlint hammer` now accepts registered capability modules as inventory
  owners and migration destinations, in the same way it already accepted
  them as module edges. Coverage reports extracted records separately
  instead of warning about them. Three new `test_hammer.py` cases cover this.
  Suite IDs (`hammer.geometry.*`, `hammer.formats.keyvalues*`) are stable
  and unchanged.
- **Evidence** (this tree, 2026-09-25):
  - `conformance.py check --domain Q-EDITOR`: 61 suites and 1,675 checks
    pass with g++ and with clang++;
  - `parity_wine.py check --rfc 0002 --domain Q-EDITOR` with MinGW
    `x86_64-w64-mingw32-g++` under Wine: 61/61 match;
  - `hammer_ktx2_preview_conformance`, Waf-built in
    `build-rfc0008-ktx-reader`: 10 checks, 0 failures;
  - `hammer/gtk/build.sh` builds; `camera_nav_test.sh` passes;
    `baseline.py audit --check hammer.gtk-viewport-smoke` passes;
  - archlint: `check --all` (also with `--compile-deps` on both test trees),
    `hammer --verify`, `hermetic --cxx g++ --cxx clang++` (63 portable
    headers), `inventory --verify` and `baseline --verify` all pass, as do
    the 131 archlint unit tests;
  - the Waf trees `build-r03-{tests,tests-clang,tools,tools-clang,tools-worldstage}`
    were reconfigured with their declared setup lines, because each new
    subproject needs an env cache, and they build the three libraries.
    `archlint targets --verify` over the 12 declared `build-r03-*` trees
    passes.
- **Left in `hammer.formats`, for later cohorts,** with their named callers:
  - the VPK/VTF/VMT asset readers: `vpk_archive`, `vtf_image` and
    `material` (callers: `hammer/gtk/app.cpp`, `offscreen.cpp`,
    `tools/portal_pbr/portal_assets.cpp`, `material_catalog`);
  - `material_catalog` and `search_path_assets` (the GTK shell,
    `hammer/adapters/source/ktx2_preview`);
  - the FGD parser `fgd` (`entity_property_sheet`);
  - VMF document features: `cordon` (`map_export`), `instancing`
    (`map_export`), `visgroups` (`map_export`), `groups`, `overlay` and
    `prefab` (tests only today);
  - `map_export` and `entity_property_sheet` (tests only today).

  The VPK/VTF/VMT readers and `fgd` are the next candidates for engine- and
  tool-shared libraries. The VMF document features may join `content.vmf`
  once a second consumer, such as the compile tools, needs them.
- **Not claimed:** no engine, compile-tool or editor product links the
  libraries yet. The legacy MFC `hammer/` tree and vbsp are unchanged.

### R08-LOOP: headless author → compile → play loop (slices 1b and 2, done 2026-09-25)

The new Hammer now builds a playable map end to end, and a conformance suite
keeps it that way.

- **Build targets.** Three new Waf targets are built in the `tools`
  configuration. Each declares its Hammer owner:
  - `hammer_app` (`hammer/core/wscript`): the whole `hammer.app` module;
  - `hammer_platform` (`hammer/adapters/platform/wscript`): the disk store
    adapters;
  - `hammer_cli` (`hammer/cli`), owned by `hammer.composition`: a composition
    root that runs command scripts over `EditorCommands` against a rooted file
    store (relative paths only, no `..`). `--commands` prints the catalog.

  They use the strict C++20 environment but not the `capability_strict`
  public-only include rule, because a composition root includes its
  adapters.
- **Hammer owners in the link check.** Waf `arch_module` and `archlint targets`
  now accept Hammer module owners.
  - One combined graph judges them: capability modules plus
    `hammerModules`.
  - Strict Hammer sources map to their module by directory. That rule has one
    owner, `capabilities.hammer_strict_module`, and archlint's HAM checks use
    it.
  - A Hammer module's recorded `includeExceptions` also grant the linked
    dependency. So `hammer.app` may link `vmf` and `kvtext` only through its
    R22 exceptions, and the exception list stays the single record.
  - This exposed `hammer_ktx2_preview_conformance`: it compiles
    `hammer.formats`, so it is a Hammer test composition. It is now owned by
    `hammer.composition`, which gains the `content.hammer-ktx2-preview` edge.
    Another session (iOS static composition) reported the build break before
    the fix.
- **Compile tool** (`tools/quality/vmf_map_build.py`). One owner of "VMF →
  published, bootable map", for the loop suite, `gyro_lab_map.py` (now a
  caller) and later the editor's Run Map and MCP.
  - It stages exactly the materials that side blocks name. Entity `material`
    keys are skipped: a physbox's `"2"` is not a texture.
  - It runs vbsp, then vvis `-fast`, then vrad as two passes, `-ldr` and
    `-hdr`.
  - A leak, in the log or as a pointfile, is a hard failure.
  - It packages the map, and can publish it (`./play`) and boot it headless
    (`portal_boot.py`: map active, player spawned, scene captured).
  - It has 12 self-tests with stub compilers, and 4 seeded mutants are
    detected.
- **Loop suite.** `tools/quality/hammer_loop.py` with
  `quality/workloads/hammer-loop-v1`: manifest `corpus.hammer.loop` (14
  checks, required when run) and baseline check `hammer.loop`.
  - `room` authors a sealed room with a light and a player start by command
    script, saves it, compiles it leak-free, and boots it.
  - `leak` (north wall missing) must stop at the leak gate.
  - `dark` (no light) must boot with a capture the lit room is at least 1.3x
    brighter than. Measured: 169.7 vs 93.1 mean.
  - Author to playable takes about 9 s. It passes through `conformance.py
    --runner corpus` and `baseline.py audit`.
- **Defects the loop found.** Each would have kept a new-Hammer map from
  compiling or looking right:
  1. **Every saved plane was inside-out for the compile tools.**
     - `ToVmf` wound points so that (p1-p0)×(p2-p0) was the outward normal,
       but vbsp's `PlaneFromPoints` uses (p0-p1)×(p2-p1). vbsp crashed with
       "no visible sides" on every brush.
     - Reading and writing agreed with each other, so round-trip tests could
       not see it. Re-saving `hammer/gtk/samples/room.vmf` turned a good map
       into one that would not compile.
     - Fixed in `PlaneToText`. `TestSavedPlanesUseCompilerWinding` applies
       vbsp's formula headlessly (248 checks), and the old winding fails 6 of
       them.
  2. **No map had HDR lighting.** Portal renders HDR, so without HDR lumps
     auto-exposure evened out the lighting, and the lit and unlit rooms
     rendered almost alike (96 vs 93). The pinned vrad cannot run `-both` in
     one process, so the tool now runs `-ldr` and `-hdr` passes. This
     applies to `gyro_lab_map.py`'s maps too.
  3. **The pinned compile tools fail when any directory on the path has an
     uppercase letter,** because the Source filesystem folds case on Linux.
     The baseline audit's timestamped results folder hit this. The tool now
     compiles in a lower-case temporary directory, copies the work back to
     `<out>/compile` (failures included), and lower-cases map names.
  4. **`hammer/gtk/samples/room.vmf` leaked:** both entities were inside the
     solid centre pillar, and the earlier smoke test never checked for leaks.
     The entities moved to open space, and the sample now compiles
     leak-free.
- **Evidence** (2026-09-25):
  - Q-EDITOR: 62 suites and 62 matched on g++ and clang++.
  - `corpus.hammer.loop` 14/14; `baseline.py audit --check hammer.loop`
    passes (17 s).
  - archlint: 132 tests; `check --all` and `hammer --verify` pass.
  - The GTK viewport smoke passes.
  - The CLI builds in `build-r03-tools`, with its build-time owner check.
- **Not claimed:**
  - The GTK UI does not route through the commands yet, and there is no
    UI-driven test (slice 3).
  - No MCP adapter (4) and no play-in-editor (5).
  - Brush entities are still folded into world geometry on load (R22).
  - The loop runs on native Vulkan desktop only.

### R08-UI-P1a: P1 domain commands (slice 3, part a, done 2026-09-26)

The headless side of the brief's P1 rows. GTK wiring and the UI-driven test
come next.

- **Controller.**
  - `SelectObjects(ids, Replace|Add|Toggle)` and `SelectNone()`. A
    selection is brushes or one entity, as clicks make it. An unknown id, or
    a result that breaks that rule, changes nothing. Selection is not an undo
    step.
  - `Hollow(id, thickness)`: an axis-aligned box brush becomes six
    non-overlapping walls inside its bounds, keeping its material. That is
    Source 2's "flip faces" on a block, made VMF-safe. It is one undo step,
    and the walls are selected.
  - `MoveSelectionBy` also moves a selected point entity.
- **Commands.**
  - `select ids="1,2" mode=replace|add|toggle`, `select_none`;
  - `move_selection delta="x y z"`;
  - `hollow id= thickness=`, which outputs the wall ids;
  - `describe id=`: brush bounds and material, or entity classname,
    origin and keyvalues.
- **Evidence.**
  - `hammer.app.editor_commands` grows from 54 to 77 checks, and the
    controller suite is 248. Both pass on g++ and clang++.
  - A hollowed block with a player start and a light, authored by
    `hammer_cli`, compiles leak-free through `vmf_map_build.py`.

### R08-UI-P1b: GTK shell on the command layer, step 1 (2026-09-26)

- `hammer/gtk/app.cpp`: `AppState` owns an `EditorCommands` over a
  `DiskFileStore`. Open and Save now run the `open`/`save` commands, so the
  shell no longer holds its own read/parse/save orchestration. A
  `RunCommand` helper shows a failure's detail in the status bar.
- Keys follow Source 2 and classic Hammer:
  - Shift+B is the Block tool, Shift+E the Entity tool and Shift+S the
    Selection tool. Plain letters are left to WASD camera movement; plain
    B/S used to switch tools.
  - **F** hollows the single selected block into a room through the `hollow`
    command, with walls one grid unit thick.
  - The tool help text names the keys.
- Evidence: `hammer/gtk/build.sh` builds, and `hammer.gtk-viewport-smoke`
  and `camera_nav_test.sh` pass.
- **Entity placement (step 2).**
  - The controller gains `Raycast`: the nearest brush, hit point, and the
    outward normal of the face entered (zero from inside a brush).
    `PickByRay` reuses it, and the pick suite (66) is unchanged.
  - `PlaceEntityOnSurface` places an entity one unit off the hit surface, as
    one undo step.
  - Commands `raycast` and `place_on_surface`; the command suite has 83
    checks.
  - In GTK, the palette's Entity Tool (Shift+E) is live, and a class
    dropdown offers `info_player_start` and `light`. In the Entity tool, a
    3D-view click runs `place_on_surface` with the camera ray: Source 2's
    "click the floor to place".
  - The tool buttons and dropdown carry accessible labels for AT-SPI.
- Evidence: the GTK shell builds, and `hammer.gtk-viewport-smoke` passes.
- **Build and Run (step 3).**
  - A new port, `hammer::ports::IMapBuilder` (`public/hammer/ports/map_builder.h`).
    `EditorCommands` takes it as an optional dependency.
  - `build_map path= [quality=fast|full] [publish=0|1]` saves atomically,
    then builds. Without a builder the command is rejected; a failed save is
    never built.
  - The adapter, `hammer::adapters::platform::ToolProcessMapBuilder`, runs
    `vmf_map_build.py` through the platform tool-process contract. It is that
    contract's first product consumer (R40), and the POSIX tool-process
    provider is now built in `platform_posix`.
  - `hammer_cli` wires it up with `--repo` and `--builds`. In GTK, **F9**
    builds and publishes (`./play <map>`), and **Shift+F9** also launches the
    game (Source 2's "load in engine after building").
  - New module edges: `hammer.adapters.platform` → `platform.contracts`, and
    `hammer.composition` → `platform.posix`.
  - `platform` is added to the `tools` subproject list.
- Evidence:
  - The command suite has 94 checks, with a fake builder: save-then-build,
    rejection before saving, failure detail, and no build after a failed
    save.
  - A `hammer_cli` script (block → `hollow` → `place_on_surface` ×2 →
    `build_map`) produces `bm_room.bsp`.
  - Q-EDITOR 62/62 on both compilers; `arch.check`, `arch.hammer` and the
    GTK smoke pass.
- Not yet:
  - F9 blocks the UI while it builds (seconds in fast mode); an async build
    with a streamed log is P2.
  - The UI-driven test is next: headless sway, vendored wlroots virtual
    pointer and keyboard protocol XML, and AT-SPI for widgets.

### R08-UI-TEST: UI-driven editor conformance (slice done 2026-09-26)

**Scope** (roadmap R08, `active`; RFC 0002 H0 "headless target" and the user's
direction for a conformance suite that drives the real UI):

- Run the real `hammer_gtk` in an isolated headless compositor, never the
  user's session.
- Drive it the way a person does: find widgets by accessible name, use the
  pointer and keyboard, and read what the editor shows.
- Author a room with a player start and a light, and build it with F9.
- The oracle judges the files the UI produced, not the UI's internal state:
  the saved VMF and the build record.
- Negative controls: the same run with the F step skipped, and with the light
  left out, must each fail their own check.
- Out of scope: the game boot (the loop suite owns it), async build, gizmos,
  and 3D-view placement (the camera starts outside the room).

**Delivered.**

- `tools/quality/hammer_ui_test.py` builds `hammer_gtk` from source into its
  output directory. Each case runs in its own session: `dbus-run-session --
  mutter --headless --virtual-monitor 1280x800 --wayland-display <unique>`.
- Inside the session it starts the AT-SPI bus launcher and registry
  explicitly (D-Bus activation of both is denied here). It then runs the
  editor with `GDK_BACKEND=x11 GTK_CSD=1 --maximized`, so window
  coordinates are screen coordinates.
- Widgets are found by accessible name over AT-SPI: the tool buttons, the
  entity class dropdown (a `combo box`) and the four views. The views are
  `GtkGLArea`s created with the `application` role and labelled with their
  view names.
- **Input goes through the compositor,** via an `org.gnome.Mutter.RemoteDesktop`
  session on the private session bus. Xwayland drops AT-SPI's XTest events.
  - Pointer moves are relative: home against the top-left corner, then move
    by the offset (there is no absolute stream without a screencast).
  - Keys are evdev keycodes; keysyms are not delivered.
  - The first key from the new virtual keyboard carries the keymap and is
    lost, so the driver sends a lone Shift first.
- **Positions come from the status bar.** The driver hovers two points in a
  view, reads the coordinates the editor shows, and derives the
  pixel-to-world map. It checks that map at a third point.
- **The steps:**
  1. `[` twice (grid 64 → 16, confirmed on the status bar) and the Block Tool
     button;
  2. drag a 384x384 block in the top view, then Return (the brush count
     reads 1);
  3. F (the count reads 6);
  4. the Entity Tool button, then a player start clicked into the front
     view;
  5. the dropdown opened and stepped to "light" (Down, Return), and a light
     clicked in;
  6. F9 (save and compile; the help line reports the result).
- **The oracle** reads the saved VMF: exactly six world solids, and exactly
  one `info_player_start` and one `light`, each inside the room's interior.
  It also reads the build record: compiled, and `leaked` false.
- **Controls:**
  - `no-hollow` must fail `walls`. It also leaks, since its entities sit
    inside solid.
  - `no-light` must fail `light`.
  - Both must still be driven to the end.
- **Oracle self-tests:** 7 cases in `tools/quality/tests/test_hammer_ui_test.py`,
  including an entity outside the room, two lights, brush-entity solids,
  and missing files. Five seeded oracle mutants are all detected.
- **Editor changes** (`hammer/gtk/app.cpp`):
  - viewport accessible role and names;
  - the `--maximized`, `--builds DIR` and `--no-publish` flags (a test build
    does not publish into `./play`);
  - classic `[`/`]` grid keys through `set_grid`. The shell had no grid
    control, and hollowing needs walls thinner than the default 64-unit
    grid.
- **Registered:** manifest `corpus.hammer.ui` (required, `min_checks` 12)
  and baseline check `hammer.ui` (runtime, serial, budget 60 s), with a new
  `at-spi` tool probe.

**Evidence (2026-09-26, this host).**

| Check | Result |
| --- | --- |
| `conformance.py check --runner corpus --suite corpus.hammer.ui` | pass, 12 checks, 52.9 s |
| Direct repeats with a prebuilt shell | 3 of 3 pass (11 checks each) |
| Room case, UI start to built map | 11.2 s |
| `baseline.py audit --check hammer.ui --check hammer.loop --check arch.check --check arch.hammer --strict` | 4 pass, 0 deviations |
| Q-EDITOR headless suites | 61 of 61 |
| `corpus.hammer.loop` | pass |
| `test_hammer_ui_test.py`, `test_vmf_map_build.py` | pass |
| `archlint check --all`, `archlint hammer --verify` | pass |

`hammer.loop` took 60 s against its 40 s budget in that audit. It ran
beside other checks on a loaded host; the budget is not strict.

**Unverified.**

- No hosted CI lane runs this suite; it needs mutter, AT-SPI and a GPU-less
  GL context.
- Only the X11 backend is driven: the Wayland backend has no global
  coordinates for placement.
- Placement in the 3D view is not tested.
- Hammer's own GTK style debt outside the edited lines is not addressed.

### R08-MCP: an MCP server over the command layer (slice done 2026-09-26)

**Scope** (roadmap R08, `active`; the user's direction: "I can imagine an
MCP on top as well"):

- A `hammer.adapters.mcp` adapter maps the `EditorCommands` catalog to Model
  Context Protocol tools, over newline-delimited JSON-RPC 2.0 (MCP
  2025-06-18). It handles `initialize`, `notifications/initialized`, `ping`,
  `tools/list` and `tools/call`. It is a protocol handler with no I/O: one
  line in, at most one line out.
- There is one authority: each tool call is one `EditorCommands::Execute`, so
  an agent edits the same document, with the same undo history and errors,
  as the GTK host and scripts.
- Command failures are tool results with `isError`. Protocol faults are
  JSON-RPC errors: -32700 parse, -32600 invalid request (including batches),
  -32601 unknown method, and -32602 unknown tool or a bad argument type.
- `hammer_cli --mcp [--root DIR]` is the composition root, on stdin/stdout,
  with the same rooted file store and map builder.
- Oracles:
  - a headless C++ suite, `hammer.adapters.mcp`, over the handler, covering
    every error class, string escapes, and a room authored through tool
    calls, then saved and read back;
  - a Python end-to-end check that parses every line with an independent
    JSON reader, through the real `hammer_cli --mcp` process;
  - negative controls: malformed, truncated and deeply nested input, and
    unknown tools must each produce their own error without state change.
- Out of scope: a live-editor MCP endpoint (the GTK host serving a socket),
  resources and prompts, and streaming build logs.

**Delivered.**

- **`hammer/adapters/mcp`** (Waf `hammer_mcp`, owner `hammer.adapters.mcp`,
  strict C++20). Its only edges are `hammer.app`, `hammer.ports` and
  `foundation`; the composition may use it.
  - `McpServer::HandleLine` maps each catalog command to one tool. Every
    argument is declared as a string. Numbers, booleans (1/0) and arrays of
    scalars (vectors) are also accepted, so `"mins": [-64, -64, 0]` works.
  - A tool call runs one `EditorCommands::Execute`. Tool results read `ok`,
    the command's output, or `<command>: <status>: <detail>` with `isError`.
  - `tools/*` before `initialize` is refused. Notifications, and client
    replies to server requests, get no response.
  - The server offers the client's protocol version when it is 2025-06-18,
    2025-03-26 or 2024-11-05, and 2025-06-18 otherwise.
- **`json_value.{h,cpp}`**, private to the adapter:
  - numbers keep their source text, so ids echo verbatim, and objects keep
    member order;
  - it rejects duplicate members, trailing text, invalid escapes, unpaired
    surrogates and more than 64 nested containers;
  - the writer escapes control characters, so a reply is always one line.
- **One status-name owner.** `hammer::app::CommandStatusName` now serves
  both `hammer_cli`'s messages and MCP tool errors; it was a private helper
  in the CLI.
- **`hammer_cli --mcp [--root DIR] [--repo DIR] [--builds DIR]`** serves MCP
  on stdin/stdout with the same rooted file store and map builder. Stdout
  holds only replies. A client entry looks like
  `{"command": "build-r03-tools/hammer/cli/hammer_cli", "args": ["--mcp",
  "--root", "<map source dir>"]}`. No client configuration is checked in.

**Oracles.**

- **`hammer.adapters.mcp`** (headless, 90 checks, g++ and clang++):
  - the JSON reader and writer, 13 malformed inputs, and the depth bound at
    64, 65 and 100,000;
  - every JSON-RPC error class, with number and string ids echoed verbatim;
  - version negotiation, and a catalog-to-schema match;
  - a room authored through tool calls that saves byte-identically to the
    same room authored by command script;
  - four faults (malformed, bad argument, degenerate block, unknown tool)
    that leave `info` unchanged, and undo through the shared history.
- **`corpus.hammer.mcp`** / baseline **`hammer.mcp`** (21 checks, 0.2 s):
  - it drives the real `hammer_cli --mcp` process and parses every reply
    line with Python's `json`;
  - it authors the UI suite's room and `build_map`s it, and
    `hammer_ui_test.judge` judges the VMF and the leak-free build (one oracle
    for "the authored room" across UI and MCP);
  - the same controls run on the live stream.
- **Self-tests** (`tools/quality/tests/test_hammer_mcp_check.py`): a fake
  server with a stdout banner, and one with an unsolicited trailing line,
  must each fail the check.
- **Seeded faults:** 13 of 14 adapter mutants are detected. They covered
  version echo, unknown tool, null and empty-vector arguments, the
  initialize gate, the `ok` text, a response to a notification, surrogate
  decoding, control escapes, duplicates, trailing text, the depth
  off-by-one, and verbatim numbers. The survivor removes the top-level
  object check. That mutant is equivalent: a batch then fails as a request
  without a method, with the same -32600.
- **Found and fixed:** the reader first allowed 65 nested containers.

**Evidence (2026-09-26).**

| Check | Result |
| --- | --- |
| Q-EDITOR headless suites, g++ and clang++ | 62 of 62 each |
| `corpus.hammer.mcp`, `corpus.hammer.loop` | pass (21 and 14 checks) |
| `baseline.py audit --check hammer.mcp --check hammer.loop --check arch.check --check arch.hammer --strict` | 4 pass, 0 deviations |
| `archlint check --all --compile-deps build-r03-tools` | pass (27 strict units) |
| `archlint targets --verify --partial build-r03-tools` | pass (13 targets judged) |
| `archlint hammer --verify` | pass |
| stylelint on the changed files | clean |
| Python self-tests (MCP check, UI oracle, `vmf_map_build`) | pass |

`hammer.loop` ran over its 40 s advisory budget (53 s). Each room boot took
26 s at load average 9 from other sessions; compile and authoring are
unchanged.

**Unverified.**

- No MCP client product (such as Claude Code) has been run against the
  server.
- The live GTK editor does not yet serve MCP. An agent edits a headless
  document, not the one on screen.

### R08-ASYNC-BUILD: F9 builds off the UI thread (slice done 2026-09-26)

**Scope** (roadmap R08, and the first product consumer of R10's task
runners). Today F9 compiles on the GTK main thread, so the editor freezes
for the whole compile.

- **`hammer::app::MapBuildQueue`** (`hammer.app`) owns "one build at a time,
  off the editing thread". It borrows an `IMapBuilder`, a work runner and a
  reply runner (`platform.task-runner.v1`). The builder's result is posted
  back to the reply runner. Starting a build while one runs is refused.
  Destroying the queue with a build in flight is safe: the reply is
  dropped.
- **Saving stays on the editing thread** through the `save` command. Only
  the compile moves.
- **`hammer::gtk::GlibTaskRunner`** is an `ISingleThreadTaskRunner` on a GLib
  main context (idle and timeout sources), so replies land on the GTK
  thread. It runs the same shared `platform.task_runner` suite.
- **The GTK shell:** F9 saves, starts the queue on a `ThreadTaskRunner`, and
  reports "Building ..." at once, then "Built" or the failure when the
  reply arrives. Shift+F9 runs `./play` after a successful build.
- **Oracles:**
  - `hammer.app.map_build_queue` (headless, `ManualTaskRunner` on virtual
    time): runner identity, refusal while busy, the reply order, and
    destruction in flight;
  - the GLib runner through the shared runner suite (a corpus command
    suite, since it compiles with `pkg-config`);
  - `corpus.hammer.ui` must still pass with the asynchronous F9.
- **Out of scope:** cancelling a running compile, a streamed build log.

**Delivered.**

- **`public/hammer/app/map_build_queue.h` and `hammer/core/app/map_build_queue.cpp`**
  (Waf `hammer_app`; `hammer.app` gains the `platform.contracts` edge).
  - State is shared with in-flight tasks, so destroying the queue drops the
    reply.
  - `Start` from outside the reply sequence aborts.
- **`hammer/gtk/glib_task_runner.{h,cpp}`**. Its owner thread is cleared at
  shutdown.
- **The GTK shell.** `AppState` declares `uiRunner`, `buildThread` and
  `builds` in teardown order: the queue goes, then a running compile
  finishes, then its reply is dropped, then the builder goes.
  - F9 saves through `save`, starts the queue, and shows "Building <map>
    ..." at once.
  - The reply shows "Built <map>" or `build_map: <status>: <detail>`.
    Shift+F9 then runs `./play`.
  - `build.sh` compiles the queue, the GLib runner and
    `ThreadTaskRunner`.
- **The runner contract** (`platform.task-runner.v1`, clause 5) is refined:
  - The GLib runner exposed an ambiguity. A main-loop runner's thread is
    the owner's own thread, so a single-thread runner belongs to its thread
    inside and outside tasks.
  - The shared suite now asks each driver whether it is the runner's
    thread. The negative provider that claims every thread is still
    caught.

**Evidence (2026-09-26).**

| Check | Result |
| --- | --- |
| `hammer.app.map_build_queue`, g++ and clang++ | 15 checks, pass. Start does not build inline, one build at a time, build on the work runner, reply on the reply runner, destroyed in flight, work runner shut down, and with real threads Start returns in under 100 ms while a 200 ms build runs |
| `corpus.hammer.glib-runner` | 20 checks, pass: the shared suite on a private GLib context, and a post from another thread runs on the owner |
| `corpus.hammer.ui` / `hammer.ui` with the asynchronous F9 | pass (12 checks) |
| `hammer.gtk-viewport-smoke`, `hammer.mcp` | pass |
| Q-EDITOR + Q-FOUNDATION + Q-JOBS headless, g++ and clang++ | 122 of 128 each; the 6 skips are the optional TSan lanes |
| `baseline.py audit` of 7 related checks | 0 deviations; `arch.check`, the viewport smoke and `quality.selftest` ran over their advisory budgets on a loaded host |
| `archlint check --all`, `hammer --verify`, `hermetic` | pass |
| stylelint on the slice's files and changed lines | clean |

**Unverified.**

- Quitting during a full-quality compile waits for it to finish, since
  there is no cancellation.
- No UI check observes the editor answering while it builds: the room
  compiles in about 0.1 s, too fast to see. The queue suite covers the
  non-blocking start.

### R08-DOMAIN: headless domain logic for the editor (slice done 2026-09-28)

**Scope** (roadmap R08, feeding R13, R22, R23 and R24; user goal
2026-09-28: "implement all the headless domain logic for hammer, ready to
be hooked up to the UI", and "build this in a modular and unit testable
way"). `EditorController` was a feasibility model: brush entities
flattened into world brushes, texture axes regenerated on save,
connections, groups and visgroups dropped, and one class holding the
tools. This slice builds the domain layers RFC 0002 declares, so the GTK
shell only turns input into calls and draws what presenters produce.

**Layers** (each a strict module; arrows point down):

| Module | Owns (this slice) |
| --- | --- |
| `world.map-geometry` | `vec3.h` value arithmetic; `transform.h` (Source angles, affine maps, plane transforms; `vmf::AngleMatrix` now routes here); `polytope.h` (plane sides, closed solids, convex hulls, overlap, the integer snap policy); `BrushFace::sourcePlane` |
| `hammer.scene` | `MapDocument`: full-fidelity content (solids, sides with authored plane points and texture axes, displacements verbatim, entities with ordered keys and connections, groups, visgroups, cordons, cameras, unknown blocks); `DocumentEdit`/`ChangeSet` (copy-on-write staging, lossless patches); `DocumentReader`; solid geometry; structural queries |
| `hammer.ports` | `IMapCodec`, `IEntityCatalog`, `IMaterialInfo` (plus the existing file store and map builder) |
| `hammer.formats` | the full-fidelity VMF codec; the FGD-backed entity catalog; a material-info adapter over `MaterialCatalog` |
| `hammer.app` | `EditSession` (one document, selection, labeled history of change sets, change events, draft guards); operation families as pure functions over `DocumentEdit`; clipboard; map check; the session command catalog |
| `hammer.viewport` | 2D/3D cameras and projection, the grid policy, picking (2D, 3D, marquee), render-snapshot extraction |
| `hammer.tools` | normalized input values, the tool contract and manager, and the tools (selection, block, entity, clip, vertex, texture, cordon) |
| `hammer.presenters` | inspector, face inspector, outliner, class palette, material browser, history, status bar, visgroups, problems and the action catalog |

**Decisions** (agent, under the user's standing "no questions;
recommended defaults" instruction):

- **Identity.** `ObjectId` is 64-bit: the document serial in the high
  half, a never-reused counter in the low half. Stale and foreign ids
  resolve to nothing, which is RFC 0002's required stale-reference check.
  VMF ids stay separate persistent fields (overlay `sides` keys name side
  ids).
- **Authority.** A side's three authored points are the plane authority
  (exact round trip, exact transforms); an entity's key list is the
  authority for origin and angles.
- **Transactions.** Operations stage changes in a `DocumentEdit` and never
  see the session. `Finish()` records per-object before/after values and
  drops no-op changes, so undo installs recorded values and never inverts
  geometry.
- **Ports over formats.** `hammer.app` may not depend on `hammer.formats`;
  it takes `IMapCodec` and `IEntityCatalog` from the composition root.
- **Numeric policy.** Scene geometry snaps vertices within 1e-4 of an
  integer (clipping 1e5 quads leaves ~1e-11 noise), so bounds and grid
  math are exact.
- **No escape hatches** (user requirement, 2026-09-28: "all state loads
  into actual classes or data structs and round trip"). The model has no
  verbatim block, no "extra" key list and no opaque node: displacements,
  version and view settings, and editor `logicalpos`/`comments` are typed.
  The VMF codec refuses what it cannot model, naming the block path and
  line. Entity and world key lists stay ordered key/value lists: they are
  the entity data model itself.
- **Migration.** `hammer_cli` (scripts and MCP) now runs on
  `SessionCommands` over `EditSession` with the strict VMF codec.
  `EditorController` stays the live authority of the GTK shell only, until
  the shell binds `presenters::EditorWorkspace`. Deletion condition: no
  caller outside its own suites.

**Delivered** (2026-09-28). Every module has a contract record under
`unittests/hammertest/contracts/` and its own suite in the conformance
manifest (migration `R08-DOMAIN`), with negative checks.

- **Geometry** (`world.map-geometry`): transforms, polytopes (incremental
  hull, ray entry, closed-solid test), `BrushFace::sourcePlane`.
- **Scene:** the typed document, staged edits and change sets with
  `ValidateEdit`, solid and displacement geometry, structural queries, the
  one visibility rule, and `DocumentIndex` for whole-map passes.
- **Ports and formats:** `IMapCodec` with the strict `VmfMapCodec`,
  `IEntityCatalog` with `FgdEntityCatalog` (the FGD parser now keeps
  helpers, inputs, outputs, flag defaults and `@KeyFrameClass`),
  `IMaterialInfo` with an adapter over `MaterialCatalog`.
- **Application:** `EditSession` (one authority for content, selection and
  labeled history; guards may commit drafts), `EditorSettings`, document
  I/O, `SessionCommands` (about 100 named commands) over the operation
  families: create (legacy stock primitives, arches, entities with catalog
  defaults), transform (texture lock, overlays, displacements), CSG (clip,
  carve, hollow), vertex and face editing, texture (justify/fit/align,
  smoothing groups), structure (delete cleanup, groups, brush entities,
  quick hide), entities (keys, class, flags, outputs, rename with
  references), displacements (create, power, sculpt, alpha, sew),
  clipboard and Paste Special, visgroups, cordons, the map check with
  fixes, prefabs, instance collapse, decals and overlays, and entity
  find/replace.
- **Viewport:** 2D/3D cameras, the grid policy, picking with one ordering
  policy, and render snapshots (displacement meshes included) with a
  revision-keyed incremental cache.
- **Tools:** normalized input, the tool contract and manager, the shared
  interaction policy, selection, block, entity, clip, vertex and face tools,
  and the camera controller.
- **Presenters:** entity and face inspectors, outliner, class palette,
  material browser, history, status bar, visgroups, problems, the action
  catalog, and `EditorWorkspace`, the one object a UI host binds.
- **Fixtures:** the Portal 2 maps `sp_a2_trust_fling` and `zoo_mechanics`
  are vendored in `unittests/hammertest/fixtures/portal2/` with
  `provenance.json`.

**Evidence (2026-09-28).**

| Check | Result |
| --- | --- |
| Q-EDITOR headless, g++ and clang++ (`-Wall -Wextra -Werror`) | 122 of 122 suites each; the 59 R08-DOMAIN suites hold 2,925 checks |
| `hammer.formats.vmf_portal2_roundtrip` | both Portal 2 maps decode with no warnings, reload SameContent with identical ids, re-save byte-identically, and equal the original file semantically; seeded changes are caught |
| `hammer.formats.vmf_map_codec.sensitivity` | 20 seeded bad codecs (including a lenient one that drops what the strict decoder rejects) each caught; 32 rejection cases with exact messages and lines |
| `hammer.app.portal2_workflow` (unoptimized) | sp_a2_trust_fling: open 0.17 s, move everything 0.18 s, undo 0.003 s, map check 0.06 s (4.6 s before `DocumentIndex`), save 0.08 s |
| `hammer.presenters.editor_workspace` | 49 checks driving the workspace like a UI: shortcuts, drags, undo keys, 3D placement, inspector drafts, save/reopen, host requests, camera input, focus loss |
| `corpus.hammer.loop`, `corpus.hammer.mcp` on `hammer_cli` over `SessionCommands` | pass (14 and 21 checks): the scripted room compiles leak-free and boots headless on native Vulkan |
| Waf `hammer_scene`, `hammer_ports`, `hammer_app`, `hammer_formats`, `hammer_viewport`, `hammer_tools`, `hammer_presenters`, `hammer_cli` | build (tools tree) |
| `archlint hammer --verify`, stylelint `--changed` | pass; `check --all` reports only two ARCH105 findings in `game/shared/fstop` (other work) |

**Not done.**

- The GTK shell still runs on `EditorController`; binding it to
  `EditorWorkspace` is the next slice (R24/R25 scope).
- Legacy blocks the strict codec rejects because the model has no field
  for them yet: `quickhide`, `autosave`, the `cordonsolid` editor key,
  pre-release `dispinfo` `uaxis`/`vaxis`, world-level `connections`.
- Vertex editing in 3D, shear handles, a cordon tool, the compiler's
  remapping of other position/angle keys in instances, and prefab
  libraries.


### Source 2 ergonomics brief (slice 3 design input, 2026-09-25)

A research agent assembled this from the Valve Developer Community Source 2
Hammer pages. The wiki blocks automated fetching, so every claim comes from
search excerpts of the cited page, and anything unconfirmed is marked
unverified. The principle: adopt Source 2's interaction model now, and its
free-form mesh data model only once USD owns persistence (R60).

The interaction model is:

- a 3D-first viewport;
- a Tool Properties panel;
- a searchable entity and asset palette;
- one-key Build+Run;
- repeatable command history.

**P1: click together a room with a light and player start, save, run.**

| Source 2 behavior | Command layer | GTK |
| --- | --- | --- |
| Block tool (Shift+B): drag on the 3D workplane, set height with a handle, Enter | `create_block`; the controller gains a workplane and 3D pending input | Cast the drag onto the workplane; draw the box and height handle |
| Block → room (F flips faces inward) | NEW `hollow id= thickness=`: six wall brushes, one undo step (VMF cannot hold inward faces) | F key / Tool Properties "Make room" |
| Entity tool (Shift+E): search box and categories (Player Start, Point Light), click the floor in 3D | `place_entity`; NEW `raycast origin= dir=` returning hit point, normal and id | Searchable class list; offset along the hit normal |
| Selection | NEW `select ids= mode=replace\|add\|toggle`, `select_none` (also for entities) | Clicks through `PointerDown`/`PickByRay` |
| Move | NEW `move_selection delta=` | Arrow-key nudge (T gizmo in P2) |
| Object Properties | `set_entity_property`; NEW `describe id=` | Property grid, one command per edit |
| Grid [ / ] | `set_grid` | Keys and status bar |
| Build Map F9, "load after build" | NEW `build_map profile=fast\|full run=`, through a `hammer.ports` compile/launch port over `vmf_map_build.py` | Build dialog with streamed log and cancel |
| Camera: RMB+WASD, Z fly | — | 3D view editable; one-pane / four-pane layout switch |

Tool keys follow Source 2: Shift+B, Shift+E and Shift+S; T translate; R
rotate; Space cycles selection modes; F9 builds; Ctrl+S saves.

**P2:**

- per-face material (right-click, Faces mode);
- `place_model` from the Asset Browser;
- T/R/scale gizmos with a pivot tool (Insert);
- face extrude (Shift+drag);
- clip (Shift+X);
- duplicate;
- a filterable Outliner;
- hide / hide unselected / unhide;
- Command History with Shift+G `repeat_last` and a script export (our commands are already serializable);
- undo-history jump;
- build stage toggles.

**P3:** these need the USD mesh document (R60) or later work:

- vertex/edge modes, bevel, bridge, face cut and the polygon tool;
- hotspot texturing (Ctrl+G), tile meshes and selection sets;
- prefab Collapse;
- play-in-editor and a ray-traced light preview.

Sources: Valve Developer Community pages cited in the research brief:

- Half-Life: Alyx *Creating Your First Room*;
- the Source 2 Hammer Overview;
- Dota 2 Workshop Tools pages: Navigation, Mesh Editing 1–3, Mesh Texturing, Asset Browser, Compile and Run, Command History, and Prefabs and Instances;
- Half-Life: Alyx *Hotspot Texturing* and *Creating an Addon*.

## Decisions (answers to the RFC's open questions)

These are recorded **Decisions** in the RFC's evidence vocabulary: intentional
targets with acceptance rules. They are revisable from H0/R1 measurements and are
not gate completions.

### D1 — Supported product profile (initial)

- **Primary portability target:** `linux-headless-core` (Linux x86_64, no display,
  no GPU). Geometry, scene, ports, formats, app, and viewport-query modules build
  and test here first. This matches the RFC's "Linux 64-bit headless core is the
  initial portability target."
- **Shell delivery target:** `linux-gtk-desktop` (GTK4 + libadwaita, X11 and
  Wayland), gated on H2–H4 and R1. Not promised for Windows/macOS in this program.
- **Rationale vs. goals:** AGENTS.md requires "a working, testable consumer at
  every new boundary" and portable code that "requests behavior, not OS/backend
  identity." A headless core is the only profile that can be tested without the
  unresolved renderer/host questions, so it anchors H1–H4.

Recorded in `architecture/hammer_compatibility.json` (profiles).

### D2 — Legacy build reproducibility (HAM-BUILD-001, honest H0 resolution)

- **Observed:** The historical Hammer editor is described by a Windows/MFC VPC
  project (`hammer/hammer_dll.vpc`) and depends on MFC (`afxwin.h`, `afxext.h`)
  through `hammer/stdafx.h`. There is **no `hammer/wscript`** and the root
  `wscript` does not define an editor target.
- **Decision (updated with Wine/Proton evidence):** Two distinct things were
  conflated in the original "not reproducible" claim, now separated:
  - **`windows-pe-core` — VERIFIED.** The strict, MFC-free core builds as native
    Windows PE (MinGW-w64 g++ 16.1.1, static) and all 19 conformance suites pass
    under **Wine 11 Staging and GE-Proton11-5**, matching the Linux gcc/clang runs.
    Harness `tools/quality/parity_wine.py`, profile
    `quality/profiles/windows-pe-wine.json`, evidence
    `architecture/hammer_windows_parity.json`. This is real Windows↔Linux parity
    for everything extracted so far.
  - **`windows-msvc-legacy-core` — VERIFIED.** The real MSVC v142 + MFC toolchain
    (cl 19.29.30159, toolset 14.29.30133 with `atlmfc`) is now provisioned via
    `mstorsjo/msvc-wine`. The **unmodified** legacy `hammer/boundbox.cpp` compiles
    with the real Microsoft compiler against the real `mathlib/vector.h`, links
    against the extracted `hammer::geometry::AxisAlignedBox`, and every observable
    operation is byte-identical — including the `SnapToGrid` `.5`-boundary that
    pins `V_rint`'s round-half-away-from-zero. A mutant re-introducing
    `std::rint` round-half-to-even is caught (exit 1), so the oracle is sound.
    This is stronger than the MinGW self-parity above: the extracted library
    reproduces the *actual legacy code compiled by the actual MSVC compiler*.
    Evidence `architecture/hammer_legacy_parity.json`; driver
    `scratchpad legacy-parity/run_legacy_parity.sh`.
  - **`windows-mfc-legacy` — still `unverified`.** With the toolchain provisioned,
    individual legacy translation units compile with the real MSVC+MFC compiler,
    but the full MFC *shell* (`afxwin.h`) still does not LINK here: it needs the
    whole Source Windows engine (tier0 / vgui2 / materialsystem / filesystem import
    libraries), which is out of scope for the extraction gate.
- **Consequence:** The strict extractions are verified on the Windows target on two
  toolchains (MinGW self-parity, and now real-MSVC-vs-legacy parity), not just
  Linux. Preserved legacy *runtime* behavior (the full MFC shell) remains gated on
  a full VS/MFC engine build.

### D3 — Renderer bridge approach (R1 / HAM-RENDER-001)

- **Decision:** The R1 spike evaluates **approach 1 first** — execute the adapted
  Source renderer into the `GtkGLArea`-provided context/target during the permitted
  render callback — because it avoids a cross-context resource-sharing dependency
  for the first viewport. **Approach 2** (offscreen render + shared/transferred
  image resources) is the fallback if state/lifetime handling in the shared context
  proves unacceptable. The choice is recorded with measurements when R1 runs; a
  wireframe substitute does **not** pass the Source-material fidelity gate.
- **Blocked on:** the H0 minimal render contract and the pinned GTK/Wayland profile.
- **2026-09-25:** the engine render seam is `done` (R15, `render.profile.v1`), and
  so are the presentation bridges (R16, `render.presentation.v1`). Neither has
  a GTK host pair. HAM-RENDER-001 is still `inventoried`, and the contract IDs
  it names (`render.baseline.v1`, `host.surface.v1`) exist nowhere else in the
  tree. The GTK shell's GL viewport is a substitute renderer, so it does not
  count toward this decision.

### D4 — Toolkit and language boundary (initial pins, finalized at H5/R03)

- **GTK/libadwaita minimums (provisional):** GTK ≥ 4.12, libadwaita ≥ 1.4. Pinned
  and tested exactly at H5; the RFC's toolkit references may describe newer APIs.
- **C++ dialect:** New strict editor modules (`public/hammer/**`, `hammer/core/**`)
  target C++20 per RFC 0006, with compiler/standard-library evidence established by
  R03/HAM-BUILD-001. Legacy MFC-facing headers keep their supported dialect; new
  language/library types must not cross an unchanged binary interface.
- **Content/fidelity/docking scope:** deferred to H5 with H0/R1 measurements, per
  the RFC. Recorded here when decided.

### D5 — Sibling UI shells stand on a shared Source-value seam (HAM-SOURCEADAPTER-001)

- **Decision:** The "thin MFC/GTK UI as siblings" structure is realized by giving
  the reusable strict libraries their own value types (`hammer::geometry::Vec3`
  etc.) and marshalling Source engine types across a single named boundary owner,
  `hammer.adapters.source` (`hammer/adapters/source/vector_interop.h`). Both thin
  UI siblings — `hammer.adapters.mfc` and `hammer.adapters.gtk` — route Source
  geometry through this one seam; neither open-codes `Vector`↔`Vec3` copies (DRY).
- **Why the seam is non-strict:** `mathlib/vector.h` transitively pulls in the full
  `platform.h`/`threadtools.h` stack, which `#error`s outside a real Source build.
  The strict libraries therefore must not touch it; the adapter is the one place
  allowed to, so it compiles only on profiles that provide the Source platform
  layer (the engine build, or the MSVC/Wine parity lane), never on
  `linux-headless-core`. This is the architectural reason the strict modules own
  their own `Vec3`.
- **Proof (the substitution guarantee):** Under the **real MSVC toolchain (cl 19.29
  under Wine)**, for the unmodified legacy `BoundBox`, running a reusable geometry
  operation directly equals `ToAABB → op → FromAABB` for `SnapToGrid(16)` (incl. the
  `.5` boundary), `SnapToGrid(8)`, and `Rotate90` on every axis, with exact
  `Vector`↔`Vec3` round-trips. A deliberately-broken seam (y/z swap) is caught by 9
  clauses. Contract `adapters.source.vector_interop.v1`; suites
  `unittests/hammertest/adapters/test_vector_interop{,_negative}.cpp`; evidence
  `architecture/hammer_legacy_parity.json` (adapters section); driver
  `scratchpad legacy-parity/run_adapter_parity.sh`.
- **Consequence:** This is the enabling dependency for advancing HAM-GEOMETRY-001
  from `characterized` toward `cutover`: a live MFC-side `BoundBox` caller can now
  delegate geometry to the reusable library through the seam and get byte-identical
  results. The adapter is permanent sibling infrastructure, not a retirement target.

### D6 — A real thin GTK sibling + a real file-store provider (HAM-GTKSHELL-001, HAM-DISKSTORE-001)

- **Decision:** Realize the "thin MFC/GTK UI as siblings" structure concretely by
  (a) giving the reusable app libraries their first **production** `IFileStore`
  provider, `hammer::adapters::platform::DiskFileStore` (atomic temp-write+rename),
  and (b) standing up an actual **thin GTK4/libadwaita shell**
  (`hammer/adapters/gtk/hammer_gtk_shell.cpp`) whose every action delegates to
  `hammer::app::EditorDocument` persisted through that store. The shell holds only
  toolkit glue and view state; no editor policy.
- **Verified here:**
  - `IFileStore` is now an enforced polymorphic contract (`ports.file_store.v1`):
    one shared suite runs against the real `DiskFileStore`, the in-memory fake, and
    a deliberately broken provider (caught) — green on gcc, clang, and MinGW PE
    under Wine.
  - The GTK sibling **compiles and links to a real ELF binary** against gtk4 4.22 +
    libadwaita 1.9 + the reusable libraries and nothing else
    (`unittests/hammertest/adapters/build_gtk_shell.sh`), proving it carries no
    hidden editor logic and no MFC dependency.
  - The shell's delegated workflow — New → Set Key → Save → reopen → Undo/Redo — is
    verified headlessly with the real disk store (`adapters.gtk_shell.v1`,
    `test_shell_workflow.cpp`), since a GUI needs a display (none here).
- **Honest gaps:** the GUI is not run here (no headless display); full H5 editor
  parity (multi-view map rendering, inspector, textures, the R1 renderer viewport)
  is not built; and the MFC sibling is not built (needs the Source Windows engine,
  D2). This is the sibling architecture proven end-to-end for **one** buildable
  shell, not a shipped editor. Profile `linux-gtk-desktop` is now `unverified`
  (was `planned`) to reflect the buildable-but-not-run status.

### D7 — The MFC sibling too (HAM-MFCSHELL-001) — the D2 legacy-shell caveat does not block a NEW thin MFC shell

- **Correction to earlier framing:** D2's "the full MFC shell needs the whole Source
  Windows engine" is true of the **legacy** Hammer shell (`CMapDoc`/`CMapView`), not
  of a **new** thin MFC sibling. A new sibling — parallel to the GTK one — depends
  only on MFC + the reusable, engine-free libraries, so it builds against the
  extracted libraries alone.
- **Verified here (local MSVC+MFC/Wine gate):**
  - `hammer/adapters/mfc/hammer_mfc_shell.cpp` (a `CWinApp`/`CFrameWnd` whose command
    handlers delegate to `EditorDocument` + `DiskFileStore`) **compiles and links to
    a real PE32+ GUI executable** with the real MSVC+MFC toolchain (cl 19.29,
    atlmfc, shared `mfc140.dll`) against MFC + the reusable library sources and
    nothing else.
  - `test_mfc_workflow.cpp` — a console **MFC-linked** binary — **runs under Wine**
    and exercises New → Set Key → Save → reopen → Undo/Redo through
    `EditorDocument` + the real `DiskFileStore` (exit 0), proving MFC and the
    reusable engine-free libraries coexist in one binary and the delegated workflow
    executes at runtime.
  - Contract `adapters.mfc_shell.v1`; evidence `architecture/hammer_mfc_parity.json`;
    repro `unittests/hammertest/adapters/build_mfc_shell.sh`; profile
    `windows-mfc-sibling` = `verified` (distinct from `windows-mfc-legacy`).
- **Both siblings now build on the same reusable libraries** — `adapters.gtk`
  (GTK4/libadwaita) and `adapters.mfc` (MFC) — persisting through the same
  `DiskFileStore` and serializing via the same `hammer::formats` codec, with only
  toolkit glue per sibling (DRY). That is the "thin MFC/GTK UI as siblings" shape
  realized for both shells.
- **Honest gaps (unchanged):** neither GUI is run here (no display); full H5 editor
  parity (map rendering, inspector, textures, R1 viewport) is not built; the
  **legacy** MFC shell (`windows-mfc-legacy`) is still unverified (needs the whole
  Source engine). This proves the factored architecture on both UI sides, not a
  shipped editor.

### D8 — A shared entity editor: both siblings drive one selection + property-editing authority (HAM-SEL-001)

- **Gap this closes:** D6/D7 proved the GTK and MFC shells are thin siblings over
  the same `EditorDocument`, but the only editing verb was `SetFirstBlockKey` — a
  placeholder that edits the first block. The RFC's multi-selection property model
  (`PropertyValue`, empty-vs-unset-vs-mixed) existed as an **orphaned oracle**: no
  method produced it from a live selection, so any real entity editing would have
  been re-derived independently in each shell. That is exactly the duplication the
  factoring goal forbids.
- **Decision / landed:** the shared **entity editor** now lives in `hammer.app` as
  the single authority both siblings route through:
  - `hammer::app::EntitySelection` (`public/hammer/app/entity_selection.h` +
    `hammer/core/app/entity_selection.cpp`): the ordered, deduplicated set of entity
    indices a shell owns as view state. No editor policy.
  - `EditorDocument::EntityCount/EntityName/EntityClassName` enumerate the document's
    top-level entities; `AggregateProperty(selection, key)` folds a property across
    the selection into `PropertyValue` under an explicit **presence-only** policy
    (a non-contributor never forces `Mixed`; a `""` value stays `Single("")`, not
    `Unset`); `SetPropertyOnSelection(selection, key, value)` applies a property to
    every selected entity as **one atomic history unit** — a single `Undo` reverts
    the whole group.
  - Contract `app.entity_editor.v1`; suites `hammer.app.entity_editor` (+`.sensitivity`,
    a non-atomic per-entity editor the oracle catches), green under gcc and clang on
    `linux-headless-core` via `run_headless.sh`.
- **The GTK sibling now drives it end to end.** `hammer/adapters/gtk/hammer_gtk_shell.cpp`
  gained a multi-select entity list; selection changes call `AggregateProperty` to
  show the value (with `<multiple values>` / `(unset)` states), and "Set Key" calls
  `SetPropertyOnSelection` — no editor policy in the shell. It still builds to a real
  ELF against gtk4 4.22 + libadwaita 1.9 + the reusable libraries alone
  (`build_gtk_shell.sh`), and the delegated workflow (New → select both entities →
  Set Key as one atomic edit → Save → reopen → Undo/Redo) is verified headlessly by
  `test_shell_workflow.cpp` (no display needed). So tested logic == shipped logic.
- **Why this is the factoring the goal asks for:** selection, multi-selection
  aggregation, and atomic multi-entity editing are now defined **once** and consumed
  by both the GTK sibling (D6) and the MFC sibling (D7) through the same
  `EditorDocument` — neither re-derives them. The empty-vs-unset distinction can no
  longer be silently lost in a shell.
- **Honest gaps:** the MFC sibling's `test_mfc_workflow.cpp` still exercises the
  older single-target `SetFirstBlockKey` verb (routing it through the new entity
  path is the concurrent MFC owner's follow-up); entities here are addressed by
  index, not yet scene `NodeHandle`s; the property-draft commit policy
  (commit-valid/report-invalid/discard) and legacy `CSelection`/property-sheet
  cutover remain (they need the legacy build). Status stays `characterized`: legacy
  Hammer is still the live authority. See `HAM-SEL-001`.

## Installed H0 machinery (this change)

Delivered as the first bounded H0 slice (HAM-INVENTORY-001 and HAM-RATCHET-001
foundations). All of it is executable and tested; none of it certifies a delivery
gate.

| Artifact | Purpose | Status |
| --- | --- | --- |
| `architecture/modules.json` → `hammerModules` | Editor module ownership + allowed dependency edges + strict include roots + native-token list | Installed; graph validated acyclic |
| `architecture/hammer_inventory.json` | Owned-file inventory with responsibility/effect/state/factorization/evidence and symbol splits | Installed; **coverage partial** (13/452 at install; 46 records, 46/530 files on 2026-09-25) |
| `architecture/hammer_migrations.json` | Migration ledger (IDs, dependency DAG, status, contracts, test selectors, retirement conditions) | Installed; 8 seed migrations (28 on 2026-09-25) |
| `architecture/hammer_baseline.json` | Exact editor-rule (HAM003) violations; strict modules zero-debt | Installed; **empty** (strict roots populated since; still zero entries on 2026-09-25) |
| `architecture/hammer_compatibility.json` | Declared features/formats/profiles/limitations per gate | Installed; H0 declares no editor features |
| `tools/archlint/archlint.py` → `hammer` subcommand | Validates the four artifacts + module graph; runs HAM003 native-token scan over strict roots | Installed |
| `tools/archlint/archlint.py hammer --coverage` | Diffs the hammer source universe against the inventory; lists unclassified files with lexically-detected effects; warns when the authored total drifts | Installed (HAM-INVENTORY-001 aid) |
| `tools/archlint/archlint.py hammer --scaffold` | Emits schema-valid inventory stubs (`evidence: "hypothesis"`, with `reviewTODO`) for a batch of unclassified files; never mutates the artifact | Installed (HAM-INVENTORY-001 aid) |
| `tools/archlint/tests/test_hammer.py` | Negative fixtures proving the validator rejects bad ownership, edges, cycles, statuses, and native tokens; coverage/scaffold accuracy fixtures | 24 tests, passing (rerun 2026-09-25) |
| `unittests/hammertest/{contracts,fixtures}/` | Contract-suite and characterization-corpus scaffolding | Conventions at install; 31 contract records on 2026-09-25, `fixtures/` still README only |

### Commands

```sh
python3 tools/archlint/archlint.py hammer --verify
python3 tools/archlint/archlint.py hammer --coverage            # what is still unclassified
python3 tools/archlint/archlint.py hammer --scaffold --path hammer/ --limit 25 > /tmp/stubs.json
python3 -m unittest discover -s tools/archlint/tests -v
```

`--coverage` and `--scaffold` assist HAM-INVENTORY-001: they enumerate the
source universe, report the gap, and emit reviewable stubs. Every scaffolded
field is a lexical hypothesis (`evidence: "hypothesis"`); an author must read the
file, correct the fields, drop `reviewTODO`, and re-evidence before merging.
Coverage flips to `complete` only by that human review, never by the tool.

The loader freeze commands are unchanged and remain authoritative for RFC 0001:

```sh
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
```

### Enforcement increments (what these checks do and do NOT prove)

Per the RFC's three-increment plan, only increment 1 is installed:

1. **Installed:** deterministic inventory/schema/DAG validation and the HAM003
   lexical native-token ratchet over strict roots.
2. **Partly installed (2026-09-25):** HAM002's include edges and cycle check
   over the strict roots (see the resolution below). Not yet: hermetic header
   builds for Hammer, and resolved target/link-graph checks (HAM001
   ownership-in-build-graph, HAM002 via the link graph).
3. **Not yet:** compiler-grounded transitive include and symbol checks (HAM004
   ambient access, HAM005 transaction authority, HAM009 pick-ID narrowing).

HAM003 currently finds zero occurrences because the strict include roots do not
exist yet; the scan is proven live by `test_hammer.py`, not by a populated
baseline. No gate claims a stronger guarantee than these installed checks provide.

2026-09-25: the roots `public/hammer/` and `hammer/core/` now hold strict
sources, and HAM003 still finds zero occurrences. Because increment 2 is not
installed, the strict sources already use edges `modules.json` does not allow,
and nothing fails:

- `hammer/geometry/brush.h` and `displacement.h` include
  `hammer/formats/keyvalues.h`. `hammer.geometry` declares no edges, and
  `hammer.formats` → `hammer.geometry` is allowed, so this is a
  geometry↔formats cycle at the include level.
- `hammer/app/editor_document.h` and `hammer/core/app/editor_controller.cpp`
  include `hammer/formats/keyvalues.h`; `hammer.app` does not allow
  `hammer.formats`.

Either the graph or the includes must change before increment 2 can pass.

**Resolved 2026-09-25 (R04-HAMGRAPH).** Increment 2's include check is
installed as HAM002 (forbidden module edge or architectural cycle, RFC 0002's rule table) in `archlint hammer --verify`. Each strict file
(`hammer/core/<area>/`, `public/hammer/<area>/`) belongs to `hammer.<area>`,
and its includes of another Hammer area or of a registered capability module
must be allowed edges. The module include graph the sources really form must
be acyclic; no exception can excuse a cycle. A deviation needs an exact-count
`hammerModules.includeExceptions` entry with an owner row, a tracking record
and a removal condition. Stale or miscounted entries fail. There are
7 fixtures, and three mutants (cycle, edge, stale exception) are each
detected.

- **The cycle is gone.** The VMF decoders `BuildSolidFromBlock`,
  `BuildSceneFromDocument` and `ParseDispInfo` moved from `hammer.geometry`
  into `hammer.formats` (`public/hammer/formats/vmf_geometry.h`,
  `hammer/core/formats/vmf_geometry.cpp`, namespace `hammer::formats`) with
  unchanged bodies. Geometry exposes the key-value-free helpers they need:
  `ParseVec3`, `FindFaceOnPlane`, `WorldScene::AddSolid` and
  `WorldScene::AddDisplacement`. `EditorController::BuildScene` now uses the
  last two, so the scene-bounds rule has one owner.
- **App → formats is a recorded exception.** `editor_document.h` and
  `editor_controller.cpp` hold one include each, owned by R22. The removal
  condition is that the document content and the controller's VMF
  read/write go through a `hammer.ports` persistence contract that formats
  implements, as RFC 0002 requires.
- **Evidence.**
  - Q-EDITOR 60/60 with g++ and with clang++, 1,570 checks each, the same
    counts as before the move.
  - The Wine parity run 60/60.
  - `build.tools` and `hammer.gtk-viewport-smoke` pass.
  - `archlint hammer --verify` passes (104 archlint tests).
  - The static audit shows 0 deviations.

## Gate status

| Phase | State | Notes |
| --- | --- | --- |
| H0 | **active / partial** | Migration schema, module graph, ratchet increment 1, corpus scaffolding, and a **headless strict C++20 build+test target** (`unittests/hammertest/run_headless.sh`, HAM-BUILD-001) installed and passing under gcc 16 and clang. Remaining for H0 exit: exhaustive inventory coverage and the versioned semantic comparator with a negative corpus. 2026-09-25: the comparator and its negative case exist (`CompareKeyValues`, HAM-CORPUS-001); still open are inventory coverage (38/531 on 2026-09-26), the RFC minimum fixture corpus and a CI-integrated target with R03 flags. `archlint hammer --verify` passes again ([current state](#current-state-2026-09-25)). |
| H1 | **active (geometry + scene seams)** | `hammer.geometry`: `AxisAlignedBox`, `RoundHalfAwayFromZero` (DRY grid-rounding owner), angle policies. `hammer.scene`: generational `HandleTable` (stale-reference rejection, independent documents) and `SceneGraph` (handle-addressed, validated/atomic reparent with cycle rejection, atomic subtree delete). All headless-verified under gcc + clang with negative providers. See below. |
| H3 | **partial (independent models)** | Ahead-of-authority reference models landed and pinned: `DocumentHistory` (revision vs saved-position, no-op neutrality, undo-to-saved clears modified), `PropertyValue` (empty-vs-unset-vs-mixed), and `UpdateHint` (HAM-UPDATEHINT-001) — the reusable core of MFC `CUpdateHint`, composing `hammer.geometry` + `hammer.scene` (notify-code buckets + unioned affected region, legacy `MAX_NOTIFY_CODES`=16 preserved; legacy source is orphaned dead code, so this is a reconstruction of intent). Not yet the live authority (needs H2 + legacy cutover). The **shared entity editor** (`EntitySelection` + `EditorDocument` entity methods, HAM-SEL-001/D8) now wires `PropertyValue` into live multi-selection editing with atomic multi-entity undo, and the GTK sibling drives it end to end. |
| H2 | **partial (codec + save seams)** | `hammer.formats`: VMF/keyvalues parser + writer + versioned semantic comparator (unknown-chunk preservation, malformed diagnostics, data-loss detection). `hammer.ports` + `hammer.app`: `IFileStore` port and `SaveDocument` transactional save (temp-write + atomic rename; failed save preserves prior file), verified with an in-memory fault-injecting fake. Remaining: VMF↔scene import/export, real file-store provider, fixtures corpus. 2026-09-25: `DiskFileStore` is the real provider (D6); `EditorController::LoadVmf`/`ToVmf` round-trip world brushes, per-face materials and point entities, but not brush entities or texture axes, and not through `hammer.scene` handles; 11 format/geometry cores are `extracted` ([additional work](#additional-work-recorded-2026-09-25)). Detached import, compile acceptance and the fixture corpus remain. |
| H4–H7, R1 | planned | Blocked on H0 exit and, for R1, the render contract + GTK profile. 2026-09-25: R15 and R16 are `done` and `roadmap.py` reports R17 (R1) startable; HAM-RENDER-001 is still `inventoried` and no Source-material GTK viewport exists. A bounded `linux-gtk-desktop` **boot + VMF-render feasibility slice** now exists (below): it is feasibility evidence, not R1/H5 completion — no editing tools, live-document authority, or material fidelity yet. |

**Strict-module conformance suites** are registered in the shared RFC 0005 runner
(`quality/conformance.manifest.json`, `tools/quality/conformance.py`): the RFC 0002
Q-EDITOR suites (positive + sensitivity pairs), each proven to catch a seeded
regression, green under gcc and clang on `linux-headless-core` (23 matched on the
current manifest) and cross-verified as native Windows PE under Wine (23/23,
`architecture/hammer_windows_parity.json`). Inventory now classifies 40 files;
14 migrations tracked. (A UTF-8 decode crash in the shared
runner's git-evidence step was fixed here so the gate survives binary files in the
working tree — this also unblocked the RFC 0003 lane on the same runner.)
2026-09-25: the manifest now registers 60 Q-EDITOR suites. The committed
`hammer_windows_parity.json` records 26/26, and the R02 evidence records 60/60
under Wine. Inventory has 46 records and the ledger 28 migrations.

### First H1 extraction — `hammer.geometry` AABB (HAM-GEOMETRY-001)

The first real code extraction is landed as a working, testable consumer at a new
boundary:

- **Owner:** `public/hammer/geometry/aabb.h` + `hammer/core/geometry/aabb.cpp`
  define `hammer::geometry::AxisAlignedBox` (and an owned `Vec3`), a
  dependency-free port of legacy `hammer/BoundBox`. No MFC, no `tier0/platform.h`,
  no PCH, no GPU — it compiles on the `linux-headless-core` profile with only the
  C++ standard library, under **gcc 16 and clang** with `-Wall -Wextra -Werror`.
- **Oracle:** `unittests/hammertest/geometry/test_aabb.cpp` characterizes the
  BoundBox behavior (reset invalidity, point/box growth, open-face intersection vs
  closed-face containment, size-preserving grid snap, `Rotate90`). Run via
  `unittests/hammertest/run_headless.sh`.
- **Intentional difference (declared, not silent):** `SnapToGrid` rejects a
  non-positive grid size instead of dividing by it as the legacy `Snap()` did.
  Documented in the header and covered by the oracle; recorded in the migration
  ledger's `intentionalDifferences`.
- **Authority discipline:** legacy `BoundBox` remains the **sole live authority**;
  no dual mutable model exists. Status is `characterized`. Advancing to `extracted`
  requires routing `BoundBox` callers through the shared owner via a named
  `Vector`↔`Vec3` boundary adapter (`hammer.adapters.source`/`mfc`) — the next
  bounded step, which needs the Source `Vector` (and thus the platform config) on
  the legacy side of the seam.
- **Why this proves the toolchain question:** compiling any `tier0`/`mathlib`
  header headlessly requires the full Waf-generated Source platform configuration
  (`platform.h` hard-errors otherwise). A strict `hammer.geometry` that owns its
  own value type is therefore the only way to get a genuinely hermetic, portable,
  testable geometry module now — which is exactly the RFC's "None" dependency rule
  for that module.

### VMF→brush geometry bridge and first `linux-gtk-desktop` boot (feasibility)

A second `hammer.geometry` extraction and the first bootable shell for the D1
delivery target landed together as a working, testable consumer at a new boundary.

- **Headless geometry bridge (HAM-GEOMETRY-001, tested).**
  `public/hammer/geometry/brush.h` + `hammer/core/geometry/brush.cpp` turn a
  parsed VMF (the `hammer.formats` keyvalues tree) into renderable convex solids:
  each brush `solid`'s `side` planes are intersected into per-face polygons with
  outward normals. It is dependency-free (C++ stdlib + `hammer.formats`), computed
  in double precision, and **winding-independent** — outward orientation is derived
  from an interior point found from the polytope's own vertices, not from VMF point
  order (this fixed a real bug where an origin-relative heuristic mis-oriented an
  off-origin brush). Contract: `unittests/hammertest/contracts/geometry.brush.v1.md`.
  Suites `hammer.geometry.brush` (+`.sensitivity`, an unclipped provider the oracle
  catches) pass under gcc and clang on `linux-headless-core` via
  `run_headless.sh`. This is `characterized`: legacy Hammer's own VMF/brush code
  remains the sole live authority; this is the strict, render-facing importer, not
  a replacement of a legacy caller, and it does not yet cover displacements,
  texture axes, or scene→VMF export.
- **Shell delivery target — first boot (`hammer/gtk/`, separate product).** A GTK4
  + libadwaita host composes the strict core (`EditorDocument` + the geometry
  bridge, over a host-owned `IFileStore`) and:
  - boots an Adwaita window and **opens a simple VMF** (File▸Open / `--open`);
  - lays out the **classic Hammer UI** — menu bar, toolbar, tool palette, the four
    viewports (3D camera + 2D top/front/side, resizable panes), the object bar
    (Select · texture group · current texture · VisGroups · Show/Edit/Mark) and a
    status bar with a live world-coordinate read-out;
  - renders a shaded 3D preview and 2D wireframe-with-grid views, with
    touchpad-native navigation (two-finger pan/orbit, pinch- and ⌃-scroll zoom
    anchored at the cursor, kinetic scrolling).
  Native GTK/GDK/GL detail is confined to `hammer/gtk/`; the core it drives stays
  headless. See `hammer/gtk/README.md`.
- **Verified.** Builds `-Wall -Wextra -Werror` under g++ 16 / clang 22 against
  system gtk4 4.22 + libadwaita 1.9 + epoxy. The interactive window boots on the
  live Wayland session with a GL 4.6 core context on all four viewports and stays
  running (no crash). The 3D preview is checked **without a window server** through
  a shared offscreen EGL path: `--screenshot` (single view) and `--quad` (the 2×2
  camera/top/front/side composite) render the sample `hammer/gtk/samples/room.vmf`
  correctly; `hammer/gtk/tests/viewport_smoke.sh` asserts non-blank geometry and
  the expected solid/triangle counts. (External capture of the on-screen window is
  blocked by the session compositor's screenshot policy; the offscreen path renders
  the identical viewport content.)
  - **Toolchain note:** GtkGLArea defaults to a GLES context here, which crashed
    Mesa's GLSL linker on the `#version 330 core` shaders; forcing desktop GL core
    (`gtk_gl_area_set_allowed_apis(GDK_GL_API_GL)` + required 3.3) fixes it and
    matches the offscreen EGL profile.
- **Not claimed.** This closes **no** gate. It is R1/H5 feasibility only, and the
  legacy MFC Hammer remains the authority. Still absent: entity/property editing,
  non-box brush *editing* (they render and round-trip, but the Block tool only
  makes boxes), vertex/clip tools, real material/texture rendering, displacements.

### Core editing loop and interaction authority (HAM-WORKFLOW-001, feasibility)

A single headless **interaction authority**, `hammer::app::EditorController`
(`public/hammer/app/editor_controller.h` + `hammer/core/app/editor_controller.cpp`),
now owns the core Hammer UX flows over *normalized* input, with the GTK shell as a
thin presenter that maps gestures/keys to it. This is the H3/H4 "one
selection/mutation/history owner" realised for spatial editing:

- **Flows:** Block tool (drag a grid-snapped rectangle in a 2D view → extruded
  brush, live pending box shown; Enter commits), Selection tool (pick/move/delete),
  and **undo/redo** — all through **one** `DocumentHistory` stack (no second stack;
  `IsModified`/`MarkSaved` delegate to it). Grid snapping routes through the shared
  `RoundHalfAwayFromZero` owner; geometry through the shared brush bridge.
- **Real brush shape (not bounding boxes):** a brush is stored as its set of
  half-space planes (+ per-face materials), so a loaded non-box brush (e.g. a
  triangular-prism ramp) **renders and round-trips as its true shape** — verified
  by the `hammer.app.editor_controller` suite (a 5-plane wedge stays 5-faced through
  load → `BuildScene` → `ToVmf` → reload) and visually via `hammer_gtk --cquad`
  (the front view shows a triangle, not a rectangle). The AABB is only a pick/grid
  cache. This resolves the earlier "loads as bounding box" limitation.
- **One save path (DRY):** save = `EditorController::ToVmf()` → `SaveDocument` over
  `hammer::adapters::platform::DiskFileStore`; load = `DiskFileStore::Read` →
  `LoadVmf`. The host holds no second `EditorDocument` or file store; the
  host-local `posix_file_store.h` was retired in favour of `DiskFileStore`.
- **Conformance UI test:** `unittests/hammertest/app/test_editor_controller.cpp`
  drives the controller with simulated input to build, snap, extrude, select, move,
  delete, undo/redo, preserve a non-box shape, and save-round-trip a map;
  `_negative.cpp` proves the grid-snap oracle catches a no-snap provider. Contract
  `unittests/hammertest/contracts/app.editor_controller.v1.md`, green under gcc and
  clang on `linux-headless-core`. The panels of the shell are drag-resizable
  (nested `GtkPaned`). Single-authority design reviewed by the sibling sessions.

#### Texturing + point entities: a full VMF built from scratch (HAM-WORKFLOW-001, this change)

The same authority now covers the two remaining pieces the contract flagged as
"later migrations" for a from-scratch authoring flow — **per-face texturing** and
**point-entity placement/keyvalue editing** — both routed through the one history
stack, with no new document, global, or second selection owner:

- **Material tool:** a click on a brush (`Tool::Material`) applies the active
  material (`SetActiveMaterial`) to every face; `ApplyActiveMaterialToSelection()`
  is the menu/button form. Both route through one private `RetextureBrush(id)`
  (DRY), recording **one** history unit iff a face actually changed.
- **Entity tool:** a click (`Tool::Entity`) places a point entity of the active
  class (`SetEntityClass`, default `info_player_start`) at the snapped location;
  `SetEntityProperty(id,key,value)` sets extra keys (`targetname`, `angles`, …) —
  one unit iff changed, with `classname`/`origin`/`id` reserved. The Selection tool
  picks/deletes entities too (nearest within a half-grid tolerance when no brush is
  hit); placement/deletion undo/redo through the shared stack.
- **VMF round-trip:** `ToVmf()` emits sibling top-level `entity` blocks
  (`classname`, extra keys, `origin`); `LoadVmf` reconstructs point entities from
  them (skipping brush entities, which still load as world brushes — a declared
  limitation). Entities and per-face materials survive save → reload in order.
- **The headline evidence** in `test_editor_controller.cpp` (`TestBuildRoomFromScratch`)
  builds a complete little room **from scratch through simulated input**: four walls
  (Block tool), a `BRICK/BRICKWALL001A` material applied to all four with the
  Material tool, and an `info_player_start` + `weapon_portalgun` placed with the
  Entity tool — then parses the saved VMF (4 world solids, 24 brick sides, 2 named
  entities at the expected origins) and reloads it to confirm every brush, material,
  and entity round-trips. `TestEntityPlaceDeleteUndo` covers place/delete undo/redo.
  All 59 RFC 0002 Q-EDITOR suites remain green under gcc and clang on
  `linux-headless-core`. Contract `app.editor_controller.v1.md` updated to match.
  **Not claimed:** brush (solid) entities, non-box brush *editing*, vertex/clip
  tools, and multi-select remain later migrations.

## Pre-existing failures (reported separately, not introduced here)

- **RFC 0001 loader gate is currently failing** at the working tree: `check --all`
  and `baseline --verify` report **51 new and 31 stale** occurrences, and
  `inventory --verify` reports stale. The AGENTS.md snapshot at `87955f67` recorded
  10 new / 3 stale; HEAD (`2d7e01d5`) has drifted further. This is the R01/R07
  reconciliation item from AGENTS.md — an intentional review action, not a
  regression from RFC 0002 work. This change touches no scanned source and no
  loader manifest section, so it does not affect that count.
- 2026-09-25 at `d6260d90`: `check --all` and `baseline --verify` report 64 new
  and 1 stale occurrence (0 relocated). `inventory --verify` reports 13
  uninstrumented native sites. None of them is in a Hammer path.
- 2026-09-25: `archlint hammer --verify` failed on the `render.contracts` edge
  that the RFC 0007 PBR schema slice added to `hammer.formats`. It passes again
  after the validator change above ([current state](#current-state-2026-09-25)).

### VPK + texture (VTF/VMT) loading: asset catalog and textured viewport (HAM-ASSET-001)

- **Gap this closes:** the shell rendered brushes flat-shaded; there was no path
  from a Source game's packed assets to a material preview or a textured viewport.
  The legacy Hammer path drove everything through the engine `IMaterialSystem` /
  `IMaterial` globals, which the strict core may not use.
- **Strict-core codecs (dependency-free: C++ stdlib + narrow ports only).**
  - `hammer::ports::IByteStore` (`public/hammer/ports/byte_store.h`): ranged,
    read-only byte source. Adapter `hammer::adapters::platform::DiskByteStore`
    (`<fstream>`, non-strict) so a texture is pulled from a data archive without
    reading the whole file.
  - `hammer::ports::IAssetSource` (`asset_source.h`): logical asset lookup +
    enumeration. `hammer::formats::SearchPathAssets` composes several providers
    first-hit-wins (a `.vmt` in one VPK naming a `.vtf` in another).
  - `hammer::formats::VpkArchive` (`vpk_archive.{h,cpp}`): dependency-free VPK v1/v2
    directory reader over `IByteStore`, resolving inline/preload/external chunks —
    the clean-core replacement for the tier2-coupled `vpklib` `CPackedStore`.
  - `hammer::formats::VtfImage` / `DecodeVtf` (`vtf_image.{h,cpp}`): VTF 7.1–7.5
    decoder to mip-0 RGBA8 (RGBA/BGR/BGRA/… + DXT1/3/5), header read by byte offset
    (packing/endian independent). Unsupported formats fail with a diagnostic.
  - `hammer::formats::MaterialCatalog` (`material_catalog.{h,cpp}`): enumerates
    `materials/**.vmt`, canonicalizes authored names, resolves `$basetexture`
    (following one level of `patch` `include`), and decodes it to RGBA, caching
    both. It **builds on** `hammer::formats::ParseMaterial` / `Material`
    (`material.{h,cpp}`, owned separately, HAM-MATERIAL-001) — one VMT parser, no
    duplicate. A patch's own `$basetexture` (incl. `replace`/`insert`) comes from
    the parser's `ResolvedParam`; the catalog additionally follows the patch
    `include` one level. Deeper include chains are out of scope.
- **Conformance (RFC 0005 runner, `linux-headless-core`, g++ 16.2 + clang 22.1,
  `-Wall -Wextra -Werror`).** Seven new suites, positive + sensitivity, all green:
  `hammer.formats.vpk_archive[.sensitivity]`, `.vtf_image[.sensitivity]`,
  `.material_catalog[.sensitivity]`, `.search_path_assets`. Fixtures are built by
  independent serializers (a VPK-blob builder, a VTF-blob builder) — the reader is
  never its own oracle — and the sensitivity suites reject malformed archives,
  truncated/unsupported VTFs, and missing/corrupt textures. Full RFC 0002 Q-EDITOR
  gate is 53/53. Contracts: `formats.{vpk_archive,vtf_image,material_catalog,
  search_path_assets}.v1.md`.
- **Real shipped content (integration smoke, not a required gate — asset absent in
  CI).** Mounting HL2 `hl2_misc_dir.vpk` + `hl2_textures_dir.vpk` (18 796 + 5 232
  entries) enumerated 5 292 materials and decoded DXT1/DXT5 512×512 base textures
  end-to-end through the search path.
- **GTK frontend (`hammer/gtk/`, additive).** The renderer textures the 3D view:
  a location-3 `aTexCoord` + `sampler2D`, per-face texture bind keyed by
  `BrushFace.material` via a borrowed `MaterialCatalog`, world-planar UVs at
  Source's 0.25 texels/unit, lazily-uploaded/cached GL textures. **Safety net:** no
  catalog → a single flat draw, byte-for-byte the prior mesh path (the offscreen
  `--quad`/`--demo`/`--cquad`/`--screenshot` paths and 2D/grid/highlight are
  unaffected). New `--textured OUT.ppm MAP.vmf VPK[,VPK]` offscreen mode is the
  headless evidence: `room.vmf` over the HL2 VPKs renders the `dev/dev_measure`
  grid texture tiled on the faces (13 → 126 distinct colours vs the flat render);
  `viewport_smoke.sh` still PASSes. A material-browser dock (thumbnail grid),
  object-bar current-texture swatch/name, and a `File ▸ Mount Game Assets…` folder
  chooser (auto-discovers `*_dir.vpk`) wire the catalog into the live shell.
- **Not claimed.** No R17/R25 material-fidelity gate is closed: UVs are
  world-planar (VMF `uaxis`/`vaxis` are not yet parsed), only `$basetexture` is
  shown (no bumpmaps/proxies/blends), and HDR/float VTF and cubemaps are out of
  scope per `vtf_image.v1`. This is the asset-loading + preview slice, not the full
  Source viewport.

### 3D camera navigation and view interaction parity (HAM-WORKFLOW-001, feasibility)

Brought the GTK shell's view interaction in line with legacy MFC Hammer for the
core navigation/selection flows the user exercises constantly. All additive; the
existing touchpad/orbit/pan and 2D editing paths are unchanged.

- **Classic 3D free-fly camera.** `Z` toggles mouse-look (cursor hidden; pointer
  motion turns the camera); `W/A/S/D` fly along the view, `Q/E` rise/fall, `Shift`
  boosts speed. Movement is time-integrated on a frame-clock tick that runs only
  while a key is held. Confined to the presenter and renderer: new
  `hammergtk::Renderer::FlyMove`/`FlyLook` operate on the existing orbit camera
  (FlyLook rotates in place keeping the eye fixed; FlyMove translates the eye along
  the view basis), so no camera-state duplication. GTK4 cannot warp the pointer, so
  look uses pointer deltas with a hidden cursor rather than MFC's recenter-each-frame
  loop — same feel, noted limitation.
- **3D click-to-select.** A non-drag left click in the camera view builds a world
  ray (`Renderer::PixelToRay`, matching the view's projection) and calls the new
  `EditorController::PickByRay` — an AABB broad phase (the **same cached bound the
  2D pick uses**, so selection stays one policy) followed by a precise ray/convex-
  polytope test against the brush's real face planes, so a click through a non-box
  brush's empty AABB corner correctly misses. It mutates only the selection (no undo
  step), like the 2D pick, and a miss deselects. Selection stays owned by the single
  `EditorController` authority.
- **Right-click context menus.** 2D views show a `GtkPopoverMenu` (a selection menu
  when a brush is selected, otherwise the default view menu), reusing the existing
  `app.*` actions. Faithful to MFC, the 3D view has no right-click menu.
- **2D view interaction.** **Space + left-drag** pans (MFC's pan idiom); **Tab**
  cycles a view's orientation Top → Front → Side (renderer mode + controller edit
  axes move together); **arrow keys** nudge the selection one grid step through the
  new `EditorController::MoveSelectionBy` (one undo unit, single authority);
  **+ / −** zoom anchored at the cursor; **1–9 / 0** are preset zoom levels /
  frame-all.
- **Multi-selection.** **Ctrl-click** (2D or 3D ray pick) toggles a brush in/out of
  a selection set; nudge, drag-move and delete all operate on the whole set as one
  undo unit; every selected brush is highlighted. Implemented as a primary
  (`m_selection`) plus an extra-set (`m_extraSelected`) in the one `EditorController`
  authority, so the single-select path is unchanged and all 60 Q-EDITOR suites still
  pass. The renderer gained `SetHighlights(set)` for the multi-highlight.
- **Evidence.** New headless oracles, both green under g++/clang:
  - `hammer.app.editor_controller.pick` conformance suite
    (`unittests/hammertest/app/test_editor_controller_pick.cpp`, registered in
    `quality/conformance.manifest.json`, Q-EDITOR): nearest-along-ray selection,
    reversed-ray far pick, miss/facing-away deselect, no-undo invariant, a wedge
    (non-box) case proving the convex test rejects an empty-AABB-corner ray a
    bounding-box pick would falsely hit, the arrow-nudge move/undo path, and
    multi-select (Ctrl-add/toggle, count, move-all, delete-all + restore).
  - The full **60-suite Q-EDITOR domain** still passes under g++ after the selection
    model change, confirming the single-select drag/move/delete/undo paths are intact.
  - `hammer/gtk/tests/camera_nav_test.sh` + `test_camera_nav.cpp`: FlyMove/FlyLook/
    PixelToRay camera math (eye stays fixed under look, forward/strafe/vertical
    move correctly, 2D views are no-ops). The full shell builds `-Werror`, style
    checker clean, `--cquad`/`viewport_smoke` render paths unaffected.
- **Not claimed.** No H5/R25 workflow gate is closed. Live GTK event dispatch
  (Z/WASD/right-click in a real window) is not automated here (needs a display;
  this environment's compositor blocks external capture) — the interaction *logic*
  is what the headless oracles cover. **View navigation and selection parity with
  MFC is complete** for the shipped editing tools: Z/WASD/QE fly + mouse-look,
  space+drag pan, orbit, 3D + 2D single- and multi-select (Ctrl), face-precise 3D
  picking, right-click menus, arrow-nudge, Tab view-cycle, and +/-/number zoom.
  Space+RMB strafe is covered by A/D + Q/E (the same camera motion via the keyboard).
  What remains is **not view-interaction parity** but separate editing *tools* with
  their own RFC rows — displacement (R33/H6), vertex and clip editing — each a
  feature family, not a uniform view behaviour. The GTK4-vs-MFC mouse-look nuance
  (pointer delta + hidden cursor vs recenter-each-frame) is a platform constraint,
  not a deferred feature.

## Additional work recorded 2026-09-25

The ledger and git history show Hammer work that the sections above do not
describe. The ledger is authoritative for each entry.

- **Format and geometry cores (2026-09-22).** Eleven strict headless modules
  are `extracted` with `authority: new`. Each has a contract record and a
  positive and sensitivity suite:
  - HAM-DISP-001 (`geometry.displacement.v1`);
  - HAM-INSTANCE-001 (`formats.instancing.v1`);
  - HAM-FGD-001 (`formats.fgd.v1`);
  - HAM-CORDON-001 (`formats.cordon.v1`);
  - HAM-OVERLAY-001 (`formats.overlay.v1`);
  - HAM-VISGROUP-001 (`formats.visgroups.v1`);
  - HAM-MATERIAL-001 (`formats.material.v1`);
  - HAM-GROUP-001 (`formats.groups.v1`);
  - HAM-PREFAB-001 (`formats.prefab.v1`, on the shared `vmf_transform`);
  - HAM-EXPORT-001 (`formats.map_export.v1`: instances, then hidden
    visgroups, then cordon);
  - HAM-PROPSHEET-001 (`formats.entity_property_sheet.v1`, FGD rows for the
    entity editor).

  No legacy caller routes through them, and none closes an H2 or H6 family
  gate. HAM-ASSET-001, named in the VPK/VTF section above, has no ledger entry.
- **`checks-v1` results (R02, `2134571b`).** Every Q-EDITOR suite reports one
  `testing::ReportConformance` record. `run_headless.sh` fails when a listed
  compiler is missing ([RFC 0005 Q1](0005-progress.md#q1--r02-runner-shared-conformance-runner)).
- **PBR material validation (RFC 0007 R47, `7035c29e`).** `MaterialCatalog`
  validates `PBRMetalRough` definitions against
  `public/render/pbr_material_schema.h` and follows primary and fallback patch
  chains with depth and cycle limits. The `render.pbr-material-schema` suite
  (Q-CONTENT) compiles the Hammer catalog sources. This slice added the
  `render.contracts` edge, which `archlint hammer --verify` accepts since
  2026-09-25 as an edge to a registered capability module. See the
  [RFC 0007 record](0007-progress.md).
- **KTX2 preview (RFC 0008 F3, `97e298c6`).** `MaterialCatalog` prefers a
  packaged KTX2 asset over the VTF of the same `$basetexture` through a decoder
  injected at composition. `hammer/adapters/source/ktx2_preview.cpp` supplies
  it to the GTK shell. The Waf-built `hammer_ktx2_preview_conformance` suite
  recorded 10 checks and 0 failures. Compressed-format preview and
  cross-provider precedence remain open. See the
  [RFC 0008 record](0008-progress.md#f3-hammer-ktx2-preview-caller-2026-09-23).

Still true on 2026-09-25: the MFC sibling (`hammer_mfc_shell.cpp`,
`test_mfc_workflow.cpp`) still uses `SetFirstBlockKey`, not the shared entity
editor (D8).

## Next dependency-ready migrations

2026-09-25 status: item 1 is open. For items 2–4, the first step named below
is done and the rest remains, as noted in each item. `archlint hammer
--verify` passes again ([current state](#current-state-2026-09-25)).

1. **HAM-INVENTORY-001 (continue):** expand `hammer_inventory.json` toward
   exhaustive per-file/per-symbol coverage; flip `coverage.status` to `complete`
   only when every hammer source file resolves to one owner and a negative fixture
   proves unowned files fail. Use `hammer --coverage` to see the remaining files
   and `hammer --scaffold` to draft stubs for review.
   - **Observed (tooling):** `hammer --coverage` measures the live source universe
     (files under `hammer/` plus the strict roots such as `public/hammer/`) and
     warns when it exceeds the authored `coverage.totalHammerSourceFiles` (still
     `452`). The tree has grown past that figure (new `hammer/*.cpp` and
     `public/hammer/geometry/aabb.h`); run the command for the current count and
     update the authored total as part of the next reviewed inventory write, not
     automatically. On 2026-09-25 the universe is 530 files, with 46 classified.
2. **HAM-BUILD-001:** establish the Linux 64-bit headless editor test target and
   record the final C++20 flags (coordinated with R02/R03).
   - 2026-09-25: `run_headless.sh` is the headless target, and CI runs it.
     Still open: a Waf-integrated editor target and R03 flag validation.
3. **HAM-CORPUS-001:** author the versioned semantic comparator and its negative
   corpus alongside the first VMF/geometry fixtures.
   - 2026-09-25: the comparator exists (`CompareKeyValues`) and its negative
     case passes. Still open: the RFC minimum fixture corpus and its provenance
     records under `unittests/hammertest/fixtures/`.
4. **HAM-GEOMETRY-001:** once a strict target exists, extract `BoundBox` into
   `hammer.geometry`, compile without the MFC PCH, and route callers through it.
   - 2026-09-25: `AxisAlignedBox` is extracted and characterized, and the
     `Vector`↔`Vec3` seam exists (D5). No legacy `BoundBox` caller routes
     through it yet, so the status stays `characterized`.
