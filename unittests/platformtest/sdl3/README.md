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

## Two-controller input

`input_gamepad_slots_test` links the production `inputsystem` provider and SDL3,
then attaches two virtual gamepads. Build using the existing Portal 2 profile:

```sh
WAFLOCK=.lock-waf-p2 python3 ./waf build --targets=input_gamepad_slots_test
export LD_LIBRARY_PATH="$PWD/build-p2/inputsystem:$PWD/build-p2/tier0:$PWD/build-p2/vstdlib:$PWD/build-p2/stub_steam${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export SDL_JOYSTICK_HIDAPI=0 SDL_JOYSTICK_RAWINPUT=0
export SDL_JOYSTICK_LINUX_CLASSIC=0 SDL_JOYSTICK_LINUX_JOYSTICK=0
build-p2/inputsystem/input_gamepad_slots_test -nosteamcontroller -nogyro
# Required negative control: nonzero exit and CONFORMANCE failures.
build-p2/inputsystem/input_gamepad_slots_test -nosteamcontroller -nogyro -seed-slot-collapse
```

Disconnect physical controllers when running the deterministic fixture. The SDL
hints disable common physical-device backends, but are not a universal hardware
filter. Physical pads occupying the fixture's slots fail the run, not skip it.

The snapshot contract lives in
[`IGamepadSlots`](../../../public/inputsystem/igamepadslots.h). Checks cover
independent buttons, stick axes, triggers and rumble; stable slot ownership;
excess devices; removal while held; replacement generations; reset and disabled
input; and the unchanged legacy slot-0 projection. The negative control wraps the
real provider with a deliberately incorrect slot-0-only implementation and runs
the same checks. Assertions remain active in release builds.

This is input evidence only. Portal 2 still has one local player/command stream
and one main view. Real two-controller gameplay, controller hardware, HUDs,
portal crossings, map transitions and platform qualification remain unverified.
