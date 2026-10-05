# Not a gap — retail evidence that closes a row without an implementation

Same evidence rules as `g*.md`. Each entry is a `BEHAVIOR_GAPS.md` row (or a
part of one) whose retail binary evidence shows no implementation is owed.
`BEHAVIOR_GAPS.md` stays the authority; this file carries the proof and is
referenced from the row when it is updated.

## 1. `info_paint_sprayer` radius keys (G16) — recorded 2026-09-28

Retail `server.so` has no `start_radius_*`, `end_radius_*` or
`radius_grow_time_*` keys. Blob size comes from `paintblob_min_radius_scale`
/ `paintblob_max_radius_scale`. Already recorded on the row.

## 2. `CBaseEntity` detail levels (part of G07) — 2026-10-04

`mincpulevel`, `maxcpulevel`, `mingpulevel`, `maxgpulevel` are present in
retail as `CBaseEntity` keyfields (`FIELD_CHAR`, `FTYPEDESC_SAVE|KEY`:
entries `0x011b57e0`, `0x011b5820`, `0x011b5860`, `0x011b58a0`, owner
`CBaseEntity`), **and our tree already implements them** —
`game/shared/baseentity_shared.cpp:331-352`, `CBaseEntity::KeyValue`, guarded
by `#if defined( PORTAL2 ) && !defined( CLIENT_DLL )`, writing
`m_nMinCPULevel` / `m_nMaxCPULevel` / `m_nMinGPULevel` / `m_nMaxGPULevel`.

Mechanism differs (code override with `atoi` versus a saved `char` keyfield),
map-visible behavior matches, and 1681+ map uses exist mostly on
`env_fog_controller`. Not a gap for these four names. The rest of G07
(`drawinfastreflection`, `disableshadowdepth`, `shadowdepthnocache`,
`DisableDraw`, `EnableDraw`) remains a real gap — see
`g07-g14-base-game-keys.md`.

## 3. `PaintPower` (part of G15) — 2026-10-04

Retail has two `PaintPower` keyfields over `m_PrePaintedPower`
(`0x011f2580`, `0x01201f20`), and our tree already declares it:
`game/shared/portal2/prop_paint_power_user.h:126`
(`DEFINE_KEYFIELD( m_PrePaintedPower, FIELD_INTEGER, "PaintPower" )`), used by
`game/server/portal2/prop_weightedcube.cpp:397`. Not a gap. Remaining G15
work is `allowfunnel` and the paint-bomb spawn-sound key.

## 4. `BombType` (part of G15) — 2026-10-04

The string `BombType` does not exist in retail `server.so` or `client.so`
(byte search, both binaries), while the retail FGD declares it
(`portal2.fgd:693`) and five shipped maps set it on `prop_paint_bomb`.
Retail ignores it, so there is no behavior to reproduce. Not a gap — same
classification as G16.

## 5. `info_placement_helper` `target_size` / `usesizelimit` (G19) — 2026-10-04

- Retail defines `target_size` **only** on `CNPC_Bullseye`
  (entry `0x011d45c0`, array `0x011d4480` idx 5, initializer
  `FUN_0085f7e0`, `className="CNPC_Bullseye"`), and our
  `game/server/hl2/npc_bullseye.cpp:93` already has
  `DEFINE_KEYFIELD(m_nTargetObjectSize, FIELD_INTEGER, "target_size")`.
- Retail's `info_placement_helper` table (`0x011f9240`) has no `target_size`
  entry, and the string `usesizelimit` does not exist in either binary.
- The shipped maps set both keys on `info_placement_helper` (7 uses each in
  3 maps); retail ignores them.
- Our Portal 2 product builds `game/server/portal2/info_placement_helper.cpp`
  (`server_portal2.vpc:219`), which has neither key — matching retail. The
  `game/server/fstop/info_placement_helper.cpp` copy that declares both is in
  `server_fstop.vpc`, the Portal 1 product.

Not a gap: retail implements neither key on that entity, and the key we do
share with retail (`npc_bullseye` `target_size`) is already present.

## 6. `logic_playerproxy` `LowerWeapon` (part of G20) — 2026-10-04

`LowerWeapon` does not exist in retail `server.so` or `client.so`, is absent
from the retail FGD, and appears as an entity key in zero shipped maps (one
raw byte hit elsewhere in one BSP, not an entity key). Not a gap. The rest of
G20 (`SetMin/MaxPitch`, `SetMaxPitch`, `SetCanShoot`, `UseAttachmentEyes`,
`PlayerCanShoot`) was closed on 2026-10-29 per the row.

## What is *not* decided here

- Keys that retail implements but no shipped map uses (`NoPlacementHelper`,
  `noemitterparticles`, `TeleportEntity`) are still gaps: parity with retail,
  not map usage, is the criterion.
- Keys whose retail spelling differs from the FGD/maps (`Play Spawn Sound`
  versus `playspawnsound`) are gaps with a deviation note, not closures.
