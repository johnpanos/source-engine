# RFC 0004 progress: Box3D Primary Physics Backend

Updated: 2026-09-22
Source revision at assessment: `87955f67` (working tree; AGENTS.md portfolio rows: R09, R19, R31, R34, R37, R44, R45)

This file is the human-readable, durable progress record for RFC 0004. It tracks the phased implementation of replacing IVP/Havana with Box3D as Source's primary rigid-body physics backend.

## Status

| Work item | Phase | Status | Evidence |
| --- | --- | --- | --- |
| Interface & Consumer Inventory | A | Not started | — |
| Asset Corpus & IVP Baselines | A | Not started | — |
| Event & Contact-mutation Prototypes | A | Not started | — |
| Box3D Build Integration (Pinned) | B | Not started | — |
| VPhysics Adapter & World Lifecycle | B | Not started | — |
| Legacy Geometry Decoder | B | Not started | — |
| Trace & Query Foundation | B | Not started | — |
| One-ragdoll & Impact Event | B | Not started | — |
| Constraints & Player Controllers | C | Not started | — |
| Persistence & Materials | C | Not started | — |
| Fluids & Vehicles | D | Not started | — |
| Tool Workflows (`studiomdl`, `vbsp`) | D | Not started | — |
| Parallel Scheduler Integration | E | Not started | — |
| Performance Budgets & Packaging | E | Not started | — |
| Independent Collision Cooking | F | Not started | — |
| IVP Simulation Retirement | F | Not started | — |

## Scoped Actionable Tasks

### Phase A: Inventory and Feasibility (R09)
* **A1. Interface & Consumer Inventory:** Map all `IPhysics*` interface usages across engine, game DLLs, and tools. Produce a method-level inventory recording consumers, implementation strategy, tests, and profile requirements.
* **A2. Asset & Content Corpus Selection:** Identify the first supported game configuration (e.g., Linux x86-64 HL2-family). Collect representative maps, problematic assets (oversized hulls, multi-material meshes), and save games.
* **A3. IVP Baseline Measurements:** Measure memory usage, load time, and median/p95/p99 frame execution time for the content corpus using the existing IVP backend. Set explicit performance budgets.
* **A4. Pre/Post Collision State Prototype:** Build a prototype resolving the callback timing difference between Source (reads velocities in `PreCollision`) and Box3D (buffers contacts). 
* **A5. Contact Mutation Prototype:** Implement and verify a strategy for `IPhysicsFrictionSnapshot` mutations (clearing friction impulses, deleting contacts).
* **A6. Ragdoll Limits Analysis:** Fit Source's 3-axis ragdoll limits to Box3D's spherical joint cone/twist limits and determine if a solver extension is required.

### Phase B: Runtime Vertical Slice (R19)
* **B1. Box3D Build Integration:** Pin Box3D (`9e5a4cde...`), configure C17 compilation via Waf under a private target, preserving standard/FP settings without inheriting Source's C++ flags.
* **B2. Adapter & World Lifecycle:** Implement `vphysics_box3d` export, `IPhysicsEnvironment` wrapping a Box3D world, and provider selection via `--physics-backend`.
* **B3. Legacy Geometry Decoder:** Implement a decoder to read legacy `.phy` and BSP physics lumps to generate Box3D hulls, baked compounds, and static meshes, preserving IDs and surface properties.
* **B4. Trace Compatibility:** Implement basic raycasts and swept AABB traces for `IPhysicsCollision`, adding explicit handling for inside-starts, backfaces, and exit fractions.
* **B5. Single Dynamic Prop & Event Bridge:** Spawn a compound dynamic prop, deliver a verified impact event using Phase A's prototype, and ensure basic sleep/wake states match.
* **B6. One Ragdoll & Basic Save:** Spawn a single ragdoll utilizing the Phase A limits prototype. Implement a basic save/restore round trip for supported objects.

### Phase C: Core Compatibility (R31)
* **C1. Advanced Traces & Filters:** Complete `IConvexInfo` filtering, displacement surface collision, compound model traces, and exact clipping tolerances.
* **C2. Constraint Completion:** Implement hinge, sliding, fixed, ball socket, and length joints with tested equivalence for limits, motors, and break behavior.
* **C3. Shadow & Player Controllers:** Port `vphysics/physics_shadow.cpp`. Map shadow controllers and player pushes to Box3D kinematic/dynamic paradigms.
* **C4. Motion Controllers:** Implement local/global force and acceleration controllers with the correct step cadence and attachment lifetime rules.
* **C5. Full Persistence:** Finalize `Save`/`Restore` protocol. Ensure identity remapping, controller reconstruction, and strict dependency-order restoration.
* **C6. Materials & Object Lifecycle:** Finalize deferred deletion, cross-world `TransferObject`, and exact mapping of mass/drag/contents metadata.

### Phase D: Gameplay Completion (R34)
* **D1. Fluids & Buoyancy:** Implement fluid volume sensors, submerged volume computation, drag, buoyancy overrides, and touch events.
* **D2. Vehicles:** Implement all raycast and wheel-joint vehicle modes, covering steering curves, suspension, traction, script reloading, and water interactions.
* **D3. Complex Constraints:** Implement pulley semantics (coupled rope length/gearing) and constraint groups with group-specific solver iteration policies.
* **D4. Game Variants & Portal Support:** Validate specific variants like Portal object transfer, shadow clones, and related collision rules.
* **D5. Tool Workflows:** Update `studiomdl`, `vbsp`, and inspection utilities to seamlessly handle Box3D representations and size limits.

### Phase E: Performance & Rollout (R37)
* **E1. Parallel Scheduler Integration:** Bridge Box3D's worker system to Source's job system (RFC 0003), ensuring deterministic worker counts and preventing deadlocks.
* **E2. Platform Expansion:** Expand validation checks and builds to Windows x86-64, macOS, iOS, and Android.
* **E3. Profiling & Optimization:** Optimize against the Phase A baseline budgets. Confirm memory, load time, and frame cost meet targets.
* **E4. Soak Testing:** Run extended multi-player and client/server dedicated sessions to verify stability, networking determinism, and authority.
* **E5. Packaging & Rollback:** Finalize the `--physics-backend=ivp\|box3d` packaging, guaranteeing a safe rollback path with documented limitations.

### Phase F: Retirement (R44, R45)
* **F1. Native Cooking Pipeline:** Define a new versioned collision format. Support pre-cooking Box3D native geometry to avoid runtime legacy conversion overhead.
* **F2. Independent Legacy Decoder:** Decouple the legacy `.phy` parser completely from the old IVP implementation.
* **F3. IVP Simulation Removal:** Remove the IVP runtime simulation once all profiles have successfully promoted Box3D as the default.

