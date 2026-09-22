# SDL3 native launcher conformance

`sdl3_launcher_conformance` is a Waf target in the SDL3 client profile. It links
the selected production `appframework` provider and real SDL3. Run it with
`SDL_VIDEO_DRIVER=wayland` on a native Wayland session with Vulkan-capable SDL3
window support. A missing display/provider fails; it is not skipped or replaced
with a dummy driver.

The test checks a real Vulkan-only window, idempotent initialization, keyboard
modifiers, full Unicode text delivery, mouse buttons and relative movement,
focus/quit delivery, the 100-event pump boundary, keyboard queue consumption,
window-reference release and recreation, shutdown, deliberately failed video
initialization, and a clean subsequent instance. Checks stay active in release
builds and the final `CONFORMANCE <checks> <failures>` records actual assertions.

This certifies the launcher window/input slice. It does not certify GPU frames,
swapchain recovery, installed Portal gameplay, graphics budgets, or other OS
profiles. Those require their separate native product/presentation runs.
