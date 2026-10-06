# Vendored Wayland protocol code

Generated client code for `color-management-v1` (staging), which the SDL3–Vulkan
bridge uses to give a game surface the output's own image description
(`sdl3_dynamic_range_wayland.cpp`). The protocol XML is MIT licensed; the
copyright notices are kept in the generated files.

- Source: `wayland-protocols` 1.49, `staging/color-management/color-management-v1.xml`
  (sha256 prefix `18a2678e3352c3be`).
- Generator: `wayland-scanner` 1.26.0.

Regenerate:

```sh
X=/usr/share/wayland-protocols/staging/color-management/color-management-v1.xml
wayland-scanner client-header $X color-management-v1-client-protocol.h
wayland-scanner private-code  $X color-management-v1-protocol.c
```
