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

## `ss_map` refusal (2026-10-05)

The menus' `ss_map <map> [*mp]` was an unknown console command. `engine/host_cmd.cpp`
now registers `ss_map` (client builds only), which warns that local split-screen is
unsupported and changes nothing, so the running game is untouched. This is a
placeholder refusal, not admission: it is replaced when the engine owns two
local-player identities. Not built or run here; no gate changes.

## Retail class layouts (2026-10-05)

`tools/portal2/dsym/class_layout.py` dumps full class layouts from the 852_3
client and server dSYMs into `external/portal2_steam2_decompiled/layouts/`, with
the split-screen members tabulated in its README (player slot, owner/partner
handles, prediction slot, the active-slot guard, `CInput::CheckSplitScreenMimic`).
Reference only; the retail-binary comparison and the engine-side classes (the dSYMs
cover game DLLs only) remain open.

### Retail comparison (2026-10-05)

Against the retail Linux `server.so` (Ghidra project `portal2_retail`): retail's
`IsLocalSplitScreen` script function tests that a player's split-screen vector is
non-empty, and its `GetSplitScreenPlayers()` accessor is `this + 0x11f8`. The 2010
dSYM has that vector at `+0x121c`, so retail `CBasePlayer` differs by 0x24 bytes
and the dSYM offsets are not retail offsets. Only that one member was verified.
Retail client and engine are not imported, so the client layouts and engine classes
remain uncompared. Details in
`external/portal2_steam2_decompiled/layouts/README.md`.

### Retail client and engine imported (2026-10-05)

`client.so` and `engine.so` (retail Linux) are now in the Ghidra project
`portal2_retail` beside `server.so`, analyzed (`ghidra/import_client_engine.sh` in
the research directory). First engine findings (stripped; names from strings):

- `ss_map` (registered at 0x00bc6700, handler `FUN_004995a0`) calls the shared map
  launch helper `FUN_004991f0(args, 8)`. Flag 8 is the split-screen launch mode.
  It is the same helper family as `map`; this fork's `ss_map` refusal stands in
  for it.
- `connect_splitscreen <server> <# of players>` (`FUN_003cced0`, `cl_main.cpp`)
  parses the address, requires at least one player and no more than a global
  maximum (`DAT_00bc3e64`), then runs a per-local-slot loop (a slot guard at
  `cl_main.cpp:0x42f`, vtable slot +0x94 per slot, apparently a disconnect of
  each slot) before one connect call that carries the player count. Slot guard
  and callee names are not yet recovered.

Client-layout comparison against the dSYM is still to do; nothing here is
implemented in the engine.
