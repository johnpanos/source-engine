# Platforms, builds and distribution

Part of [AGENTS.md](../../AGENTS.md). Read before platform, toolchain, profile, packaging or distribution work.


| Target | Runtime path | Native acceptance obligations |
| --- | --- | --- |
| Linux | SDL3 + Vulkan; headless server/tool compositions | X11/Wayland and GPU profiles, surface loss/resize, input, filesystem; no desktop dependencies in headless products |
| macOS | SDL3 + Vulkan via MoltenVK | Apple SDK, portability subset, lifecycle, app bundle and distribution |
| iOS | Static first-party composition; SDL3 + Vulkan via MoltenVK | arm64 device, separate simulator, no shared-module discovery, background/surface recreation, touch, memory pressure, packaging |
| Android | SDL3 + Vulkan | Pinned SDK/NDK, ABIs, activity/surface recreation and process death, touch, permissions, signed APK/AAB |

- Query and validate MoltenVK's portability features; keep Metal private to
  the Apple bridge. A cross-build or smoke frame is not platform acceptance.
- Every product/OS/architecture has a versioned profile that owns its facts
  (toolchain, SDK, ABI, dialects, render capabilities, packaging, required
  tests); CI and reports consume it. Clean isolated output per profile; host
  tools separate from targets; pinned dependencies; no sibling checkouts or
  unpinned downloads.
- Apple and MSVC runners are optional (user decision, 2026-09-25): reported
  unavailable when missing; no gate waits on them. The Fold7 and Android
  x86_64 are optional (user decision, 2026-10-08): Android's required device
  evidence is an arm64-v8a run on any declared device (the Tab S8 Ultra
  qualifies). A support claim on an optional runner still needs its own
  evidence.
- iOS links first-party modules statically and resolves them through typed
  factories; no `dlopen`/`dlsym` or name lookup. Prove it on the final link
  map and with empty module-search paths.
- Store builds download no native code and use no JIT, private APIs or
  background bypasses. Use documented platform APIs, approved write
  locations and minimal permissions; never disable sandboxing or certificate
  checks. Use ordinary platform packaging and signing; build no custom
  signing, attestation or SBOM infrastructure. Check Apple's App Review
  Guidelines (2.5.1/2.5.2) and the Android store's rules before
  distribution-sensitive work. Keep the repository's provenance warning.
