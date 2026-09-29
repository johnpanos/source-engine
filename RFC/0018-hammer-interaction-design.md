# RFC 0018: Hammer Interaction and UI/UX Design

- Status: Proposed (2026-09-28); no implementation gate complete
- Date: 2026-09-28
- Scope: What a person does in the Hammer editor and what the editor shows
  back: flows, tools and their state machines, keys and pointer gestures,
  dialogs and panels, layout, scale, theming and accessibility. Also the
  UI-driven check that proves each flow, and the order in which the missing
  flows land.
- Editor architecture: [RFC 0002](0002-hammer-responsibility-factorization.md)
  owns the modules, contracts, state ownership and migration protocol this
  RFC builds on (sections "Selection and property editing", "Input and tool
  contracts", "Persistence contracts", "Assets and asynchronous jobs",
  "Rendering and host contracts"). Progress: [0002-progress.md](0002-progress.md).
- USD authoring: [RFC 0009](0009-usd-native-map-authoring.md) U3 owns the
  role-aware USD workflow gate; this RFC's flows are the VMF-era base it
  extends.
- Viewports: [RFC 0016](0016-render-core.md) owns the render core the views
  draw through ("Editor viewports" decision in AGENTS.md).
- Assets and builds: [RFC 0015](0015-asset-identity-content-build-graph.md)
  owns the asset index and the content build graph behind mount, reload and
  build.
- Presentation and play-in-editor: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns `render.presentation.v1`.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md), Q-EDITOR.
- Roadmap: no new row. Slices are children of R08 (headless target and UI
  suite), R17 (viewports), R23 (application authority), R24 (tools and
  presenters), R25 (GTK workflow) and R60 (USD editor workflow).

## Decision and boundary

1. **A companion RFC, not more sections in RFC 0002.** Agent decision under
   the user's standing instruction, 2026-09-28.
   - RFC 0002 is 1,082 lines about factorization: modules, contracts,
     inventories, ratchets. Interaction design is a product specification
     with its own flows, key map, acceptance table and gap list. It changes
     at the pace of UI slices, not migrations.
   - Keeping it separate leaves RFC 0002's contracts authoritative and
     unedited. This RFC only states what the user sees on top of them.
   - It must also outlive VMF: RFC 0009 U3 reuses these flows for USD.
2. **One command authority.** Every document change a flow makes is one
   `app::SessionCommands` command (one undo step), whether it comes from a
   menu, a key, a tool gesture's commit, a script or MCP. A flow without a
   command is incomplete; a UI-only mutation path is a defect.
3. **The catalog owns bindings; this RFC owns the target.** The installed
   authority for labels, menus and chords is `presenters::ActionCatalog`
   (`hammer/core/presenters/action_catalog.cpp`). The shortcut map below is
   the reconciled target and a list of decisions. It is not a second
   registry: each slice that changes a binding changes the catalog, and the
   map here records only why. Host-only accelerators (Save As, Quit, Reset
   Views, captures, reload) move into the catalog as `Host` actions, so one
   table drives menus, keys and the F1 shortcuts dialog.
4. **The author → save → compile → play loop always works.** Every slice
   keeps `corpus.hammer.ui`, `corpus.hammer.loop` and `corpus.hammer.mcp`
   passing. A slice that breaks one is not done.
5. **Legacy muscle memory by default, Source 2 where it is clearly better.**
   Where the two disagree, the resolution is a recorded decision with its
   sources. No binding is silently changed.
6. **No dead controls.** A widget that does nothing, or shows static sample
   data as if it were live, is removed until its flow lands. Showing a
   control promises its behavior (compare the GTK accessibility rule that
   "a role is a promise" [G1]).
7. **UI checks prove wiring; headless suites prove logic.** Tool and
   presenter behavior is tested in the Q-EDITOR headless suites
   (`hammer.presenters.editor_workspace` drives the workspace like a UI).
   A UI-driven case proves that real input reaches that logic and that the
   files or frames it produces are right. Each flow has both.

This RFC changes no content format, no RFC 0002 contract and no command
semantics. It certifies no gate.

## Observed starting point (2026-09-28)

Observed by reading source at `c5e61bb4` plus the dirty tree, rechecked
against `3c24e1c3` (R17 closure: the UI suite's `viewport` case on X11,
Wayland and two scales). Another session was editing
`public/hammer/presenters/entity_inspector.h` (class choices, colors,
`Settle()`) and adding an Object Properties window at the time; that work
is in flight and not counted as landed.

| Layer | What exists | Where |
| --- | --- | --- |
| Commands | 97 named commands with arguments and help text | `hammer/core/app/session_commands.cpp` |
| Actions | 43 catalog actions in File, Edit, View, Tools, Map; `Command`, `Tool` and `Host` targets; enable and check rules | `public/hammer/presenters/action_catalog.h` |
| Workspace | One document, tools, four cameras, snapshot cache, every presenter; one input routing policy | `public/hammer/presenters/editor_workspace.h` |
| Tools | Selection, Block, Entity, Clip, Vertex, Face; camera controller; shared interaction policy | `public/hammer/tools/*.h` |
| Presenters | Entity and face inspectors, outliner, class palette, material browser, history, status bar, visgroups, problems | `public/hammer/presenters/*.h` |
| GTK shell | Menu bar from the catalog, toolbar, tool palette, 2x2 views on the render core, object bar, status bar, Texture Application window, 2D context menu | `hammer/gtk/app.cpp` |
| MCP | The command catalog as MCP tools (`hammer_cli --mcp`), not yet in the live editor | `hammer/adapters/mcp/mcp_server.h` |
| UI suite | `room`, `no-hollow`, `no-light`, `viewport`; X11 and Wayland clients; 2x and 1.33x scale rows | `tools/quality/hammer_ui_test.py`, `quality/product_profiles/hammer-gtk-linux.json` |

What the GTK shell binds today:

- **Bound:** the catalog (menus, keys), the workspace input routing, the
  status bar's message slot (`Status().Message()`), `set_material` and
  `set_entity_class`, `MapBuildQueue` for F9.
- **Not bound:** `EntityInspector` (Alt+Enter says "Properties: not in this
  shell yet"; a properties-dialog session is adding it), `FaceInspector`,
  `Outliner`, `ClassPalette`, `MaterialBrowser`, `HistoryPanel`,
  `VisgroupPanel`, `ProblemsPanel` and most of `StatusBar`.
- **Dead or static controls:**
  - the palette's Magnify, Camera, Clipping Tool, Vertex Tool, Apply Texture
    and Apply Decal buttons ("laid out; not yet functional");
  - the object bar's Groups/Objects/Solids toggles, which are not connected,
    and show Objects while `EditorSettings::granularity` defaults to Groups;
  - the texture-group dropdown ("brick", "concrete", ...) and the visgroup
    list ("World geometry", "Entities", ...), which are sample strings, with
    Show/Edit/Mark buttons that do nothing;
  - the entity class dropdown, which offers two hard-coded classes
    (`kEntityClasses`).
- **Unsafe:** Quit (Ctrl+Q) and closing the window discard unsaved changes
  without asking. The title shows `*` when the map is modified, but nothing
  else guards it.

Input conflicts found while reading the routing code (each has a decision in
[Shortcut map](#shortcut-map-reconciled-with-action_catalogh)):

| # | Conflict | Evidence |
| --- | --- | --- |
| K1 | Shift+W/A/S/E/Q from rest run catalog actions (snap, Face tool, Selection tool, Entity tool, Select None). They fly fast only once another fly key is already held | `EditorWorkspace::OnKey` offers the camera first only when `Flying()` or it has capture |
| K2 | Shift+X while the Clip tool is active re-activates the tool and is consumed; the tool's keep-mode cycle (Front → Back → Both) is unreachable from the keyboard | `ExecuteAction` → `ToolManager::Activate` returns early for the active tool |
| K3 | Alt+Left in the 3D view captures for orbit on press; the Face tool's Alt+click lift (3D only) never arrives | `camera_controller.cpp` Down: Alt+Left returns `Capture()` unarmed |
| K4 | Delete is bound twice: `edit.delete` in the catalog and the Selection tool's key handler. The catalog always wins, so the tool branch is dead | `selection_tool.h` keys; `action_catalog.cpp` |
| K5 | Shift at a move: our policy constrains to the dominant axis; legacy Hammer clones [L1][L3]; Source 2 copies on Shift+translate [V4] | `interaction_policy.h` |
| K6 | Space: legacy and ours hold-to-pan (2D) or look (3D); Source 2 cycles selection modes [V2] | `camera_controller.h` |
| K7 | Ctrl+R: GTK host binds Reset Views; legacy creates a prefab [L1] | `app.cpp` accelerators |
| K8 | Shift+G: legacy Magnify tool [L1]; Source 2 Repeat Command [V5] | — |
| K9 | Ctrl+W: the catalog toggles Ignore Groups (a legacy binding per the catalog header; not in `editorkeys.txt`); GNOME closes a window [G2] | `action_catalog.cpp` |
| K10 | Tab: legacy cycles a 2D view's axes; GNOME moves focus [G2] | `app.cpp` host key |
| K11 | Alt+Enter, Alt+P, Alt+S, Alt+A (legacy); GNOME advises against Alt shortcuts [G2] | `action_catalog.cpp` |
| K12 | The window-level key fallback sends keys to the hovered view; a focused text field in the main window must keep its typing and editing keys | `app.cpp` key handlers |

## Sources and how they were consulted

Valve Developer Community pages return HTTP 403 to fetching (observed
2026-09-28, as the 2026-09-25 brief also found), and archive.org is not
reachable from this host. Claims marked **excerpt** come from search-result
excerpts of the cited page. Claims marked **unverified** were not
confirmed by any excerpt. **Repo** claims were read in this checkout.

| Id | Source | Used for | Confidence |
| --- | --- | --- | --- |
| V1 | [Dota 2 Workshop Tools: Navigation](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Navigation) | Hold the right mouse button to look and WASD to move; Z enters fly mode without holding it; fly mode does not work in CS2 Hammer | excerpt |
| V2 | [Mesh Editing 1](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Basic_Construction/Mesh_Editing_1) | The Select tool's modes (vertices, edges, faces, meshes) cycle with Space | excerpt |
| V3 | [Mesh Editing 2](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Basic_Construction/Mesh_Editing_2) | Modes can be entered directly; 2 enters Edges. Other number keys | excerpt; others unverified |
| V4 | [Mesh Editing 3](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Basic_Construction/Mesh_Editing_3) | Shift with Translate makes a copy | excerpt |
| V5 | [Command History](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Command_History) | Shift+G repeats the last command (Edit → Repeat Command); the panel repeats selected commands a number of times | excerpt |
| V6 | [Asset Browser](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Asset_Browser) | Drag materials, models or prefabs onto the map; a dragged model creates its entity | excerpt |
| V7 | [Compile and Run](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Compile_and_Run) | F9 opens Build Map; Settings chooses the components; "Load in engine after building" | excerpt |
| V8 | [Mesh Texturing](https://developer.valvesoftware.com/wiki/Dota_2_Workshop_Tools/Level_Design/Basic_Construction/Mesh_Texturing) | Faces mode; right-click applies the active material; an Alt variant carries mapping over | excerpt; Alt detail unverified |
| B | The 2026-09-25 [Source 2 ergonomics brief](0002-progress.md#source-2-ergonomics-brief-slice-3-design-input-2026-09-25) | P1–P3 behaviors and their sources | as recorded there |
| L1 | [`hammer/editorkeys.txt`](../hammer/editorkeys.txt) | Legacy key list | repo |
| L2 | [Hammer 3.4 Hotkey Reference](https://documentation.help/Valve-Hammer-Editor-3.4/Hotkey_Reference.htm), [valvearchive copy](https://valvearchive.com/hammer/guides/wc_3.x/Hotkey_Reference.htm) | Z toggles mouselook with WASD; PgUp/PgDn step through the "hit" list | excerpt (both refuse or redirect a fetch) |
| L3 | [`hammer/ToolSelection.cpp`](../hammer/ToolSelection.cpp) (`NudgeObjects(..., bClone)`, Shift begins a clone), [`hammer/hammer.cpp`](../hammer/hammer.cpp) `CHammer::Autosave`, [`hammer/mainfrm.cpp`](../hammer/mainfrm.cpp) autosave timer | Clone on Shift; autosave options `bEnableAutosave`, `iTimeBetweenSaves`, `iMaxAutosavesPerMap`; `.vmf_autosave` files; "loaded from an autosave" rename prompt | repo |
| L4 | [Valve Developer Union: Entity I/O](https://valvedev.info/guides/entity-interactions-in-sources-input-output-system/) | Object Properties Outputs tab with Add…, "My output named"; the Inputs tab lists entities that target this one | excerpt |
| L5 | [Steam guide: Hammer tips](https://steamcommunity.com/sharedfiles/filedetails/?id=2240989867) | H hides, U unhides, Ctrl+H hides the unselected | excerpt |
| G1 | [GTK 4 accessibility](https://docs.gtk.org/gtk4/section-accessibility.html) | Roles promise keyboard behavior; `LABEL`; `HELP_TEXT` for non-standard keys | fetched |
| G2 | [GNOME HIG: keyboard](https://developer.gnome.org/hig/guidelines/keyboard.html) | Tab and Ctrl+Tab focus; F10 menu; Shift+F10 or Menu for context menus; Esc closes transients; avoid Alt shortcuts; test keyboard-only use | fetched |

## Principles

### Adopted from Source 2 Hammer

| Behavior | Source | Our form |
| --- | --- | --- |
| Hold right mouse to look, WASD to move; Z toggles fly without holding | V1 | Fly keys act only while look is held or fly mode is on (decision K1) |
| Click-to-place entities on the surface under the pointer, from a searchable class list | B | Entity tool 3D placement; `ClassPalette` (search, categories, recent) |
| One key builds, one key builds and runs | B, V7 | F9 builds; Shift+F9 builds, publishes and runs. Build settings move to a separate chord (see below) |
| Block then "make room" | B | F hollows (`tools.hollow`, one undo step) |
| Faces mode with right-click apply | V8 | The Face tool (Shift+A): right-click applies the active material, Alt+right-click applies with alignment |
| Shift+translate copies | V4 | Shift at a move clones (decision K5) |
| Repeat last command, and a command history panel | V5 | Shift+G `edit.repeat_last`; the History panel shows labeled entries and jumps (`jump`). Multi-repeat is later |
| Drag assets into the viewport | V6 | Drag a material onto a face; drag a model to place a prop (after the model browser) |
| Tool Properties panel | B | A panel showing the active tool's options (block primitive, arch, marquee mode, clip keep mode, nudge step) |
| Filterable outliner, undo-history jump | B | `Outliner`, `HistoryPanel` bound as panels |
| Load in engine after building | V7 | Shift+F9 and "Run after build" in build settings |

### Deliberately not adopted

| Source 2 behavior | Why not | Revisit |
| --- | --- | --- |
| Space cycles selection modes (V2) | Space-hold pan in 2D and Space+left look in 3D are legacy muscle memory that Source 2 has no 2D equivalent for, and they are installed | R60, when USD meshes add vertex/edge/face modes |
| Faces as a selection mode of one Select tool | VMF faces are brush sides; the Face tool keeps texture work in one place with the Texture Application window. Legacy users reach it with Shift+A | R60 |
| Mesh modes, bevel, bridge, polygon tool, hotspot texturing, tile meshes | Need the USD mesh document (B, P3) | R60 |
| F9 opens a dialog every time (V7; legacy too) | The loop must be one key. F9 builds with the last settings; Ctrl+F9 opens settings | — |
| Arbitrary floating docking | RFC 0002 makes full docking a separate decision; fixed regions with tabs and resizable panes cover the flows | After R25 |

### Legacy Hammer kept

- Tool keys Shift+S/B/E/X/V/A, grid `[` `]`, Shift+W snap, Ctrl+G/U group,
  Ctrl+T tie to entity, Ctrl+Shift+C carve,
  Ctrl+B snap selected, Alt+Enter properties, Alt+P check, Tab cycles 2D
  axes, Space pans, Alt frees snapping, Escape cancels [L1].
- Classic 2x2 view layout (camera, top, front, side) as the default.
- Shift+Z maximizes the view under the pointer and restores it; Ctrl+E
  centers every view on the selection; 1–9 set preset 2D zooms; PgUp/PgDn
  step through the hit list under the last click [L1][L2]. All four are
  planned; none is installed.
- The Object Properties window (Alt+Enter), modeless, with Class Info,
  Flags, Outputs and Inputs [L4].
- Autosave to a recovery location on a timer, keeping several per map, with a
  rename prompt when a map was opened from an autosave [L3]. Our form differs
  (see [Recovery and autosave](#f14-two-documents-recovery-and-autosave)).

## User flows

Each flow lists its entry points, the steps and states, what the editor
shows, errors, undo, the current state, and pointers. "MCP" means the
command's MCP tool (`hammer_cli --mcp` today; the live editor later, F13).

### F1. Block out a room and hollow it

| Entry | Form |
| --- | --- |
| Menu | Tools → Block Tool; Tools → Make Hollow |
| Keys | Shift+B, then drag, Enter; F hollows |
| Pointer | Left drag in a 2D view (base), then drag in another 2D view (height); or in 3D, drag on the workplane and drag the height handle |
| Palette | Block Tool button |
| MCP | `create_block mins= maxs= [material=]`, `hollow [ids=] thickness=` |

Steps and states (`block_tool.h`):

1. Shift+B. Status: "Block". Cursor: crosshair.
2. Drag in the top view. The pending box shows with 8 handles. Its depth is
   the previous pending box's, else the selection's, else one grid step.
   Status shows `w h d`.
3. Drag in the front view to set the height; drag a handle to resize; drag
   inside to move (Shift constrains, Alt frees snapping).
4. Enter creates the block ("Create block"), selects it and clears the
   pending box. Escape during a drag restores the box before the drag;
   Escape with no drag discards the box.
5. F hollows the selected solid into six walls, one undo step, walls
   selected. Thickness: 16 by the catalog default. **Decision (agent,
   2026-09-28):** F uses the current grid size as the thickness, as the
   GTK shell's first slice did and the UI suite assumes (walls one grid unit
   thick); the Tool Properties panel shows and overrides it. The catalog's
   fixed `thickness=16` changes to a grid-derived argument rule.

Errors: Enter with no pending box does nothing; F with no box solid selected
is disabled ("Make Hollow is not available now"); a hollow that would leave
walls thicker than the box is refused with the reason.

Undo: one step per Enter and per F. The pending box is tool state; undo never
brings it back.

Now: implemented. Keyboard-only creation is missing (see
[Keyboard-only operation](#keyboard-only-operation)).

### F2. Carve, clip and edit vertices

| Entry | Form |
| --- | --- |
| Menu | Tools → Carve, Clipping Tool, Vertex Tool |
| Keys | Ctrl+Shift+C carve; Shift+X clip (again: cycle keep mode); Shift+V vertex; Enter applies a clip |
| Pointer | Clip: left drag a line in a 2D view, drag its end handles. Vertex: click handles (Ctrl toggles), drag in 2D |
| MCP | `carve [ids=] [targets=]`, `clip normal= point= [keep=] [ids=]`, `move_vertices id= indices= delta=`, `push_face`, `extrude_face` |

Steps (`clip_tool.h`, `vertex_tool.h`):

- **Carve:** select the carver, Ctrl+Shift+C. Every solid it touches is split
  into pieces outside it; one undo step. Legacy warns that carve makes bad
  brushes; we show the piece count in the status message.
- **Clip:** Shift+X. Drag a line in a 2D view; the kept pieces preview as
  clip-colored edges. Shift+X cycles Front → Back → Both (status shows the
  mode). Enter applies ("Clip"), keeps the pieces selected and clears the
  line. Escape clears the line.
- **Vertex:** Shift+V. Handles at vertices and edge midpoints of the
  selected solids. Click selects the handles stacked under the pointer
  (Ctrl toggles). Drag moves them, snapped. An invalid result previews in
  the error color and the release is refused with the reason.

Errors: Enter with no selected solid or a plane that misses all of them
(status message, no edit); a concave or flat vertex result (refused).

Undo: one step per carve, clip and vertex drag.

Now: partial. Shift+X cycling is unreachable (K2); vertex drags and clip
lines are 2D only (3D picks only); legacy's Ctrl-drag to move the whole clip
line and Ctrl+F split faces are not implemented; the palette's Clip and
Vertex buttons are dead.

Decision K2 (agent, 2026-09-28): a `Tool` action whose tool is already active
passes the key on to that tool instead of consuming it. This is legacy's
behavior for Shift+X [L1] and keeps one binding per chord.

### F3. Texturing

| Entry | Form |
| --- | --- |
| Menu | Tools → Face Edit Tool, Apply Current Material, Texture Lock; File → Texture Application… (moves to Tools) |
| Keys | Shift+A face tool (opens Texture Application); Shift+T apply current material to the selection; Shift+L texture lock |
| Pointer | 3D, Face tool: left click selects a face (Ctrl toggle, Shift whole solid), Alt+click lifts its texture, right-click applies, Alt+right-click applies with alignment |
| Drag | A material from the browser onto a face in the 3D view applies it (V6; planned) |
| MCP | `set_material`, `apply_material`, `select_faces`, `set_texture`, `shift_texture`, `justify mode=`, `align_texture mode=`, `replace_material`, `set_smoothing_group` |

Steps:

1. Mount game assets (F12's neighbor flow, F12 below) so materials exist.
2. Pick a material in the browser (search by keywords, "used in map" with
   face counts, recent list). It becomes the active material
   (`EditorSettings::faceTexture`), shown in the object bar swatch.
3. Apply: Shift+T to the selected objects or faces, or right-click faces
   with the Face tool.
4. Adjust: the face inspector shows material, shift U/V, scale U/V,
   rotation and lightmap scale for the selected faces as Single or Mixed.
   Editing a field is one undo step. Justify (left, right, top, bottom,
   center, fit), align to world or face.

Errors: no active material (status); an unknown material shows the
missing-material placeholder and stays named (RFC 0002 "Assets"); justify
without the material port is refused.

Undo: one step per apply and per inspector edit.

Now: partial. The Texture Application window picks and applies materials,
but its comment says the face model has no texture coordinates, which is no
longer true (`FaceInspector` exists). No face inspector, no material
browser presenter binding (the window keeps its own list), no used-in-map
filter, K3 blocks Alt+click lift, and no drag-and-drop.

### F4. Place and edit entities

| Entry | Form |
| --- | --- |
| Menu | Tools → Entity Tool; Edit → Properties |
| Keys | Shift+E; Alt+Enter opens Object Properties |
| Pointer | Entity tool: click a surface in 3D (placed on it), or a point in 2D (depth from the last 3D placement) |
| Palette | Entity Tool button; the class palette (search box, categories, recent) |
| MCP | `place_entity classname= origin=`, `place_on_surface`, `set_entity_class`, `set_entity_property`, `set_key`, `remove_key`, `rename_key`, `set_class`, `set_flag flag= on=`, `rename_entity`, `describe` |

Steps:

1. Shift+E. The class palette gets focus (a search field). Typing filters
   (prefix matches rank first); Enter picks the top match and returns focus
   to the view under the pointer.
2. Hover shows the pending marker where a click would place it. Click places
   it ("Place entity") and selects it.
3. Alt+Enter opens Object Properties for the selection: class (a searchable
   list limited to point or brush classes, `ClassChoices`), SmartEdit rows
   with types, a raw-keys toggle, Flags, Outputs, Inputs.
4. Multi-edit: with several entities, rows show Mixed where values differ;
   editing a Mixed row writes all of them.
5. Colors: `color255`/`color1` rows show a swatch; clicking it opens the
   color chooser; the picked color replaces the first three components and
   keeps brightness.
6. Flags: check boxes; each toggle is one undo step immediately.
7. Text edits are drafts. Enter or Apply commits all drafted keys as one
   "Edit properties" step. A selection change commits a valid draft first
   and is vetoed by an invalid one (the row shows why). Escape in a field
   reverts that field; closing the window settles the draft (`Settle()`).

Errors: no class (status); unknown or brush class for the Entity tool; no
surface under the pointer in 3D; invalid values per `ValidateKeyValue`.

Undo: one step per placement, class change, flag toggle and draft commit.

Now: partial. Placement works; the class dropdown offers two classes. The
Object Properties window (`hammer/gtk/properties_dialog.*`, R08-UI-PROPS in
the RFC 0002 record) does steps 3 to 7 except the Outputs and Inputs pages.
Brush entities: Ctrl+T ties the selected solids to a new entity of the
palette's brush class, Ctrl+Shift+W moves them back to the world (both
installed as actions).

### F5. Entity I/O

| Entry | Form |
| --- | --- |
| Window | Object Properties → Outputs, Inputs [L4] |
| MCP | `add_output output= target= input= [parameter=] [delay=] [times=] [ids=]`, `remove_output_at`, `remove_outputs`, `rename_entity` (updates references) |

Steps:

1. Outputs tab: one row per connection (My output named, Targets entities
   named, Via this input, With a parameter override, After a delay,
   Limit to times). A row whose target matches no entity, or whose
   output or input the class does not declare, shows a warning icon with
   the reason (`ObjectLabel`'s target rule, the catalog).
2. Add… creates a row; the output list comes from the class; the target
   field completes from targetnames and classnames in the map; the input
   list comes from the matched targets' classes.
3. Each add, replace or remove is one undo step.
4. Inputs tab: read-only rows of connections that target the selection;
   double-click selects the source entity (legacy "Mark").
5. Renaming a targetname with references asks nothing: `rename_entity`
   updates every reference in one step, and the status message says how
   many.

Errors: empty output or input names are refused; an unknown target is
allowed but warned (maps legitimately target runtime names).

Now: presenter rows and commands exist (`EntityInspector::Outputs()`,
`Inputs()`); no GTK view yet. Viewport connection lines are out of scope for
this RFC.

### F6. Grouping, visgroups and hiding

| Entry | Form |
| --- | --- |
| Menu | Tools → Group, Ungroup, Ignore Groups, Select Groups/Objects/Solids; View → Hide, Hide Unselected, Unhide All |
| Keys | Ctrl+G, Ctrl+U, Ctrl+W (ignore groups), H, Ctrl+H, U [L1][L5] |
| Panel | Visgroups tab: tree with eye toggles, member counts, Mark (select members), New, Rename, Delete, Add selection, Remove selection, Move selection (exclusive) |
| Outliner | Eye toggle per row (quick hide), click to select |
| MCP | `group`, `ungroup`, `set_granularity mode=`, `hide`, `hide_unselected`, `unhide_all`, `visgroup_create/rename/delete/move/add/remove/show` |

Rules:

- Quick hide (H) is editor state saved with the map (VMF `quickhide` is
  still rejected by the strict codec; see gaps). Visgroup visibility is
  document content. `scene::IsVisible` is the one visibility rule.
- Hidden objects are not pickable, not in marquee results and not drawn.
  The status bar shows "N hidden" while any are hidden, so a map never
  silently looks empty.
- Every visgroup action is one undo step.

Now: commands and presenters exist. The object bar's Visgroups panel
(`hammer/gtk/visgroups_panel.cpp`, R08-UI-VISGROUPS, 2026-09-28) is bound to
`VisgroupPanel`: the document's tree with show/hide checks (inconsistent for
Mixed), member counts, New from the selection (`visgroup_create
selection=1`, one step), Add, Remove, Move selection, Mark, Rename, Delete,
Move To and drag to reparent; `corpus.hammer.ui` case `visgroups` judges the
frames and the saved `visgroupshown`. **Decisions:** membership is edited
in the panel (its rows mark the selection's groups), not in an Object
Properties VisGroup page; Delete asks nothing (one undo step); drag
reparents (legacy's combine-on-drop is not adopted). No outliner view; no
auto visgroups (the presenter has none); no "N hidden" status yet.

### F7. The selection model

Objects:

- Granularity: Groups (default), Objects, Solids (`EditorSettings`;
  object-bar toggles and Tools menu). Ctrl+W toggles groups.
- Click selects (Replace); Ctrl+click toggles; click on empty clears;
  marquee in 2D selects inside the box (`ToolSettings::marquee`, Inside by
  default; legacy's Shift+Enter "entirely inside" becomes this setting);
  Ctrl+marquee adds.
- Selection changes on release, never on press. The object under a press
  shows as hover until then.
- Shift+Q or Escape (no gesture) selects none; Ctrl+A all; Ctrl+Shift+I
  inverts.
- Selection is not an undo step but passes the property draft guard.

Faces: Face tool only (F3). Face selection and object selection are
separate sets; switching tools keeps both.

Vertex handles: Vertex tool state only, dropped when the document changes
under it.

Cycling (planned, legacy [L1][L2]): a click remembers its ordered hit list
(`Pick2D`/`PickRay` order). PgDn selects the next hit, PgUp the previous,
wrapping; the status message says "2 of 4: light_spot". **Decision:**
PgUp/PgDn mean hit-list cycling, not camera positions, because the legacy
Camera tool is not adopted.

Selection sets (Source 2, P3): named saved selections. Deferred to R60;
visgroups cover most uses with VMF.

Find: Edit → Find Entities (class or name, trailing `*`) runs
`select_class` / `select_name`; the dialog is planned, the commands exist.

### F8. Camera and 2D navigation

Bindings belong to `tools::CameraController` (`camera_controller.h`).

| Input | 2D views | 3D view |
| --- | --- | --- |
| Middle drag | Pan | Pan |
| Space + left drag | Pan | Look |
| Right drag | — (a click opens the context menu) | Look; while held, WASD/EQ fly, Shift fast |
| Z | — | Toggle fly mode: mouse looks without a button, WASD/EQ fly (V1, L2) |
| Alt + left drag | — | Orbit about the pivot (armed until it drags; K3) |
| Wheel | Zoom about the cursor | Dolly |
| `=` / `-` | Zoom about the center (Ctrl: all 2D views, legacy) | — |
| 1–9 | Preset zooms (planned, legacy) | — |
| Tab | Cycle the view's axes Top → Front → Side | — |
| Ctrl+E | Center on the selection (planned) | Back off to the selection (planned) |
| Shift+Z | Maximize/restore this view (planned) | Same |
| Touchpad | Scroll pans, Ctrl+scroll zooms, pinch zooms | Scroll dollies |

Decision K1 (agent, 2026-09-28): WASD/EQ fly only while the look button is
held or fly mode is on (Z), as Source 2 does [V1]. From rest, those letters
fall through to actions and tools. This frees Shift+W/A/S/E/Q for their
catalog actions without a timing rule, and plain W/A/S/D/E/Q in the 3D view
do nothing when not flying.

Fly mode shows "Fly mode: Z or Escape to leave" in the help line and a
crosshair; Escape, Z or focus loss leaves it. It needs pointer confinement,
which is a host request (Wayland pointer constraints); without confinement
it falls back to "hold right mouse" and says so.

Now: partial. WASD fly works whenever the 3D view has keys (K1), Z fly,
Ctrl+E, Shift+Z and 1–9 are missing, and orbit captures unarmed (K3).

### F9. Grid and snapping

| Entry | Form |
| --- | --- |
| Menu | View → Larger Grid, Smaller Grid, Snap to Grid, Show Grid (planned) |
| Keys | `]` / Alt+S larger, `[` / Alt+A smaller, Shift+W snap, Shift+R show grid (planned, legacy) |
| Modifiers | Alt during a drag frees snapping |
| MCP | `set_grid size=`, `set_snap on=` |

Rules (`interaction_policy.h`): sizes are powers of two in [1, 1024]; a drag
snaps its reference point (the bounds' minimum corner, a handle edge, a
vertex), not the raw delta; nudge steps by the grid, Ctrl by one unit.
The status bar always shows `Grid 16 · Snap on`. The UI suite reads it.

Now: implemented, except grid visibility (Shift+R) and the 2D grid's
highlight of every 1024 units (legacy), which are planned.

### F10. Problems and fix-ups

| Entry | Form |
| --- | --- |
| Menu | Map → Check for Problems, Fix All Problems |
| Keys | Alt+P opens the Problems panel and focuses it |
| Panel | Rows: severity icon, code, message, object labels; Go To selects and centers; Fix; Fix All |
| Build | A build that fails or leaks adds its rows (leak, missing materials) |
| MCP | `check_map`, `fix_problem code= [id=]`, `fix_all` |

Rules: the panel is modeless and live (`ProblemsPanel` rescans lazily when
read after an edit). Go To selects the objects and runs Ctrl+E. Fix is one
undo step per row; Fix All one step. Errors block nothing: F9 still builds,
but the build panel shows "N errors" before the log.

Now: presenter and commands exist; Alt+P runs the check and shows only a
status message. No panel.

### F11. Compile, run and play-in-editor

| Entry | Form |
| --- | --- |
| Menu | Map → Build Map, Build and Run, Build Settings… (planned), Play in Editor (planned) |
| Keys | F9 build, Shift+F9 build and run, Ctrl+F9 settings (planned), F5 play/stop in editor (planned) |
| Toolbar | Build, Build and Run, Play/Stop buttons (planned) |
| MCP | `build_map path= [quality=fast\|full] [publish=0\|1]` |

Steps (F9): save to the map path (Save dialog if new); build off the UI
thread (`MapBuildQueue`); the status bar shows "Building…" with a spinner;
the Build panel opens on its log tab and streams the compile output; on
success "Built <map> in 4.2 s"; on a leak "Leaked: pointfile loaded", with
the leak line drawn in every view (legacy pointfile) and a Problems row; on
missing materials, a row per material. Shift+F9 then runs the map
(`run_map` host request; published to `./play`).

Build settings (Ctrl+F9, modal): quality fast or full, lighting profile
(`MapBuildRequest::lighting`), publish, run after build. The settings are
per-map editor state, not document content.

Cancel: the Build panel's Cancel stops the compile; the previous published
map stays intact (RFC 0015's atomic publication).

Edits during a build: allowed. The build compiled the saved file; the
result says "built revision N; map changed since" when the document moved.

Play-in-editor (planned; AGENTS.md "Play-in-editor"): F5 compiles if the map
changed, starts the engine as a child process through
`render.presentation.v1`, and shows its frames in the camera view's place
(dmabuf, shared with the viewport presentation step). The game view owns the
keyboard and pointer while focused; **Shift+Escape** returns them to the
editor (Escape stays the game's). F5 again or the Stop button ends the
child. Edits while playing are allowed; they do not reach the running game
until the next F5.

Errors: no builder in the composition (the action is disabled with the
reason); a failed save is never built; a second F9 while building is refused
("a build is running").

Undo: builds make no undo steps.

Now: F9 and Shift+F9 work asynchronously. No streamed log, no cancel, no
settings, no pointfile, no PIE. `build_map` has no `lighting` argument.

### F12. Asset mounting and reload

| Entry | Form |
| --- | --- |
| Menu | File → Mount Game Assets…, Reload Game Assets (moves to a Game menu or Tools; see layout) |
| Keys | Ctrl+Shift+R reload |
| Start | `--mount VPK[,VPK…]` |
| MCP | none yet (host concern; F13) |

Rules:

- Mounting is application state per game profile, not per document. The
  mounted list persists in the editor's settings file under
  `$XDG_CONFIG_HOME` and is restored at start.
- Reload re-reads the same archives and swaps the viewports' material source
  while frames are in flight (the `viewport` case tests three reloads).
- A missing archive is a status error naming the path; other archives still
  mount.
- After RFC 0015 R83, mounting reads the package asset index instead of
  scanning.

Now: implemented for VPKs; not persisted; no model or sound browsing.

### F13. View captures

| Entry | Form |
| --- | --- |
| Menu | File → Save View Captures |
| Keys | F12 |
| Output | `<map>-<view>.png` per view beside the map, or `HAMMER_GTK_CAPTURE_DIR` |
| MCP | planned: `capture_views [dir=]` as a host tool of the live editor, returning the paths |

A capture is the frame the view showed when F12 was pressed, not a new
render (the `viewport` case checks this). Captures are for bug reports and
for agents: once the live editor serves MCP, an agent can edit, capture and
look, in one session with the user's undo history.

Now: F12 works. The live-editor MCP is not implemented.

### F14. Two documents, recovery and autosave

**Two documents.** Decision (agent, 2026-09-28): one window per document,
each with its own `EditorWorkspace`, views and panels, inside one
`GtkApplication`. Shared at application scope: the clipboard (copy in one
map, paste in the other, as legacy MDI allowed), mounted assets and the
material catalog, the render core, the build queue (one build at a time),
recent materials and classes, settings. Ctrl+O opens into a new window
when the current map is modified or not empty; Ctrl+F4 closes a document
window (legacy MDI; K9 keeps Ctrl+W). The clipboard today is owned by each
workspace, so this needs an application-scoped `MapFragment` passed through
`WorkspaceServices`.

**Closing with changes.** Closing a modified document asks (modal alert):
"Save changes to <map> before closing?" with Save, Discard and Cancel.
Quit asks once per modified document. A build in flight keeps the process
alive until it finishes or is cancelled, with a notice.

**Autosave.** Legacy saves on a timer into an autosave directory, keeping a
bounded count per map [L3]. Ours:

- every 5 minutes of edited time (default; 0 turns it off), when the
  document changed since the last autosave;
- into `$XDG_STATE_HOME/hammer/autosave/<map key>/<timestamp>.vmf`, keeping
  the newest 5 per map (a platform-approved location; never beside the
  map);
- through the same codec as Save, from an immutable copy of the document,
  encoded off the UI sequence;
- never advancing the save position or clearing the modified mark
  (RFC 0002 "Persistence contracts").

**Recovery.** At start, and when opening a map, if an autosave is newer
than the map file (or the map was never saved), a banner (not a modal
dialog) offers "Recover autosave from 14:32" or "Discard". Recover opens it
as a modified, unsaved document with the original path, so Ctrl+S writes
the real map. This replaces legacy's rename prompt [L3]. Discard deletes
that map's autosaves. A clean exit deletes the autosaves of saved maps.

Undo: history is per document and does not survive a restart; recovery
restores content, not history.

Now: one document only; no close prompt (data loss today); no autosave.

## Interaction model

### Tools and state machines

Every tool follows the `tool.h` rules: it changes nothing during a gesture,
previews through the overlay, and commits at most one command when the
gesture completes. The tools differ only in when they commit:

| Tool | Pending state (not content) | Commits on | Cancel (Escape, focus loss, capture loss, tool switch) |
| --- | --- | --- | --- |
| Selection | press, drag, handle mode | release (move, scale, rotate, marquee, click) | drops the gesture; selection unchanged |
| Block | pending box | Enter | drag: box before the drag; no drag: discards the box |
| Entity | hover marker | release | nothing placed |
| Clip | clip line, keep mode | Enter | drag: previous line; no drag: clears the line |
| Vertex | handle selection, drag | release | handle selection before the press |
| Face | none | release of a click | a press that drags is abandoned |
| Camera | look, pan, orbit, fly | — (never edits) | releases keys and drags |

Rules added by this RFC:

- **Visible pending state.** A tool with pending state names it in the help
  line ("Block: Enter creates 384 x 384 x 128, Escape discards"). A pending
  box or clip line is never silently lost: switching tools with one pending
  shows "Block discarded" in the status message.
- **One gesture at a time.** A pointer press in a second view while one view
  has the capture is ignored (installed in `ToolManager`).
- **Tool Properties panel.** Shows the active tool's settings and pending
  values as editable fields: the Block tool's primitive and exact bounds, the
  Clip tool's keep mode, the Selection tool's marquee mode and nudge step,
  the hollow thickness. Editing a pending value edits tool state; Enter
  commits as the key does.

### Modal and modeless windows

| Window | Kind | Rule |
| --- | --- | --- |
| Open, Save As, Mount | Modal file dialog (`GtkFileDialog`, portal) | — |
| Save changes? | Modal alert | Only on close or quit with changes |
| Build Settings (Ctrl+F9), Transform (Ctrl+M, planned) | Modal | Short settings; Enter applies, Escape cancels |
| Object Properties (Alt+Enter) | Modeless, one per document window | Follows the selection; drafts per `EntityInspector` |
| Texture Application (Shift+A) | Modeless, one per document window | Follows the face selection |
| Find Entities | Modeless | Runs `select_class`/`select_name` |
| Problems, Build log, History, Outliner, Visgroups, Materials, Classes | Panels, not windows | See layout |
| Recovery | Banner | Never blocks editing |
| Shortcuts (F1) | Modeless dialog generated from the catalog | — |

No tool opens a dialog (RFC 0002: tools return host requests). A modal
window cancels the active tool's gesture (it is a focus loss).

### Focus and keyboard routing

Keys are routed in this order:

1. **A focused text-editing widget** (a property field, a search box, a
   number field) owns text keys: characters, Backspace, Delete, arrows,
   Home/End, Ctrl+A/C/V/X/Z/Y inside the field, Enter (commits the field),
   Escape (reverts the field). K12 decision: the window-level fallback never
   takes these keys from a focused text widget.
2. **Global chords** reach the catalog from anywhere in the window, even
   from a text field: Ctrl+S, Ctrl+Shift+S, Ctrl+O, Ctrl+N, Ctrl+Q, Ctrl+F4,
   F9, Shift+F9, Ctrl+F9, F5, F12, F1, Ctrl+Shift+R. None of them edits text.
3. **The view under the pointer** (legacy and Source 2: the active view
   follows the mouse) gets every other key through
   `EditorWorkspace::OnKey`: catalog chords, then the camera, then the
   active tool.
4. **A focused panel** (outliner, problems) gets its own navigation keys
   (arrows, Enter activates the row, Delete in the outliner runs
   `edit.delete` on the selected rows).

Focus moves:

- F6 and Shift+F6 cycle the regions: views, right panel, bottom panel,
  object bar. Ctrl+Tab does the same where Tab has a meaning [G2] (K10:
  Tab in a view keeps cycling its axes).
- F10 opens the menu bar; Shift+F10 or the Menu key opens the context menu
  of the view under the pointer, at the pointer (keyboard route to the
  existing 2D context menu) [G2].
- Clicking a view gives it focus. After a palette pick or a class choice,
  focus returns to the view that had it.
- Focus loss cancels the active gesture and releases held camera keys
  (installed: `OnFocusLost`).

### Shortcut map (reconciled with `action_catalog.h`)

"Now" is the installed owner: `catalog` (action id), `host` (a GTK
accelerator or host key), `camera` (`CameraController`), `tool` (the active
tool's key handler), or `—`. "Decision" records the target. Chords are in
`NormalizeChord` form.

**File and application**

| Chord | Now | Target | Legacy / Source 2 | Decision |
| --- | --- | --- | --- | --- |
| Ctrl+N | catalog `file.new` | same; new window if the current map is modified or not empty | same | — |
| Ctrl+O | catalog `file.open` | same | same | — |
| Ctrl+S | catalog `file.save` | same | same | — |
| Ctrl+Shift+S | host | catalog `file.save_as` (Host target) | Save As | move into catalog |
| Ctrl+F4 | — | close document window | legacy MDI close | adopt (K9) |
| Ctrl+Q | host | catalog `file.quit` (Host) | — (GNOME) | move into catalog; asks about changes |
| Ctrl+Shift+R | host | catalog `file.reload_assets` (Host) | — | move into catalog |
| F12 | host | catalog `view.capture` (Host) | — | move into catalog |
| F1 | — | shortcuts dialog | legacy F1 help (the shell's status text already says "press F1") | adopt |

**Edit**

| Chord | Now | Target | Legacy / Source 2 | Decision |
| --- | --- | --- | --- | --- |
| Ctrl+Z, Alt+Backspace | `edit.undo` | same | same | — |
| Ctrl+Y | `edit.redo` | add Ctrl+Shift+Z | legacy Ctrl+Y; GNOME Shift+Ctrl+Z [G2] | add the alternate; no conflict |
| Ctrl+X, Shift+Delete / Ctrl+C, Ctrl+Insert / Ctrl+V, Shift+Insert | catalog | same | same [L1] | — |
| Ctrl+Shift+V | — | Paste Special (offset, rotation, name suffix) | legacy menu item; key unverified | adopt with this chord; ops exist |
| Ctrl+D | `edit.duplicate` | same | legacy displacement tool; Source 2 duplicate | kept (catalog decision) |
| Delete | `edit.delete` and Selection tool | catalog only | same | K4: remove the tool's dead Delete branch |
| Ctrl+A | `edit.select_all` | same | legacy autosize views | kept (catalog decision) |
| Shift+Q | `edit.select_none` | same | same [L1] | — |
| Ctrl+Shift+I | `edit.invert_selection` | same | legacy Ctrl+I flips | kept |
| Alt+Enter | `edit.properties` (Host) | same | same | K11: keep legacy Alt chords; recorded GNOME deviation |
| Shift+G | — | `edit.repeat_last` | legacy Magnify; Source 2 Repeat [V5] | K8: adopt Source 2 (Magnify is obsolete) |
| Ctrl+M | — | Transform… (move, rotate, scale by numbers) | legacy [L1] | adopt |
| Ctrl+F | — | Find Entities… (outside the Vertex tool) | legacy morph split faces is Ctrl+F in the Vertex tool | Ctrl+F reaches the Vertex tool when it is active (K2 rule); Find otherwise |
| PgUp / PgDn | — | previous / next hit | legacy [L1][L2] | adopt (F7) |

**View**

| Chord | Now | Target | Legacy / Source 2 | Decision |
| --- | --- | --- | --- | --- |
| H / Ctrl+H / U | catalog | same | same [L5] | — |
| `]`, Alt+S / `[`, Alt+A | catalog | same | same | — |
| Shift+W | `view.snap_to_grid` | same | same | K1 frees it from fly-fast |
| Shift+R | — | Show Grid | legacy [L1] | adopt |
| Ctrl+E | — | Center views on selection | legacy [L1] | adopt |
| Ctrl+R | host Reset Views | catalog `view.reset` (Host) | legacy create prefab | K7: keep Reset Views; prefabs stay keyless (not adopted) |
| Shift+Z | — | Maximize/restore view | legacy [L1] | adopt |
| Tab | host | same, in a focused view | legacy; GNOME focus [G2] | K10: keep; Ctrl+Tab/F6 move focus |
| `=` / `-` | camera | same; Ctrl syncs all 2D views | legacy | add the Ctrl form |
| 1–9 | — | 2D preset zooms | legacy [L1] | adopt |
| Z | — | 3D fly mode | legacy mouselook [L2]; Source 2 fly [V1] | adopt |
| W A S D E Q | camera, always | camera, only while look held or fly mode | Source 2 [V1] | K1 |

**Tools**

| Chord | Now | Target | Legacy / Source 2 | Decision |
| --- | --- | --- | --- | --- |
| Shift+S / B / E / X / V / A | catalog tool actions | same; again passes the key to the tool | legacy [L1]; Source 2 B, E, S [B] | K2 |
| Shift+C | — | none | legacy Camera tool | not adopted: F8's navigation replaces it; VMF `cameras` still round-trip |
| Ctrl+G / Ctrl+U | catalog | same | same | — |
| Ctrl+T / Ctrl+Shift+W | catalog | same | same | — |
| Ctrl+Shift+C | `tools.carve` | same | same | — |
| F | `tools.hollow` | same; thickness = grid | legacy Ctrl+H; Source 2 [B] | kept (catalog decision) |
| Ctrl+B | `tools.snap_selected` | same | same | — |
| Shift+T | `tools.apply_material` | same | — | — |
| Shift+L | `tools.texture_lock` | same | — | — |
| Ctrl+W | `tools.ignore_groups` | same | legacy per the catalog header | K9: keep; Ctrl+F4 closes |
| Enter | tool (Block, Clip) | same | same | — |
| Escape | camera, then tool | same | same | — |
| Arrows | Selection tool nudge | same; Shift nudges a clone (legacy `bClone` [L3]) | legacy | K5 |

**Map**

| Chord | Now | Target | Legacy / Source 2 | Decision |
| --- | --- | --- | --- | --- |
| Alt+P | `map.check` | same; opens the Problems panel | legacy [L1] | K11 |
| F9 | `map.build` | same | legacy run map; Source 2 opens Build | one-key build (see F11) |
| Shift+F9 | `map.build_and_run` | same | Source 2 run after build [V7] | — |
| Ctrl+F9 | — | Build Settings… | — | adopt |
| F5 | — | Play in editor / stop | Unreal-style PIE | adopt when PIE lands |
| Shift+Escape | — | leave the game view (PIE) | — | adopt |

**Modifiers during gestures**

| Modifier | Meaning | Decision |
| --- | --- | --- |
| Alt | Frees snapping (legacy) and free rotation | — |
| Shift, moving objects (drag or arrows) | Clone, then move: one "Clone" undo step (legacy [L1][L3], Source 2 [V4]) | K5 (agent, 2026-09-28): Shift means "the other thing a move can do": clone where a move moves objects; constrain to the dominant axis where nothing can be cloned (the pending box, clip points, vertices) |
| Shift, rotating | 15° steps (legacy and ours) | — |
| Ctrl | Add or toggle (selection, handles, faces); nudge by one unit | — |
| Ctrl, clip tool drag | Move the whole line (legacy [L1]) | planned |

### Pointer gestures per view kind

| Gesture | 2D view (Top, Front, Side) | 3D view |
| --- | --- | --- |
| Left click | Tool: select, place entity, clip point, handle pick | Tool: select, place on surface, face select |
| Left drag | Tool: move, scale/rotate handles, marquee, box, clip line, vertex drag | Block workplane and height; Selection: no drag today, T/R gizmo later |
| Ctrl+left | Toggle / add | Toggle / add |
| Shift+left drag | Clone-move (objects); constrain (box, vertices) | Block: none; later gizmo clone |
| Alt+left | Drag without snapping | Click: Face tool lift; drag: orbit (armed, K3) |
| Right click | Context menu (selection or view menu) | Face tool apply; else nothing |
| Right drag | — | Look (+ WASD fly) |
| Middle drag | Pan | Pan |
| Wheel | Zoom at cursor | Dolly |
| Space+left drag | Pan | Look |
| Double click | Select and open Object Properties (planned; agent decision) | Same |
| Hover | Status coordinates; hover highlight; Entity tool marker | Status coordinates of the surface hit; hover highlight |

The drag threshold is 3 logical pixels per axis; handles are hit in logical
pixels (`box_handles.h`). Both scale with the display (see HiDPI).

### Status bar and help line

Regions, left to right, each with a stable accessible name:

| Region (accessible name) | Content | Owner |
| --- | --- | --- |
| `Help` | The active tool's hint with its keys, or the last message (errors in the error style) | `ToolStatus()`, `StatusBar::Message` |
| `Selection` | "2 solids, 1 entity (light)"; "N hidden" when any are | `StatusBar` |
| `Size` | `w 384 h 128 d 384` | `StatusBar` |
| `Pointer` | `x 12 y -4` (2D axes), or all three in 3D | `StatusBar` |
| `Grid` | `Grid 16 · Snap on` | `StatusBar` |
| `Tool` | `Block` | `StatusBar` |
| `Build` | idle, "Building…", or the last result | host from `MapBuildQueue` |

Messages stay until the next message or committed change (installed rule).
Today the shell writes its own strings ("Grid: 16  ·  Block", "6 brush(es)")
and the UI suite parses them with regular expressions. Decision: the shell
switches to the `StatusBar` presenter's texts and the named regions in one
slice with the UI suite, which then finds regions by name.

## Layout and presentation

### Window layout

```
+-----------------------------------------------------------------------+
| Menu bar: File Edit View Tools Map Help                                |
| Toolbar: New Open Save | Undo Redo | Build Build&Run Play | Reset     |
+----+------------------------------------------+-----------------------+
| T  |  camera           |  top (x/y)           | Tool Properties       |
| o  |                   |                      +-----------------------+
| o  +-------------------+----------------------+ [Properties|Outliner| |
| l  |  front (x/z)      |  side (y/z)          |  Visgroups|Materials| |
| s  |                   |                      |  Classes|History]     |
+----+------------------------------------------+-----------------------+
| Bottom panel (collapsed by default): [Problems | Build log | Commands] |
+-----------------------------------------------------------------------+
| Help | Selection | Size | Pointer | Grid | Tool | Build                |
+-----------------------------------------------------------------------+
```

Decisions (agent, 2026-09-28):

- **Left:** the tool palette, one button per installed tool, in catalog
  order, each with its accessible name and chord in the tooltip. Dead
  buttons go (principle 6).
- **Center:** the view grid. Default 2x2 (legacy). Shift+Z maximizes one.
  View → Layout offers 2x2, one view, and "3D large + 2D column" (Source 2's
  3D-first arrangement). The layout and pane positions persist per user.
- **Right:** Tool Properties on top (always visible, small), then a tabbed
  utility pane. Tabs bind presenters: Properties (the docked inspector, the
  same `EntityInspector` as the Alt+Enter window, so one draft), Outliner,
  Visgroups, Materials (`MaterialBrowser`), Classes (`ClassPalette`),
  History (`HistoryPanel`). The legacy object bar's selection-mode toggles
  and current-material swatch move to the top of this pane.
- **Bottom:** a collapsible pane with Problems, Build log and (later) the
  command history of the session's commands as a script (Source 2 Command
  History [V5]; our commands are already serializable).
- **Menus:** the catalog's categories plus Help. Host entries (Save As,
  Mount, Reload, Captures, Reset Views, Quit) become catalog `Host` actions
  (decision 3). "Texture Application…" moves from File to Tools.
- **Docking:** fixed regions, tabs, resizable and collapsible panes; no
  floating or re-dockable panels (RFC 0002 leaves full docking to a later
  decision).

### HiDPI and fractional scale

- Views render at logical size × scale (the device pixel size) and are
  shown at logical size; the `viewport` case checks frame size against
  logical size and scale on X11 (integer `GDK_SCALE`) and Wayland
  (fractional monitor scale).
- Input stays in logical pixels. Drag threshold, handle size, marker size
  and pick tolerances are logical. Line widths and handle drawing scale with
  the device pixels so they look the same at 1x and 2x.
- A scale change (moving to another monitor) re-renders at the new size
  without a reset of cameras: the camera keeps its world-per-logical-pixel
  zoom.
- Text in overlays (planned: labels, measurements) is drawn by the render
  core at device resolution, not scaled bitmaps.

### Theming

- Chrome follows libadwaita's light or dark style from the system.
- Viewport colors are one table owned by the render adapter
  (`hammer.adapters.render`), not the GTK theme. The views keep one dark
  palette in both styles, so screenshots, oracles and muscle memory stay
  stable.
- The table is `hammer/adapters/render/scene_geometry.cpp`: selected
  objects orange, selected faces red, hover in its overlay role. The same
  colors mean the same thing in 2D and 3D. A high-contrast table follows the
  system's high-contrast setting (planned).
- Status-bar errors use libadwaita's error style, never color alone: the
  text says "error".

### Accessibility and stable names

The UI suite finds widgets by accessible name and role over AT-SPI, so these
names are a contract. Renaming one is a change to this RFC and the suite in
the same commit.

| Widget | Accessible name | Role (as AT-SPI reports it) | Status |
| --- | --- | --- | --- |
| 3D view | `camera` | `frame` (GTK `application` role) | installed |
| 2D views | `top (x/y)`, `front (x/z)`, `side (y/z)` (follows Tab) | `frame` | installed |
| Tool buttons | `Selection Tool`, `Block Tool`, `Entity Tool`, `Clipping Tool`, `Vertex Tool`, `Face Edit Tool` (the catalog labels) | `toggle button` | first three installed |
| Entity class chooser | `Entity Class` | `combo box`, then a search `entry` | planned (today: named by the selected class) |
| Status regions | `Help`, `Selection`, `Size`, `Pointer`, `Grid`, `Tool`, `Build` | `label` | planned |
| Panels | `Tool Properties`, `Properties`, `Outliner`, `Visgroups`, `Materials`, `Classes`, `History`, `Problems`, `Build Log` | `panel` / `tab` | planned |
| Object Properties window | `Object Properties` | `dialog` | installed; editors are named by their key |

- Views set `HELP_TEXT` naming their non-standard keys [G1] ("Arrows nudge
  the selection; Tab cycles axes; Shift+F10 opens the menu").
- Every toolbar button has a label, not only an icon and tooltip.
- A Wayland toplevel reports no ACTIVE state over AT-SPI in the private
  session (GTK 4.22, mutter 50; R17 closure), so UI cases judge a key by its
  effect on Wayland. Named status regions make that judgment direct.
- The views are not screen-reader-usable spaces. The outliner, inspector,
  problems panel and Tool Properties fields are, so a map can be inspected
  and edited without a pointer.

### Keyboard-only operation

Goal: every flow except freehand drawing can be done without a pointer
[G2], and the blocked-out room can be made with typed numbers.

| Flow | Keyboard route |
| --- | --- |
| Create a block | Shift+B; F6 to Tool Properties; type mins and maxs; Enter creates (`create_block`) |
| Hollow, carve, group, hide | Catalog chords |
| Place an entity | Shift+E; type a class; Enter; in Tool Properties type the origin, or press Enter again to place at the 3D view's center hit (`place_on_surface` along the camera's forward ray) |
| Select | Outliner (F6, arrows, Enter, Ctrl+Enter adds); Find Entities |
| Move, rotate, scale | Arrows nudge; Ctrl+M Transform with numbers |
| Properties and I/O | Alt+Enter; Tab through fields |
| Texturing | Materials tab search, Enter sets active; Shift+T applies to the selection; face fields in the face inspector |
| Build, run, captures | F9, Shift+F9, F12 |
| Context menu | Shift+F10 or Menu |

## Acceptance

Each flow has a UI-driven case (real `hammer_gtk`, headless mutter, AT-SPI
names, compositor input; `tools/quality/hammer_ui_test.py`) and a headless
equivalent. Every UI case judges files or frames the editor produced, never
the UI's own claims, and has a negative control driven to the end that the
oracle must reject. Planned cases group several flows so the suite's
per-session cost (about 11 s) stays within a 120 s total budget.

| Flow | UI case (oracle; negative control) | Headless and MCP equivalent |
| --- | --- | --- |
| F1 block, hollow | **installed** `room`: VMF has six walls and both entities inside; build leak-free. Controls `no-hollow`, `no-light` | `hammer.presenters.editor_workspace`; `corpus.hammer.loop`, `corpus.hammer.mcp` (`create_block`, `hollow`) |
| F1 keyboard-only | planned `room-keys`: the same room made through Tool Properties fields and chords only; same oracle. Control: one wall's typed max off by 16 → `walls` fails | same |
| F2 carve, clip, vertex | planned `shape`: clip a block with Shift+X cycled to Both, Enter; drag a vertex; carve a doorway. Oracle: solid count and planes in the VMF, every solid convex (`check_map` clean). Control: Shift+X not pressed → only one side kept | `hammer.tools.clip`, `.vertex`, `hammer.app.ops.*`; MCP `clip`, `carve`, `move_vertices` |
| F3 texturing | planned `texture`: mount the generated VPK, search a material, Shift+T, Face tool right-click one face, set scale 0.5 in the face inspector. Oracle: VMF side materials and `uaxis`/`vaxis` scale; camera frame shows both texture colors. Control: apply skipped → material check fails | `hammer.presenters.face_inspector`, `.material_browser`; MCP `apply_material`, `set_texture` |
| F4 entity edit | planned `entity`: search "light" in the class palette, place, Alt+Enter, set `_light` by color chooser, a flag, a targetname; type an invalid brightness and try to click away. Oracle: VMF keys and spawnflags; the invalid value never reaches the file. Control: the flag toggle skipped | `hammer.presenters.entity_inspector` (draft guard cases); MCP `set_entity_property`, `set_flag` |
| F5 entity I/O | planned `io`: a `logic_relay` and a `light`; add an output in the Outputs tab with target completion; rename the light. Oracle: VMF `connections` updated to the new name. Control: rename done by raw key edit (no reference update) → connection target stale | inspector outputs/inputs cases; MCP `add_output`, `rename_entity` |
| F6 grouping, visgroups, hiding | planned `organize`: group two solids, make a visgroup, hide it, H one entity, save. Oracle: VMF `group`, `visgroup` and visibility fields; hidden objects absent from the camera frame. Control: visgroup not hidden → frame still shows it | `hammer.presenters.visgroup_panel`, `.outliner`; MCP `group`, `visgroup_*`, `hide` |
| F7 selection | planned in `organize`: PgDn cycles through two stacked solids; marquee; Ctrl+click toggle; the status `Selection` region read by name. Oracle: `describe` of the selection via the live-editor MCP, else the Properties title. Control: PgDn not pressed | `hammer.tools.selection`, pick suites; MCP `select`, `raycast` |
| F8 navigation | planned `nav`: Z fly with W, look, Shift+Z maximize, Ctrl+E center. Oracle: frames change as expected (the selected block centered in the top frame). Control: Shift+W from rest must toggle snap (K1), not move the camera | `hammer.tools.camera_controller`, workspace K1/K3 checks |
| F9 grid | **installed** in `room` (`[` twice, Grid 16 read back) | workspace suite; MCP `set_grid` |
| F10 problems | planned `problems`: a map with an unknown target; Alt+P; Go To; Fix. Oracle: VMF has the key removed; panel empty. Control: Fix not pressed | `hammer.presenters.problems_panel`; MCP `check_map`, `fix_problem` |
| F11 build and run | **installed** F9 in `room` (build record leak-free). Planned `build`: a leaking map shows the pointfile and a Problems row; Cancel keeps the previous published map. Control: the no-leak map shows no pointfile | `hammer.app.map_build_queue`; `corpus.hammer.loop`; MCP `build_map` |
| F11 play-in-editor | planned `play`: F5 shows engine frames in the view, Shift+Escape returns keys, F5 stops; child exits 0. Control: F5 on an unbuildable map shows the error and starts no child | `render.presentation.v1` suites |
| F12 assets | installed `viewport`: textured frames; three reloads during frames in flight restore textures | `hammer.adapters.render.*`; `hammer_ktx2_preview.py` |
| F13 captures | installed `viewport`: F12 capture equals a shown frame | planned MCP `capture_views` in the live editor |
| F14 close, autosave, recovery, two documents | planned `recover`: edit, wait for an autosave (interval shortened by a flag), kill the process, restart, Recover, Ctrl+S. Oracle: saved VMF equals the edited content. Control: Discard → file lacks the edit. Planned `quit-unsaved`: Ctrl+Q with changes, Cancel keeps the editor running, Save writes. Planned `two-docs`: copy in one window, paste in the other; each saves its own file | `hammer.app.document_history`, `save_orchestrator` suites; a new autosave suite (interval, count, no save-position change) |
| Layout, scale | installed `viewport` on X11, Wayland, 2x and 1.33x (frame size at scale; divider resize) | `hammer.adapters.render.service.vulkan` SV1–SV4 |
| Accessibility names | planned `names`: every name in the table exists with its role. Control: a renamed widget fails | — |
| Shortcuts | planned headless `hammer.presenters.action_catalog` check: no chord maps to two actions; every Host accelerator is a catalog entry; the K-decisions hold (Shift+X reaches an active Clip tool; Shift+W from rest toggles snap) | — |

Every planned case needs the conformance manifest row, a `min_checks`, and
the baseline budget raised in the same change.

## Gap table

State at `3c24e1c3` plus the dirty tree. Pointers are where the fix goes.

| Flow | State | What is missing | Pointers |
| --- | --- | --- | --- |
| F1 block, hollow | implemented | Tool Properties panel; hollow thickness from grid; keyboard-only block | `hammer/gtk/app.cpp`; `action_catalog.cpp` (`tools.hollow` args) |
| F2 carve, clip, vertex | partial | K2 (Shift+X cycling); dead palette buttons; 3D clip and vertex drags; Ctrl clip-line move; Ctrl+F split | `editor_workspace.cpp` `ExecuteAction`; `clip_tool.h`; `vertex_tool.h` |
| F3 texturing | partial | Face inspector and material browser views; Alt+click lift (K3); drag-and-drop; stale Texture Application code | `app.cpp` Texture Application; `face_inspector.h`; `material_browser.h`; `camera_controller.cpp` |
| F4 entities | partial | Class palette view (two hard-coded classes); Object Properties Outputs and Inputs pages; double-click to open | `app.cpp` `kEntityClasses`; `class_palette.h`; `hammer/gtk/properties_dialog.*` (installed: class, SmartEdit and raw rows, flags, Apply/Cancel/Settle; UI case `properties`) |
| F5 I/O | missing (UI) | Outputs/Inputs tabs, target completion | `entity_inspector.h` `Outputs()`, `Inputs()` |
| F6 grouping, visgroups, hiding | partial | Outliner view; auto visgroups; hidden count; `quickhide` VMF block rejected by the strict codec (visgroups panel installed, R08-UI-VISGROUPS) | `visgroups_panel.cpp`; `visgroup_panel.h`; `outliner.h`; VMF codec |
| F7 selection | partial | Hit-list cycling; object-bar granularity toggles unwired and wrong default; Find dialog | `selection_tool.h`; `app.cpp` `MakeObjectBar` |
| F8 navigation | partial | K1 fly gating, Z fly, K3 armed orbit, Ctrl+E, Shift+Z, 1–9, Ctrl zoom sync | `camera_controller.cpp`; `editor_workspace.cpp` `OnKey`; `app.cpp` layout |
| F9 grid | implemented | Shift+R grid visibility | `action_catalog.cpp`; render adapter grid |
| F10 problems | partial (no UI) | Problems panel; Go To centering | `problems_panel.h`; `app.cpp` |
| F11 build, run, PIE | partial | Streamed log, cancel, settings, pointfile, `build_map lighting=`; PIE | `map_build_queue.h`; `map_builder.h`; `session_commands.cpp` |
| F12 assets | partial | Persisted mounts; model browser | `app.cpp` `MountAssets` |
| F13 captures | implemented (F12) | Live-editor MCP `capture_views` | `hammer/adapters/mcp`; `app.cpp` |
| F14 documents, recovery | missing | Close/quit prompt (data loss today); autosave; recovery banner; multi-window; application-scoped clipboard | `app.cpp` `ActionQuit`, window `close-request`; `editor_workspace.h` `WorkspaceServices` |
| Status bar | partial | Presenter texts, named regions, hidden count, build region | `status_bar.h`; `app.cpp` `MakeStatusBar`; `hammer_ui_test.py` |
| Keys | partial | K1–K12 decisions; host accelerators into the catalog; F1 shortcuts dialog; Ctrl+Shift+Z; Shift+G; Ctrl+M; PgUp/PgDn | `action_catalog.cpp`; `app.cpp` `AddActions` |
| Layout | partial | Right utility pane with tabs; bottom pane; layouts; persistence | `app.cpp` `OnActivate` |
| Accessibility | partial | Names table; `HELP_TEXT`; F6 region cycling; Shift+F10 | `app.cpp` |
| Dead controls | present | Remove Magnify, Camera, Apply Decal; wire or hide Clip, Vertex, Face buttons; texture-group dropdown (visgroup sample rows removed) | `app.cpp` `MakeToolPalette`, `MakeObjectBar` |

Top gaps, in order of harm:

1. **Unsaved work is lost on quit or window close** (F14). No prompt, no
   autosave.
2. **Dead and fake controls** in the palette and object bar: they look like
   working features.
3. **Entity editing is not reachable** from the UI beyond placing two
   classes (F4, F5); the presenters are ready.
4. **Key routing defects K1–K3**: tool keys that fire during fly starts,
   Shift+X that cannot cycle clip modes, and Alt+click lift that cannot
   arrive.
5. **No texture adjustment UI** (F3) although `FaceInspector` exists.
6. **Builds give no log, cancel or leak display** (F11).
7. **Panels are unbound**: outliner, history, problems (visgroups bound).
8. **One document only**, and the clipboard is per workspace.

## Delivery order

Each slice keeps `corpus.hammer.ui`, `corpus.hammer.loop` and
`corpus.hammer.mcp` passing, adds its own UI case with a negative control,
and updates its progress record in RFC 0002's progress file. Slices touching
`hammer/gtk/app.cpp` coordinate with the session that owns it at the time.

| Slice | Content | Why here | Proof |
| --- | --- | --- | --- |
| UX1 Safe and honest shell | Close and quit prompts; remove dead controls; wire the granularity toggles (default Groups); host accelerators into the catalog; F1 shortcuts dialog; `StatusBar` texts and named regions (with the suite) | Stops data loss and false affordances; cheap | `quit-unsaved`; `room` still passes on named regions; catalog uniqueness check |
| UX2 Entities | Object Properties window (in progress elsewhere); class palette view with search; Outputs and Inputs tabs; double-click opens properties | Presenters are ready; completes the room-with-lights loop by UI | `entity`, `io` |
| UX3 Autosave and recovery | Autosave off the UI sequence; recovery banner; autosave suite | Crash safety before the editor is used for longer sessions | `recover` |
| UX4 Input conflicts | K1 (fly gating and Z), K2, K3 (armed orbit), K4, K5 (Shift clone), Ctrl+Shift+Z, Shift+G repeat, PgUp/PgDn, Ctrl+E, Shift+Z, Shift+F10, F6 | Behavior changes users feel; needs the shortcuts dialog from UX1 to announce them | workspace checks per K; `nav` |
| UX5 Texturing | Materials tab (browser presenter), face inspector, Texture Application rewired, drag material onto face | Needs mounted assets (installed) and K3 | `texture` |
| UX6 Build panel | Streamed log, cancel, build settings (Ctrl+F9), leak pointfile, `build_map lighting=` | Uses `MapBuildQueue`; R08-ASYNC-BUILD's open items | `build` |
| UX7 Panels | Right utility pane (Outliner, Visgroups, History), bottom pane (Problems), Tool Properties panel, layouts and persistence, hidden count | Presenters are ready; needs the layout frame | `organize`, `problems`, `room-keys` |
| UX8 Two documents | Window per document; application clipboard and assets | Needs UX1's close prompts | `two-docs` |
| UX9 3D editing | T/R gizmo with Shift clone, 3D selection drag, 3D vertex and clip, Transform dialog (Ctrl+M) | Source 2 P2; largest tool work | `shape`, gizmo headless suites |
| UX10 Live MCP and captures | The live editor serves MCP over the same `SessionCommands`, plus host tools (`capture_views`, camera framing) | Agents share the user's session | `corpus.hammer.mcp` against the live editor |
| UX11 Play in editor | Child engine via `render.presentation.v1`, F5, Shift+Escape | Depends on the presentation bridge and R17 dmabuf step | `play` |
| UX12 USD modes | Role-aware tools, mesh selection modes (Space may be revisited), selection sets | RFC 0009 U3 / R60 | RFC 0009 gates |

UX1–UX3 are dependency-ready now. UX2 is partly in flight in another
session.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The shortcut map here drifts from the catalog | The catalog stays the authority; the planned catalog check enforces the K-decisions; this RFC records reasons only |
| Changing legacy bindings (K1, K5) surprises users | Shortcuts dialog lists changes; each K decision cites both editors; Shift clone matches both references |
| UI suite cost grows with every flow | Cases group flows; logic stays in headless suites; 120 s budget |
| Concurrent sessions editing `app.cpp` | Slices name their owner; small commits through `commit_paths.py` |
| Source 2 claims rest on search excerpts | Confidence column; unverified items are not used for bindings |
| Autosave hitches on large maps | Encode from an immutable copy off the UI sequence; measure against a budget before enabling by default |

## Open decisions and required evidence

- Autosave interval and count defaults (5 minutes, 5 files) are agent
  choices; revisit with a measured encode time on sp_a2_trust_fling.
- Whether "3D large + 2D column" should become the default layout, after
  user use.
- Source 2's direct selection-mode keys (V3) beyond 2 = Edges are
  unverified; R60 must confirm them before binding.
- Paste Special's legacy chord is unverified; Ctrl+Shift+V is our choice.

## Proposed decision

Adopt this RFC as the interaction and UI/UX specification for the Hammer
GTK editor. Deliver it by the slices above, as children of the existing
roadmap rows. Each slice lands with its UI case, negative control and
headless equivalent, and keeps the author → save → compile → play loop
working.
