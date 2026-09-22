# Shader compatibility boundary conformance

This suite tests the retained `ShaderDLL004` mod extension and the typed linked
material/render binding. The extension host keeps native module handles and
`CreateInterfaceFn` negotiation in its implementation. Its header exposes only
the owned host and borrowed shader interface. The filesystem outlives the host;
shader borrowers are released before host destruction.

`test_shaderextension_host.cpp` compiles the actual host implementation against
the existing ABI headers and substitutes acquisition/factory entry points. It
tests invalid names, failed load, absent factory, wrong interface version,
partially failed connect, successful use, and disconnect-before-unload order.
The exact fixture file is listed under `legacyAbi.paths` in
`architecture/modules.json`; this grants no factory permission to consumers.

The capability manifest separately owns the host header, private host
implementation, fixture, and typed renderer provider header. Only the host and
ABI fixture may include their enumerated legacy headers. The SDK-free headers
have no native or legacy include permission. Existing strict leak fixtures
continue to reject portable access to native adapters and undeclared SDKs.

The native DXVK fixture presents sampled, transformed indexed geometry and
checks framebuffer pixels before and after device reset. The material binding
fixture tests typed provider selection and the retained compatibility adapter.
These checks supplement the real Portal scene; they do not certify complete
renderer parity or all extension families.

For the configured Portal Vulkan profile:

```sh
WAFLOCK=.lock-waf-portal-vulkan ./waf build \
  --targets=shader_extension_conformance,material_binding_conformance,dxvk_math_conformance,dxvk_presentation_conformance
build-portal-vulkan/unittests/shaderextensiontest/shader_extension_conformance
```

Native fixtures require the configured SDL3/DXVK runtime library paths and an
active Wayland session. Product runs capture their actual provider, GPU,
source/build identity and images through `tools/quality/portal_boot.py`.

The reviewed loader inventory changes for this slice remove material-system
renderer `Sys_LoadModule`/`Sys_GetFactory` calls, classify the extracted mod
shader host as `retained-extension`, and retain the ABI fixture's factory stub
as `tool-indirection`. The remaining loader in `shadersystem.cpp` is explicitly
classified as `first-party-composition`: it still loads built-in `stdshader` and
debug shader libraries. This is an open migration, so global RFC 0001 retirement
Phase D is not complete. Typed backend selection and a working Portal Vulkan
composition are narrower, independently testable outcomes.

Shader artifact generation and its independent binary/permutation conformance
tests are described in `tools/quality/shader_artifacts.md`.
