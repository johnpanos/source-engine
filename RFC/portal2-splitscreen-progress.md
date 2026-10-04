# Portal 2 local split-screen progress

User request: 2026-10-03. First input setup: **two controllers**. Work stays in
this checkout, as requested. Status: **partial; playable split-screen is not
implemented**. LAN pairing (RFC 0017/R93) is a separate task.

## Observed starting point

The PC co-op menu already submits `ss_map mp_coop_lobby_3`
(`game/client/portal2/gameui/portal2/vcoopmode.cpp`), but the engine implements
neither `ss_map` nor `connect_splitscreen`. The matchmaking server adapter
explicitly downgrades `ss_map` to `map`.

`engine/client.h` exposes one `CClientState cl`; engine local-player queries
return its player slot. `game/client/c_baseplayer.h` aliases every local-player
slot to its single local player, and reports no split-screen partners.
`game/shared/portal2/portal2_base_compat.h` sets `SPLIT_SCREEN_STUBS`, limits the
local-player count to one, and removes active-player guards.
`portal2_engine_compat.cpp` refuses any active slot other than zero and remote
split-screen views. The engine server client reports `IsSplitScreenUser=false`.
Merely enabling the menu or increasing `MAX_SPLITSCREEN_PLAYERS` cannot create a
second playable player.

The SDL adapter previously opened one gamepad, routed its events to legacy
joystick 0, and kept one rumble lease. `MAX_JOYSTICKS=1` participates in public
button/analog enum values: changing it renumbers later button codes and is not a
compatible way to add the second controller.

## First implemented prerequisite: independent controller snapshots

The selected SDL3 input provider now exposes the versioned
[`IGamepadSlots`](../public/inputsystem/igamepadslots.h) contract through its
existing `IAppSystem::QueryInterface`. That header owns snapshot units, slot
assignment, generation, sequence and lifetime obligations. It does not claim
that a controller slot is an engine local-player slot.

`CInputSystem` remains the single device owner. The second controller has its
own input and haptic state; legacy events remain a slot-0 projection. Neither
`IInputSystem`'s frozen vtable nor its button/analog enums changed. The existing
Siri Remote primary-controller policy is retained. SDL2 does not advertise the
new interface.

The working native Waf fixture and its reproduction commands are documented in
[`unittests/platformtest/sdl3/README.md`](../unittests/platformtest/sdl3/README.md).
The fixture uses the actual input shared library and SDL virtual devices;
physical controller and other-platform evidence is still unavailable.

## Remaining product boundaries

These are required work, not installed interfaces or completed gates:

- The engine must own two local-player identities, admission, per-slot command
  delivery/acknowledgment and teardown. Preserve the one-player wire protocol and
  frozen engine/client ABIs; negotiate any extension explicitly.
- Client input, prediction and player-local state must use those identities.
  One controller must not move, shoot, portal or rumble for the other player.
- Server local-player association must govern owner-only data, visibility,
  usermessages and map changes. Two camera rectangles do not prove co-op.
- Main views and HUDs need independent contexts, viewport geometry and view
  history, including portal/nested views. Rendering follows RFC 0016's ownership
  and lab-first rules; temporal history follows RFC 0019.
- The menu's split-screen launch must use the completed engine path and refuse
  unsupported compositions before changing a running game.
- Product acceptance must run both physical controllers through movement,
  independent portal shots, simultaneous portal crossings, pickup/use, death,
  pause/menu routing, controller removal/rejoin, resize, changelevel and exit;
  retain matched images and full-frame measurements. Linux is the first native
  slice; mobile and optional Apple profiles need their own runtime evidence.

No render, gameplay, local-session, multi-view or platform-support gate is
certified by this input change. No legacy render path was changed.

## Recorded verification

Evidence: `run/quality/portal2-splitscreen-input-20261003/` (local, ignored),
including logs, selected toolchain invocations, tracked dirty diff and new-source
snapshots. SDL3 3.4.16, existing `build-p2` Linux x86_64 profile; the new fixture
uses the strict C++20 dialect from `quality/toolchain/policy.json`.

- Actual provider fixture: `CONFORMANCE 102 0`, exit 0.
- Deliberate shared-slot provider: `CONFORMANCE 102 8`, exit 1 as required.
- Full architecture check, baseline verification and inventory verification pass.
- Architecture checker fixtures: 166 tests pass; style checker fixtures: 38 pass.
- Changed-line style has no input-slice violations. The shared checkout's full
  changed-line check currently fails on an unrelated concurrent edit at
  `render/material/surface_program.cpp:300`; that file was not changed here.

These results qualify this input prerequisite only. Physical-controller gameplay
and the remaining product boundaries above have not passed.
