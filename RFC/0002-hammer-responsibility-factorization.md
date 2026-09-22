# RFC 0002: Hammer Responsibility Factorization and Incremental Modernization

- Status: Proposed
- Date: 2026-09-21
- Scope: Hammer, its reusable editor libraries, and its application adapters
- Related: [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md)
- Source inspection reference: `b5eb9915bdebf5af1045c5821d4ac3d9af15dbb2`
- Implementation status: This RFC specifies future work. It does not introduce the modules, contracts, tools, or CI gates described below.

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
- Changing BSP, VMF, network, or engine content limits as part of extraction.
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
chosen C++ dialect and any missing foundation primitives must be recorded before
implementation. Snippets in this RFC describe semantics, not a required new ABI
or permission to upgrade every legacy toolchain.

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
