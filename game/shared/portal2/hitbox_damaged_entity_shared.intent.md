# Hitbox damage effect intent

This note reconstructs the behavior of `hitbox_damaged_entity_shared.cpp` from
the Portal 2 Steam2 852_3 server and client binaries. The decompiler output is
evidence, not original source; unresolved pointers, virtual calls, and optimized
locals limit certainty about some transforms and overload arguments.

## Confirmed behavior

`EmitParticles` looks up the selected `DamageInfo` and returns for an invalid
effect index. When `particleAttachment` is nonempty, it resolves that named
attachment on the owner and dispatches the particle with
`PATTACH_POINT_FOLLOW` if the attachment exists. Otherwise, a
`particleAttachmentHitbox` value of `-1` dispatches with
`PATTACH_ABSORIGIN_FOLLOW`. Other values are converted to a vector whose x
component is the configured integer and sent to a positional particle dispatch.
The decompiler cannot reliably identify the globals supplying that overload's
origin and angles, so their exact values and the meaning of the field name
`Hitbox` remain uncertain.

`EmitGibs` requires a valid parent and damage entry with at least one gib model.
It reads the parent's `placementOrigin` attachment as a local transform; if
absent, it uses an identity transform. Separately, the burst center starts at
the parent's absolute origin and is replaced by the `burstCenter` attachment
position when present. These are distinct references in the recovered code.

For each configured gib model, the routine checks that the model is available
and obtains its studio header. If that gib model has a `placementOrigin`
attachment, its local position is measured relative to the parent's
`placementOrigin`, then transformed through the parent's entity coordinate
frame. The binary also queries parent velocity and, when physics is present,
velocity at the calculated gib position. It creates a server `gib` entity or a
clientside gib, assigns the configured lifetime, and propagates the parent's
no-draw effect bit. When `gibSeparationSpeed` is nonzero, it normalizes the
vector from burst center to gib position and applies an impulse at that speed.
If the vector is zero, the server and client implementations fall back to the
gib's world-space center before normalizing.

The server and client pseudocode have the same attachment and separation
structure. Their construction details differ: the client creates a clientside
gib with position, parent velocity, angular impulse, and lifetime; the server
spawns and initializes a networked gib, then applies separation as an impulse.

## Decompiler ambiguity

- The recovered symbols identify `placementOrigin` and `burstCenter` by their
  matching binary strings, but the exact transform multiplication and which
  attachment frame the original author intended are not recoverable with full
  confidence from this optimized i386 pseudocode.
- The parent placement attachment is read with `GetAttachmentLocal`; the gib's
  placement attachment is also read in local space. The code then subtracts
  the parent placement position and applies the parent entity coordinate frame.
  Preserve this two-stage relationship when reconstructing; do not assume the
  gib should simply spawn at the parent origin or burst center.
- The particle positional-dispatch overload receives a vector containing the
  configured hitbox number in x. The decompiler renders the origin and angle as
  pointers to globals, but does not establish their names or values. Confirm
  the intended engine overload and behavior before replacing this with an
  attachment lookup or a fabricated hitbox transform.
- Server `Spawn` receives a fixed `25.0` argument, while the configured
  `gibLifeTime` is assigned afterward. Keep these as separate operations; the
  pseudocode does not show that the configured lifetime replaces the spawn
  argument.

## Implementation checklist

- Bounds-check entity type, damage entry, and particle effect index before
  accessing the database.
- Preserve named attachment follow behavior and the `-1` origin-follow case.
- Verify the positional particle overload against the engine declaration and
  historical runtime before assigning meaning to `particleAttachmentHitbox`.
- Keep parent `placementOrigin` and `burstCenter` handling separate.
- Place each gib using its own model's `placementOrigin` relative to the
  parent's placement transform, then account for parent world transform.
- Carry parent velocity into gib creation; use velocity at the gib point when a
  physics object provides it.
- Preserve client/server creation differences, lifetime assignment, no-draw
  propagation, and conditional separation impulse with the zero-vector fallback.
- Add focused checks for absent attachments, missing models, zero separation
  direction, invalid indices, and a physics parent with point velocity.

## Evidence

- `external/portal2_steam2_decompiled/852_3/server/game/shared/portal2/hitbox_damaged_entity_shared.cpp.decompiled.c`
- `external/portal2_steam2_decompiled/852_3/client/game/shared/portal2/hitbox_damaged_entity_shared.cpp.decompiled.c`
- `external/portal2_steam2_decompiled/skeleton/852_3/server/game/shared/portal2/hitbox_damaged_entity_shared.cpp.skeleton.h`
- `external/portal2_steam2_decompiled/skeleton/852_3/client/game/shared/portal2/hitbox_damaged_entity_shared.cpp.skeleton.h`
- `game/shared/portal2/hitbox_damaged_entity_shared.cpp` (current reconstruction for comparison)
