# matchmaking — Portal 2 matchmaking framework

Provenance: every file here except those listed under "Local files" is
imported from the leaked 2017 CS:GO source tree (`cstrike15_src/matchmaking`
and `cstrike15_src/matchmaking/portal2`, from the "April 22nd 2020" archive).
They are not original work of this repository. The repository's provenance
and distribution warning applies to this directory; do not redistribute it on
the assumption that it is freely licensed.

Imported subset: the framework files of `matchmaking_base_inc.vpc` needed by
an offline session (framework, events, extensions, sessions and the offline
custom session, match system, player manager and players, DLC manager, voice,
server manager, datacenter, leaderboards, QoS, net message controller, Steam
hook) and the Portal 2 title from `matchmaking_portal2_inc.vpc`
(`portal2/mm_title*.cpp`). The online sessions (`sys_session`,
`mm_session_online_*`, `ds_searcher`, `match_searcher`, `searchmanager`,
`mm_netmgr`) and the X360/PS3/Steam lobby files are not imported.

## How it is built and owned

`wscript` builds the files as objects linked into the Portal 2 server game
module (`game/server/wscript` recurses here for `--build-games=portal2`).
Retail Portal 2 loaded matchmaking as an engine app system shared by the
client and server modules. This engine does not, so:

- The server module owns the framework
  (`game/server/portal2/portal2_matchmaking.cpp`). It connects and
  initializes it in `DLLInit`, which the engine runs before it initializes
  the GameUI and the client, and publishes it through the engine's
  `IMatchFrameworkHost` (`public/engine/imatchframeworkhost.h`). It withdraws
  and shuts it down in `DLLShutdown`, after the client and GameUI.
- The server registers `IMatchServerHostPortal2`
  (`public/matchmaking/portal2/imatchhost_portal2.h`) as a matchmaking
  extension. It replaces `IServerGameDLL::ApplyGameSettings` and
  `INetSupport::GetServerInfo`, which this engine's frozen interfaces lack.
- The client module and its GameUI borrow the published framework
  (`Portal2_ConnectMatchFramework`, `game/shared/portal2/portal2_shared_compat.h`);
  the client runs its frame and reports signon changes from `HudUpdate`, and
  the GameUI reports level loading start/finish. A dedicated server runs the
  frame from `GameFrame`.

## Local files

- `mm_portal2_compat.h/.cpp`: this engine's answers to CS:GO-era engine calls
  (split-screen slots, short level name, build number, loading commands) and
  the console constants the SDK 2013 headers lack.
- `mm_matchmakingdata.h`: the `MatchmakingData` record from the CS:GO title
  header, whose other contents collide with Portal 2's.
- `portal2/portal2.spa.h`, `portal2/inc_gamemode.inc`: title constants
  reconstructed from the retail Portal 2 Linux `matchmaking.so`; see the
  header comments. The `common/xlast_portal2/*.inc` title tables were
  reconstructed from the retail `matchmaking.so` and `client.so` in the same
  way.

Local changes to imported files are marked `Portal 2 port:`. Behavior this
engine cannot provide reports itself once through `PORTAL2_UNSUPPORTED`
(`DevWarning`), or fails with a documented session error:

- Online sessions (LIVE/Steam lobbies, LAN, team and quick matches, game
  searches): `CreateSession`/`MatchSession` broadcast
  `OnMatchSessionUpdate { state error, error n/a }`; code needing them is
  behind `MM_PORTAL2_ONLINE_SESSIONS`, never defined.
- Server reservations, LAN discovery, DLC discovery, split-screen slots and
  names, split-screen `ss_map`, map launch options for `::ExecGameTypeCfg`.
- Steam: built with `NO_STEAM` because this engine builds without Steam. The
  local player is named by the `name` convar and its title data (chapter
  progress, co-op progress, game instructor) lives in memory for the process
  only; `ResetConfiguration` is broadcast when it loads.
