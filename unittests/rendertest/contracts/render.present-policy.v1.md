# render.present-policy.v1

`public/render/render_present_policy.h` owns swapchain present-mode selection
for `mat_vsync` / `ShaderDeviceInfo_t::m_bWaitForVSync` and
`RenderPresentationConfig::vsync`:

- With vsync, the presenter waits for vertical blank (FIFO, which every Vulkan
  surface supports).
- Without vsync, it never waits when it can avoid it: immediate (D3D9's
  `D3DPRESENT_INTERVAL_IMMEDIATE`) when offered, then mailbox, then FIFO as the
  only guaranteed mode.

Every native Vulkan swapchain (the legacy `CVulkanContext` and the
`render.presentation.v1` SDL3 bridge) selects through this one function and
reports the effective mode.

The suite checks all eight vsync/support combinations against the table. Its
sensitivity build runs a mailbox-first selector, which the oracle must reject.
