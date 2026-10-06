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

Engine and client findings (net messages `net_SplitScreenUser`,
`clc_SplitPlayerConnect`, `svc_SplitScreen`; `ss_connect`/`ss_disconnect`; per-slot
`SPLIT%d` engine state; the retail maximum of 2) are recorded in
[portal2-splitscreen-retail-engine.md](portal2-splitscreen-retail-engine.md),
with observed facts separated from hypotheses.

## Slice 1: split-screen wire codecs (2026-10-06)

`common/splitscreen_wire.h` holds the bit-level codecs for `net_SplitScreenUser`,
`svc_SplitScreen` and `clc_SplitPlayerConnect` (layouts from retail, ids 35/34/18
allocated by this fork; see
[portal2-splitscreen-retail-engine.md](portal2-splitscreen-retail-engine.md),
which also holds the retail flows, the CS:GO source reference and the ordered
slice plan). Contract: `unittests/enginetest/contracts/engine.splitscreen-wire.v1.md`.
Suites `engine.splitscreen-wire` and `.sensitivity` (4 seeded defects detected)
pass on `linux-headless-core`. Nothing uses the codecs yet: not registered as
net messages, no handlers, no engine state. The engine's slot owner is the ported `CSplitScreen` (`engine/cl_splitscreen.cpp`); a clean-room registry written first was deleted so there is one owner.

## Slice 2–4: engine port from the CS:GO tree (2026-10-06, in progress)

User direction: copy as much of the CS:GO engine as can be copied. It is a protobuf-era
engine, so the message layer cannot be dropped in; the split-screen logic is ported
function by function onto this engine's bitstream messages. Builds in `build-p2`
(`waf build --targets=engine`); **not run**: no map has been booted with two players.

Installed in the engine (compiles; no runtime evidence yet):

- **Messages** `net_SplitScreenUser` (35), `svc_SplitScreen` (34), `clc_SplitPlayerConnect` (18):
  `common/netmessages`, handler interfaces get refusing defaults (`public/inetmsghandler.h`).
- **Net channel multiplexing** (`engine/net_chan.*`): `SplitPlayer_t`, attach/detach, merge of
  each attached channel's reliable/unreliable/voice buffers under `net_SplitScreenUser`
  markers on send, active-channel dispatch on receive. `INetChannel` gets three methods with
  refusing defaults.
- **Server** (`baseclient`, `baseserver`, `sv_main`): split clients (`CreateSplitClient`,
  owner/partner links, queued `ss_disconnect`, "leaving splitscreen"), `clc_SplitPlayerConnect`
  admission with the retail refusals, `svc_SplitScreen` add/remove, owner-driven signon,
  `max_splitscreen_players` from the game (`IServerGameClients` version 5,
  `GetMaxSplitscreenPlayers`; engine accepts 003-005), PVS player bits, view-angle updates.
- **Client** (`cl_splitscreen.*` copied from the CS:GO engine, `baseclientstate`, `cl_main`,
  `client.cpp`): slot manager with per-slot `CClientState` (slot 0 is the global `cl`),
  active-slot guard and macros, per-slot `CL_Move` / `CL_SendMove` / extra mouse sampling.
- **Commands**: `ss_map`, `connect_splitscreen <server> <n>`, `ss_connect`, `ss_disconnect`.
  Deliberate deviation: the connectionless connect packet is unchanged. Extra local players
  join through `ss_connect` once the primary player is fully connected.
- **Interfaces** (`public/isplitscreen.h`, frozen vtables untouched): `IEngineSplitScreen`
  (slot queries for the client DLL), `IClientSplitScreen` (client DLL notifications),
  `IEngineServerSplitScreen` (server DLL queries).

Open: the game DLLs still compile their split-screen stubs (`MAX_SPLITSCREEN_PLAYERS 1`);
per-slot cvars and key routing; the dedicated (`SWDS`) engine build is unchecked; remote
clients do not learn `max_splitscreen_players` (only a listen server knows it); views, HUD,
audio, and the two-controller acceptance run.

### First engine run with a second local player (2026-10-06)

Headless Portal 2 listen server on `sp_a1_intro1` with `-maxplayers 2`, `build-p2`
(`tools/quality/portal_boot.py --game portal2 --headless`, evidence under the session
scratchpad, not retained): `ss_connect` after the map loads sends `clc_SplitPlayerConnect`;
the server admits it, spawns the coop player (`[coopdbg] Spawn team=2`), replies
`svc_SplitScreen: add slot 1 entity 2`; both channels' messages then interleave under
`net_SplitScreenUser` markers (`net_showmsg 1`); `status` lists the second player
(`"... (2)"`, address `0.0.0.0:0`, active); `ss_disconnect` ends in
`Dropped ... (leaving splitscreen)`; the run exits cleanly.

What this does not show: the client DLL still treats every slot as the one local player
(its split-screen macros are stubs), so the second player has no input of its own (slot 1
sends slot 0's commands), no view, no HUD. `ss_connect` on a map with `maxplayers 1` prints
"server full" and changes nothing. Fixed on the way: the commands were developer-only
(stripped in release builds); `ss_disconnect` was not an allowed client command and was
missing from the dedicated build; Portal 2's taunt manager refused slot 1.

### Independent input per local player (2026-10-06)

Engine (`build-p2`, `portal_boot.py --game portal2 --headless`, `sp_a1_intro4` with `-maxplayers 2`;
not retained evidence): with `ss_connect`, `cmd2 +forward` made the server see
`forwardmove 450, buttons 0x8` on player 2 and `0` on player 1; `+forward` then did the reverse
(`sv_splitscreen_status`, a new server console command). Each local player's usercmds travel on
its own channel and are applied by its own server player. The chamber holds players in place at
map start, so no origin change was observed; that is the map, not the split path.

Ported on the way (CS:GO engine / client code, each in its own file):

- Engine: per-slot command buffers with `cmd1`/`cmd2` (`cmd.cpp`), `in_forceuser` and the active slot around
  key events (`keys.cpp`), per-slot view angles, `GetLocalPlayer`, forwarded commands and
  key-value commands (`cdll_engine_int.cpp`, `cmd.cpp`), the second controller's button edges run as
  bindings of that player (`cl_gamepad_slots.cpp`, from `IGamepadSlots`).
- Client DLL: per-user input (`kbutton_t::GetPerUser`, `CInput::PerUserInput_t`), local players per
  slot (`C_BasePlayer::GetLocalPlayer(slot)`, `CheckForLocalPlayer`, split owner/partner lists),
  `game/shared/splitscreen_game.h` (the active-slot macros for the client DLL; the server DLL keeps
  one slot), `cl_splitscreen_status` (client console command).
- Fixes found by running it: split channels were created with a *null* address (every message was
  dropped; they are `0.0.0.0:0`, as in CS:GO); server shutdown crashed because an owner kept a
  pointer to a deleted split client; the per-slot `ss_*` commands were stripped from release builds.

Still open: the second controller's *analog* input (sticks) in the client's joystick code, views and
HUD per slot, prediction per slot, audio listener, menus and the two-controller acceptance run.

### Two views, per-slot local players and server data (2026-10-06)

`build-p2`, headless, `sp_a1_intro4` with `-maxplayers 2` after `ss_connect`
(`tools/quality/portal_boot.py --game portal2 --headless`; images and logs not retained):

- The window shows two views, top and bottom, one per local player: player 1 set to look up at the
  ceiling (`setang -60 0 0`) fills the top half with ceiling panels, and player 2 placed and turned by
  `cmd2 setpos ...; cmd2 setang 40 180 0` fills the bottom half with the room's corner. Split layout is
  top/bottom for windows narrower than 3:2 and left/right for wider ones (`ss_splitmode`), with the fov
  of Portal 2's own `splitscreen_config.txt`.
- `cl_splitscreen_status` lists slot 0 (entity 1, primary) and slot 1 (entity 2, attached); `cmd2 setpos`
  moved slot 1's player and the client saw its new origin.
- Server: a split-screen player's local data, weapon data and visibility ride on its owner's connection
  (`SendProxy_SetOnlyPlayerRecipients`, `CBasePlayer::GetConnectionClientIndex`, PVS merge, transmit
  rules), through the engine's `IEngineServerSplitScreen`.

Known gaps in this slice: the HUD is not drawn while two local players exist (it is laid out for the
whole window); the second player is not predicted (it is a non-predicted local player); no second
audio listener; the second player's `m_Local` area bits are empty (the owner's cover both);
no controller-driven run; `ss_map` from the menus untried; non-primary menus and VGUI input.

## Second controller slot provider (2026-10-06)

`inputsystem/joystick_sdl.cpp` now fills `IGamepadSlots` for both slots: a second SDL3 gamepad opens into slot 1
(`OpenGamepadSlot`), button/axis events update that slot's snapshot, slot 0 keeps the legacy joystick events, removal
bumps the generation, `SearchForDevice` skips owned devices, and slot-1 rumble uses `SDL_RumbleGamepad`.
Observed with two uinput pads: `ss_gamepad_status` reports both slots connected (generations 1 and 2). Button
delivery to player 2 (pad two's A held while status is read) is not yet shown; the pad scripts had ended by the time
the status ran. Dedicated (SWDS) and other-game builds remain unverified.

Two-pad acceptance (2026-10-06, headless, uinput pads): with both pads held, `ss_gamepad_status` shows slot 0 buttons 0x4 (X)
and slot 1 buttons 0x1 (A); the server sees player 1 buttons 0x20 (+use) and player 2 buttons 0x2 (+jump), so each
controller drives its own local player. Not yet shown with physical controllers, and no matched images.

## Second player's viewmodel, procedural names and console commands (2026-10-06)

- Viewmodel: `CBaseViewModel::ShouldTransmit` now sends a split-screen player's viewmodel to its owner's connection
  (`IsSplitScreenUserOnEdict`), so the second view draws its own gun (an orange P-body gun in `mp_coop_lobby_2`).
- `!player_blue` / `!player_orange` resolve to the team 3 / team 2 player (`CGlobalEntityList::FindEntityProcedural`).
- `Weapon_portalgun has no owner when trying to upgrade!` printed when map logic upgraded an unheld gun; the flag is
  kept and the message is gone (retail prints it too).
- `voicerecord_toggle` is registered without `VOICE_VOX_ENABLE`; `r_flashlightbrightness` (0.25, cheat) is defined in
  `engine/view.cpp`; `ss_force_primary_fullscreen` is defined in `game/client/view.cpp`.
- The harness's "lacks scene detail" failure did not reproduce after these changes: `mp_coop_lobby_2` with `ss_connect`
  and 300-tick waits passed 4 of 4 runs, with and without `cl_splitscreen_status`. Its earlier cause is not identified.
