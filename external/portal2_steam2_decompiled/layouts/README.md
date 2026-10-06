# Retail class layouts for local split-screen (Steam2 depot 852_3 dSYMs)

Generated with `tools/portal2/dsym/class_layout.py` from the client and server
dSYMs (2010 macOS i386; 64-bit offsets in this fork differ). Reconstruction
aids, not source. The retail-binary comparison (Ghidra projects) is a separate,
still-open step.

Files in `852_3/`: `client_players.h` (C_BasePlayer, C_BaseCombatCharacter,
C_BaseEntity, CUserCmd, CSetActiveSplitScreenPlayerGuard), `client_input.h`
(CInput, kbutton_t), `client_view.h` (CViewRender, CViewSetup),
`client_portal_player.h`, `server_players.h` (CBasePlayer, CPlayerState,
CBaseCombatCharacter, CPlayerLocalData), `server_portal_player.h`.
`CRenderView` has no complete record in the view unit; the server guard
class is not defined in the player unit.

## Split-screen members (i386)

| Class | Member / method | Offset / note |
| --- | --- | --- |
| `C_BasePlayer` (sizeof 0x1a54) | `m_hSplitScreenPlayers` (CUtlVector of handles) | +0x1880 |
| | `m_nSplitScreenSlot` | +0x1894 |
| | `AddSplitScreenPlayer`, `RemoveSplitScreenPlayer`, `GetSplitScreenPlayers`, `IsSplitScreenPartner`, `IsSplitScreenPlayer`, `GetSplitScreenPlayerSlot`, static `GetSplitScreenSlotForPlayer`, static `SetRemoteSplitScreenPlayerViewsAreLocalPlayer`, `GetLocalPlayer(int slot)` | |
| `C_BaseEntity` | `m_nSplitUserPlayerPredictionSlot` | +0x3f8; virtual `ShouldDrawForSplitScreenUser(int)` |
| `CBasePlayer` (server) | `m_bSplitScreenPlayer` | +0x1214 |
| | `m_hSplitOwner` | +0x1218 |
| | `m_hSplitScreenPlayers` | +0x121c |
| | `SetSplitScreenPlayer(bool, CBasePlayer*)`, `GetSplitScreenPlayerOwner`, `IsSplitScreenUserOnEdict`, virtual `EnsureSplitScreenTeam` | |
| `CSetActiveSplitScreenPlayerGuard` (sizeof 0x20) | `m_bChanged` +0xc, `m_pchContext` +0x10, `m_nLine` +0x14, `m_nSaveSlot` +0x18, `m_bSaveGetLocalPlayerAllowed` +0x1c | base `CVGuiScreenSizeSplitScreenPlayerGuard` |
| `CInput` | virtual `CheckSplitScreenMimic(int, CUserCmd*, CUserCmd*)` | slot-indexed input |
| `CUserCmd` | sizeof 0x5c | |

The slot model: one `CBasePlayer` per local player, linked to its owner by
`m_hSplitOwner` and to its partners by `m_hSplitScreenPlayers`; the client
resolves the active slot through the guard, and input and prediction are
slot-indexed.

## Retail comparison (Linux server.so, Ghidra project `portal2_retail`)

The retail Linux server is stripped, and the Ghidra project now holds server.so, client.so and engine.so (client and
engine imported 2026-10-05, not yet compared). Class and member names are
not recoverable, so facts come from code that uses the members:

- `IsLocalSplitScreen` (a registered VScript function, string at 0x00d2adbb,
  implementation `FUN_0056c380`) walks the player list, calls vtable slot
  +0x158 on each entity, then reads `*(int*)(GetSplitScreenPlayers() + 0xc)`
  and returns whether it is greater than zero. +0xc is `CUtlVector::m_Size`, so the
  retail test is "this player's `m_hSplitScreenPlayers` is non-empty".
- Retail's `GetSplitScreenPlayers()` accessor (`FUN_0047d880`) returns
  `this + 0x11f8`. The 2010 dSYM puts `m_hSplitScreenPlayers` at `+0x121c`.

**Finding:** the retail `CBasePlayer` split-screen vector sits 0x24 bytes
earlier than in the 2010 build (i386 both). The 2010 offsets above are therefore
not retail offsets. Use the dSYM for member names, types and relative order, and
re-derive absolute offsets from the retail binary before relying on them. The
retail server contains no `m_hSplit*` name strings, so the owner handle and
bool are not confirmed in retail either. The client classes remain uncompared
until the retail client is imported.
