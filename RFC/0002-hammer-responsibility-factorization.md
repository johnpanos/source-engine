# RFC 0002: Hammer Responsibility Factorization and Incremental Modernization

- Status: Proposed
- Date: 2026-09-21
- Scope: Hammer, its reusable editor libraries, and its application adapters
- Related: [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md)
- Future source format: [RFC 0009: USD-Native Map Authoring](0009-usd-native-map-authoring.md)
  owns the editable USD map schema and native compile workflow
- Verification: [RFC 0005: Quality and Correctness Harnesses](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006: C++20, Ownership, and Synchronization](0006-modern-cpp-ownership-and-synchronization.md)
- Source inspection reference: `b5eb9915bdebf5af1045c5821d4ac3d9af15dbb2`
- Implementation status: Phase H0 is active. The migration schema, module graph,
  enforcement increment 1 (inventory/DAG validation and the HAM003 native-token
  ratchet), and corpus scaffolding are installed; inventory coverage is partial and
  no delivery gate is complete. Gate decisions and open-question answers are in
  [0002-progress.md](0002-progress.md). Later modules, contracts, and CI gates
  remain future work.

## Summary

Extract a toolkit-independent editor from Hammer, retain its Source content
semantics, and implement a GTK4/libadwaita presentation adapter. The editor must
also run through a headless test composition. The existing MFC application is
the initial compatibility host wherever a reproducible build is available.

The governing rules are:

1. Each domain rule and authoritative state value has one owner.
2. An implementation can replace another only when it satisfies the same
   observable behavioral contract.
3. Dependencies point toward domain data and contracts; application composition
   selects concrete providers.
4. Each migration is a bounded change with named callers, a compatibility
   boundary, executable evidence, and a deletion condition.
5. Architectural debt is recorded as exact occurrences and can only shrink
   without a separately reviewed exception.

DRY means a single authoritative representation of knowledge. It does not mean
combining every similar loop, coordinate system, or format into one abstraction.
The Liskov Substitution Principle (LSP) concerns accepted inputs, results,
invariants, effects, lifetime, and operation sequences. Inheritance alone does
not establish substitutability.

The migration will proceed by working editor workflows. A representative slice
opens a VMF, selects and transforms a brush, undoes the edit, saves the map, and
compiles it. This gives each extraction a concrete consumer and prevents an
extended period of interface design without usable behavior.

That VMF workflow is a compatibility and extraction slice, not the final
authoring format. RFC 0009 targets USD as the saved authority for new maps and
separates world solids, static props, dynamic props, physics props, and
geometric entities by their editing and compile contracts. The document,
transaction, and tool seams extracted here must not require a VMF key/value
tree, BSP face ID, or one global active map in their public contracts.

## Motivation and observed state

Hammer contains substantial reusable geometry, map-format knowledge, and editor
behavior. Its current boundaries also couple that behavior to MFC, VGUI,
process-wide state, and legacy renderer presentation.

An initial lexical inventory counted 221 `.cpp` files and 231 `.h` files in
`hammer/`, totaling approximately 184,000 physical lines including comments and
blank lines. These figures are sizing observations, not estimates of reusable
code. They include source that may be inactive in a particular build.

Representative evidence:

| Observation | Source | Consequence |
| --- | --- | --- |
| Common precompiled header includes MFC and Windows facilities | [stdafx.h](../hammer/stdafx.h) | Removing a visible UI include does not establish portability. |
| Application combines MFC lifecycle and engine application-system composition | [hammer.h](../hammer/hammer.h), `CHammer` | Bootstrap, service ownership, and UI lifetime need separate owners. |
| Map document derives from `CDocument` | [mapdoc.h](../hammer/mapdoc.h), `CMapDoc` | Editing state and operations cannot currently be instantiated independently of the MFC document layer. |
| Scene bases expose selection, rendering, transforms, persistence, and notifications | [mapatom.h](../hammer/mapatom.h), [mapclass.h](../hammer/mapclass.h) | Broad base contracts hide which behaviors each object actually supports. |
| Selection commits and invalidates property-dialog state | [Selection.cpp](../hammer/Selection.cpp), `CSelection::SelectObject` | Extracting selection requires preserving the user-visible property commit policy. |
| Tools have repeated 2D, 3D, and logical input entry points | [toolinterface.h](../hammer/toolinterface.h), `CBaseTool` | Event transport and gesture logic are coupled to concrete view classes. |
| View interface returns `CWnd*` | [mapview.h](../hammer/mapview.h), `CMapView::GetViewWnd` | The current interface cannot serve an ordinary GTK or headless host. |
| Asset interface draws through `CDC`, `RECT`, and `CPalette` | [ieditortexture.h](../hammer/ieditortexture.h) | Asset metadata, loading, thumbnails, and native painting are combined. |
| Renderer passes native window handles into material-system views | [VGuiWnd.cpp](../hammer/VGuiWnd.cpp), `CVGuiWnd::DrawVGuiPanel` | Presentation ownership must be adapted explicitly. |
| Picking converts object pointers to `unsigned int` and back | [render3dms.cpp](../hammer/render3dms.cpp), `CRender3D::BeginRenderHitTarget` and `EndRenderHitTarget` | A 64-bit migration must replace pointer encoding with bounded pick identifiers. |
| Hammer is described by a Windows-oriented VPC project and has no `hammer/wscript` | [hammer_dll.vpc](../hammer/hammer_dll.vpc), [root wscript](../wscript) | A supported editor build must be established before claiming continuous build preservation. |

The existing architecture checker implements the RFC 0001 loader freeze
(`ARCH101`–`ARCH105`). Its current manifest and CI job do not yet implement all
the include-graph, target-graph, or editor contract checks proposed here. See
[archlint.py](../tools/archlint/archlint.py),
[modules.json](../architecture/modules.json), and
[tests.yml](../.github/workflows/tests.yml).

## Goals

- Make geometry, map data, edit operations, and file-format processing runnable
  without GTK, MFC, a display server, or a GPU.
- Preserve existing Source map semantics and clearly declare the supported
  editing and export feature set at every delivery stage.
- Centralize mutations, history, selection policy, validation, and asset
  identity so UI adapters share the same behavior.
- Define narrow contracts with executable substitution tests.
- Support GTK4/libadwaita as an editor shell with multiple documents, multiple
  viewports, configurable shortcuts, inspectors, asset browsers, and build logs.
- Reuse suitable Source libraries without importing their ambient globals or
  native presentation assumptions into new editor contracts.
- Establish an auditable migration inventory, dependency ratchet, and retirement
  protocol that continue to work as the codebase evolves.

## Non-goals

- Completing a GTK port or the full RFC 0001 platform migration in this RFC.
- Rewriting the renderer, compilers, or all geometry algorithms together.
- Changing BSP, VMF, network, or engine content limits as part of extraction;
  the native USD source path is separately gated by RFC 0009.
- Making all map objects share the same editing capabilities.
- Building a generic UI toolkit abstraction that mirrors MFC and GTK widgets.
- Introducing a plugin ABI, scripting language, ECS, or dependency injection
  framework as a prerequisite.
- Promising Windows/macOS GTK delivery or full legacy feature parity before
  those product compositions have passed their own gates.

## Normative language and evidence

MUST, MUST NOT, SHOULD, and MAY express requirements, recommendations, and
options for implementations of this RFC. Proposed names are illustrative unless
explicitly marked as an artifact path or stable identifier.

Every finding in the migration inventory MUST be marked as one of:

- **Observed:** supported by a source location or a recorded execution.
- **Hypothesis:** a suspected behavior or dependency requiring characterization.
- **Decision:** an intentional target design, with a testable acceptance rule.

A no-op virtual method, type test, or downcast is a review signal, not sufficient
proof of an LSP violation. An optional render hook can legitimately do nothing.
A violation requires a mismatch against the actual base contract or a caller's
documented, valid expectation.

For example, `CMapAtom::SetParent(CMapAtom*)` accepts a parent pointer without a
type restriction in its implementation, while `CMapClass::SetParent` asserts
that a non-null parent is a `CMapClass`. This is an observed narrowing of the
accepted runtime type. It is an LSP problem if the broader base contract permits
those other parents; that implicit contract must first be characterized. The
target scene API will express valid parent/child relationships directly and
reject invalid reparenting before mutating the graph.

## Responsibility and factorization model

### Classification axes

Each source file has one architectural owner. A mixed file additionally has
symbol-level extraction entries. Classification uses five independent axes:

| Axis | Values or questions |
| --- | --- |
| Responsibility | Geometry, scene, editing, interaction, presentation, persistence, asset service, platform adapter, composition |
| State ownership | Authoritative document data, per-document session state, per-view state, derived cache, external resource |
| Effects | Pure computation, document mutation, file I/O, process execution, GPU work, native UI, asynchronous delivery |
| Factorization | Keep as value/function, extract cohesive service, split mixed responsibilities, adapt legacy implementation, replace shell, retire |
| Behavioral evidence | Declared contract, consumers, characterization fixtures, conformance tests, known gaps |

File names alone MUST NOT determine ownership. `dispsew.cpp`, for example,
contains geometry work and accesses the face-edit sheet; its operations need
symbol-level classification. Moving the entire file into a geometry directory
would preserve that dependency under a new name.

### Initial responsibility inventory

This table seeds the migration inventory; it is not a completed file-by-file
audit or a promise that each family compiles independently.

| Responsibility | Existing families | Intended factorization |
| --- | --- | --- |
| Geometry and spatial algorithms | `boundbox`, `brushops`, `clipcode`, `ssolid`, `stocksolids`, geometry portions of `mapface`, `mapsolid`, displacement/subdivision files | Extract value types and deterministic operations; pass tolerances and policy explicitly. |
| Scene identity, hierarchy, and content | `mapatom`, `mappoint`, `mapclass`, `mapworld`, `mapentity`, `mapgroup`, `mapface`, `mapdisp` | Separate persistent content from render caches, UI state, and file encoding. |
| Entity representations and instances | `MapHelper`, `maplight*`, `mapsprite`, `mapstudiomodel`, `mapoverlay*`, `mapinstance`, `helperfactory` | Distinguish authored records from derived representations; model instance references explicitly. |
| Document and workspace state | `mapdoc`, `manifest`, `Selection`, `history`, `visgroup`, `updatehint`, `events` | Extract document sessions, transactions, selection policy, and typed notifications. |
| Editing use cases | Operations in `mapdoc`, property pages, `SearchReplaceDlg`, `MapDiffDlg`, `mapcheckdlg`, texture/displacement dialogs | Move validation and mutation to application operations; retain display formatting in presenters. |
| Interactive gestures | `Tool*`, `tool*`, `box3d`, `gizmo`, `keyboard`, handle classes | Normalize input; share gesture mechanics; emit edits and overlay descriptions. |
| View and scene presentation | `mapview*`, `camera`, `axes2`, `Render`, `render2d`, `render3dms`, `culltreenode`, `RenderUtils` | Split projection, picking, scene extraction, frame scheduling, and renderer execution. |
| Asset discovery and metadata | `texturesystem`, `material`, `texture`, `wadtexture`, `dummytexture`, `studiomodel`, `sprite`, `soundsystem`, `gameconfig` | Separate catalog, metadata, decoded images, preview resources, and backend bindings. |
| Persistence | `loadsave_map`, `loadsave_rmf`, VMF methods spread through scene/document classes, `saveinfo`, `prefab*` | Extract codecs and import/export policies; centralize transactional save orchestration. |
| Build and preview jobs | `runcommands`, `runmap*`, `processwnd`, `bsplighting*`, `lpreview_thread` | Separate job plans, process providers, progress events, and optional preview services. |
| Platform integration | `FileChangeWatcher`, `shell*`, native paths/settings, clock and process calls | Use RFC 0001 contracts where available; contain temporary native adapters. |
| UI and lifecycle | `hammer`, `mainfrm`, `childfrm`, dialogs, property pages, bars, `VGuiWnd`, `HammerVGui`, PAK windows | Retain MFC compatibility host during extraction; create GTK widgets and presenters. |

### Existing libraries and reuse boundaries

`mathlib`, `bitmap`, `vtf`, `vpklib`, the FGD interfaces under `public/fgdlib`,
and chunk/token parsing are candidates for direct reuse or small adapters.
Material-system, model-cache, studio-render, sound, and lighting services require
explicit composition and ownership. `matsys_controls` and `vgui_controls`
contain presentation behavior and are not the new editor's shared domain layer.
Hammer's existing link dependencies are listed in
[hammer_dll.vpc](../hammer/hammer_dll.vpc).

Source availability is not proof of a supported standalone build. For each
reused library, record its target, toolchain, transitive headers, global
initialization requirements, resource lifetime, and tested configurations.
Do not add a whole tier library to a portable target simply to obtain one
utility if that introduces forbidden dependencies.

Editor and compiler geometry MAY share a kernel after tests establish matching
semantics. Identically named operations can differ in epsilon policy, degenerate
input handling, winding, or output ordering. Such differences MUST remain
explicit; deduplication must not change compiler behavior accidentally.

## Target modules and dependency direction

The following are proposed logical modules and Waf target families. Initially,
some may be logical boundaries within one library. A physical target split is
required when necessary to enforce independence or support another consumer.

| Module | Owns | Allowed editor dependencies |
| --- | --- | --- |
| `hammer.geometry` | Geometric values, algorithms, numeric policies | None |
| `hammer.scene` | Content graph, typed IDs, document metadata, invariants | `geometry` |
| `hammer.ports` | Asset, persistence, job, and external-service contracts | `geometry`, `scene` |
| `hammer.formats` | VMF/MAP/RMF codecs and format validation | `geometry`, `scene`, `ports` |
| `hammer.app` | Sessions, queries, edit transactions, history, save/build orchestration | `geometry`, `scene`, `ports` |
| `hammer.viewport` | Projection, picking queries, presentation extraction, render contracts | `geometry`, `scene`, `ports` |
| `hammer.tools` | Gesture state machines and edit/overlay production | `geometry`, `scene`, `ports`, `app`, `viewport` |
| `hammer.presenters` | Inspector/browser/view presentation models and action routing | `app`, `tools`, `viewport`, `ports`, `scene` |
| `hammer.adapters.source` | Source asset and renderer integrations | Relevant contracts in `ports`, `viewport`, `scene`, `geometry` |
| `hammer.adapters.platform` | Files, settings, processes, clocks, watchers, scheduling | `ports` |
| `hammer.adapters.mfc` | Existing shell and compatibility glue | `presenters`, `app`, `tools`, `viewport`, `ports`, `scene` |
| `hammer.adapters.gtk` | GTK4/libadwaita widgets, input and main-context dispatch | `presenters`, `app`, `tools`, `viewport`, `ports`, `scene` |
| `hammer.interop.gtk_source` | Private GTK/render-resource exchange | `viewport`, relevant GTK and Source adapter endpoints |
| `hammer.composition` | Provider selection, wiring, startup/shutdown | All selected modules |

All modules may use explicitly approved foundation vocabulary and utilities.
This permission is not a blanket dependency on every numbered tier. External
libraries and private provider endpoints are allowlisted separately in the
architectural manifest.

Key constraints:

- `scene` cannot call `app`; `app` cannot depend on `tools` or presenters.
- `formats` implements persistence ports; `app` consumes those ports and does
  not select codecs by constructing their implementations.
- `viewport` does not fetch the active document or manipulate widgets. Its
  inputs are explicit document/presentation snapshots and view state.
- `tools` consumes application edit services and viewport queries. Application
  code receives tool results as values or callbacks at composition time.
- GTK and MFC adapters do not depend on each other's native APIs.
- Only the private interop module may include both GTK graphics internals and
  Source renderer implementation details.
- Test fakes implement the same ports as production providers. They do not
  depend on application singletons or use production-only setup backdoors.

The module graph MUST be acyclic. Data and events may flow in both directions
through a contract without creating a reverse source dependency.

An initial directory layout may be:

```text
public/hammer/{geometry,scene,ports,app,viewport,tools,presenters}/
hammer/core/{geometry,scene,formats,app,viewport,tools,presenters}/
hammer/adapters/{source,platform,mfc,gtk}/
hammer/interop/gtk_source/
hammer/composition/
unittests/hammertest/
```

Legacy files remain in their current locations until their extraction is
verified. New strict directories do not inherit the legacy directory's debt.
Private implementation headers stay out of public include roots.

## State ownership and lifetime

| State | Authoritative owner | Derived or observing consumers |
| --- | --- | --- |
| Brushes, faces, displacement data, entity properties, connections | `MapDocument` content model | History, queries, codecs, render extraction |
| Persisted editor metadata, including supported visgroup/logical layout data | Document metadata model | UI and codecs |
| Current selection, active edit transaction, history position | Per-document `EditorSession` | Tools and presenters |
| Camera, projection, viewport layout, hover and transient handles | View/session state with explicit persistence policy | Viewport controller and widgets |
| Pending text/property edits | Presenter draft with explicit commit/cancel policy | Inspector widget |
| Asset identity and metadata revision | Asset catalog | Browser and render caches |
| GPU objects and material bindings | Renderer adapter scoped to its device | Frame execution |
| Build/preview job state | Job controller scoped to a workspace/document request | Progress and diagnostics presenters |

An `EditorSession` owns the document's application services. It is not a public
bag from which every consumer retrieves arbitrary providers. Constructors and
operations receive only the services they need.

Scene objects MUST NOT borrow a `CDocument`, GTK widget, property sheet, or
renderer-owned resource. Providers outlive their borrowers. Document closure
cancels or detaches outstanding work, invalidates delivery tokens, and releases
subscriptions before session destruction. A late result cannot access a closed
document or update a new document that reused an address.

Persistent VMF identifiers, runtime handles, history identity, and GPU pick
identifiers are distinct concepts. Runtime handles include a document identity
and a generation or an equivalent stale-reference check. GPU pick IDs index a
frame-scoped lookup table; they never contain truncated object pointers.

## Contracts and behavioral substitution

### Required contract record

Every new replaceable port MUST have a record beside its tests specifying:

1. Purpose, actual consumers, and required versus optional capabilities.
2. Accepted inputs, units, coordinate spaces, encoding, and validation limits.
3. Results, error taxonomy, and guarantees after success, failure, and cancel.
4. Ownership, borrowed-value validity, threading, ordering, and reentrancy.
5. State invariants and legal operation sequences.
6. Permitted observable side effects and performance requirements, where needed.
7. A conformance suite and the providers/configurations that execute it.

Every implementation must accept all inputs allowed by the contract, provide
at least its stated result guarantees, and preserve its invariants. Providers
must not add hidden requirements such as an active document, an MFC window,
prior singleton initialization, or a particular OS.

Validation failure and unsupported capability are explicit outcomes. A provider
must not silently succeed after dropping requested content or ignoring an edit.
An implementation that cannot meet a contract must implement a narrower
contract or be rejected when composing a required capability.

New contracts follow RFC 0001's foundation result and path vocabulary. The
C++20 target policy in RFC 0006 applies to new strict editor modules, with
compiler/standard-library evidence established in H0. Use project `Expected`
results, scoped ownership, strong IDs, and bounded borrowed views where useful.
Legacy MFC-facing headers retain their supported dialect until callers migrate;
new language/library types must not leak across an unchanged binary interface.
Snippets in this RFC describe semantics, not a required new ABI or permission
to upgrade every legacy toolchain.

### Scene and geometry contracts

Scene hierarchy owns valid parent/child relationships and graph invariants.
Reparenting validates document ownership, cycles, node compatibility, and
editability before changing either side of the relationship. On failure, the
graph is unchanged. Derived indexes are updated by the same committed change.

Authored content and derived entity helpers are separate representations. An
entity light-cone visualization is not automatically a persistable scene node.
An overlay's authored face references must not disappear merely because its
display geometry is regenerated. Characterization determines which fields are
authored for each helper family before extraction.

Narrow capabilities can be implemented by small interfaces, typed operations,
or composed value types. A variant/tagged record is appropriate for a closed
set of serialized node kinds. Virtual dispatch is appropriate when behavior
actually varies behind one stable contract. This RFC does not require a virtual
interface for every data structure or a universal runtime capability registry.

Transform operations declare supported node kinds and validate the complete
selection before committing. Policies such as texture lock, pivot, snapping,
instance-local coordinates, and invalid-brush handling are explicit inputs.
Geometry functions state tolerances and degeneracy handling. Extraction MUST
preserve existing precision until a separate behavior change is approved.

### Edit transactions and history

All migrated edits go through a document-scoped transaction boundary. A
transaction validates the request, computes or records a change, commits it,
updates indexes, records history, and publishes a typed change event.

The conceptual protocol is:

```text
request + expected document revision
    -> validate and prepare
    -> commit reversible change record
    -> update document revision and derived-index invalidation
    -> publish one committed change batch
```

Failure before commit leaves content and history unchanged. No-op requests do
not add history entries or mark the document modified. A gesture can preview
many intermediate positions, but one completed drag produces one history unit.
Cancel restores the pre-gesture state and creates no committed content edit.

Undo restores recorded state or an equivalent lossless patch; it MUST NOT rely
on numerically inverting arbitrary geometry operations. Redo reapplies the
recorded result with stable object/reference semantics. Tests distinguish a
monotonic revision counter from the save/history position used to determine
whether content is modified. Undoing to the saved state can clear the modified
indicator even though the revision counter has increased.

`CHistory` may remain behind a compatibility adapter initially. Centralizing
the mutation entry point precedes replacement of its storage. A migration must
have one history authority; duplicating old and new undo stacks for the same
live document is forbidden.

Cross-document operations, including instance and manifest edits, either declare
and implement a transaction spanning the affected documents or fail before
partial mutation. They must not imply atomicity that their storage lacks.

### Selection and property editing

Selection policy belongs to the application layer. The UI owns a property draft
until commit. An action that changes selection first resolves that draft using
an explicit policy: commit valid data, report validation failure, or discard by
a user action. The policy is shared by menu, keyboard, and pointer entry points.

This preserves the behavior currently hidden in
`CSelection::SelectObject` calling the property sheet's `SaveData()`, while
removing the dependency from selection to the window. Multi-selection mixed
values remain distinguishable from empty strings and unset properties.

### Input and tool contracts

Native input adapters convert events once into editor input values. Shared
values distinguish pointer button/phase, logical keys, text/IME input, modifiers,
wheel units, view coordinates, timestamps, and cancellation/focus loss.

Tools receive only the viewport query and edit capabilities they use. Spatial
views expose projection and ray construction. A logical entity graph exposes
graph-space picking. A 2D point does not have a unique 3D inverse without depth
or an intersection plane, so the new contract MUST NOT promise a universal
`ClientToWorld` operation with unspecified semantics.

Shared input routing replaces the repeated 2D/3D/logical transport in
`CBaseTool`. It does not force identical gestures in different spaces. Spatial
and logical behaviors can be composed or registered separately.

Each tool declares its supported interaction contexts before activation. Event
results explicitly report ignored, handled, capture requested, capture released,
or edit failure as applicable. Ignoring an irrelevant event is valid; returning
success for an unsupported edit is not. Pointer capture is a lifetime-bound
request to the host. Focus loss, Escape, document closure, or capture loss must
complete or cancel the gesture according to a tested state machine.

Tools emit edits, overlay descriptions, cursor intent, and status values. They
do not paint widgets, open modal dialogs, or read key state from Win32 globals.
Shared snapping, drag thresholds, transform constraints, and hit ordering have
one policy owner and are reused across tools.

### Persistence contracts

Codecs consume/produce document data plus diagnostics. They do not select files,
show dialogs, set global active documents, or mutate the current document during
an incomplete parse. Import builds a detached candidate, validates it, and
publishes it only after success.

The supported format contract includes entity keyvalues and connections, brush
planes and texture axes, face/object IDs, displacements, overlays, instances,
visgroups, and applicable editor metadata. Tests cover duplicate keys and
ordering wherever those are semantically significant.

Round-trip tests compare semantic content using a versioned comparator. Byte
equality is required only where the format policy explicitly promises it.
Normalization rules must be enumerated; a comparator cannot discard difficult
fields to make a migration pass.

Unknown fields/chunks are preserved in an extension representation where this
can be done safely. If an edit invalidates retained opaque references, or an
export format cannot express content, the exporter returns diagnostics and
requires an explicit lossy-export policy. Unsupported content must not vanish
under an ordinary successful save. Full-fidelity editing is enabled per feature
only after its load/edit/save fixtures pass.

Save orchestration writes a temporary file, validates write completion, and
uses the filesystem provider's declared replacement semantics. The contract
distinguishes atomic replacement from crash durability and unsupported native
guarantees. Failure preserves the prior file where the provider promises that
property. Autosave targets recovery storage and does not silently advance the
ordinary save position. Multi-file manifest saves need a recoverable protocol
and failure reporting rather than an unsupported claim of global atomicity.

### Assets and asynchronous jobs

Replace `IEditorTexture`'s combined responsibilities with asset metadata,
decoded-image access, thumbnail generation, and private render bindings.
Missing, loading, ready, unsupported, and failed are distinct states. A missing
material retains its authored name and produces a visible placeholder; it
must not masquerade as a loaded resource.

Image and mapping dimensions can differ for materials. The existing
`GetImageWidth`/`GetWidth` pair must be characterized before deduplication; a
comment suggesting removal is not evidence that the values are equivalent.

Background results carry document/session identity, source revision, and request
identity. Delivery checks these before updating caches or presenters. Work runs
on RFC 0001 task sequences; widgets are updated on the UI sequence. Cancellation
has declared best-effort interruption semantics and guarantees no unsafe late
delivery. Providers report failure as values rather than displaying UI.

Build jobs use structured executable, argument, working-directory, and
environment values. Legacy command presets are migrated with fixture tests;
their quoting and special copy/delete/rename behavior is not reinterpreted by
ad hoc shell concatenation. The job controller captures output and exit status,
reports cancellation, and invalidates stale results after subsequent edits.

### Rendering and host contracts

Split viewport model, projection/picking, scene extraction, renderer execution,
and presentation ownership. A render snapshot contains stable IDs and derived
geometry/material requests, not borrowed widget or mutable scene pointers.
Persistent caches are keyed by document/content revisions and asset revisions.

The common render contract declares a baseline feature set. Textured rendering,
studio models, and lighting previews are separately negotiated features. A
wireframe-only provider cannot substitute for a provider required to render
Source materials faithfully. A headless recorder can satisfy a command-recording
contract; it cannot falsely claim to produce equivalent pixels.

GTK4's `GtkGLArea` owns a GL context and framebuffer and presents its output
through GTK. This is a candidate first bridge, subject to an integration spike.
The Source material system's existing context, state cache, framebuffer, and
swap ownership cannot be assumed compatible simply because both use OpenGL.

Two candidate bridge implementations are allowed:

1. Execute the adapted renderer into the GTK-provided context/target during
   the permitted render callback, with explicit state and lifetime handling.
2. Render offscreen and transfer or share completed image resources with GTK
   using verified context/device interoperability and synchronization.

The bridge owns native details. No `HWND`, SDL pointer, `GdkGLContext`, framebuffer
ID, or generic native-handle query enters scene, application, tool, or portable
render contracts. This specializes RFC 0001's pair-specific presentation bridge;
it does not introduce a competing window abstraction.

The spike must validate multiple viewports, resource sharing, target restoration,
resize, zero-sized/hidden views, drawable versus logical size, scale changes,
context loss/recreation, and ordered shutdown. It must test both X11 and Wayland
for the proposed Linux delivery profile. Relative mouse navigation and capture
also require explicit verification; native cursor warping is not assumed.

When GL textures are wrapped as GTK textures, resources remain immutable for
the published texture lifetime or are transferred through a documented buffered
handoff. Completion and release synchronization are mandatory. CPU readback may
serve as a diagnostic fallback but needs a measured budget before product use.

GTK/libadwaita owns windows, actions, menus, split panes, dialogs, accessibility,
focus, and widget styling. Shared presenters own display-ready editor state and
route actions. We do not create wrappers for every toolkit widget. Dense editor
controls and docking requirements are validated with real workflows; a full
arbitrary docking system is a separate feature decision.

## Mechanical evolution and enforcement

### Versioned migration artifacts

Phase H0 introduces the following artifacts. They are proposed paths, not files
created by adopting this document alone.

| Artifact | Authority and contents |
| --- | --- |
| `architecture/modules.json` | Extend the existing versioned manifest with editor ownership, allowed edges, public/private include roots, external libraries, and target mapping. |
| `architecture/hammer_inventory.json` | Exhaustive owned-file inventory plus symbol-level splits for mixed files, responsibility/effect/state classification, provenance, and extraction destination. |
| `architecture/hammer_migrations.json` | Stable migration IDs, dependency DAG, contract IDs, caller sets, phase/status, test selectors, exception references, and retirement conditions. |
| `architecture/hammer_baseline.json` | Exact editor-specific violations accepted from the legacy snapshot; existing loader debt remains in the existing loader baseline. |
| `architecture/hammer_compatibility.json` | Declared features, supported formats, provider/profile combinations, corpus IDs, and limitations for each delivery gate. |
| `unittests/hammertest/contracts/` | Contract records, provider-parameterized conformance tests, and negative provider fixtures. |
| `unittests/hammertest/fixtures/` | Small, redistributable or generated maps, golden observations, seeds, and versioned semantic expectations. |
| `RFC/0002-progress.md` | Human-readable gate decisions and links to evidence; inventory/CI artifacts remain authoritative for machine facts. |

The module manifest is the sole authority for dependency permissions. Migration
records reference module IDs rather than maintaining another list of allowed
edges. Schemas have explicit versions and migration code; unrecognized versions
fail validation. JSON arrays and generated reports use deterministic ordering.

Each source file must resolve to exactly one owner. Unowned and ambiguously owned
files fail inventory validation. Mixed files remain assigned to a legacy owner
until their last forbidden dependency is removed; per-symbol destinations track
the work without pretending the entire file is portable.

A migration record has at least these fields:

```json
{
  "id": "HAM-SEL-001",
  "title": "Move property-draft commit policy out of selection",
  "responsibility": "selection-policy",
  "evidenceKind": "observed",
  "sources": [
    {"path": "hammer/Selection.cpp", "symbol": "CSelection::SelectObject"}
  ],
  "destinationModule": "hammer.app",
  "contractIds": ["selection.change.v1", "property_draft.commit.v1"],
  "dependsOn": ["HAM-BUILD-001", "HAM-SESSION-001"],
  "status": "inventoried",
  "authority": "legacy",
  "consumerSet": "selection-entry-points",
  "testSelectors": ["selection/property_draft", "selection/two_documents"],
  "retireWhen": [
    "all selection entry points route through the application policy",
    "no property-sheet access remains in extracted selection code"
  ]
}
```

This is an illustrative record, not a claim that these contracts or tests exist.
The implemented schema must also provide an accountable owner, baseline revision,
evidence references, and any accepted deviations. Status changes require test
evidence for the current implementation revision; a manually edited status is
not sufficient to pass a gate.

### Migration state machine

Each bounded extraction moves through these states:

| State | Required evidence to enter | Exit condition |
| --- | --- | --- |
| `inventoried` | Resolved source/symbol ownership, consumers, dependencies, effects, and baseline observation | A bounded contract and fixtures are identified. |
| `characterized` | Existing behavior recorded; uncertain or unavailable behavior explicitly listed | Review accepts the contract and any intentional differences. |
| `isolated` | A narrow facade routes consumers; behavior remains in the legacy implementation | Facade callers and remaining bypasses are enumerated. |
| `extracted` | One shared implementation owns the behavior; independent target/header checks pass | Legacy host invokes that implementation through an adapter. |
| `substitutable` | Required providers pass the same contract tests; failure/lifetime cases pass | Product workflow gates pass with selected providers. |
| `cutover` | All intended callers select the new authority; measured rollout evidence exists | Old implementation and compatibility consumers are zero. |
| `retired` | Old code/glue and stale exceptions/baseline entries removed | Regression and architecture checks continue to pass. |

An explicit `blocked` annotation records a missing toolchain, fixture, provider,
or unresolved contract. It does not skip a state. Reopening a completed migration
requires a new contract revision or a recorded regression investigation.

Only one implementation is authoritative for a given live document operation.
Read-only shadow comparison may execute old and new algorithms against copies
or immutable snapshots. Shadow execution must not write files, allocate persistent
IDs in the live document, execute external commands, or apply mutations twice.

### Standard change protocol

Every implementation change that crosses a boundary follows this sequence:

1. Select a migration ID and verify the relevant baseline revision and caller
   list. Confirm no prerequisite gate is represented only by a future plan.
2. Add or identify behavior fixtures before changing the implementation. Record
   unavailable legacy behavior as a gap rather than fabricating an oracle.
3. Introduce the narrow seam and route a bounded caller set through it. Do not
   change behavior and relocate large amounts of code in the same review unit.
4. Extract cohesive logic while preserving its semantics. Keep compatibility
   types and global lookups on the legacy side of the seam.
5. Compile the extracted module without the legacy PCH and with only approved
   include/link roots. Run behavior and contract tests through real consumers.
6. Switch the declared callers, remove duplicate authority, and update the
   migration record and exact baseline reductions.
7. Delete obsolete implementations and temporary adapters once their consumer
   count is zero. Verify no build scripts or generated projects still reference
   them.

A review unit should contain one contract change or one caller cohort. Numeric
file-count limits are less useful than a demonstrable boundary and rollback.
Mechanical rename/include-case fixes are submitted separately when possible.

A rollback changes composition or reverts the bounded extraction. It does not
retain two synchronized mutable models indefinitely. File-format changes require
a separate compatibility decision; ordinary extraction does not create a new
format that prevents rollback to the previous editor.

### Architecture rules

Extend `tools/archlint` rather than introducing a competing dependency policy.
The following proposed rule IDs belong to an editor range; implementation must
check that the IDs remain unallocated when the rules land.

| Proposed rule | Rejects | Enforcement |
| --- | --- | --- |
| `HAM001` | Missing/ambiguous file or target ownership | Inventory plus resolved build graph |
| `HAM002` | Forbidden module edge or architectural cycle | Includes, compiler dependency output, target/link graph |
| `HAM003` | Native/toolkit types, headers, or platform-selection branches in strict core/contracts | Lexical fast checks, hermetic header builds, compiler-grounded inspection |
| `HAM004` | New ambient application/document/service access in extracted code | Lexical and symbol-aware checks for known accessors and globals |
| `HAM005` | Migrated document mutations bypassing the transaction API | API visibility, symbol-aware call checks, mutation/revision tests |
| `HAM006` | Provider-private native interop escaping its declared bridge | Header visibility and dependency checks |
| `HAM007` | New dependencies on retired APIs or an empty adapter scheduled for deletion | Symbol references, consumer ledger, target graph |
| `HAM008` | Invalid migration state, missing evidence/test selector, stale exemption, or stale baseline | Schema and evidence validation |
| `HAM009` | Pointer-to-pick-ID narrowing in migrated picking paths | Compiler warnings/static analysis and round-trip handle tests |

Fast scans give early feedback but do not prove dependency or mutation safety.
Full checks use actual preprocessor configurations and dependency files, and
symbol analysis where a rule requires it. Conditional code is covered across
the supported build matrix. A header-only smoke compile cannot prove that a
linked implementation is portable; target and link dependencies are checked too.

For transaction authority, the strongest enforcement is construction and access
control: migrated mutation functions require an edit context or are private to
the document operation layer. Legacy bypasses are recorded individually and
removed by caller cohort. Regex matches alone cannot certify this invariant.

### Exact ratchets and exceptions

Baseline entries identify a rule, module, repository-relative path, containing
symbol where available, dependency/call target, normalized evidence, and
occurrence identity. Line numbers are diagnostic metadata, not stable identity.
Repeated identical occurrences need distinct ordinals or syntax identities so
adding another copy cannot hide behind an existing exemption.

The rules are:

- New strict modules have zero baseline debt.
- Legacy violations may remain only if they match accepted exact entries.
- Removing a violation removes its baseline entry in the same change.
- A move is not automatically grandfathered. An approved relocation may update
  one exact entry with an old/new mapping and unchanged dependency, but cannot
  move debt into a strict module.
- A declining total count does not offset a new violation elsewhere.
- Broad path exclusions, wildcard API exemptions, and blanket suppression of a
  rule are not accepted migration mechanisms.
- A true exception names a source, dependency/rule, owner, reason, tracked
  migration, and expiry or objective removal condition.
- CI verifies baselines; it never rewrites them automatically to accept a PR.

The existing loader baseline remains authoritative for loader rules. Editor
rules must not allow a new `CreateInterfaceFn` or module-loading boundary merely
because it sits in a compatibility adapter. Any movement affecting the loader
freeze requires the same explicit retirement classification as RFC 0001.

### Duplicate knowledge and semantic drift

DRY progress is measured by authoritative owners and routed callers. Clone
detection is a review aid, not a rule that every similar block must be merged.

Track each shared policy, including selection resolution, grid snapping,
texture-lock transforms, property validation, asset-path normalization, and
save/build configuration. Record which entry points still implement it locally.
A policy is extracted only when the migrated entry points delegate to its owner.

Do not merge editor and compiler code, 2D and 3D interactions, or image/mapping
dimensions until their differences have been made explicit. Shared primitives
plus named policies are preferable when the knowledge is partly common.

### Tooling delivery and commands

The following commands already exist and verify the loader architecture work:

```sh
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
python3 -m unittest discover -s tools/archlint/tests -v
```

Editor inventory validation, compiler-dependency checks, Waf ownership checks,
and contract runners MUST be implemented with fixture tests before their gates
become mandatory. Their final commands are recorded in `0002-progress.md` when
they exist. In particular, the current archlint parser does not provide the
`--compile-deps` option proposed in RFC 0001.

Enforcement lands in three increments: deterministic inventory/lexical ratchets;
hermetic header and resolved target checks; compiler-grounded transitive and
symbol checks. Each increment records the blind spots left to the next. No gate
claims a stronger guarantee than its installed checks provide.

## Verification strategy

### Characterization corpus

Start with small fixtures that isolate a behavior and a separately identified
set of representative real maps. Fixtures record their provenance, applicable
game profile, feature tags, expected diagnostics, and comparator version.
Generated cases record seeds. Product assets needed for rendering tests are
declared separately from redistributable headless fixtures.

Minimum coverage includes convex and invalid brushes, clipping and transforms,
texture axes/locks, displacements and sewing, entity input/output connections,
groups and visgroups, overlays, nested instances, unknown VMF content, missing
assets, malformed files, and differing path case/encoding. Tool traces cover
commit, cancel, focus/capture loss, document switch, and undo/redo.

A legacy result is evidence, not automatically the desired specification. An
observed bug is recorded as a separate behavior change with its own expected
result. Updating a golden output cannot silently redefine the contract.

### Test obligations by boundary

| Boundary | Required evidence |
| --- | --- |
| Geometry | Example fixtures, degeneracy/tolerance cases, bounds/convexity invariants, deterministic seeded tests |
| Scene | No cycles, valid references, atomic reparent/delete, stale handle rejection, independent documents |
| Transactions/history | Failure atomicity, no-op behavior, one drag/one history unit, cancellation, lossless undo/redo, saved-position behavior |
| Codecs | Supported-feature semantic round trips, explicit loss reports, malformed-input handling, legacy/compiler compatibility |
| Selection/presenters | Same action semantics from different entry points, property-draft resolution, mixed-value handling |
| Tools/hosts | Same normalized event traces, declared context support, DPI/coordinate conversion, capture cancellation |
| Assets/jobs | Missing/error states, revision invalidation, reordered delivery, cancellation, close-before-completion |
| Rendering | Pick identity, viewport isolation, resize/scale, resource ownership, baseline feature fidelity, device/context lifecycle |
| Files/processes | Save failures, replacement guarantees, native/virtual path behavior, argv fidelity, exit status and cancellation |

Provider conformance tests are parameterized over every implementation claiming
the contract. Fakes run the same suite, plus relevant real-provider integration
tests. Negative providers that violate a precondition, result guarantee,
callback ordering, or ownership rule verify that the suite can detect mistakes.

For numeric comparisons, tolerances are set per operation and representation.
Do not use one broad epsilon to hide changed geometry. Undo compares restored
authored values exactly where the representation permits it; rendered pixels
can use documented device-specific tolerances. Semantic tests remain the primary
oracle when screenshots cannot distinguish data loss.

### Executable harness and oracle gates

RFC 0005 owns shared execution, evidence, failure injection, and CI mechanics.
This RFC owns Q-EDITOR's document semantics and its Q-CONTENT and Q-PRESENTATION
acceptance criteria. Test selection must reconcile required features/providers
with actual executed cases; absent assets, missing selectors, or an unavailable
required host cannot produce a successful gate.

The headless reference workflow is open VMF, resolve a property draft, select,
transform, undo, redo, save, reopen, and compile with the selected tools. Compare
authored fields/references, selection, history position, modified state, emitted
change batches, and diagnostics. Use generated seeded action sequences against
a small independent document/history model. Tests must exercise two documents,
stale generations, invalid mixed selections, failure before commit, and
completion delivered after cancel/close/revision change.

The semantic comparator has its own negative corpus: omitted unknown chunks,
changed texture axes, duplicate-key/order changes, broken IDs/references, and
lost displacement/overlay/instance metadata must be detected. Enumerate allowed
normalization instead of removing hard-to-compare fields. Use independent
reader/compiler acceptance as well as round trips so matching codec bugs cannot
certify data preservation. Fuzz malformed input and inject write/replacement
failures; distinguish atomic replacement, durability, and multi-file recovery.

Replay applicable tool/action traces through headless, MFC, and GTK providers.
Do not fabricate MFC observations when the baseline cannot run. Native GTK tests
add actual input delivery, DPI, relative navigation, focus/capture loss, multiple
viewports, and X11/Wayland coverage. R1 requires real Source materials, GPU
completion/resource ownership, and context/framebuffer restoration. A headless
recorder proves only its command contract. Buffered texture handoff and ring
reuse follow RFC 0006's completion rules.

Product tests close documents while asset/compile work remains pending and
verify argv, cancellation, stale delivery, and resource cleanup. Required
compiler availability and content profiles are gate inputs, not assumptions
based on source files being present in the repository.

### CI tiers

1. Every change: schemas, inventories, exact ratchets, dependency DAG, contract
   header compilation, affected headless unit/contract tests.
2. Every relevant merge: complete supported core toolchain matrix, codec corpus,
   sanitizer runs where supported, legacy/new behavior comparisons, normalized
   tool traces, and full transitive dependency checks.
3. Render or UI changes: software-renderer smoke tests plus a declared hardware
   matrix, GTK X11/Wayland integration, scale/resize tests, and workflow checks.
4. Release candidate: representative large maps, required compile tools, recovery
   tests, performance budgets, and explicit feature/profile acceptance.

Phase H0 selects and records the actual toolchains. Linux 64-bit headless core
is the initial portability target; the known working legacy Windows profile,
if established, is the compatibility target. Existing repository CI for the
engine does not imply Hammer has passed those configurations.

Performance gates use a fixed corpus, hardware/driver description, repetitions,
and baseline artifacts. Track load/save time, peak memory, selection/picking
latency, gesture response, frame time, and asset-browser responsiveness. Set
numerical budgets during characterization. A percentage without a measured
baseline is not an acceptance criterion.

## Delivery phases and gates

The phases form a dependency DAG. Renderer feasibility can be investigated
after H0 while core extraction proceeds, but GTK feature delivery depends on
the relevant core and rendering gates. This is not a requirement to complete
all engine platform refactoring first.

The cross-RFC portfolio order and tracked state live in [AGENTS.md](../AGENTS.md).
The phases here retain their domain exit criteria. H0 includes trustworthy test
discovery/evidence, a versioned comparator with negative fixtures, and an explicit
C++20/legacy toolchain boundary. H2–H4 require the executable headless workflow
and model-based sequences above; R1/H5 require native-host evidence; H6/H7 require
the complete declared feature corpus, budgets, recovery, and consumer retirement.

| Phase | Deliverable | Entry condition | Exit gate |
| --- | --- | --- | --- |
| H0: Baseline and enforcement | Build evidence, exhaustive inventory, migration schema, initial corpus, lexical ratchet | RFC accepted for implementation | Legacy build status and gaps recorded; strict target can compile/test; inventories reproduce; seeded boundary violations fail. |
| H1: Geometry and scene seams | First pure geometry operations, narrow scene mutation/query boundary, headless fixtures | H0 | Extracted targets build without UI/PCH/GPU dependencies; chosen invariants and two-document tests pass; legacy callers use the shared seam where runnable. |
| H2: Persistence slice | VMF reader/writer for declared features, detached import, safe save orchestration | H1 plus characterized format corpus | Supported-feature round trips pass; unknown/loss behavior explicit; failed saves/imports preserve prior state; compiler acceptance checked for selected toolchain. |
| H3: Application authority | Session, property commit policy, selection, transaction/history facade | H1 and sufficient H2 fixtures | Select/transform/undo/save sequence passes headlessly and through legacy adapter; migrated operations have one mutation/history owner. |
| H4: Interaction and presenters | Selection and block tools, input adapters, inspector and asset models | H3 | Common event traces produce equivalent supported edits; cancel/focus/close cases pass; widgets are absent from tools/presenters. |
| R1: Renderer bridge feasibility | One real Source-material viewport in a GTK host | H0 and bounded renderer contract | Target/context ownership, materials, resize, scale, input capture, and shutdown demonstrated on selected X11/Wayland profiles. |
| H5: GTK workflow slice | Open/save VMF, multiple views, selection/transform/history, entity inspector, textures, compile/run; keep document and tools format-neutral | H2–H4 and R1 | Representative compatibility map workflow passes with no hidden MFC runtime dependency; fidelity and unsupported-feature policy declared. RFC 0009's USD authoring gate remains separate. |
| H6: Feature families | Displacements, instances/manifests, advanced texture/overlay tools, previews, remaining workflows | H5 and per-family characterization | Each family passes load/edit/undo/save/build fixtures and lifecycle/performance gates. |
| H7: Retirement | Remove superseded implementations, adapter glue, and obsolete build paths | Declared product parity gate met | Legacy consumer counts reach zero; baselines/exceptions shrink to the accepted remainder; release/recovery evidence passes. |

H0 MUST resolve the legacy baseline honestly. If the historical MFC build cannot
be reproduced, record missing dependencies and retain source-based tests plus
any available reference outputs. Work can continue on independent extractions,
but claims of preserved legacy runtime behavior remain gated until supported by
evidence. No requirement here assumes the historical editor already builds.

R1 is a bounded uncertainty-reduction task. If adapting the current renderer
cannot meet the host contract at acceptable cost, record the measurements and
choose a separately reviewed rendering approach. A simple substitute viewport
may help test tools, but does not pass the Source-material fidelity gate.

## Worked migration: selection, property drafts, and transform

This is the recommended first application workflow after geometry/model seams
are available. It addresses an actual dependency rather than starting with a
large directory move.

1. Characterize selection changes with an empty, valid, and invalid property
   draft; with multi-selection; and across two documents. Record what currently
   commits, what remains pending, and what updates the modified indicator.
2. Add an application selection action that accepts an explicit draft-resolution
   result and selection request. The MFC presenter supplies the existing draft
   behavior through a temporary adapter.
3. Move selection resolution and selection-change events behind this action.
   Remove calls from extracted selection code to `GetMainWnd()` and property
   pages. Keep exactly one authoritative selection set.
4. Add a transform request with selected IDs, pivot, constraint/snap policy,
   texture-lock policy, and expected document revision. Adapt `CHistory` behind
   the transaction seam initially.
5. Replay a drag as begin/update/commit and begin/update/cancel. Verify one undo
   unit, stable face IDs, unchanged content after cancel, and no callback into a
   closed document.
6. Route menu, keyboard nudge, and tool drag through the same edit operation.
   These entry points may choose different explicit policies; they share the
   implementation of each policy.
7. Run the workflow through a headless presenter, the MFC adapter, and later the
   GTK adapter. Compare content, history, and declared selection outcomes rather
   than widget internals.
8. Remove the now-unused property-sheet hooks, bypass calls, and baseline
   entries. Record consumer counts and the passing workflow evidence.

This migration proves both desired properties: the edit policy has one owner,
and different presentation adapters satisfy the same application contract.

## Completion, maintenance, and change control

Each gate report records the code revision, toolchain/profile, fixture versions,
commands/test selectors, pass/fail results, performance measurements where
required, remaining exceptions, and the next dependency-ready migrations.
Evidence from an earlier revision is invalidated when the relevant contract,
provider, comparator, or consumer changes.

Track progress with the following metrics:

- Files/symbols classified versus unresolved, with inactive code distinguished.
- Exact forbidden edges and ambient-access occurrences remaining, by module.
- Migrated operations with exclusive transaction authority versus remaining
  bypasses.
- Shared policies with all known callers routed versus locally duplicated rules.
- Contracts with passing claimed providers and lifecycle/error coverage.
- Supported features with end-to-end corpus coverage.
- Compatibility adapters with remaining consumers and their removal conditions.
- Measured performance relative to the accepted corpus baseline.

Lines moved, number of interfaces, or a raw downcast count are not success
metrics. A type-specific operation on a typed node can be appropriate. A small
interface with undocumented lifetime behavior can still be unsafe to substitute.

A contract change includes its caller impact, provider impact, compatibility
decision, and updated conformance tests. First-party source APIs can evolve
through coordinated changes. Persisted data and genuine extension ABIs require
versioned compatibility plans under RFC 0001. Optional capabilities must not
silently become mandatory for previously supported profiles.

The dependency manifest and tests are reviewed as production architecture.
New features declare their owner and contract before introducing a new cross-
module dependency. Every accepted temporary exception has a scheduled review or
an objective deletion trigger; expired exceptions fail validation.

## Risks and mitigations

| Risk | Mitigation and decision point |
| --- | --- |
| Extraction changes undocumented editor behavior | Characterize workflows first; record intentional differences separately; compare authored state and tool outcomes. |
| A new facade preserves all the old dependencies | Enforce hermetic headers, transitive includes, target dependencies, and no ambient service access in strict modules. |
| Excessive interfaces add complexity without substitution value | Require an actual consumer and behavior contract; prefer values/functions for deterministic algorithms. |
| UI and model each become authoritative | Explicit ownership table, transaction-only migrated mutations, one selection/history authority, revision-tagged caches. |
| Format normalization hides data loss | Versioned semantic comparator, unknown-content fixtures, explicit lossy-export policy, compiler/legacy checks. |
| Renderer integration dominates delivery | Run R1 early, measure real materials and lifetime behavior, gate further integration on evidence. |
| Undo or clones leave dangling identity/reference state | Typed IDs, stale-reference checks, exact restoration fixtures, destruction/cancellation tests. |
| Legacy toolchains prevent useful comparisons | Resolve in H0; isolate unavailable evidence; continue only work whose claims can be verified. |
| DRY work merges different geometric policies | Preserve named tolerances and context-specific policy; require differential evidence before sharing kernels. |
| Long-lived adapters become a permanent second architecture | Consumer ledger, strict adapter directions, explicit retirement state, deletion gates. |

## Effort and planning assumptions

This is a substantial editor program. The early GTK shell is unlikely to be the
largest cost; behavior extraction, format fidelity, rendering interop, and the
long tail of editing workflows dominate uncertainty.

The initial planning range—two or three experienced engineers for
six to nine months to a useful alpha, twelve to eighteen months for a dependable
daily-use scope, and potentially eighteen to twenty-four months for broad
parity—should be treated as a planning hypothesis. It assumes Source/editor
experience, a usable reference build, accessible assets/toolchains, and a
bounded Linux-first product profile. These milestones are cumulative ranges,
not estimates to add together or commitments established by this RFC.

H0 and R1 must produce an updated estimate based on actual toolchain status,
renderer feasibility, corpus coverage, and the first completed extractions.
Subsequent forecasts use completed migration throughput and remaining feature
families. They must include testing, packaging, recovery, and performance work.

## Alternatives considered

### Direct widget-by-widget conversion

This can make selected dialogs appear quickly, but leaves model, history,
rendering, and input dependent on UI semantics. It is unsuitable as the governing
strategy. Individual UI screens may still be replaced after their behavior has
an extracted application owner.

### Complete editor rewrite

A rewrite permits clean boundaries but places all content and workflow semantics
at risk simultaneously. It remains an option for a bounded component whose
existing implementation is harder to isolate than to replace, provided the same
behavior and compatibility tests are used.

### A universal editor base class or service registry

This repeats the broad contracts and hidden dependencies already visible in
`CMapClass`, `CBaseTool`, and application globals. Explicit composition and narrow
consumer contracts provide clearer obligations and stronger enforcement.

### Finish the entire engine platform refactor first

This unnecessarily couples editor delivery to unrelated engine work. Hammer
should use RFC 0001's contracts and ownership rules as they become available,
with contained, tracked legacy adapters for the remaining services.

## Proposed decisions and initial implementation backlog

Acceptance of this RFC establishes the architecture and migration protocol; it
does not mark any delivery gate complete.

The initial backlog is listed below. The rendering investigation can start after
H0 without waiting for the later application extractions.

1. `HAM-BUILD-001`: record exact legacy build reproducibility and establish the
   initial headless editor test target/toolchain profile.
2. `HAM-INVENTORY-001`: implement exhaustive responsibility/effect ownership and
   migration schemas, with reproducibility and coverage checks.
3. `HAM-RATCHET-001`: add editor lexical rules and exact baselines, preserving the
   existing loader freeze; add negative checker fixtures.
4. `HAM-CORPUS-001`: establish minimal VMF/geometry/selection characterization
   fixtures and a versioned semantic comparator.
5. `HAM-GEOMETRY-001`: extract a bounded geometry operation and compile it without
   MFC/PCH dependencies; route its existing callers through the shared owner.
6. `HAM-SESSION-001`: introduce the per-document application seam and explicit
   state ownership; pass two-document independence tests.
7. `HAM-SEL-001`: execute the worked selection/property/transform migration.
8. `HAM-RENDER-001`: run R1 after H0 and a minimal render contract are ready;
   record bridge feasibility and remaining renderer dependencies.

Before H5, record the supported game/content profile, precise GTK/libadwaita
minimum versions, required build-tool availability, material/model fidelity
scope, docking scope, and unsupported-feature behavior. These decisions can be
made from measured H0/R1 results without preventing independent core extraction.

## References

- [RFC 0001](0001-capability-based-platform-architecture.md): capability contracts,
  ownership, rendering/presentation bridges, result/path types, and enforcement.
- [RFC 0001 Phase A progress](0001-phase-a-progress.md): implemented loader freeze
  and its current limitations.
- [GTK4 GLArea](https://docs.gtk.org/gtk4/class.GLArea.html): widget-owned context,
  framebuffer, render callback, and lifecycle.
- [GDK GLTextureBuilder](https://docs.gtk.org/gdk4/class.GLTextureBuilder.html):
  GL texture import, owning context, synchronization, and texture immutability.
- [GTK4 Native](https://docs.gtk.org/gtk4/iface.Native.html): native surfaces and
  widget/native ownership.
- [libadwaita adaptive layouts](https://gnome.pages.gitlab.gnome.org/libadwaita/doc/main/adaptive-layouts.html):
  split views and utility-pane composition.

Toolkit references were consulted during the source assessment. Their current
documentation may describe APIs newer than the eventual minimum dependency
version; implementation must pin and test the selected versions explicitly.
