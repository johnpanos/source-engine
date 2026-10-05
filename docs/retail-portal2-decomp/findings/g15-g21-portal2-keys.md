# G15, G17–G21 — Portal 2 entity keys and inputs in retail `server.so`

Evidence: `ghidra/out/dd_batch_a.txt`, `dd_batch_c.txt`, `dd_batch_e.txt`,
`dd_batch_f.txt`, `out/fo_*.txt`, `out/dump_*.txt`; retail FGD
`Portal 2/bin/portal2.fgd`; map counts from `/tmp/opencode/mapkeys.py` over
106 loose BSPs. Addresses are Ghidra addresses. `BEHAVIOR_GAPS.md` remains
the gap authority.

## Summary table

| Gap | Retail key / input | Retail member | Type / flags | Owner (retail) | Entry | Handler | Maps / FGD |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G15 | `allowfunnel` | `m_bAllowPortalFunnel` | BOOL `0x0006` | `CPhysicsProp` (array `0x01206100` idx 10) | `0x01206380` | — | `prop_physics` 132, `prop_weighted_cube` 75, `prop_paint_bomb` 8; FGD L369/L700/L106 |
| G15 | `Play Spawn Sound` | `m_bPlaySpawnSound` | BOOL `0x0006` | `CPropPaintBomb` (array `0x01201540` idx 1) | `0x01201580` | — | FGD declares `playspawnsound` (L703); maps use `playspawnsound` (5 maps, 8 uses), **never** `Play Spawn Sound` |
| G15 | `PaintPower` | `m_PrePaintedPower` | INT `0x0006` | see note — two entries, one attributed `CNPC_Portal_FloorTurret` | `0x011f2580`, `0x01201f20` | — | FGD L357 (`prop_weighted_cube`), L821 (`prop_physics_paintable`) |
| G17 | `AutoAimEnabled` | `m_bAutoAimEnabled` | BOOL `0x0006` | `CPortalLaser` (array `0x011f7dc0` idx 19) | `0x011f8280` | writer `FUN_009d9c00` | 34 (`env_portal_laser`); FGD L285 |
| G17 | `NoPlacementHelper` | `m_bNoPlacementHelper` | BOOL `0x0006` | `CPortalLaser` (idx 20) | `0x011f82c0` | same initializer | **0 map uses**; FGD L270 |
| G18 | `ModelSkin` | `m_nSkin` | INT `0x0006` (a second entry maps `skin` with `0x000e`) | `CBaseAnimating` (array `0x011b3f00` idx 4) | `0x011b4000` | writer `FUN_0072c850` | 7 (`npc_personality_core`); FGD L508 |
| G18 | `altmodel` | `m_bUseAltModel` | BOOL `0x0006` | `CNPC_PersonalityCore` (array `0x011faee0` idx 9) | `0x011fb120` | — | 3 (1 map); FGD L516 |
| G18 | `EnableReceivingFlashlight` / `DisableReceivingFlashlight` | `InputEnableReceivingFlashlight` / `InputDisableReceivingFlashlight` | VOID `0x0008` | **`CBaseEntity`** (array `0x011b5720` idx 119 / 120) | `0x011b74e0` / `0x011b7520` | writer `FUN_007554f0` | inputs |
| G19 | `target_size` | `m_nTargetObjectSize` | INT `0x0006` | `CNPC_Bullseye` (array `0x011d4480` idx 5) | `0x011d45c0` | writer `FUN_0085f7e0` | 7 uses — but all on `info_placement_helper`, see below |
| G21 | `noemitterparticles` | `m_bNoEmitterParticles` | BOOL `0x0006` | `CPropTractorBeamProjector` (array `0x01202580` idx 2) | `0x01202600` | — | **0 map uses**; FGD L623 |
| G21 | `IsTransparent` | `m_bIsTransparent` | BOOL `0x0006` | `CVGuiScreen` (array `0x01219b00` idx 5) | `0x01219c40` | writer `FUN_00b10330` | 6 (`vgui_screen`) |
| G21 | `alwaystransmit` | `m_bAlwaysTransmitToClient` | BOOL `0x0006` | `CNPC_Bullseye` (array `0x011d4480` idx 4) | `0x011d4580` | writer `FUN_0085f7e0` | 8 maps / 32 uses (`npc_bullseye`) |
| G21 | `trackspeed` | `m_trackSpeed` | FLOAT `0x0006` | `CTriggerCamera` (array `0x012157c0` idx 19) | `0x01215c80` | — | 7 maps / 8 uses (`point_viewcontrol`) |
| G21 | `SetTrackSpeed` | `InputSetTrackSpeed` | — `0x0008` | `CTriggerCamera` (same table, idx ~28) | `0x01215f00` | `FUN_00ade630` | input |

`point_viewcontrol` is implemented by `CTriggerCamera` in Source, which is why
the `trackspeed` / `SetTrackSpeed` entries sit on that class.

## Table dumps used

`CPropPaintBomb` table at `0x01201540` (file `0x11e1540`):

| idx | type | member | key | flags |
| --- | --- | --- | --- | --- |
| 0 | INT | `m_nPaintPowerType` / `PaintType` | — | `0x0002` (save only) |
| 1 | BOOL | `m_bPlaySpawnSound` | `Play Spawn Sound` | `0x0006` |
| 2 | BOOL | `m_bAllowSilentDissolve` | — | |
| 3 | VOID | `InputDissolve` | `Dissolve` | input, `FUN_00a21a80` |
| 4 | VOID | `InputSilentDissolve` | `SilentDissolve` | input, `FUN_00a21aa0` |
| 5 / 6 | VOID | `InputDisablePortalFunnel` / `InputEnablePortalFunnel` | | input |
| 7 / 8 | CUSTOM | `m_OnFizzled` / `m_OnExploded` | | `0x0016` output |

`PaintPower` table at `0x011f2540` (file `0x11e1540` window):

| idx | type | member | key | flags |
| --- | --- | --- | --- | --- |
| 0 | INT | `m_iPaintPower` | — | `0x0002` |
| 1 | INT | `m_PrePaintedPower` | `PaintPower` | `0x0006` |
| 2 | CUSTOM | `m_nOriginalMaterialIndex` | — | `0x0002` |

`info_placement_helper` table at `0x011f9240` (no `target_size`):

`m_strTargetProxy`/`proxy_name`, `m_strTargetEntity`/`attach_target_name`,
`m_flRadius`/`radius`, `m_bSnapToHelperAngles`, `m_bForcePlacement`,
`m_bDisabled`/`StartDisabled`, `m_flDisableTime`, `m_bDeferringToPortal`,
`InputEnable` (`FUN_009d85d0`), `InputDisable` (`FUN_009d8730`),
`m_OnObjectPlaced`, `m_ObjectPlacedSize`/`OnObjectPlacedSize`.

## Notes

**G15 `PaintPower` — already a keyfield in our tree.**
`game/shared/portal2/prop_paint_power_user.h:126` declares
`DEFINE_KEYFIELD( m_PrePaintedPower, FIELD_INTEGER, "PaintPower" )`, and
`game/server/portal2/prop_weightedcube.cpp:397` consumes it. Retail has two
entries; one (`0x011f2580`, table start `0x011f2540`) is attributed by
`FindOwner` to `CNPC_Portal_FloorTurret` (consistent with gel on turrets and
with `PropPaintPowerUserI16CNPC_FloorTurretE` in the binary), the other
(`0x01201f20`) has no extractable class name within `0x1000`. The FGD declares
the key on `prop_weighted_cube` and `prop_physics_paintable`. Verify our
remaining paintable classes expose it; the key itself is not missing.

**G15 `allowfunnel` — real gap.** One retail definition on `CPhysicsProp`
covers all three FGD entities because `prop_weighted_cube` and
`prop_paint_bomb` derive from it. Our tree has no `allowfunnel` string
anywhere.

**G15 paint bomb spawn sound — key-spelling defect.** Retail's key is the
literal `Play Spawn Sound` (with spaces). The retail FGD and every shipped map
use `playspawnsound`, and the string `playspawnsound` does not exist in
retail `server.so` or `client.so`, so retail ignores the value its own maps
set. Implement the retail key; also accept `playspawnsound` so the shipped
maps behave as their FGD intended, and record the deviation.

**G17 `NoPlacementHelper` — implement although unused.** Retail and the FGD
both carry it; no shipped map sets it. Parity, not usage, is the criterion.

**G18 ownership differs from the gap row.** `ModelSkin` is a
`CBaseAnimating` key (every animated model, which is why personality cores
inherit it), and the two flashlight inputs are on `CBaseEntity`, not on
`npc_personality_core`. Only `altmodel` is personality-core-specific. Our
implementation should follow retail's placement so every `CBaseAnimating`
consumer gets `ModelSkin`.

**G19 `target_size` / `usesizelimit` — retail does not implement them on
`info_placement_helper`.** Retail defines `target_size` only on
`CNPC_Bullseye` (and our `game/server/hl2/npc_bullseye.cpp:93` already has
that keyfield). The seven `target_size` and seven `usesizelimit` uses in the
shipped maps are all on `info_placement_helper`, whose retail table has
neither key and whose binary has no `usesizelimit` string at all. Retail
therefore ignores them, and our Portal 2 build
(`game/server/portal2/info_placement_helper.cpp`, selected by
`server_portal2.vpc`) matches that. The `fstop/info_placement_helper.cpp`
copy that has both keys belongs to the Portal 1 (`server_fstop.vpc`) product.
See `not-a-gap.md`.

**G21 — three keys, no map or FGD pressure on two of them.**
`noemitterparticles` (FGD L623) and `alwaystransmit` are declared/implemented
but `noemitterparticles` is used by no shipped map, and `alwaystransmit` is
used by 32 map entities. Implement all three for parity.

**Shipped FGD is narrower than retail code and shipped maps.**
`Portal 2/bin/portal2.fgd` (910 lines) declares `NoPlacementHelper`,
`ModelSkin`, `altmodel`, `BombType` and `playspawnsound`, but contains none of
`UseLandmarkAngles`, `IsTransparent`, `TrackSpeed`/`trackspeed`,
`AlwaysTransmit`, `target_size`, `usesizelimit`, `LowerWeapon`,
`drawinfastreflection`, `shadowdepthnocache`, `FadeReverse`,
`DestroyImmediately`, `StopPlayEndCap`, `GetSpeed` or `TeleportEntity` —
yet retail implements most of them and shipped maps set several. The FGD is
not a complete statement of the entity API; the binary is.

## Verdict

| Gap | Verdict |
| --- | --- |
| G15 | **partial**: `PaintPower` already implemented here; `allowfunnel` is a real gap; `Play Spawn Sound` is a real gap plus a key-spelling deviation; `BombType` is not a gap |
| G17 | **real gap** (both keys) |
| G18 | **real gap**, but move `ModelSkin` to `CBaseAnimating` and the flashlight inputs to `CBaseEntity` |
| G19 | **not a gap** — see `not-a-gap.md` |
| G20 (`LowerWeapon`) | **not a gap** — see `not-a-gap.md` |
| G21 | **real gap** (`noemitterparticles`, `IsTransparent`, `alwaystransmit`, `trackspeed`/`SetTrackSpeed`) |
