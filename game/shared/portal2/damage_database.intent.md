# Damage database reconstruction notes

This note records the behavior visible in the Steam2 Portal 2 server binary. It
is an intent aid for reviewing `damage_database.cpp`, not recovered Valve source.
The strongest references are the depot 852 v3 and 841 v1 server pseudocode and
DWARF skeletons listed below. Their binaries are old i386 builds, so layouts,
control flow and some strings must be checked against current callers and assets.

## File-by-file checklist

- `damage_database.h`: preserve the single entity type (`DAMAGED_TURRET`), the
  `DamageInfo` field meanings and the public lookup methods. The 841 DWARF
  skeleton reports `sizeof(DamageInfo) == 0xba8` on i386; that is historical
  ABI evidence, not a requirement to force this layout in a new build.
- `damage_database.cpp`: load the turret table once, resolve cross-entry
  exclusions, precache its models and particles, and return candidates matching
  both damage bits and hitbox. Keep the database shared by client and server;
  precache calls are server-only.
- `hitbox_damaged_entity_shared.cpp`: consumers use the database's hitbox,
  gib, attachment, particle and fatal-damage fields. Check those consumers
  before changing defaults or interpreting selection output.
- `external/portal2_steam2_decompiled/skeleton/841_1/server/game/shared/portal2/damage_database.h.skeleton.h`:
  declaration names, field order, capacities, and i386 offsets.
- `external/portal2_steam2_decompiled/skeleton/841_1/server/game/shared/portal2/damage_database.cpp.skeleton.h`:
  historical source line mapping, prototypes and local names.
- `/home/john/Downloads/portal2-steam2-research/recovered/server/game/shared/portal2/damage_database.cpp.decompiled.c`:
  readable control-flow reference for depot 852 v3. The corresponding depot
  841 server file and 841/852 client files provide cross-build comparisons.

## Data and parsing behavior

`LevelInitPreEntity` gets the filesystem and encryption key through game rules,
then calls `ReadDamageCutoutDataFromFile` with
`scripts/damageinfo_turret`. The reader uses `ReadEncryptedKVFile`. It only reads
when the destination vector is empty; a failed read or an already-populated
vector returns false. The root `ModelPath` supplies a prefix for gib model
paths. Each top-level true subkey is one effect; its KeyValues name becomes
`damageEffectName`.

The 852 binary retains these case-sensitive KeyValues spellings:

| Key | Meaning |
| --- | --- |
| `ModelPath` | Root-level gib model directory/prefix |
| `SwapModel` | Replacement model to precache/use for the damaged entity |
| `HitBoxes` | Hitbox indices; parsed as an integer list, maximum 8 |
| `ExclusionList` | Names of other effects excluded by this entry, maximum 16 |
| `DeathThroes` | Animation name |
| `Type` | Damage-kind names; matching damage bits are ORed together |
| `Fatal` | Boolean fatal-damage flag |
| `Gibs` | Gib model names; maximum 16, expanded using `%s/%s.mdl` with `ModelPath` |
| `Gib Lifetime` | Float lifetime |
| `Gib Speed` | Float separation speed |
| `ParticleAttach` | Particle attachment name |
| `ParticleAttachHitbox` | Attachment hitbox, default `-1` |
| `ParticleEffect` | Space-separated particle-effect and integer-chance pairs, maximum 8 |

`Type` recognizes these strings case-insensitively and ORs the Source damage
bits: `heat` → `DMG_BURN`, `explosion` → `DMG_BLAST`, `fizzler` →
`DMG_DISSOLVE`, `corrosive` → `DMG_ACID`, `slice` → `DMG_SLASH`, `crush` →
`DMG_CRUSH`, and `impact` → `DMG_FALL`. Unknown type words add no bits.

The list helpers copy into 1024-byte scratch buffers and stop at the fixed array
capacity. Integer tokens use `V_atoi`; exclusions resolve case-insensitively to
other effect names after all entries are loaded. Each unresolved exclusion is
left at index `-1` and emits `Unable to find exclusion %s for damage effect %s.`
The gib helper formats every token as `%s/%s.mdl`; it does not visibly preserve
an already-qualified model path.

`ParticleEffect` is parsed as space-separated `effect chance` pairs by
`ParseStringToEffectArray`, filling parallel effect-name and integer-chance
arrays (capacity 8). The database sums those chances for weighted selection by
the consumer. The key is established by the binary's PIC relocation and the
adjacent string table; this is stronger evidence than the decompiler's
unresolved relocation label.

## Lifecycle and selection

After reading, `LevelInitPreEntity` precaches every `SwapModel`, each gib model,
and each particle system. The server binary also has a `PrecacheAssets` method
with the same loops; DWARF maps it to a separate source function, but the
decompiled `LevelInitPreEntity` visibly performs these loops after loading.
Preserve that observed order until caller/build evidence establishes which
path is active in the target configuration.

`GetDamageInfoVector(type)` returns the vector for that entity type. The
historical implementation indexes directly; bounds validation is a reasonable
portability hardening, but changes invalid-enum behavior. Candidate lookup
returns zero for an empty vector or nonpositive output capacity. It visits
entries in database order and accepts entries when `(entry.damageType &
damageType) != 0`. A query hitbox of `-1` accepts every matching damage type;
otherwise an entry must have at least one configured non-negative hitbox equal
to the query. A negative configured hitbox does not match a concrete query in
the recovered branch. It stops at
`maxDamageEffects` and returns the number written. This routine does not apply
exclusion relationships; those indices are data for later effect selection.

The two ConVars are `max_hitbox_damage_effects_per_entity` (default `2`) and
`hitbox_damage_enabled` (default `0`). Their descriptions and use belong to the
consumer path; the database lookup itself takes an explicit output capacity.

## Decompiler limits and implementation review

The pseudocode is generated from Ghidra and matching dSYM/DWARF metadata, not
original source. It recovers names and much of the control flow, but loses the
particle-list KeyValues string, obscures one-character delimiters, and emits
pointer arithmetic in place of structure fields. Depot 841 and 852 binaries
also differ from current retail and from this repository's engine APIs. The
current reconstructed implementation should therefore be checked for these
specific points before it is treated as behaviorally faithful:

- exact field spellings and encrypted file path above;
- exact damage-kind vocabulary (the seven historical names above);
- gib path construction with `%s/%s.mdl`;
- not matching a specific hitbox when an entry declares no hitboxes;
- exclusion resolution and unresolved-name warning;
- server precache timing and all three asset categories;
- particle-dispatch overload semantics for `ParticleAttachHitbox`, which remain
  unresolved from the available pseudocode.

Reference provenance and tool limitations are documented in
[`external/portal2_steam2_decompiled/README.md`](../../../external/portal2_steam2_decompiled/README.md)
and [`game/shared/portal2/MISSING.md`](MISSING.md).
