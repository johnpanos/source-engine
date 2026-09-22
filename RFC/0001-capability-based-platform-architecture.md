# RFC 0001: Capability-Based Platform Architecture

- Status: Proposed
- Date: 2026-09-21
- Scope: Tier 0–3, application bootstrap, and platform backends

## Summary

Source currently organizes shared code into numbered tiers. The tiers provide a
useful approximate dependency order, but they also combine unrelated utility
code, platform adaptation, service discovery, lifecycle management, and mutable
global interface pointers.

This RFC proposes a capability-based architecture with one governing rule:

> Features depend on contracts. Platforms provide contracts. Applications
> compose providers.

Platform identity will be confined to backend implementations and application
composition. Portable engine code will depend on narrowly scoped capabilities
such as monotonic time, threading, virtual memory, dynamic-library loading, and
platform paths. Higher-level engine systems will continue to expose domain
contracts such as filesystem, input, rendering, and audio, but consumers will
receive their required contracts explicitly instead of discovering them through
tier globals.

The existing tier libraries and `CreateInterface` ABI will remain operational
during migration. A compatibility layer will populate existing globals for
legacy consumers while new and migrated code uses explicit capabilities.

Dynamic-library loading will be the first reference migration because it is a
small, well-defined platform behavior currently implemented in Tier 1 despite
being foundational to module and interface discovery.

## Motivation

The existing structure has several strengths:

- dependencies generally flow from higher-numbered tiers to lower-numbered
  tiers;
- application systems have explicit connect, initialize, shutdown, and
  disconnect stages;
- versioned interfaces permit modules to communicate across shared-library
  boundaries;
- Windows and POSIX implementations are already separated in some source files.

However, a tier currently serves three different roles:

1. a build and dependency level;
2. a collection of utilities with similar historical dependency weight;
3. a service locator that places discovered interfaces into mutable globals.

This makes the boundaries coarse and difficult to enforce. Tier 2, for example,
connects filesystem, material, input, network, model, queued-loading, and
Perforce services. Tier 3 connects rendering, VGUI, caches, video, physics,
sound-emitter, texture, and content-tool services. These systems happen to have
similar dependency depth, but they do not share one responsibility.

Portability is also not contained by Tier 0. Platform checks and native headers
appear in public Tier 0 and Tier 1 headers, Tier 1 implements native module
loading, and portable callers can inspect OS identity directly. Adding an OS
therefore requires auditing code across multiple tiers rather than implementing
one defined backend contract.

Finally, globals such as `g_pFullFileSystem`, `materials`, and `g_pInputSystem`
hide dependencies from APIs. They complicate testing, permit partially
connected states, and rely on process-wide initialization order. Connect
functions do not describe which capabilities are required, and missing
interfaces are generally detected later by individual consumers.

## Goals

This RFC has the following goals:

- make the required capabilities of a subsystem visible in its construction or
  initialization API;
- confine native OS APIs, headers, and OS-selection macros to platform backend
  targets;
- allow a new OS to be added by implementing documented contracts and passing a
  shared conformance suite;
- make named targets such as Xbox, Windows PC, macOS, and Linux predefined
  compositions of the same capability system rather than separate engine-wide
  platform abstractions;
- distinguish platform capabilities from engine-domain services;
- select render backends independently from operating-system identity and split
  device, presentation, compilation, and feature-policy responsibilities;
- support changes to a presentation surface's drawable size at runtime, including
  phone and tablet orientation changes, without restarting the renderer;
- represent required and optional dependencies explicitly;
- preserve current module ABI and legacy callers during incremental migration;
- give client, dedicated-server, editor, and command-line applications distinct
  compositions without scattering platform checks through their features;
- improve unit testing through deterministic test providers;
- replace numbered dependency buckets over time with cohesive domain packages.

## Non-goals

This RFC does not propose:

- rewriting all tier libraries at once;
- immediately removing `CreateInterface`, `IAppSystem`, or versioned interfaces;
- replacing every compile-time option with runtime dispatch;
- making platform backends hot-swappable while the engine is running;
- defining a new renderer, filesystem, input, or audio API;
- wrapping language primitives or every operating-system call in a virtual
  interface;
- moving code merely to produce a new directory layout without improving its
  dependency boundary.

## Terminology

**Capability**
: A narrow contract representing behavior required by a consumer. A capability
  is named after what it does, not where it is implemented. Examples include a
  monotonic clock and dynamic-library loader.

**Provider**
: An implementation of one or more capabilities. A provider may be backed by an
  OS, a third-party library, an engine subsystem, or a deterministic test
  implementation.

**Platform backend**
: The set of providers selected for a supported platform. Only a backend may
  include native OS headers or branch on OS identity.

**Engine service**
: A domain-level contract such as `IFileSystem`, `IInputSystem`, or
  `IMaterialSystem`. Engine services may use platform capabilities but are not
  themselves part of the platform foundation.

**Composition root**
: Application bootstrap code that selects providers, establishes ownership,
  validates dependencies, and starts the application. Platform identity and
  product policy are permitted here.

**Compatibility bridge**
: Temporary code that adapts explicitly composed capabilities and services to
  existing global pointers and `Sys_*` functions.

## Design principles

### Depend on behavior, not platform identity

Portable code MUST NOT select behavior with `IsWindows()`, `IsLinux()`,
`IsOSX()`, `POSIX`, or equivalent OS tests. It MUST request the behavior it
needs through a contract.

For example, a consumer requests dynamic-library loading. The composition root
may satisfy it with a Win32, POSIX, console, or test provider. A renderer
requests a render-device contract; it does not infer OpenGL from POSIX.

Conventional platform labels are degenerate cases of this capability system.
Xbox, Windows PC, macOS, Linux, and other supported targets are named
composition presets that select a known set of providers. A constrained target
may have exactly one valid provider for a capability, so its selection can be
fixed at build time, but consumers still receive the same capability contracts
as targets with multiple runtime choices. A platform label may select defaults
in the composition root; it MUST NOT introduce a parallel `IPlatform`
abstraction or authorize platform branches in portable feature code.

Compile-time checks for compiler syntax, CPU instructions, structure layout,
and other properties that cannot reasonably be runtime capabilities remain
permitted in narrowly scoped foundation headers.

### Keep contracts narrow

There will be no monolithic `IPlatform` interface. Consumers should not acquire
unrelated authority merely because the platform implements it.

A subsystem that requires a clock and filesystem should receive those two
contracts. It should not receive a global capability bag from which it can
silently acquire additional dependencies.

### Compose centrally, inject locally

The application host owns long-lived providers. Individual subsystems borrow
only the capabilities they declare. A composition object may temporarily hold
the full provider set while building the application, but it MUST NOT become a
globally accessible service locator.

### Separate mechanism from product policy

Platform foundations provide mechanisms such as clocks, memory, modules, and
paths. Decisions such as enabling software lighting, choosing a renderer,
starting VGUI, or requiring a crash reporter belong to an application or
feature composition.

### Make unsupported behavior explicit

A provider MUST NOT silently emulate an unsupported capability unless the
contract specifies that behavior. Optional capabilities are represented as
optional during composition; required capabilities cause startup to fail with a
diagnostic naming the missing requirement and its consumer.

### Preserve deterministic lifecycle

Providers are constructed before their consumers, initialized in dependency
order, shut down in reverse order, and destroyed only after all borrowers have
stopped. Interfaces returned by a dynamic module cannot outlive the module
handle that owns their code.

## Proposed architecture

The target dependency structure is:

```text
                  Application composition
                           |
         +-----------------+-----------------+
         |                 |                 |
      Rendering            UI             Tooling
         |                 |                 |
         +---------- Engine services --------+
                           |
                   Portable core code
                           |
                  Platform contracts
                     ^             ^
                     |             |
              Windows backend   POSIX backend
                                    ^
                                    |
                               New-OS backend
```

Platform backends are vertical adapters. Named platform targets, including
Xbox, Windows PC, macOS, and Linux, are presets over these adapters and may
reuse providers where their contracts and implementations match. They do not
form another horizontal tier that feature code may include.

An initial directory structure may be:

```text
public/platform/contracts/
    clock.h
    dynamic_library.h
    threading.h
    virtual_memory.h
    paths.h
    process.h

platform/backends/windows/
platform/backends/posix/
platform/backends/test/

public/services/
    filesystem/
    input/
    rendering/
    audio/

features/
    vgui/
    studio_render/
    choreography/
    content_tools/
```

The exact directory names may change during implementation. The dependency
rules are normative; the proposed names are not.

## Capability contracts

### Initial foundation capabilities

The first platform contract set SHOULD cover behavior already implemented in
Tier 0 or the platform-specific portion of Tier 1:

| Capability | Responsibility |
| --- | --- |
| Monotonic clock | Monotonic timestamps, elapsed-time conversion, and resolution |
| Wall clock | Civil/UTC time where required; kept separate from monotonic time |
| Threading | Thread creation, identifiers, naming, priority, sleep, and synchronization backends |
| Virtual memory | Page size, reserve, commit, protect, decommit, and release |
| Dynamic library | Load, symbol lookup, and unload with structured errors |
| Platform paths | Executable, user-data, temporary, and native-library paths |
| Process environment | Arguments, environment variables, process identifiers, and debugger state |
| Diagnostics | Native debug output and optional crash/minidump integration |

Not every existing free function must become a virtual method. Atomics, CPU
pause instructions, endian helpers, fixed-width types, and other low-level
operations may remain compile-time implementations when runtime substitution
does not improve portability or testing.

### Domain capabilities

Filesystem, input, rendering, audio, networking, UI, and content-processing
contracts remain domain services. They should follow the same ownership and
injection rules, but they are not folded into a generic platform interface.

This distinction lets a dedicated server omit windows, clipboard, rendering,
and local input while still using the same clock, threading, memory, module, and
network foundations.

### Required and optional capabilities

Requirements MUST be validated at composition time.

Required dependencies are represented by references or an equivalent non-null
handle in consumer APIs:

```cpp
class CModelCache
{
public:
    CModelCache( IFileSystem &files, IRenderDevice &renderDevice );
};
```

Optional dependencies use a nullable pointer or a project-compatible optional
handle and MUST have defined absence semantics:

```cpp
class CToolHost
{
public:
    CToolHost( IFileSystem &files, IClipboard *clipboard );
};
```

These examples are illustrative. Final interfaces must follow the C++ dialect
and ABI constraints of their target modules.

## Render-device capability family

Rendering should use the capability model, but it should not be represented as
one large platform capability. A render device is an engine-domain service with
its own backend selection. The operating system supplies window and surface
integration; a graphics backend supplies GPU behavior; the material system
consumes the resulting render services.

This distinction is important because OS and graphics API are independent
choices. Windows may use D3D, Vulkan, OpenGL, or a translation backend. A POSIX
platform may use Vulkan, OpenGL, or a headless/null provider. Renderer selection
must therefore not be derived from `IsWindows()`, `IsPosix()`, or a similar
platform test.

### Current responsibilities

The existing rendering interfaces already contain several potential seams, but
their responsibilities overlap:

- `IShaderDeviceMgr` discovers adapters and modes, applies configuration policy,
  selects an adapter, creates a device, and returns another interface factory;
- `IShaderDevice` owns presentation, native windows/views, resource allocation,
  shader compilation, shader creation, device loss, and loading-screen behavior;
- `IShaderAPI` combines mode changes, render-state mutation, command submission,
  material binding, and draw behavior;
- `IMaterialSystemHardwareConfig` combines hardware facts, selected quality
  policy, API identity, driver workarounds, and mutable feature state;
- `CMaterialSystem` chooses and loads the shader module, binds its global
  interfaces, coordinates resource restoration, and passes an untyped native
  window pointer into mode creation.

The current split can remain behind a compatibility adapter, but it should not
be copied into a new backend contract. In particular, a new contract should not
use `void *hWnd`, `m_nDXLevel`, `IsOpenGL()`, or compile-time
`DX_TO_GL_ABSTRACTION` branches as portable concepts.

### Target capability family

Rendering should be decomposed into the following cooperating contracts:

| Contract | Responsibility |
| --- | --- |
| Render backend provider | Identifies the backend, enumerates adapters, reports backend availability, and creates devices |
| Render device | Owns a logical GPU device, immutable capability data, queues, and GPU resources |
| Render presentation | Owns a presentation surface/swapchain, display mode, resize state, synchronization policy, and presentation |
| Presentation bridge | Privately joins one window provider to one render provider and creates presentation objects |
| Render command context | Records or issues render commands according to the backend's threading model |
| Render device capabilities | Immutable hardware and driver facts used for validation and feature selection |
| Render feature profile | Application policy selected from requested quality, device capabilities, and documented workarounds |
| Shader compiler | Converts supported source or intermediate forms into backend-consumable shader artifacts |
| Render diagnostics | Debug labels, validation messages, capture integration, and device-failure information |

These contracts may be exposed by one module, but consumers should depend only
on the portions they use. The interfaces should not force modern explicit APIs
to emulate D3D9 lifetime or presentation semantics at their public boundary.

### Backend selection

A render backend is selected by the application composition root from an
explicit backend identifier and requirements. The selection process may
consider user configuration, command-line overrides, available modules,
adapter capabilities, and product policy. It must not be hard-coded as an OS
consequence.

A provider descriptor should include at least:

- a stable backend identifier such as `d3d9`, `vulkan`, `gl`, or `null`;
- a human-readable name and implementation version;
- whether the backend supports presentation, offscreen rendering, or both;
- the window-surface interop mechanisms it can consume;
- whether runtime shader compilation is available;
- diagnostic and validation modes supported by the provider.

The provider is loaded through the dynamic-library capability described in this
RFC. Backend module lifetime encloses all devices, presentation objects,
commands, and resources created from it.

### Adapter and device creation

Adapter enumeration should return stable value descriptions rather than expose
backend-native objects. Device creation should consume a requirements structure
and return either a device or a structured failure explaining which requirement
could not be satisfied.

An illustrative boundary is:

```cpp
struct RenderDeviceRequest_t
{
    RenderAdapterId_t m_Adapter;
    RenderFeatureSet_t m_RequiredFeatures;
    RenderFeatureSet_t m_OptionalFeatures;
    bool m_bEnableValidation;
};

abstract_class IRenderBackendProvider
{
public:
    virtual int GetAdapterCount() const = 0;
    virtual bool GetAdapterInfo( int index, RenderAdapterInfo_t *info ) const = 0;
    virtual IRenderDevice *CreateDevice( const RenderDeviceRequest_t &request,
                                         RenderCreateError_t *error ) = 0;
};

abstract_class IRenderDevice
{
public:
    virtual const RenderDeviceCaps_t &GetCapabilities() const = 0;
    virtual IRenderQueue &GetGraphicsQueue() = 0;
};

abstract_class IRenderPresentationBridgeFactory
{
public:
    virtual IRenderPresentation *CreatePresentation(
        IRenderDevice &device,
        IRenderSurface &surface,
        const RenderPresentationConfig_t &config,
        RenderCreateError_t *error ) = 0;
};
```

The names and ownership representation are illustrative. The required semantic
properties are:

- adapter descriptions remain valid independently of a temporary native
  enumeration object;
- required features are validated before the device is returned;
- device capabilities do not change silently after construction;
- child resources cannot outlive their device;
- a failed creation returns structured information rather than relying only on
  log output;
- presentation is optional and separate from device creation.

### Window and presentation boundary

The window system and render backend need a deliberate interop boundary. The
portable renderer must not receive a raw `HWND`, SDL window pointer, X11 handle,
Wayland object, or Cocoa object as `void *`.

The window provider should expose an opaque `IRenderSurface` capability. A
presentation-bridge factory selected by the composition root joins the chosen
window and render providers. The bridge may use private, versioned provider
endpoints to exchange native handles. Only that bridge may include both native
API families; neither public provider contract exposes a native-handle query.

Presentation is deliberately not created by `IRenderDevice` directly. Putting
surface interpretation on the device would force every render provider to know
every window representation or would create a generic native-handle union. The
bridge is the pair-specific dependency and makes that dependency visible in the
build graph.

Presentation is a child of a render device and a surface. It owns behavior such
as:

- swapchain or back-buffer creation;
- fullscreen/windowed policy;
- color format and color space;
- refresh and synchronization policy;
- resizing and minimized/occluded state;
- presentation and recoverable presentation errors.

This allows an application to create a device without a window, use multiple
windows without treating them as device-global state, and test material or
resource behavior with an offscreen provider.

Runtime resizing is a required presentation behavior, not an application
restart or backend-reselection event. When a window's drawable extent changes
at runtime—including when a phone or tablet rotates—the surface provider must
report the new extent and the presentation object must recreate or resize its
swapchain/back buffers as required while keeping the logical render device and
backend alive. The renderer must use the new drawable extent for subsequent
frames. Transient zero-sized, minimized, or otherwise non-presentable states
during the transition must suspend presentation without being treated as fatal;
presentation resumes when the surface becomes presentable again.

### Capabilities, profiles, and quirks

Three categories that are currently mixed together must be represented
separately:

1. **Device capabilities** are facts reported by the provider: supported
   formats, resource limits, shader stages, sample counts, synchronization
   features, and presentation support.
2. **Feature profile** is application policy: the rendering feature set and
   quality level the engine chooses to use on that device.
3. **Quirks** are documented workarounds selected from backend, driver, adapter,
   and version data.

Portable rendering code should ask semantic questions such as whether a format
supports sampled sRGB reads or whether a depth target can be paired with the
requested color target. It should not ask whether the backend is OpenGL and use
that identity as a proxy for behavior.

`m_nDXLevel` may remain as legacy material/shader compatibility policy during
migration, but it must not appear in a new device capability contract. The
composition or material policy layer can translate a legacy DX level into a
requested feature profile.

Quirks should be explicit data with a reason and affected provider range. This
keeps workarounds observable and prevents backend identity checks from spreading
through materials and shaders.

### Shader compilation

Shader compilation is not device presentation and should be independently
available. Some products compile shaders offline, some permit runtime
compilation, and a shipping runtime may only create shader objects from
precompiled artifacts.

An `IShaderCompiler` capability should define source language or intermediate
format, target profile, compile options, output artifact identity, and
structured diagnostics. `IRenderDevice` consumes a compatible artifact to
create a shader or pipeline resource.

This separation permits:

- command-line shader tools without a window or live GPU device;
- cached artifacts keyed by compiler and backend version;
- a runtime without compiler dependencies;
- backend-specific compilation without backend checks in material code.

The existing `IShaderDevice::CompileShader` behavior remains available through
the legacy adapter until its consumers migrate.

### Command model and threading

The capability design should describe the backend's command and queue behavior,
not assume one global owner thread. A backend may implement immediate commands,
recorded command buffers, or an adapter over the existing queued render context.

The contract must define:

- which objects are thread-safe;
- where command recording is permitted;
- how work is submitted and ordered;
- when resources may be destroyed;
- how CPU/GPU completion is observed;
- which thread receives device and presentation events.

The first implementation may adapt the current `IShaderAPI` and
`CMatQueuedRenderContext` rather than introduce a new draw-command API. The
important first step is to make their ownership and threading requirements part
of the bound render service instead of process-global assumptions.

### Device loss and resource restoration

Device loss is a lifecycle event, not an invitation to replace global interface
pointers during arbitrary feature execution. The render service should report a
state transition such as available, presentation unavailable, device lost,
recovering, or fatal.

The material system owns restoration policy for material resources. The backend
owns recreation of native device objects. Resource owners register through a
scoped observer or restoration contract whose lifetime is bounded by the render
service; raw global callback lists should not be added.

Backends that do not support device recovery report a fatal device failure. A
test backend should be able to simulate loss and recovery deterministically.

### Headless and null rendering

A dedicated server should normally omit the render capability entirely. Absence
is different from a device that intentionally accepts rendering operations and
produces no output.

The existing empty shader API is useful as the basis for a `null` provider for
tests, tools, and products that exercise render-facing code without a GPU. That
provider must follow the same lifetime and validation contracts as production
providers and clearly identify which operations are simulated.

### Render backend conformance tests

Every render backend should pass shared tests covering:

- adapter enumeration and stable adapter descriptions;
- structured failure for invalid adapters and unsatisfied required features;
- headless device creation where advertised;
- presentation creation, repeated runtime resizing, orientation-style aspect
  ratio changes, minimization or transient zero-sized states, and destruction
  where advertised; resizing must not recreate the logical render device;
- multiple presentation surfaces when advertised;
- buffer, texture, shader-artifact, and command-context lifetime;
- immutable capability reporting;
- submission ordering and completion behavior;
- simulated or real device-loss behavior according to provider claims;
- complete destruction before backend-module unload;
- absence of native window and graphics API types from public portable headers.

Backend-specific image correctness and performance suites remain separate from
the contract tests.

### Render migration sequence

Rendering should migrate after the dynamic-library reference slice establishes
provider loading and ownership:

1. Inventory `IsOpenGL()`, `DX_TO_GL_ABSTRACTION`, and DX-level decisions and
   classify each as a capability, feature policy, quirk, or backend
   implementation detail.
2. Define immutable adapter descriptions, device capabilities, feature
   profiles, and structured creation failures.
3. Expose the current `shaderapidx9` and `shaderapiempty` modules as legacy
   render backend providers without initially changing draw behavior.
4. Make application composition select a render provider and pass its binding
   to `CMaterialSystem`; stop making `CMaterialSystem` derive backend choice
   from a default DLL name.
5. Introduce the window-to-render-surface adapter and remove native `void *`
   window handles from the new boundary.
6. Represent presentation as a separately owned object while adapting current
   `SetMode`, `Present`, and view behavior underneath it.
7. Bind `IShaderDevice`, `IShaderAPI`, `IShaderShadow`, and hardware config into
   a scoped legacy render-services adapter rather than independent globals.
8. Separate actual device capabilities from legacy DX-level feature policy and
   explicit driver quirks.
9. Migrate resource and material consumers incrementally; retain the existing
   shader API command model until a separate renderer RFC replaces it.
10. Validate a genuinely different backend through the same provider and
    conformance contracts before treating the boundary as stable.

The render capability migration is complete when application code can select a
backend independently of OS identity, dedicated servers can omit rendering,
window-system native types are confined to interop adapters, and the material
system can be tested against the null provider without installing process-global
shader interfaces.

## Ownership and lifecycle

The application composition root owns providers and engine services. Consumers
receive non-owning references whose validity covers the consumer's complete
lifecycle.

The startup sequence is:

1. select a platform backend;
2. construct foundation providers;
3. construct engine services from their declared dependencies;
4. validate all required capabilities;
5. connect legacy compatibility bridges where still needed;
6. initialize services in dependency order;
7. run the application.

Shutdown reverses this sequence. New providers SHOULD use scoped ownership so a
partial startup failure automatically releases everything constructed so far.
Where current compiler or ABI constraints prevent modern RAII types, the same
ownership rule must be implemented explicitly.

A provider MUST NOT be replaced while consumers are connected. Dynamic modules
MUST remain loaded until all interfaces created from them are disconnected and
destroyed.

## Capability discovery and modules

`CreateInterface` remains the cross-module ABI during this migration. It is a
module interface registry, not a platform abstraction, and should remain
separate from native library loading.

The intended flow is:

```text
IDynamicLibrary provider
        |
        v
load module and obtain symbol
        |
        v
CreateInterface adapter
        |
        v
typed engine service binding
```

New composition code SHOULD bind known versioned interfaces into typed
references once during startup. Feature code SHOULD NOT repeatedly perform
string-based lookups.

Lookup failures must carry enough information to diagnose:

- the requested capability or interface version;
- the requesting subsystem;
- the provider or module searched;
- the native loading or symbol error, when applicable.

## Build-system rules

The build selects exactly one named target composition and its provider catalog.
That composition may resolve each required capability to a target-specific or
shared provider; where only one implementation is possible, this is the
degenerate single-provider case. OS definitions and native include paths SHOULD
be private to the provider targets that require them.

Portable targets may include platform contract headers but MUST NOT include
backend headers. Backend selection should happen in Waf or the applicable
composition target rather than through source-level OS branches scattered among
consumers.

An automated dependency check SHOULD reject:

- native OS headers outside approved backend directories;
- new `IsWindows()`, `IsLinux()`, `IsOSX()`, or equivalent branches in portable
  code;
- includes from portable targets into backend directories;
- upward dependencies from platform foundations into engine services or
  features.

Exceptions require a documented reason and a removal plan.

## Dependency-boundary enforcement

The dependency rules in this RFC MUST be machine-enforced. Code review remains
useful for deciding whether a capability is well designed, but it must not be
the only mechanism preventing portable code from acquiring a dependency on an
SDL, Vulkan, Win32, X11, Wayland, or other provider-specific detail.

Enforcement operates on three related graphs:

1. the source include graph, including transitive includes;
2. the Waf target dependency and external-library graph;
3. the architectural module graph declared by this RFC.

Checking only source text is insufficient. A portable source file may include a
nominally portable header that leaks a native dependency, or its target may link
a provider directly without including any provider header. Checking only link
targets is also insufficient because native types and preprocessor decisions
may leak through public headers.

### Architectural module manifest

The repository should contain a versioned, dependency-free manifest at
`architecture/modules.json`. JSON is preferred initially so the checker does
not add a package dependency to the bootstrap toolchain.

Each architectural module declares:

- the source and public-header path patterns it owns;
- the Waf targets it owns;
- the architectural modules it may depend on;
- the external include families and Waf `uselib` values it may use;
- whether it is a contract, portable implementation, provider, private interop
  adapter, composition root, or legacy module;
- narrowly scoped exceptions, if any.

An illustrative subset is:

```json
{
  "modules": {
    "platform.contracts": {
      "kind": "contract",
      "paths": ["public/platform/**"],
      "allow": ["foundation.contracts"]
    },
    "render.contracts": {
      "kind": "contract",
      "paths": ["public/render/**"],
      "allow": ["foundation.contracts", "platform.contracts"]
    },
    "render.generic": {
      "kind": "portable",
      "paths": ["render/**"],
      "allow": ["foundation.contracts", "platform.contracts", "render.contracts"]
    },
    "platform.sdl3": {
      "kind": "provider",
      "paths": ["platform/backends/sdl3/**"],
      "allow": ["foundation.contracts", "platform.contracts"],
      "externalIncludes": ["SDL3/**"],
      "uselib": ["SDL3"]
    },
    "render.vulkan": {
      "kind": "provider",
      "paths": ["render/backends/vulkan/**"],
      "allow": ["foundation.contracts", "platform.contracts", "render.contracts"],
      "externalIncludes": ["vulkan/**"],
      "uselib": ["VULKAN"]
    },
    "interop.sdl3_vulkan": {
      "kind": "interop",
      "paths": ["platform/interop/sdl3_vulkan/**"],
      "allow": ["platform.contracts", "render.contracts", "platform.sdl3", "render.vulkan"],
      "externalIncludes": ["SDL3/**", "vulkan/**"],
      "uselib": ["SDL3", "VULKAN"]
    },
    "client.composition": {
      "kind": "composition",
      "paths": ["launcher/client/**"],
      "allow": ["platform.contracts", "render.contracts", "platform.sdl3", "render.vulkan", "interop.sdl3_vulkan"]
    }
  }
}
```

The exact paths will be selected as code moves. The important property is that
`render.generic` cannot name `render.vulkan` or `platform.sdl3`, while the
composition root may select both. The only module permitted to see both native
APIs is the private SDL3-to-Vulkan surface adapter.

The manifest is an allowlist. An unlisted dependency is rejected. Adding a new
provider therefore cannot silently make its API available to generic code.

### Source dependency checks

A repository tool at `tools/archlint/archlint.py` should provide a fast lexical
mode and a compiler-grounded full mode.

The fast mode scans changed source and header files and rejects:

- a direct include whose owning module is not allowed by the importing module;
- native header families outside their declared provider or interop module;
- platform selection macros and helpers in portable modules;
- provider-native names in contract headers, including `SDL_*`, Vulkan `Vk*`,
  Win32 handle types, and equivalent native surface types;
- relative include paths used to escape an architectural boundary.

The full mode consumes the compiler-generated dependency data for every
translation unit. This validates the resolved, transitive include graph using
the actual defines and include paths instead of attempting to reproduce the C
preprocessor in the linter. It catches a contract header that indirectly
includes SDL or Vulkan even when the consuming source file appears clean.

Contract and provider-factory headers should also have hermetic compile tests.
Each public header is compiled from a one-line translation unit using only the
include roots allowed to its module. Contract-header tests deliberately omit
SDL, Vulkan, and native SDK include paths. A provider's public factory header
must compile without its implementation SDK; native dependencies remain in its
private implementation.

### Waf target checks

Waf targets should declare an architectural owner alongside their existing
`name`, `source`, and `use` fields:

```python
bld.shlib(
    name = 'render_vulkan',
    arch_module = 'render.vulkan',
    use = ['tier0', 'render_contracts', 'VULKAN'],
    # ...
)
```

A small Waf tool should validate the complete task-generator graph after
subprojects are loaded:

- every new target has exactly one architectural owner;
- each `use` edge is allowed by the source module's allowlist;
- each external `uselib` value is permitted for that module;
- native include directories are not attached to portable targets;
- public headers do not come from a provider-private directory;
- the resulting architectural graph contains no dependency cycle.

Target ownership prevents a clean include surface from disguising an invalid
link dependency. For example, adding `SDL3` or `render_vulkan` to the generic
material system's `use` list fails even if no source file includes their
headers.

During migration, existing targets may be mapped to modules in the manifest so
that every `wscript` need not be edited at once. New targets MUST declare
`arch_module` directly. The sidecar mapping should shrink as target definitions
are migrated.

### Baseline and exceptions

The present tree already contains platform leakage, so enforcement must start
as a ratchet rather than requiring a flag-day cleanup. The initial full scan
records each existing violation in `architecture/baseline.json` as an exact
fingerprint containing the rule, importing file or target, and imported file or
target.

CI permits a matching baseline violation but rejects:

- any new violation;
- a violation whose dependency target changed;
- a broad directory or rule exemption;
- an obsolete baseline entry, so repaired violations are removed rather than
  silently retained.

New contract, provider, interop, and composition directories are strict from
their first commit and receive no baseline entries.

A true exception belongs in the main manifest, not the baseline. It must name
one source module, one dependency or native-header family, the rule being
waived, a reason, an owner, a tracking issue, and an expiry or removal
condition. Exceptions may not be expressed as unrestricted directory globs.

### Developer and CI workflow

The intended commands are:

```text
python3 tools/archlint/archlint.py check --changed
python3 tools/archlint/archlint.py check --all --compile-deps
python3 tools/archlint/archlint.py baseline --verify
```

The changed-file check should be fast enough for normal local use. CI runs the
full source, transitive dependency, Waf target, cycle, and baseline checks
before expensive build jobs. The linter itself requires fixture tests for
direct leaks, transitive leaks, invalid target edges, valid provider factories,
the SDL3-Vulkan interop exception, cycles, and stale baselines.

Diagnostics must explain the architectural correction rather than merely name
the rejected token. For example:

```text
ARCH002 forbidden target dependency
  materialsystem (render.generic) -> render_vulkan (render.vulkan)
  render.generic may depend on render.contracts, not a provider.
  Request IRenderDevice through application composition instead.
```

This enforcement is considered effective when a generic target cannot acquire
a provider dependency through a direct include, a transitive public include, a
Waf target edge, an external include path, or a native type in a public
contract.

## Backend conformance tests

Every backend must pass a shared contract suite. The initial suite should test:

- monotonic time never moves backward and reports a valid resolution;
- thread creation, joining, identifiers, naming, mutexes, condition variables,
  and thread-local storage obey their contracts;
- virtual-memory reservation, commitment, alignment, protection, and release;
- a fixture shared library can be loaded, queried for a symbol, and unloaded;
- missing libraries and symbols return stable structured failures;
- executable, temporary, and user-data paths have documented normalization and
  encoding;
- unsupported optional capabilities are reported explicitly;
- shutdown releases handles and permits a clean second application instance in
  the same test process where supported.

The test backend should additionally offer deterministic clocks, synchronous or
controlled scheduling, in-memory paths/files, and fake modules. This backend is
for subsystem tests and is not a production OS port.

## Reference migration: dynamic-library loading

Dynamic-library loading is the first capability to migrate.

Today, native loading behavior is implemented in `tier1/interface.cpp` using
Win32 and POSIX APIs, while `public/tier1/interface.h` exposes both module loading
and the `CreateInterface` registry. The two responsibilities should be split.

### Proposed contract

The precise names and error types will be finalized during implementation, but
the contract should resemble:

```cpp
abstract_class IDynamicLibrary
{
public:
    virtual ~IDynamicLibrary() {}
    virtual void *FindSymbol( const char *name ) = 0;
};

abstract_class IDynamicLibraryLoader
{
public:
    virtual ~IDynamicLibraryLoader() {}
    virtual IDynamicLibrary *Load( const char *path,
                                   DynamicLibraryError *error ) = 0;
    virtual void Unload( IDynamicLibrary *library ) = 0;
};
```

The implementation may instead use an opaque value handle with scoped ownership
if that better preserves existing conventions. The following semantics are
required regardless of representation:

- a loaded library has one unambiguous owner;
- symbol addresses cannot outlive the loaded library;
- errors are available without parsing log text;
- `no-load` behavior, where supported, is explicit rather than encoded as an OS
  assumption;
- path resolution policy is separate from the primitive load operation.

### Migration steps

1. Add contract tests and a small fixture module exporting a known symbol.
2. Introduce platform-neutral dynamic-library contracts.
3. Move Win32 loading code into the Windows backend.
4. Move `dlopen`, `dlsym`, and `dlclose` code into the POSIX backend.
5. Keep `InterfaceReg` and `CreateInterface` in portable module-registry code.
6. Adapt `Sys_LoadModule`, `Sys_UnloadModule`, `Sys_GetFactory`, and
   `CDllDemandLoader` to the selected capability behind a narrowly scoped legacy
   bridge.
7. Convert `CAppSystemGroup` to receive the module loader explicitly.
8. Remove native loader headers and OS branches from the public Tier 1 interface
   header.
9. Migrate direct `Sys_LoadModule` callers as their owning subsystems are
   touched.

The legacy bridge may hold the default loader required by existing `Sys_*`
functions, but only bootstrap code may install it. New code must not access that
default directly.

### Acceptance criteria

The reference migration is complete when:

- existing applications load the same modules and interfaces as before;
- Windows and POSIX pass the same loader conformance suite;
- module-loading implementation contains no OS branches outside backend targets;
- `CreateInterface` registration has no dependency on a native loader;
- a test application can supply a fake loader without initializing Tier 1
  globals;
- load and symbol failures identify the path, operation, and provider error.

## Dynamic-module-loader retirement

Introducing a platform-neutral dynamic-library capability is a containment step,
not an endorsement of dynamic loading as the normal dependency mechanism. The
long-term objective is:

> No general-purpose module loader is available to feature or service code.
> Native loading exists only inside named extension hosts with an explicit ABI,
> trust policy, and lifetime.

This objective does not require eliminating every shared library. A first-party
system may remain a shared-library build target while being linked through an
import library and constructed through a typed factory. The behavior being
retired is runtime discovery by filename followed by untyped string lookup.

### Current uses

The current loader serves several materially different purposes:

| Use | Examples in the current tree | Target disposition |
| --- | --- | --- |
| Mandatory application composition | engine, input system, material system, data cache, studio render, physics, VGUI, and video services loaded by the launcher | Link into the product composition and construct through typed factories |
| App-system lifecycle and service location | `CAppSystemGroup` loads a module, asks `CreateInterface` for a named singleton, then performs connect/init/shutdown/disconnect | Preserve lifecycle temporarily, replace filename and string discovery with explicit instances and declared dependencies |
| Game and mod binaries | client and server game DLLs plus game-requested shared app systems | Retain behind a dedicated game-extension host while mod ABI compatibility is required |
| Server and UI plug-ins | server plug-ins, GameUI modules, tool dictionaries, and add-ons | Retain only as explicit plug-in hosts with separate policy and ABI |
| Render and shader selection | `shaderapidx9`, `shaderapiempty`, and material shader DLLs | Use a typed render-provider catalog; retain mod shader loading only as a separate compatibility host |
| Optional product providers | VR, video codecs, haptics, Sixense, LCD support, phoneme extractors, and similar integrations | Link providers selected by the product where practical; otherwise give each extension family a typed host |
| Tool implementation indirection | launchers for VRAD, VVIS, shader compilation, texture compilation, VTEX, MPI workers, and tool dictionaries | Build a normal executable or directly linked library; use a child process where isolation or replacement is required |
| Tool-only integrations | Perforce and MySQL wrappers | Link into the tool product or load through a tool-specific optional-provider host, never the engine-wide loader |
| Native API and vendor probing | `USER32`, D3DX, Steam libraries, and other direct symbol probes | Keep private to the owning platform/provider target; prefer ordinary or delay linking when appropriate |
| Path, validation, and depot policy | filesystem `LoadModule` resolves search paths, obtains local copies, and checks whether a binary is approved | Separate resolution and verification from native loading |

The `LoadModule( CreateInterfaceFn )` overload is not dynamic loading at all. It
places an already-linked factory into the same table as loaded libraries. It
should be renamed and removed first because it hides static composition behind
module terminology.

`CreateInterface` is also distinct from the native loader. It is both a legacy
binary ABI and an in-process string service registry. The binary ABI may remain
at genuine extension boundaries after the general registry has disappeared
from first-party composition.

### Target decomposition

The current mechanism should be replaced by four separate concepts:

1. **Provider catalog**: a typed, build-assembled set of first-party factories.
   Application configuration may choose a provider identifier only from this
   already-present catalog.
2. **Module resolver**: resolves a declared extension identifier to a local,
   verified binary path. Search paths, depot access, signatures, allowlists,
   and product policy live here.
3. **Native library primitive**: the private platform operation that opens one
   resolved path, finds a symbol, and closes the handle.
4. **Extension host**: owns a specific ABI, uses the resolver and native
   primitive, validates the entry point and version, supplies a restricted host
   API, and encloses every object created by the extension.

Only extension-host implementation targets may depend on the native library
primitive. Consumers receive a typed result from a particular host; they do not
receive a library handle, symbol lookup function, or `CreateInterfaceFn`.

For example, render selection should resemble:

```cpp
RenderProviderCatalog providers;
providers.Add( CreateD3D9RenderProviderFactory() );
providers.Add( CreateVulkanRenderProviderFactory() );
providers.Add( CreateNullRenderProviderFactory() );

IRenderBackendProvider *renderer = providers.Create( configuredBackend,
                                                      request,
                                                      &error );
```

Waf decides which factory functions are linked into each product. SDL3 and
Vulkan are therefore ordinary provider dependencies of the client composition,
not filenames discovered by the material system. A dedicated-server build does
not link either provider.

Where a retained extension ABI still uses `CreateInterface`, the extension host
owns the compatibility lookup and immediately converts the result to its typed
contract. It must not publish the extension's factory to a process-wide factory
list.

### Retirement phases

#### Phase A: Inventory and freeze

- instrument every load with requesting subsystem, resolved path, requested
  entry point or interface, success, lifetime, and unload result;
- classify every site as first-party composition, retained extension, optional
  provider, tool indirection, or native symbol probe;
- add architecture-linter rules rejecting new direct uses of `Sys_LoadModule`,
  `Sys_GetFactory`, filesystem `LoadModule`, and `CDllDemandLoader` outside the
  recorded baseline;
- prohibit new APIs that accept or return `CreateInterfaceFn` except in the
  legacy ABI package.

#### Phase B: Remove pseudo-modules

- rename `LoadModule( CreateInterfaceFn )` to a temporary `AddLegacyFactory`;
- let `CAppSystemGroup` accept explicit `IAppSystem` instances;
- convert linked cvar, filesystem, SDL, and tool systems to explicit instances;
- stop treating an already-linked factory as a module with an unload lifecycle.

#### Phase C: Link mandatory first-party systems

- give engine, input, material, cache, studio-render, physics, VGUI, and video
  services typed construction functions;
- assemble product compositions in the launcher and dedicated-server roots;
- preserve existing `IAppSystem` lifecycle ordering through a typed lifecycle
  graph during migration;
- remove filename lookup and `CreateInterface` discovery for these systems.

This phase may retain DLL build outputs where binary separation is useful. They
are normal linked dependencies rather than runtime plug-ins.

#### Phase D: Replace backend loading with provider catalogs

- register SDL3, Vulkan, D3D, null, audio, input, and video providers through
  typed factories selected by Waf;
- move configuration-based selection to catalogs owned by composition roots;
- keep native SDK headers and any optional vendor-library probing private to the
  provider;
- split mod-supplied material shaders into a separately named shader-extension
  compatibility host.

#### Phase E: Simplify tools

- replace launchable-DLL wrappers with normal executable entry points or
  directly linked tool libraries;
- prefer a child-process protocol when a tool needs crash isolation, alternate
  versions, or remote execution;
- keep Perforce, database, and proprietary integrations out of runtime engine
  compositions.

#### Phase F: Quarantine retained extensions

- create distinct hosts for game modules, server plug-ins, tool plug-ins, and
  any retained mod shader ABI;
- give each host a fixed entry point, explicit ABI version, capability-limited
  host API, resolver policy, and deterministic shutdown order;
- keep legacy `CreateInterface` negotiation inside compatibility hosts;
- prevent one extension family from looking up interfaces belonging to another
  family through a global factory chain.

In-process native plug-ins are not a security boundary. Validation and signing
can establish product trust policy, but an untrusted extension requires a
separate process rather than a more elaborate loader interface.

#### Phase G: Remove the public loader

- remove module loading from `IFileSystem`; leave it responsible only for file
  and path behavior;
- remove `Sys_LoadModule`, `Sys_UnloadModule`, `Sys_GetFactory`, and
  `CDllDemandLoader` from public Tier 1 headers;
- place the native library primitive in a private platform package visible only
  to approved extension hosts;
- move `InterfaceReg` and `CreateInterface` into a legacy extension-ABI package;
- make the architecture linter reject native loading everywhere else without a
  baseline.

### Retirement acceptance criteria

The general loader is retired when:

- first-party runtime systems are selected from typed, build-assembled provider
  catalogs rather than module filenames;
- generic engine, material, input, and UI code cannot access a native library
  handle, symbol lookup, or `CreateInterfaceFn`;
- SDL3 and Vulkan providers are composed without runtime module discovery;
- filesystem code no longer opens executable libraries;
- every remaining dynamic load belongs to a named extension host with a
  documented ABI and trust policy;
- adding a new direct load site fails architecture lint and CI;
- a build can enumerate its retained extension boundaries from its Waf graph.

## Legacy tier migration

The tiers remain available while consumers migrate.

### Compatibility bridge

A temporary bridge will receive explicitly composed services and assign legacy
globals such as `g_pCVar`, `g_pFullFileSystem`, and `g_pStudioRender`. The bridge
owns none of these services. It clears every assigned global during disconnect
and asserts that no duplicate installation occurs.

The bridge should be divided by domain rather than reproduced as a generic
global capability registry. New services must not add new public global aliases.

### Domain decomposition

Code should move out of numbered tiers only when a cohesive ownership boundary
is established. Likely domains include:

- portable containers and serialization;
- console variables and command processing;
- filesystem utilities;
- rendering support;
- audio and RIFF utilities;
- input and keybinding support;
- VGUI integration;
- model and studio-render support;
- choreography and scene processing;
- content tools and source-control integration.

Dependency depth remains enforced by the build graph, but it is no longer the
package's name or primary responsibility.

## Stack-ranked delivery plan

The following is one strict portfolio order. A lower-ranked package may begin in
parallel when its declared dependencies are stable, but staffing a higher rank
must not delay an unfinished architectural prerequisite.

Sizes are aggregate focused engineering effort, not calendar promises:

- **S**: 1–3 engineer-weeks;
- **M**: 3–8 engineer-weeks;
- **L**: 6–16 engineer-weeks;
- **XL**: 12–32 engineer-weeks;
- **XXL**: more than 32 engineer-weeks and must be delivered incrementally.

The bands overlap intentionally because integration surface and uncertainty are
part of the classification; the numeric range on each package is the more useful
estimate.

The ranges assume an engineer familiar with C++, Waf, and one existing Source
subsystem. They include implementation, focused tests, build integration, and
review fixes, but not unrelated bug repair, third-party certification, or
performance work discovered after profiling. Confidence is moderate for ranks
1–6, low for ranks 7–13, and deliberately low beyond the first native Vulkan
vertical slice.

A lexical inventory used for scoping found 61 Waf program/library declarations
across 66 `wscript` files, 44 source files mentioning direct `Sys_LoadModule`,
24 filesystem-mediated module-load call sites, 270 files mentioning
`CreateInterfaceFn`, 45 `AppSystemInfo_t` declarations, 39 first-party files
with SDL API usage, 89 files with legacy render-backend decisions, and roughly
499 files referencing representative tier service globals. These are exposure
indicators, not one-to-one migration tasks.

| Rank | Work package | Scope and exit gate | Depends on | Size |
| ---: | --- | --- | --- | --- |
| 1 | Architecture guardrails | Land `archlint`, module ownership manifest, direct and transitive include checks, Waf target-edge checks, exact legacy baseline, CI, and fixture tests. New capability/provider directories are strict. | — | M, 3–5 |
| 2 | Foundation vocabulary | Add `Expected<T, E>`, compact domain errors, strong IDs, explicit ownership/borrow annotations or types, and common test matchers. No service or provider is introduced here. | 1 | M, 3–5 |
| 3 | Composition and lifecycle kernel | Add typed provider descriptors, required/optional dependency validation, deterministic start/stop ordering, rollback on partial startup, and domain-scoped legacy bridges. Convert the unit-test runner as the reference composition. | 1–2 | M, 4–7 |
| 4 | Dynamic-loader containment | Add scoped native-library ownership, structured load errors, load-site telemetry, a private loader provider, and `Sys_*` compatibility adapters. Freeze new loader and `CreateInterfaceFn` use. | 1–3 | M, 4–7 |
| 5 | Paths and module resolution | Introduce `NativePath` and `VirtualPath`; separate path search, depot/local-copy behavior, validation, and signature policy from opening a library. Convert the extension resolver and loader bridge. | 2, 4 | M–L, 5–9 |
| 6 | Dedicated-server composition | Construct the dedicated server from explicit providers, omit render and desktop UI capabilities, and keep old globals only in scoped compatibility bridges. Add startup/shutdown integration tests. | 3–5 | M, 4–7 |
| 7 | Window/input contracts with SDL2 adapter | Define window, event source, cursor, clipboard, message-box, gamepad, touch, and opaque render-surface contracts. Move current SDL2 calls behind a provider without changing behavior. | 1–3 | L, 8–14 |
| 8 | Render provider seam | Add render provider/device/capability/profile contracts, structured creation errors, the `LegacyRenderBackendProvider` around `IShaderDeviceMgr`, a scoped `LegacyRenderServices` bundle, and a conforming null backend. | 1–3 | L, 10–16 |
| 9 | Presentation bridge seam | Remove native window interpretation from the new device contract. Implement pair-specific bridge factories for the current supported window/render pairs and headless-null; add repeated runtime resize, orientation/aspect-ratio change, minimize/zero-size recovery, multi-window, and destruction-order tests. | 7–8 | L, 6–10 |
| 10 | SDL3 provider | Implement SDL3 window/input providers, translate SDL3 events into portable events, add current-renderer presentation bridges, remove SDL from generic include paths, and reach behavior parity with the SDL2 provider. | 7, 9 | L, 8–14 |
| 11 | Sequences, clocks, and deterministic scheduling | Add monotonic clock, task runner, sequenced runner, delayed scheduling, virtual-time test provider, and diagnostic sequence checks. Adapt existing queues; do not replace the job system wholesale. | 2–3 | L, 6–12 |
| 12 | Remaining platform foundation | Extract virtual memory, physical-thread infrastructure, process launch, environment, executable/user/temp paths, and diagnostics behind platform providers with shared conformance tests. | 2, 5, 11 | L–XL, 10–18 |
| 13 | Vulkan compatibility waypoint | Validate Vulkan deployment, adapter/profile selection, shader artifact flow, and SDL3 presentation using the smallest viable compatibility route, such as DXVK Native where suitable. This is an architectural proof, not the native backend. | 8–11 | M, 4–8 |
| 14 | Native Vulkan bootstrap | Create a native provider that enumerates adapters, validates the required Vulkan profile, creates instance/device/queues, presents through the SDL3-Vulkan bridge, emits validation diagnostics, and renders a smoke-test frame. | 8–11 | M, 4–8 |
| 15 | New-OS architecture proof | Bring up the dedicated composition plus SDL3/Vulkan smoke application on one additional OS using only provider and composition changes. Pass foundation conformance suites and treat edits to portable features as defects. This is not full game certification. | 6, 10, 12, 14 | L–XL, 12–24 plus port-specific work |
| 16 | Native Vulkan functional MVP | Implement buffers, images, samplers, render targets, shader artifacts, descriptor/pipeline management, command recording, uploads, synchronization, swapchain recreation, and enough material-system adaptation to render a representative map in an opt-in build. | 11, 14 | XL–XXL, 24–40 |
| 17 | Vulkan parity and hardening | Cover required material/shader permutations, feature profiles, explicit quirks, device/presentation loss, multiple surfaces, capture/debug tooling, cache persistence, correctness corpus, and representative performance budgets. Default selection remains a separate product decision. | 16 | XL, 16–32 |
| 18 | First-party module retirement | Replace runtime filename lookup for mandatory engine, input, material, cache, studio-render, physics, VGUI, video, SDL3, and Vulkan systems with build-assembled typed catalogs. Preserve shared-library builds only where linked components are useful. | 3–10 | L–XL, 12–20 |
| 19 | Tool executable and process cleanup | Replace launchable-DLL wrappers with normal executables or linked libraries; use child-process protocols where isolation, remote execution, or version replacement is required. Migrate Perforce/database integrations to tool-only providers. | 3–5 | L, 8–14 |
| 20 | Retained extension hosts and public-loader removal | Define family-owned, size-versioned C ABIs for game modules, server plug-ins, tool plug-ins, and retained mod shaders. Add resolver/trust policy and fixture suites, then remove loader APIs from Tier 1 and `IFileSystem`. | 4–5, 18–19 | L–XL, 12–20 |
| 21 | Tier globals and domain decomposition | Migrate consumers to explicit dependencies domain by domain, delete each compatibility global after its last consumer, create cohesive domain targets, and retire numbered tiers only when empty of ownership responsibility. | 1–6; incremental thereafter | XXL, 40–80 |

### Critical path and parallel work

The critical dependency spine is:

```text
guardrails -> vocabulary -> composition -> loader/path containment
                                      |
                                      +-> window -> render -> presentation -> SDL3
                                                                  |
                                      sequences -> platform ------+-> Vulkan
                                                           SDL3 --+-> new OS proof
```

After rank 4, loader inventory and extension-host design may proceed alongside
window work. After rank 8, null-renderer tests, SDL3 implementation, and task
runner adaptation may proceed in parallel. Tool cleanup may proceed alongside
Vulkan after the path/resolver boundary is stable. Tier-global migration is a
continuous ratchet, but broad cleanup must not consume the team needed to prove
the new provider path.

### Delivery checkpoints

Each checkpoint produces a usable architectural outcome:

1. **Architecture controlled — ranks 1–4.** New leakage is impossible, the test
   application composes explicit providers, and native loading is contained.
2. **Headless product proven — ranks 5–6.** Dedicated startup has no renderer or
   desktop UI dependency and path/loading policy is separated.
3. **Replaceable desktop platform — ranks 7–10.** SDL2 and SDL3 implement the
   same window/input contracts and render interop is pair-specific.
4. **Portability and Vulkan feasibility proven — ranks 11–15.** Foundation
   capabilities pass conformance tests, an SDL3 Vulkan surface presents,
   validation is clean, and a second OS uses the same portable code.
5. **Vulkan opt-in product — rank 16.** A representative game workload renders
   through the native provider; unsupported features fail explicitly.
6. **Vulkan release candidate — rank 17.** Correctness, recovery, diagnostics,
   and performance meet declared product budgets.
7. **General loader retired — ranks 18–20.** Only named extension hosts perform
   native loading; first-party composition is typed and build-assembled.
8. **Legacy architecture retired — rank 21.** Tier globals are no longer
   architectural dependencies and numbered tiers no longer own unrelated
   services.

### Explicit scope boundaries

The following are not silently included in the estimates:

- redesigning the material language or replacing all legacy shaders;
- achieving bit-identical output across D3D9, translation, and Vulkan backends;
- sandboxing native in-process plug-ins, which requires a process boundary;
- rewriting the job system before task-runner adapters prove insufficient;
- converting every compile-time OS conditional, including legitimate compiler,
  CPU, ABI, and structure-layout decisions;
- retiring `CreateInterface` before retained extension hosts have replacement
  ABIs;
- deleting an entire tier merely to satisfy a directory-layout milestone;
- making Vulkan the default before the parity checkpoint and a separate product
  readiness decision.

### Program sizing interpretation

Ranks 1–10 form the clean SDL3/platform program. Ranks 11–17 form the foundation,
new-OS proof, and Vulkan program. Ranks 18–21 complete loader retirement and
broad legacy cleanup.

Using the ranges above, the deliberately rough aggregate scope is:

| Program | Aggregate effort |
| --- | ---: |
| Governable architecture plus clean SDL3 platform path, ranks 1–10 | 55–94 engineer-weeks |
| Foundation, new-OS proof, and Vulkan feasibility through parity, ranks 11–17 | 76–142 engineer-weeks |
| Loader retirement, tool cleanup, and tier migration, ranks 18–21 | 72–134 engineer-weeks |
| Complete RFC program | 203–370 engineer-weeks |

These totals must not be converted directly into a ship date. Some packages can
run concurrently, while integration, review bandwidth, platform availability,
and discoveries in the first Vulkan vertical slice add calendar overhead. The
ranges primarily show that clean SDL3/Vulkan enablement is a bounded program,
while complete tier and loader retirement is a second, longer modernization
program.

The first two programs are large but bounded. The third is a repository-wide
modernization program and must be funded as a sequence of domain migrations,
not presented as one terminal cleanup change. Estimates should be recalibrated
after checkpoints 1, 3, and 4 using observed migration rates and the Vulkan
smoke implementation.

## Performance considerations

Capability-based design does not require a virtual call for every low-level
operation. Providers are selected at composition time, and hot subsystems may
retain direct references, function tables, or backend-specific internal types
behind a portable facade.

The following should generally remain compile-time or inline:

- byte order and fixed-width types;
- atomics when supplied by compiler intrinsics or a portable library;
- CPU pause and prefetch intrinsics;
- data-layout constants;
- performance-critical implementation details contained entirely within a
  selected provider.

Abstraction is warranted when there are multiple production implementations,
when a deterministic test implementation is valuable, or when behavior and
ownership must be isolated from native APIs.

## Risks and mitigations

### A capability bag becomes a new service locator

Mitigation: the full provider set exists only in composition code. Consumer APIs
receive narrow dependencies, and architectural checks prohibit global access to
the composition object.

### Excessive abstraction obscures native behavior

Mitigation: contracts describe only behavior the engine uses. Provider-specific
diagnostics retain native error codes and messages. Unsupported semantics are
reported rather than normalized away.

### Migration creates two lifecycle systems

Mitigation: the application host owns both new providers and legacy adapters and
maintains one dependency order. Compatibility bridges do not own services or
introduce an independent lifecycle.

### ABI changes break existing modules

Mitigation: retain current versioned interfaces and `CreateInterface` during
migration. Add new versions when contract changes are unavoidable; do not alter
published vtable layouts in place.

### Virtual dispatch harms hot paths

Mitigation: resolve providers once, retain direct references, and avoid
capability interfaces for operations better expressed through compiler
intrinsics or selected implementation units. Measure before replacing an
interface with a more specialized binding.

### Platform behavior diverges subtly

Mitigation: shared conformance tests define observable behavior. Platform
extensions remain provider-private until promoted into a documented optional
capability.

## Alternatives considered

### Continue extending platform macros

This has the lowest immediate cost but preserves a porting surface spread across
public headers and multiple tiers. It does not improve testing, ownership, or
dependency visibility.

### Split every platform branch into a separate source file

This improves source organization but does not prevent feature code from
depending on OS identity or tier globals. Source-file separation is useful
inside a backend, not sufficient as the architecture.

### Introduce one `IPlatform` interface

A single interface centralizes native operations but creates a broad dependency
available everywhere. It becomes difficult to test, version, and divide among
dedicated-server, client, and tool compositions.

### Introduce a global typed service registry

This is safer than untyped global pointers but still hides dependencies and
retains process-global lifetime. A typed registry may be used transiently by
the composition root or to adapt dynamic modules, but not as the consumer API.

### Rewrite all tiers before adding another OS

This creates excessive risk and delays useful portability work. Incremental
adapters allow each extracted capability to improve the architecture while the
engine remains buildable.

## Resolved design decisions

The following decisions are intentionally opinionated. They adopt useful
Chromium patterns without importing Chromium's implementation framework:
directory-level dependency enforcement, small public service surfaces, explicit
construction injection, scoped native resources, task sequences instead of
feature-owned physical threads, and strong separation between public contracts
and private implementations.

Relevant Chromium precedents include its
[modularity principles](https://chromium.googlesource.com/chromium/src/+/main/docs/chrome_browser_design_principles.md),
[service directory rules](https://chromium.googlesource.com/chromium/src/+/main/services/README.md),
[dependency checker](https://chromium.googlesource.com/chromium/src/+/main/buildtools/checkdeps/README.md),
[threading model](https://chromium.googlesource.com/chromium/src/+/main/docs/threading_and_tasks.md),
[`expected` result type](https://chromium.googlesource.com/chromium/src/+/main/base/types/expected.h),
[native path value](https://chromium.googlesource.com/chromium/src/+/main/base/files/file_path.h),
and
[scoped native-library ownership](https://chromium.googlesource.com/chromium/src/+/main/base/scoped_native_library.h).
These are design references, not new Source dependencies.

### Foundation result and error type

Foundation contracts use a no-exception `Expected<T, E>` value type and
`Expected<void, E>` for operations that return no value. This follows the shape
of Chromium's `base::expected` while remaining implementable in the C++ dialect
supported by this repository.

`Expected` is a vocabulary type in the foundation contract library. It is
`nodiscard` where supported, performs no hidden logging, and does not allocate.
Accessing a value when an error is present is a programmer error and asserts or
terminates according to the build policy; recoverable failures must be inspected
explicitly.

Each domain owns a compact error enum. A shared error payload contains the
domain code, an optional provider-native integer code, and a stable operation
identifier. Human-readable context is added at the application boundary where
the input path, provider, or request is known. Foundation code must not flatten
all failures into one global error enum or allocate arbitrary diagnostic
strings on every failure.

This C++ value type is not an extension ABI. Stable extension entry points use
explicit C-compatible result structs with a structure size and version; the
host converts those results into `Expected` values.

### Dynamic-library ownership

A loaded native library is represented by a non-copyable, scoped value object
containing an opaque handle. It closes the handle in its destructor and may be
moved or explicitly transferred where the compiler permits. It is not a
reference-counted object and does not require one virtual interface allocation
per library.

The loader provider creates the scoped value, but the value captures everything
required to close its handle safely. A loader provider cannot be destroyed while
handles it created remain live. Symbol pointers are borrowed values bounded by
the scoped library lifetime.

This type is private to approved extension hosts after loader retirement. It is
modeled after Chromium's scoped native-library handle, not exposed as an engine
service.

### Threads, sequences, and injectable scheduling

Atomics, mutexes, condition variables, thread-local storage, and the minimal
physical-thread wrapper remain compiler- or platform-backed foundation
primitives. They are not virtual capabilities and are not injected into every
consumer.

Features depend on execution behavior rather than physical threads:

- `ITaskRunner` posts independent work;
- `ISequencedTaskRunner` guarantees ordered, non-overlapping execution;
- `ISingleThreadTaskRunner` is reserved for APIs with actual physical-thread
  affinity;
- clocks and delayed scheduling are injectable so tests can use virtual time;
- thread creation, naming, affinity, and priority are owned by the scheduler or
  a small number of process infrastructure components.

Sequences are preferred to dedicated threads. Feature code must not create a
thread merely to obtain serialization, and it must not sleep to coordinate
work. Thread-affinity and sequence-affinity checks are enabled in diagnostic
builds. Blocking file or module operations must declare that they may block and
run on an appropriate runner.

The first migration introduces runners around existing engine queues; it does
not require replacing the job system before capability extraction begins.

### Path representation and encoding

There is no single string encoding that losslessly represents every native
path on every supported system. Contracts therefore use two distinct value
types:

- `NativePath` is an opaque, immutable platform path that preserves the native
  representation: UTF-16 semantics on Windows and an uninterpreted byte path on
  POSIX-like systems. Portable code manipulates components through path methods
  and cannot retrieve the native storage.
- `VirtualPath` is a validated UTF-8, forward-slash-separated engine namespace
  used for assets, search-path-relative names, manifests, and serialized product
  data. It is not implicitly convertible to a native path.

Conversion occurs only at filesystem/provider boundaries and returns a result.
UI display uses an explicit lossy/display conversion. Network and persistent
formats never serialize the private native representation unless their protocol
defines the platform dependency. This follows Chromium's central lesson from
`FilePath`: preserve native path semantics rather than treating every filename
as UTF-8, while making the native representation less available to consumers.

### ABI identifiers and versions

First-party, linked capability contracts do not use runtime string identifiers.
They use C++ types and typed factories.

Every genuine extension family owns its ABI beside its public contract under
`public/<domain>/abi/`. There is no new central registry equivalent to the Tier
1 interface list. An extension family defines:

- one stable C entry-point name;
- a family identifier;
- a major and minor ABI version;
- size-versioned C-compatible request, host-API, and result tables;
- ownership and shutdown rules;
- compatibility tests and fixture extensions.

Adding fields to the end of a size-versioned table may advance the minor
version. Layout or semantic incompatibility advances the major version.
`CreateInterface` string names remain only in `public/legacy_abi/` compatibility
definitions and are not used by new contracts.

Waf emits an extension-boundary manifest for each product from the ABI targets
it includes. ABI declarations depend only on foundation ABI types and cannot
include an implementation or engine feature header.

### First applications to migrate

Migration proceeds by process role, beginning with compositions that require
the fewest services:

1. the unit-test runner, using explicit test providers and no ambient factory
   chain;
2. the dedicated server, with rendering and desktop UI absent rather than
   stubbed;
3. headless command-line tools such as VVIS, VRAD, VTEX, StudioMDL, and shader
   compilation, converting launchable DLL indirection as each tool migrates;
4. UI tools with a window provider but no game client;
5. the main client and editor compositions last.

The unit-test runner is the reference composition because it immediately tests
whether providers can be replaced. The dedicated server is the reference
product composition because it proves optional capabilities are genuinely
optional. This resembles Chromium's use of explicit process roles with smaller
service sets instead of one universal application initialization path.

### First legacy render adapter boundary

`IShaderDeviceMgr` and `SHADER_DEVICE_MGR_INTERFACE_VERSION` are the narrowest
existing entry point for wrapping a legacy shader module. The first
`LegacyRenderBackendProvider` queries only that entry point initially and owns
all legacy objects obtained through the factory returned by `SetMode`.

The adapter binds `IShaderDevice`, `IShaderAPI`, `IShaderShadow`, and hardware
configuration into one scoped `LegacyRenderServices` object. Those interfaces
do not escape independently into new code, and no new provider implements the
legacy manager interface merely for compatibility.

The raw window pointer and the factory returned by `SetMode` remain contained
inside this adapter until the presentation bridge replaces that path. The
adapter is therefore a migration membrane, not the design of the new render
contract.

### Window and render interop

Portable code receives an opaque `IRenderSurface` owned by the window provider.
It exposes only portable lifetime and surface-state information. It has no
`GetNativeHandle`, generic property bag, platform enum, or query-by-string
escape hatch.

The composition root selects an `IRenderPresentationBridgeFactory` for the
specific pair of providers. The bridge implementation is allowed to include the
private headers of both providers and creates the presentation object from a
surface and device. Examples include SDL3-Vulkan, Win32-D3D, and a headless-null
bridge. For SDL3-Vulkan, only the bridge calls `SDL_Vulkan_CreateSurface`.

This is pair-specific double ownership rather than a universal native-handle
union. The build graph makes every supported pair explicit, unsupported pairs
fail during composition, and generic render code sees only the resulting
`IRenderPresentation`. Surface invalidation and window destruction invalidate
the bridge before either provider is destroyed.

If a future process boundary is introduced, `IRenderSurface` may become a
strongly typed surface token transported over IPC while native ownership stays
in the window/GPU process. No public contract change is required.

## Decision

If accepted, new portability work should follow this RFC immediately:

- new portable code depends on capabilities rather than OS identity;
- new native behavior is implemented inside a selected backend;
- new subsystem dependencies are explicit;
- no new public tier globals are introduced;
- render providers are selected by declared requirements rather than inferred
  from OS identity;
- dynamic-library loading is first contained behind a scoped platform primitive,
  then retired from first-party composition and retained only inside approved
  extension hosts;
- new asynchronous work targets task sequences rather than feature-owned
  physical threads;
- native filesystem paths and portable virtual paths remain distinct;
- stable extension ABIs are family-owned, size-versioned C contracts rather
  than additions to the global `CreateInterface` string namespace.

Existing code is migrated opportunistically according to the phased plan rather
than through a flag-day rewrite.
