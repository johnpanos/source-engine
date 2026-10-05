# G07–G14 — base-game keys and inputs in retail `server.so`

Evidence: `ghidra/out/dd_batch_b.txt`, `dd_batch_f.txt`, `out/fo_*.txt`,
`out/decomp_drawinfast.txt`; map counts from `/tmp/opencode/mapkeys.py` over
106 loose BSPs in `portal2/maps`. Addresses are Ghidra addresses (see README
for the mapping). `BEHAVIOR_GAPS.md` remains the gap authority.

Retail class attribution comes from `FindOwner.java`: the entry sits in a
`.data` table whose static initializer assigns `className = "C…"`.

## Summary table

| Gap | Retail key / input | Retail member | Type / flags | Owner (retail) | Entry | Handler | Maps |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G07 | `mincpulevel` | `m_nMinCPULevel` | CHAR `0x0006` | `CBaseEntity` (array `0x011b5720` idx 3) | `0x011b57e0` | — | 1681+ uses (mostly `env_fog_controller`) |
| G07 | `maxcpulevel` | `m_nMaxCPULevel` | CHAR `0x0006` | `CBaseEntity` | `0x011b5820` | — | ditto |
| G07 | `mingpulevel` | `m_nMinGPULevel` | CHAR `0x0006` | `CBaseEntity` | `0x011b5860` | — | ditto |
| G07 | `maxgpulevel` | `m_nMaxGPULevel` | CHAR `0x0006` | `CBaseEntity` | `0x011b58a0` | — | ditto |
| G07 | `DisableDraw` | `InputDisableDraw` | VOID `0x0008` | `CBaseEntity` | `0x011b7460` | written by `FUN_007554f0` | input |
| G07 | `EnableDraw` | `InputEnableDraw` | VOID `0x0008` | `CBaseEntity` | `0x011b74a0` | slot `0x011b74b8`, ref `0x00755764` in `FUN_007554f0` | input |
| G07 | `drawinfastreflection`, `disableshadowdepth`, `shadowdepthnocache` | — (code path, not datadesc) | `atoi` compares | `CBaseEntity::KeyValue` (`FUN_0046f0e0`) | code `0x0046f545` / `0x0046f56a` / `0x0046f58f` | — | 2587 / 2959 / 1341 key uses |
| G08 | `colortransitiontime` | `m_flColorTransitionTime` | FLOAT `0x0006` | `CEnvProjectedTexture` (array `0x011c4400` idx 18) | `0x011c4880` | — | 316 (`env_projectedtexture`) |
| G08 | `brightnessscale` | `m_flBrightnessScale` | FLOAT `0x0006` | `CEnvProjectedTexture` (idx 16) | `0x011c4800` | — | 316 |
| G08 | `simpleprojection` | `m_bSimpleProjection` | BOOL `0x0006` | `CEnvProjectedTexture` (idx 6) | `0x011c4580` | — | 82 |
| G08 | `SetLightStyle` | `InputSetLightStyle` | INT `0x0008` | `CEnvProjectedTexture` (idx 37) | `0x011c4d40` | `FUN_007d9280` | 31 maps reference the input |
| G08b | `colortransitiontime` (second owner) | `m_flColorTransitionTime` | FLOAT `0x0006` | `CSunlightShadowControl` (array `0x01211280` idx 9) | `0x012114c0` | — | — |
| G09 | `FadeReverse` | `InputReverseFade` | VOID `0x0008` | `CEnvFade` (array `0x011c15c0` idx 4) | `0x011c16c0` | `FUN_007c6ec0` | 4 maps |
| G09 | `ReverseFadeDuration` | `m_flReverseFadeDuration` | FLOAT `0x0006` | `CEnvFade` (idx 2) | `0x011c1640` | — | 327 (`env_fade`) |
| G10 | `OnChangedFromMax` | `m_OnChangedFromMax` | CUSTOM `0x0016` | `CMathCounter` (array `0x011da500` idx 22) | `0x011daa80` | — | 6 |
| G10 | `OnChangedFromMin` | `m_OnChangedFromMin` | CUSTOM `0x0016` | `CMathCounter` (idx 21) | `0x011daa40` | — | 3 |
| G11 | `UseLandmarkAngles` | `m_bUseLandmarkAngles` | BOOL `0x0006` | `CTriggerTeleport` (array `0x01216600` idx 1) | `0x01216640` | writer `FUN_00ae6ae0` | 69 maps (109 occurrences) |
| G12 | `FadeAndKill` | `InputFadeAndKill` | VOID `0x0008` | `CDynamicProp` (array `0x01206900` idx 25) | `0x01206f40` | `FUN_00a52920` | 2 maps |
| G12 | `AnimateEveryFrame` | `m_bAnimateEveryFrame` | BOOL `0x0006` | `CDynamicProp` (idx 13) | `0x01206c40` | — | 341 (`prop_dynamic_override`) |
| G12 | `SuppressAnimSounds` | `m_bSuppressAnimSounds` | BOOL `0x0006` | `CBaseAnimating` (array `0x011b3f00` idx 40) | `0x011b4900` | writer `FUN_0072c850` | 143 + 26 |
| G13 | `DestroyImmediately` | `InputDestroy` | VOID `0x0008` | `CParticleSystem` (array `0x011e21a0` idx 81) | `0x011e35e0` | `FUN_008f4ec0` | 6 |
| G13 | `StopPlayEndCap` | `InputStopPlayEndCap` | VOID `0x0008` | `CParticleSystem` (idx 80) | `0x011e35a0` | `FUN_008f4ed0` | 2 |
| G14 | `GetSpeed` | `InputGetSpeed` | VOID `0x0008` | `CFuncRotating` (array `0x011b9780` idx 19) | `0x011b9c40` | `FUN_007840e0` | 2 |
| G14 | `OnGetSpeed` | `m_OnGetSpeed` | CUSTOM `0x0016` | `CFuncRotating` | `0x011b9e40` | — | with the input |
| G14 | `TeleportEntity` | `InputTeleportEntity` | VOID `0x0008` | `CPointTeleport` (array `0x011ee680` idx 3) | `0x011ee740` | `FUN_00968390` | 0 |
| G14 | `TeleportToCurrentPos` | `InputTeleportToCurrentPos` | VOID `0x0008` | `CPointTeleport` (idx 7) | `0x011ee780` | `FUN_009683c0` | 1 |

## Notes

**G07 CPU/GPU detail levels — already implemented here.**
Retail stores them as `CBaseEntity` keyfields (`FIELD_CHAR`, saved). Our tree
handles the same four names in `CBaseEntity::KeyValue`
(`game/shared/baseentity_shared.cpp:331-352`, guarded by
`#if defined( PORTAL2 ) && !defined( CLIENT_DLL )`) into `m_nMinCPULevel` …
`m_nMaxGPULevel`. The mechanism differs (override vs keyfield, `int` vs
`char`), the map-visible behavior matches, and the row is not a gap for these
four names. See `not-a-gap.md`.

**G07 draw keys — real gap.** `drawinfastreflection`, `disableshadowdepth`
and `shadowdepthnocache` are compared in `FUN_0046f0e0`, which is the retail
`CBaseEntity::KeyValue` (confirmed by
`cstrike15_src/game/shared/baseentity_shared.cpp:361`, `bool
CBaseEntity::KeyValue`, which contains exactly those three `FStrEq` blocks at
lines 396 and following). Our tree has neither the three keys nor the code
(`grep` over `game/` finds nothing), while 2587/2959/1341 map key uses exist.
Port from the base-game drop, as `BEHAVIOR_GAPS.md` already says.

**G07 `DisableDraw`/`EnableDraw` — real gap.** Both are `CBaseEntity` inputs
on the same table as `EnableReceivingFlashlight`; the slot writers are the
`FUN_007554f0` initializer that already fills `CBaseEntity`'s other entries.

**G12 `SuppressAnimSounds` owner is `CBaseAnimating`, not `prop_dynamic`.**
Retail defines it once on `CBaseAnimating`, and `CDynamicProp` inherits it —
which is why maps set it on `prop_dynamic` and `prop_dynamic_override`. Our
implementation should live on the base class to match.

**G08 `colortransitiontime` has two retail owners** (`CEnvProjectedTexture`
and `CSunlightShadowControl`). The gap row is about `env_projectedtexture`;
the second owner is recorded so a later `sunshadowcontrol` parity pass does
not rediscover it.

**Input names in maps.** Inputs appear in the BSP entity lump as *values*
(output lines), so a key-only scan undercounts them. A raw byte scan across
the 106 BSPs gives: `SetLightStyle` 31, `DestroyImmediately` 6, `FadeReverse`
4, `FadeAndKill` 2, `StopPlayEndCap` 2, `GetSpeed` 2, `TeleportToCurrentPos`
1, `TeleportEntity` 0. The `BEHAVIOR_GAPS.md` map counts stay authoritative;
these are corroboration.

## Verdict

| Gap | Verdict |
| --- | --- |
| G07 | **partial**: four detail-level keys already implemented (see `not-a-gap.md`); the three draw keys and the two draw inputs remain real gaps |
| G08 | **real gap** — all four keys/inputs present in retail on `CEnvProjectedTexture` |
| G09 | **real gap** |
| G10 | **real gap** |
| G11 | **real gap** |
| G12 | **real gap**, owner for `SuppressAnimSounds` is `CBaseAnimating` |
| G13 | **real gap** |
| G14 | **real gap** |
