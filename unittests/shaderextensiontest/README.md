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
The `--resize-stress` mode presents 516 complete sampled frames across two native
window/device lifetimes and 128 odd/even grow/shrink requests. Each request
presents the prior complete image while resize is asynchronous, acknowledges the
compositor size, uses the actual drawable pixels for reset, verifies viewport
convergence, and checks every pixel in three resulting frames. The
`--negative-resize-pixel-oracle` mode corrupts one expected pixel after resize to
prove the oracle rejects a divergence. Dimensions and the first bad pixel are
reported for reproduction.

GPU readback before each successful present proves the submitted images remain
complete. It does not measure compositor-visible flicker or interactive latency;
those require the separate real Portal capture and native window evidence.
These checks supplement that scene and do not certify complete renderer parity
or all extension families.

For the configured Portal Vulkan profile:

```sh
WAFLOCK=.lock-waf-portal-vulkan ./waf build \
  --targets=shader_extension_conformance,builtin_shader_conformance,material_binding_conformance,dxvk_math_conformance,dxvk_presentation_conformance
LD_LIBRARY_PATH="$PWD/build-portal-vulkan/materialsystem/stdshaders:$PWD/build-portal-vulkan/tier0:$PWD/build-portal-vulkan/vstdlib" \
  build-portal-vulkan/unittests/shaderextensiontest/builtin_shader_conformance
build-portal-vulkan/unittests/shaderextensiontest/shader_extension_conformance
```

Native fixtures require the configured SDL3/DXVK runtime library paths and an
active Wayland session. Product runs capture their actual provider, GPU,
source/build identity and images through `tools/quality/portal_boot.py`.

The built-in standard material shaders are now a normal linked dependency.
`public/render/builtin_shader_provider.h` supplies typed host borrows and a
descriptor; the launcher selects a declared ID or compatible legacy basename
and binds a copied descriptor before the material system connects. The material
API retains a Waf-selected default for old clients without an explicit binding.
There is no built-in filename search or ABI factory negotiation. The dedicated
composition contains only its local shader library. Other catalogs must be added
explicitly by their product build before they can be selected.

`test_builtin_shaders.cpp` links the actual standard shader library and exercises
required-service rejection, unknown/debug/path selection rejection, real shader
enumeration, duplicate connection rejection, CVar registration and cleanup, and
two complete connections. This caught and fixes the legacy `ConVar_Register`
constructor list being consumed on the first connection: the linked provider now
retains its own static CVar registrations while the mod ABI accessor is unchanged.
The CVar registry and shader borrowers must outlive the provider connection;
materials and dictionaries must drain before disconnect.

Filename requests from the public compatibility API refer either to the existing
linked builtin or the separately owned mod extension host. Repeated requests for
an already connected extension retain that connection; replacement requires a
material drain and `ModShutdown` first. This prevents reconnecting a legacy
singleton and then invalidating it while disconnecting its previous host.

The reviewed inventory removes all direct loader calls from `shadersystem.cpp`;
the extracted mod host remains `retained-extension`. The broader RFC 0001 Phase D
result also depends on the window, renderer, audio, input, and video catalogs and
the native product evidence recorded in its progress document.

Shader artifact generation and its independent binary/permutation conformance
tests are described in `tools/quality/shader_artifacts.md`.
