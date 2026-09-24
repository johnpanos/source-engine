# Portal 2 shared source import

The initial 70 `.cpp` and `.h` files moved from the local `portal` source drop
match the corresponding files in the local `cstrike15_src/game/shared/portal`
archive byte for byte. Four additional shared files came from the local game
drop, and `paint_enum.h` came from the archive. The original files retain their
copyright headers and line endings. The aggregate SHA-256 of sorted original
file names, NUL separators, and each original file's SHA-256 digest is
`efd90b1a1bf85ff619401357da49b1266853e898e429eb83c827ffab32259b6f`.

This is the shared-code cohort for a **separate Portal 2 client/server target**.
The existing `portal` target selects `game/shared/portal` explicitly through
`game/client/client_portal.vpc` and `game/server/server_portal.vpc`. Its source
selection must remain independent of these files.

The separate Portal 2 Waf configuration is available through `./play_p2
--configure-only`. The 94 selected gameplay sources that have Steam2
pseudocode, with their headers, have been reconstructed from it; every such
file carries a `Portal 2 reconstruction` header and is not original Valve
source. The VPC source maps still name 42 selected sources without pseudocode.
[MISSING.md](MISSING.md) lists them, the remaining include gaps and the
probe-compile status of the reconstructed files. Waf stops before compiling an
incomplete Portal 2 client or server. The source target needs those files, the
Source SDK 2013 base-game and engine API adaptations the Portal 2 code expects,
and game-specific conformance runs before it can be launched.
Historical Steam2 source references are isolated under
[`external/portal2_steam2_xsi`](../../../external/portal2_steam2_xsi/README.md),
[`external/portal2_steam2_xsi_legacy`](../../../external/portal2_steam2_xsi_legacy/README.md),
[`external/portal2_steam2_scripts`](../../../external/portal2_steam2_scripts/README.md),
and [`external/portal2_steam2_maps`](../../../external/portal2_steam2_maps/README.md).
The first two contain XSI tool source; the others contain VScript, Lua, FGD, and VMF
variants. [Debug-symbol-guided pseudocode](../../../external/portal2_steam2_decompiled/README.md)
is also retained as a reconstruction reference. None supplies buildable
gameplay C++ or enters the current build.
After moving every exact-path match from the local game drop, the other local
source archive has no exact-path copy of a remaining VPC source except a
PS3-specific `vjobutils.cpp`. Remaining gameplay sources need implementation;
same-named files from other games require semantic review before reuse.

`./play_p2 --prepare` stages the installed Portal 2 VPKs by symlink into a
private runtime. `./play_p2 --retail` starts the installed retail binary when
immediate play is needed; the default command targets this repository's build.
The installed Linux `portal2_linux` and game modules are 32-bit, while this
branch's configured Linux target is 64-bit, so the retail modules cannot fill
the missing source target.

`./play_p2` runs on Box3D (`vphysics_box3d`), as `./play` does;
`PHYSICS=vphysics ./play_p2` or an explicit `-physics` argument selects IVP.
On 2026-09-24, player movement and noclip were compared headless against IVP on
`sp_a2_triple_laser` and `sp_a1_intro3`. The check covered falling, walking,
friction, jumping, crouching, pushing a cube, noclip in all directions, and the
arrival-elevator start. Client-view positions (`getpos`) matched on both
backends. That needed three base fixes:
- Single-player local transfer now carries the origin z that Portal 2 sends
  separately from XY (`engine/dt_localtransfer.cpp`); before, the view froze
  vertically.
- `func_tracktrain` gained Portal 2's `MoveToPathNode`, `TeleportToPathNode` and
  `SetMaxSpeed` inputs and `OnArrivedAtDestinationNode` output.
- A train stopped at a destination node coasts only the 0.1 s look-ahead
  (`game/server/trains.cpp`). Otherwise it pushed against the player every tick
  and held walking near 26 u/s.

Known gap: `sp_a1_intro3` hangs in `LevelShutdown` on quit, in
`CPSCollisionEntity::UpdateOnRemove`. That map has pre-placed portals.

The repository's provenance and distribution warning in the root README also
applies to this import.
