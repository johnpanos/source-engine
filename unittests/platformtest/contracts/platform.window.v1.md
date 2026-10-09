# Contract: `platform.window.v1`

Module: `platform.window-contracts` · Types: `platform::window::IWindowSystem`,
`IEventSource`, `ICursor`, `IClipboard`, `IMessageBox`, `IGamepads`, `Event`
Headers: `public/platform/window/window_system.h`,
`public/platform/window/window_events.h`
Shared rules: `platform/window/input_normalizer.h` (normalization),
`platform/window/window_surfaces.h` (portable surfaces)
Shared suite: `unittests/platformtest/window/window_conformance.h`
Conformance: `unittests/platformtest/window/test_window.cpp` (fake provider and
composition), `test_window_negative.cpp` (sensitivity),
`unittests/platformtest/window_sdl3/test_sdl3_window.cpp` (SDL3 provider)
Test backend: `unittests/platformtest/window/fake_window_system.h`
Providers: the headless fake; SDL3 (`platform/sdl3/window_system`)
RFC: 0001 rank 7 · Migration: R14 · Domain: Q-FOUNDATION

## 1. Purpose, consumers, required vs optional

A window provider gives portable code windows, their render surfaces and
normalized input without exposing SDL, Win32, Cocoa, UIKit or Android types.
Consumers declare which capabilities they need through the composition kernel
(`platform.composition.v1`):

| Capability | Name | Required |
| --- | --- | --- |
| `IWindowSystem` | `platform.window-system` | yes |
| `IEventSource` | `platform.event-source` | yes |
| `ICursor` | `platform.cursor` | optional |
| `IClipboard` | `platform.clipboard` | optional |
| `IMessageBox` | `platform.message-box` | optional |
| `IGamepads` | `platform.gamepads` | optional |

A provider that cannot do something omits the capability; it never exports a
no-op. An absent optional capability is a null reference in the consumer's
dependency view, and a composition that needs a missing required capability
fails by name before any provider starts.

Launcher product policy is not part of the provider: focus-gated motion,
recentring warps, raw-input selection, the rendered-to-window scale and the
legacy `CCocoaEvent` encoding stay with the consumer.

## 2. Accepted inputs

- Every call is made on the composition's owning sequence. The provider adds no
  thread requirement and starts no thread.
- `WindowDesc` sizes are positive window coordinates. Zero or negative sizes are
  `InvalidArgument` (`create.rejects_zero_size`, `create.rejects_negative_size`,
  `resize.rejects_zero`).
- `WindowId` values come from `Create` on the same provider. A never-created or
  destroyed id is `UnknownWindow` on every operation that takes one
  (`size.unknown_window`, `title.unknown_window`, `resize.unknown_window`,
  `cursor.warp_unknown_window`, `cursor.relative_unknown_window`,
  `message.unknown_parent`, `destroy.twice_unknown`).
- `CursorShape` must be below `Count` (`cursor.shape_invalid`).
- Rumble intensities are in [0, 1] (`gamepad.rumble_invalid_intensity`); an
  unknown device instance is `UnknownDevice` (`gamepad.rumble_unknown_device`).

## 3. Results and guarantees

Errors are `WindowError` values (`foundation::Error<WindowStatus,
WindowOperation>`) naming the status and the operation; they allocate nothing.

Windows:

- `Create` returns distinct, nonzero ids that are never reused while the
  provider lives (`create.distinct_ids`, `destroy.ids_not_reused`).
- `GetSize` is the logical size in window coordinates and `GetPixelSize` the
  drawable size in pixels (`size.logical`).
- `SetSize` is a request; completion is reported by `WindowResized` and
  `WindowPixelSizeChanged`, which may arrive after later polls on asynchronous
  platforms (`resize.request`, `resize.event`, `resize.pixel_event`,
  `resize.size`).
- A hidden window exists and reports a zero drawable extent
  (`create.hidden`, `create.hidden_zero_extent`, `create.hidden_destroy`).
- `WindowCloseRequested` never closes the window; only `Destroy` does
  (`window.close_keeps_window`).

Surfaces (the window side of `render.presentation.v1`):

- Each window has one `render::IRenderSurface`, distinct per window, available
  after `Create`, whose drawable extent equals `GetPixelSize`
  (`create.surface_available`, `create.surface_extent_matches_pixels`,
  `create.surfaces_distinct`). An unknown window has no surface
  (`surface.unknown_window_null`).
- The drawable extent is zero while the window is hidden or minimized and is
  restored with it (`window.minimized_zero_extent`, `window.restored_extent`).
- The drawable extent already holds the new size when a size event is delivered
  (`resize.extent_before_delivery`, `resize.surface_matches_pixels`).
- One presentation listener per surface (`surface.attach`,
  `surface.single_listener`).
- Entering the background tells the attached presentation, then makes the
  surface unavailable; returning restores it with a new generation
  (`surface.background_release`, `surface.foreground_restore`).
- `Destroy` tells the attached presentation while the native window still
  exists, marks the surface `kDestroyed`, drops every queued event for the
  window and leaves other windows untouched (`destroy.listener_notified`,
  `destroy.surface_destroyed`, `destroy.surface_lookup_null`,
  `destroy.size_unknown`, `destroy.drops_queued_events`,
  `destroy.keeps_other_windows`, `destroy.first`, `destroy.last`,
  `destroy.all_unknown`). The surface object stays valid while a presentation
  remains attached; the provider reclaims it afterwards.

Events:

- `Poll` copies up to `out.size()` pending events in native order; the rest
  stay queued. Nothing is dropped or duplicated, and an empty span consumes
  nothing (`poll.empty_span_consumes_nothing`, `poll.capacity_one_no_loss`,
  `poll.keys_after_skipped_text`, `poll.fifo_mixed`). Expansion of one native
  event is bounded by `EventFifo::kMaxExpansion`.
- `Wait` returns whether `Poll` would now return at least one event.
- Only the member of `Event` named by `type` is meaningful. Application and
  gamepad events carry no window.

Normalization (one owner: `InputNormalizer`; the rules of the launchers'
existing behavior):

- Keys report the USB HID usage of the physical key and the usage the active
  layout makes it act as (letters and `= - [ ] ; ' , . /` by legend, remapped
  modifiers by the modifier they act as). Modifier state follows the layout
  usage, tracks left and right separately, counts caps lock while held and is
  reported after the transition (`key.usage_and_window`, `key.layout_usage`,
  `key.modifier_after_transition`, `key.sides_independent`,
  `key.caps_while_held`, `key.modifier_by_layout`, `key.all_modifiers`).
- Text arrives one Unicode scalar value per event with the current modifiers;
  malformed UTF-8 becomes U+FFFD (`text.codepoints`, `text.modifiers`,
  `text.malformed_replacement`).
- Native buttons 1/2/3 are left/middle/right; higher buttons fold onto X1
  (even) and X2 (odd). Events carry the held-button mask after the event
  (`mouse.button_fold`, `mouse.held_mask`, `mouse.motion`).
- A press within the double-click time and distance of the previous press is a
  double click and starts a new pair (`mouse.double_click`,
  `mouse.double_click_distance`, `mouse.double_click_timeout`).
- Wheel values are detents; positive is right and away from the user,
  whatever the platform's scroll-direction setting (`mouse.wheel`).
- Sticks map [-32768, 32767] onto [-1, 1] and triggers [0, 32767] onto [0, 1];
  out-of-range values clamp. Platforms need not report an unchanged axis
  (`gamepad.added`, `gamepad.count`, `gamepad.buttons`, `gamepad.axes`,
  `gamepad.removed`).
- Touch positions and deltas are normalized to the touch surface
  (`touch.sequence`, `touch.values`).

Cursor, clipboard and message box (when exported):

- `SetVisible` and `IsVisible` agree (`cursor.hide`, `cursor.show`).
- `Warp` moves the pointer, and the motion the platform synthesizes for the
  warp itself is not delivered; later motion is (`cursor.warp`,
  `cursor.warp_motion_suppressed`, `cursor.motion_after_warp`).
- Relative mode toggles or fails with a structured `Unsupported` error
  (`cursor.relative_toggle`, `cursor.relative_structured_error`); every shape
  below `Count` can be set (`cursor.shapes`).
- The clipboard round-trips UTF-8, overwrites, and reads an empty clipboard as
  an empty string (`clipboard.set`, `clipboard.roundtrip_utf8`,
  `clipboard.overwrite`, `clipboard.empty`).
- `ShowError` is modal and returns after dismissal (`message.shown`; recorded
  as a skip on native providers, which need a user to dismiss it).

## 4. Lifetime, ordering and failure

- The provider follows the composition lifecycle: `Connect`, `Initialize`,
  `Shutdown`, `Disconnect`. Shutdown destroys every window (telling attached
  presentations first), closes devices and clears queued events; a second
  instance after teardown behaves like the first (repeat instance).
- A provider over process-global platform state (SDL) admits one connected
  instance per process; a second `Connect` fails structurally and a new one
  succeeds after teardown.
- Provider-generated window events may arrive at any time; consumers and the
  shared suite judge only the events they caused.

## 5. Native providers

The SDL3 provider (`platform_sdl3::Sdl3WindowSystem`) runs the shared suite
with native input pushed as SDL3 events through its real decoding path, SDL
virtual gamepads on SDL's device path, and the real video driver for windows,
sizes, surfaces, cursor and clipboard: offscreen in `platform.window.sdl3`,
and real Wayland and X11 drivers in private sessions in
`platform.window.sdl3.native-drivers` (`tools/quality/window_sdl3_lane.py`). It keeps the
product SDL3 launcher's behavior (`appframework/sdl3mgr.cpp`) where the rules
leave room, checked by the suite's `sdl3.product.*` clauses:

- a flipped (natural) wheel is reported unflipped;
- pointer positions truncate like the launcher's; fractional relative motion
  carries into the next event instead of being lost;
- text input starts with each window, except where it would raise an on-screen
  keyboard;
- gamepad input is withheld while none of the process's windows has focus
  (SDL's default and the launcher's behavior); `gamepadsWithoutFocus` is the
  composition root's explicit opt-out, which the native suite sets because a
  private compositor gives its windows no focus;
- its surfaces are `platform/sdl3/render_surface` objects, so the SDL3
  presentation bridges present to its windows through `RenderSurfaces()`; a
  destroyed window's surface is held while a presentation is attached and
  reclaimed after it detaches.

Two SDL behaviors are the driver's to account for, not provider defects: SDL
holds a released Guide button for a minimum time, and a virtual trigger rests
at half travel.

## 6. Negative evidence

`platform.window.sensitivity` runs the same suite against deliberately broken
fakes and requires each seeded defect to fail its named check.
