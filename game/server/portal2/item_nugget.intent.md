# `item_nugget.cpp` intent recovery

Evidence: the depot 852_3 DWARF skeleton and Ghidra pseudocode at
`external/portal2_steam2_decompiled/{skeleton/852_3/server/game/server/portal2/item_nugget.cpp.skeleton.h,852_3/server/game/server/portal2/item_nugget.cpp.decompiled.c}`.
This note recovers behavior, not original source text.

## Recovered behavior

- `CItem_Nugget` is a touch collectible with map fields for respawn delay, value,
  and an optional availability group. It uses four value/skin tiers: 1, 5, 10,
  and 25. Spawn clamps the configured value upward to the first tier at least
  as large (values below 5 become 1; values at or above 25 become 25).
- Spawn assigns the matching skin and tier-specific model scale and bounds,
  uses a nonmoving bounding-box entity in collision group 5, and registers the
  entity with `CNuggetDirector`. It also applies a fixed angular impulse and
  offsets its origin vertically. These are presentation/placement details, not
  value logic.
- On first valid player touch, the nugget sends a reliable user message naming
  the nugget classname to that player, emits its pickup sound to nearby clients,
  increments the player's frag count by the nugget value, and appears to add the
  same value to the player's team score. It then removes itself when the
  respawn delay is nonpositive, otherwise it hides and schedules respawn.
- Respawn marks the nugget hidden and schedules `MaterializeThink` after the
  configured delay. Materialization emits a sound, clears the hidden effect,
  and refreshes transmit state.
- The director tracks ungrouped nuggets in one list and named groups in a
  group-sorted list. At post-entity initialization, a server setting can trigger
  availability randomization: each ungrouped nugget is selected independently;
  one random decision is made per named group, so all nuggets in a group share
  the decision. Unavailable nuggets are hidden. Registration and removal keep
  both collections current.

## Implementation readiness and open details

This is comparatively self-contained: the skeleton names all entity/director
methods and state, while the decompile exposes pickup, hide/respawn, registration,
and group-randomization control flow. It relies on ordinary server entity,
recipient-filter, sound, and vector APIs; it does not require the projected-paint
or damage-database dependency chain.

Confirmed from the decompile: four value thresholds, hidden-effect handling,
player frag increment, respawn scheduling, grouped versus ungrouped selection,
and the availability setting gate. The exact tier scales/bounds, model and sound
asset names, user-message name, ConVar defaults/flags, and team-score vtable call
need recovery from binary strings/symbols or runtime/source references before
claiming exact compatibility. Decompiled vtable calls and relocation-based
constants are not reliable names by themselves. The sort comparator's exact
ordering also needs inspection before reproducing it.
