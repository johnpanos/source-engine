# Portal 2 split-screen: retail engine and client findings

Source: retail Linux `engine.so`, `client.so` and `server.so` in the Ghidra project
`portal2_retail` (i386, image base 0x10000 for server; addresses below are as
Ghidra shows them). The binaries are stripped, so function and field names are
inferred from strings and use; **observed** facts are separated from
**hypotheses**. Decompiler pseudocode is a research aid, not source. Recorded
2026-10-06; nothing here is implemented in this tree.

## Network messages (observed strings)

`engine.so` registers three split-screen net messages: `net_SplitScreenUser`,
`clc_SplitPlayerConnect` and `svc_SplitScreen`, with `ReadFromBuffer` for each,
and the userinfo cvars "sets userinfo string for split screen player in slot 1..3".
`CLC_SplitPlayerConnect::ReadFromBuffer` (`FUN_0028c6f0`) reads a count (8 bits,
the small-count form), then for each player two 0x104-byte strings (hypothesis:
name or userinfo, and a second string that the reader discards), and appends the
first to a vector at message +0x10. This is the wire extension the one-player
protocol must negotiate (AGENTS.md: preserve the wire protocol; extend explicitly).

## Engine console commands (observed)

- `ss_map` handler `FUN_004995a0` calls `FUN_004991f0(args, 8)`, the map-launch
  helper with mode flag 8.
- `connect_splitscreen <server> <# of players>`: count must be >= 1 and
  <= global max `DAT_00bc3e64` ("Must have at least one player.", "Too many
  players"); disconnects local slots, then one connect call carrying the count.
  `connect_splitscreen localhost:%d %d` is the string `ss_map` uses to reach it.
- `ss_connect`, `ss_disconnect <slot>`, with refusals: "split screen not
  supported when running -tools mode", "game does not support split screen",
  "not connected to game", "no more split screen player slots!", "slot %d not
  active", "no split screen users active".
- `DAT_00bc3e64` is the retail maximum local split-screen players (2 here;
  the string "Clamping split screen users to 1" is the fallback).

## Engine per-slot client state (inferred)

`FUN_003d57f0` (adds a local user in slot `param_2`, `param_3` = player/user
index) creates a record named by `"SPLIT%d"`: record +0 = in-use flag, +4 an
embedded client-state object (constructed by `FUN_003a3fd0`), +0x14 a netchannel
created by `FUN_00501c10` and registered with the engine through vtable +0x124
(`SetNetChannel(slot, chan)`), +0xcc = `param_3`, +0xd0 = `param_3 - 1`
(entity slot), +0xd4 = slot. It bumps the active count at +0xc.
`FUN_003d5240` is `RemoveSplitScreenUser`: only for max > 1 and a matching
entity slot; releases the netchannel (vtable +0x98 with the string, +0x128 on
the owner), resets +0xd0 to -1 and the in-use flag, decrements the count.

Server side (`FUN_00379280`, `FUN_003681a0`): a client has a vector of
(owner, split-client) pairs (`+0x158`, count `+0x164`) and an array of up to
`DAT_00bc3e64` split clients at `+0x18` of the owner. Disconnect reason strings:
"leaving splitscreen" (also `ss_disconnect %d\n` sent by the owner for a split
slot). A split client carries an active flag at +0x10 and a pending flag at +0x11.

## Game DLL facts

- Server (`server.so`): `IsLocalSplitScreen` script function returns true when
  any connected player's split-screen vector is non-empty; the accessor is
  `CBasePlayer + 0x11f8` (the 2010 dSYM has +0x121c: layouts differ, see
  `external/portal2_steam2_decompiled/layouts/README.md`).
- Client (`client.so`): `IsSplitScreen` (used by HUD/VGUI scripts) is
  `DAT_018c5118 > 1`, an engine-set active local-player count. The cvars
  `ss_splitmode`, `ss_verticalsplit`, `ss_pipsplit` and the `if_split_screen_*`
  panel conditions are retail UI/layout controls (strings only so far).

## `CSplitScreen` (engine, RTTI `12CSplitScreen`, vtable 0x00787458)

Layout (inferred): `+0` vptr, `+4 .. +4+4*max` pointers to per-slot records
(`max` = `DAT_00bc3e64`, 2), `+0xc` active count (it follows the two pointers).
Methods recovered from the vtable and their bodies:

| Function | Meaning (hypothesis from behavior) |
| --- | --- |
| `FUN_003d57f0(slot, userIndex)` | add local user (above) |
| `FUN_003d5240(slot, userIndex)` | remove local user (above) |
| `FUN_003d5100` | `GetActiveCount()`, returns `+0xc` |
| `FUN_003d51b0(slot)` | next valid slot after `slot` (record flag at +0 set), -1 at end |
| `FUN_003d51a0` | returns 0 (a default or disabled hook) |
| `FUN_003d5370` | thread-local flag, bit 0 of byte +2 of a per-thread block: the "split-screen context is resolvable" state the client's active-slot guard sets (hypothesis) |

The first-slot loop in `connect_splitscreen` iterates this class by that
first/next pair, so a slot is "valid" exactly when its record's in-use byte is
set. This is the retail shape a native `LocalPlayerSet` (two identities,
admission, teardown) would mirror; it is not a drop-in layout for this fork.

## Connection and admission flows (observed in retail, matched to CS:GO source)

The CS:GO engine leak (`~/src/cstrike15_src/engine`) is the source reference for
the same design: retail's `SPLIT%d`, `RemoveSplitScreenUser`, `ss_connect` text and
function shapes match it line for line (`cl_splitscreen.cpp`, `host_cmd.cpp`
`ss_connect`, `baseclientstate.cpp` Add/Remove handlers, `net_chan.cpp`
`ChangeSplitUser`/`SplitPlayer_t`). Portal 2 retail differs from CS:GO in the wire
format: it uses hand-written bit messages, CS:GO uses protobuf.

1. **One connection, several players.** A listen/dedicated connection has one
   netchannel. Players are *multiplexed*: before each group of messages for a
   non-primary slot the sender writes `net_SplitScreenUser` (a 1-bit slot, retail
   type 3); the receiver's `ChangeSplitscreenUser(slot)` switches the active
   slot until the next marker. The channel keeps a per-slot reliable and
   unreliable buffer (CS:GO `SplitPlayer_t`) and merges them on send.
2. **Initial connect.** `CBaseServer::ConnectClient` (retail `FUN_0037d510`)
   receives a vector of per-player connect records (stride 0x24; count at +0xc).
   Record 0 is the primary client. For each extra record the server prints
   "Processing Split Screen connection packet." and calls the new client's
   `ProcessSplitPlayerConnect`-style hook (vtable +0xc0) on the primary
   `CGameClient`. This is `connect_splitscreen <server> <n>`'s path.
3. **Joining later (`ss_connect`).** The client picks the first free slot
   (`IsValidSplitScreenSlot` false), fills a `clc_SplitPlayerConnect` with that
   slot's convars (userinfo), and sends it reliably on the existing channel.
   Refusals (in order): -tools mode, game does not support split screen, not
   connected, no free slot.
4. **Server admission** (retail `FUN_003679e0`, CS:GO `CGameClient::ProcessSplitPlayerConnect`):
   refuse with "No more split screen slots!" if the server max is < 2 or the
   owner already has a split client; else allocate a second client slot through
   `CBaseServer` (using the userinfo from the message), mark it a split client
   (flag at +0x14 = 1), store it on the owner (`+0x1c`), point the new client's
   channel at the owner's channel, then send the owner an `svc_SplitScreen`
   {type 0 = add, slot 1, entity index of the new client}. If the owner is already
   fully connected (signon 6) the new client is activated immediately.
5. **Client add.** `svc_SplitScreen` handler → `CSplitScreen::AddSplitScreenUser(slot,
   entityIndex)` (above): in-use record named `SPLIT%d`, own client state, channel
   registered, count +1. Remove is symmetric (`type` 1, "RemoveSplitScreenUser").
6. **Disconnect.** `ss_disconnect <slot>` (and "leaving splitscreen" on teardown)
   removes one slot; disconnecting the owner removes every split client
   ("leaving splitscreen", reverse order).

## Wire formats (retail, bit-level; observed from the `ReadFromBuffer` functions)

| Message | Retail type id | Fields |
| --- | --- | --- |
| `net_SplitScreenUser` | 3 | slot: 1 bit |
| `svc_SplitScreen` | 0x16 (22) | type: 1 bit (0 add / 1 remove), slot: 1 bit, entity index: 11 bits |
| `clc_SplitPlayerConnect` | 0x11 (17) | count: 8 bits, then per player two `0x104`-byte strings (the first is kept) |

Retail's numbering is its own (type 3 and 17 collide with this fork's
`net_Tick` and `clc_FileMD5Check`, type 22 reuses the fork's retired
`svc_TerrainMod`). **This fork cannot take retail ids**: it must allocate new ids
in an explicitly negotiated extension and keep the one-player protocol unchanged
for peers that do not advertise it. Retail interop is not a goal.

## Fork gap analysis (this tree)

- `engine/` has no split-screen code; `cl` is one global `CClientState`
  (about 586 uses in 60 files); the server has no split client concept.
- The Portal 2 game DLL compiles with `SPLIT_SCREEN_STUBS`,
  `MAX_SPLITSCREEN_PLAYERS 1` and no-op active-slot guards.
- Message classes live in `common/netmessages.h` with handler interfaces
  (`IServerMessageHandler`, `IClientMessageHandler`) that every implementor
  must extend.

## Implementation slices (ordered; each needs its own gate)

1. **Wire**: the three messages in `common/netmessages` with negotiated ids and a
   headless round-trip suite that includes bad-length/overflow cases.
2. **Engine registry**: the CS:GO `CSplitScreen` ported (`engine/cl_splitscreen.cpp`).
3. **Server admission**: `CBaseServer` split clients (`ConnectClient` vector,
   `ProcessSplitPlayerConnect`, `svc_SplitScreen` reply, teardown), dedicated and
   listen, one-player peers unchanged.
4. **Client state and netchannel multiplexing**: per-slot `CClientState`, slot
   markers, per-slot buffers, per-slot usercmds (`clc_Move` per slot).
5. **Game DLL**: remove the stubs (`MAX_SPLITSCREEN_PLAYERS 2`), active-slot
   guards, per-slot input/prediction, owner-only data, `m_hSplit*` members.
6. **Views, HUD and menus**: two viewports through the render core, HUD per slot,
   `ss_map`/`connect_splitscreen` replacing the refusal in `host_cmd.cpp`.
7. **Product acceptance** (per the progress record): two controllers end to end.

## Open

Class and field names beyond the above are unrecovered; the client
`C_BasePlayer` split-screen member offsets are not yet located in retail; the
`CSplitScreen` / `ISplitScreen` RTTI classes exist (`12CSplitScreen`,
`12ISplitScreen`) and their vtables are the next target (`VtableSlot.java`).
