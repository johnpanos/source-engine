# RFC 0009 progress: USD-native map authoring

Updated: 2026-09-25
Portfolio row: R59 (U0–U2), active. R60 (U3–U4) is planned.

Direction (user, 2026-09-25): Hammer and the game evolve together. The target
is a working author → compile → play loop, not legacy-editor parity. Each
slice lands with a map or gameplay use.

## U0: authoring schema, profile, ID policy and validator (active, 2026-09-25)

Scope: a versioned Source authoring schema profile for editable USD maps, with
these parts:

- authored roles: world structure, `prop_static`, `prop_dynamic`,
  `prop_physics` and geometric entities;
- the initial world-solid geometry profile;
- the source-to-compiled ID policy;
- a hand-authored USD room fixture;
- an independent validator, with positive and deliberately bad fixtures
  proving the role, topology, reference and composition checks.

It is not RFC 0008's compiled `SourceWorldAPI` schema, and no implementation
status is inferred from that schema. U1 (the native compiler) is not part of
this slice.
