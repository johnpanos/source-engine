# RFC index

Part of [AGENTS.md](../../AGENTS.md). The RFCs own semantics and acceptance; this index says what each one is responsible for.

| RFC | Responsibility |
| --- | --- |
| [0001](../../RFC/0001-capability-based-platform-architecture.md) | Capability-based platform, composition, SDL3/render/Vulkan, loader and tier retirement; platform code only in providers (CAP012) |
| [0002](../../RFC/0002-hammer-responsibility-factorization.md) | Headless editor core, GTK4 host, legacy Hammer extraction |
| [0003](../../RFC/0003-dependency-aware-job-system.md) | Dependency graphs, deterministic execution, one task API (J8) |
| [0004](../../RFC/0004-box3d-primary-physics-backend.md) | Box3D behind VPhysics, legacy assets, rollout |
| [0005](../../RFC/0005-quality-and-correctness-harnesses.md) | Eight harness families, shared runner and evidence |
| [0006](../../RFC/0006-modern-cpp-ownership-and-synchronization.md) | C++20 targets, results, ownership, queues, CPU/GPU publication |
| [0007](../../RFC/0007-physically-based-lighting-pipeline.md) | Light baker (vrad, Cycles), PBR materials, IBL, compile tools |
| [0008](../../RFC/0008-canonical-world-data-and-runtime-formats.md) | USD World Stage, BSP2/KTX2, modern map and model resources |
| [0009](../../RFC/0009-usd-native-map-authoring.md) | USD map source and native compiler (proposed) |
| [0010](../../RFC/0010-portable-vgui-surface.md) | VGUI beneath its frozen API (proposed, no row) |
| [0011](../../RFC/0011-runtime-indirect-lighting.md) | Runtime indirect light producers (opt-in; rows R70–R80 unranked) |
| [0012](../../RFC/0012-antialiasing-msaa-specular-alpha-coverage.md) | MSAA, alpha to coverage, filtered mips (A2 specular AA removed) |
| [0013](../../RFC/0013-opt-in-physics-capabilities.md) | Opt-in Box3D capabilities, parallel step |
| [0014](../../RFC/0014-native-vulkan-and-bsp2-debug-controls.md) | Render core and BSP2 debug controls |
| [0015](../../RFC/0015-asset-identity-content-build-graph.md) | `AssetRef`, content build graph, packages, resolver, live reload |
| [0016](../../RFC/0016-render-core.md) | Clustered Forward+ render core, ports and adapters, binding rules, anti-corruption boundary, [adapter freeze](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10) |
| [0017](../../RFC/0017-lan-discovery-and-coop-pairing.md) | LAN discovery and Portal 2 co-op pairing (proposed) |
| [0018](../../RFC/0018-hammer-interaction-design.md) | Hammer interaction and UI/UX (proposed) |
| [0019](../../RFC/0019-temporal-upscaling-contract.md) | Temporal upscaling; FSR 4.1.1 as the intended High AA (proposed) |
| [0020](../../RFC/0020-native-game-ui-qualification.md) | Native game UI qualification scenes (proposed, no row) |
| [0021](../../RFC/0021-external-render-sdk-and-runtime-provider-selection.md) | External render SDKs and runtime provider selection (proposed; V0 licensing open) |
| [0022](../../RFC/0022-opengl-es-3.1-compatibility-preset.md) | GLES 3.1 dialect of the GL adapter (proposed, child of R92) |
| [0023](../../RFC/0023-release-play-builds.md) | Release builds of the play products (proposed, no row) |
| [0024](../../RFC/0024-direct3d12-device-adapter.md) | **Withdrawn 2026-10-10** (adapter set is Vulkan, GL/GLES, null; [decision](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10)); code deleted with the adapter |
| [0025](../../RFC/0025-metal-device-adapter.md) | **Withdrawn 2026-10-10** (adapter set is Vulkan, GL/GLES, null; [decision](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10)); code deleted with the adapter |
| [0026](../../RFC/0026-box3d-beyond-ivp.md) | Box3D beyond IVP with IVP as fallback (row R98) |
| [0026 (PICA200)](../../RFC/0026-pica200-device-adapter.md) | **Withdrawn 2026-10-10** (adapter set is Vulkan, GL/GLES, null; [decision](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10)); code deleted with the adapter |
| [0027](../../RFC/0027-product-pipeline-lowering-streaming-kiln.md) | One product pipeline and `kiln`; formats stay in their libraries (row R102) |
| [0028](../../RFC/0028-direct3d9-device-adapter.md) | **Withdrawn 2026-10-10** (adapter set is Vulkan, GL/GLES, null; [decision](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10)); code deleted with the adapter |
| [0029](../../RFC/0029-webassembly-and-webgpu-platform.md) | **Withdrawn 2026-10-10** (adapter set is Vulkan, GL/GLES, null; [decision](../../RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10)); code deleted with the adapter |

Extra scope beyond the four north-star targets (tvOS, MSVC under Wine for
the dedicated server; 3DS, WebAssembly and Direct3D 12 withdrawn 2026-10-10) exists by user direction, adds no north-star
criterion, and has no ranked row unless the user ranks it.
