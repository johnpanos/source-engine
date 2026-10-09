# Portal 2 demo protocol 4: playing retail recordings

Status: implemented (2026-10-09). All 81 leaderboard demos in
`quality/fixtures/demos/portal2-board` play to their end through
`tools/quality/portal2_demo_suite.py` on the portal2 kiln profile. Scope is
**playback only**: the engine still records protocol 3, and the fork's own wire
format is unchanged. Retail interop for live play remains a non-goal
([RFC/portal2-splitscreen-retail-engine.md](portal2-splitscreen-retail-engine.md)).

## Sources and how they were used

| Source | Used for | Terms |
| --- | --- | --- |
| [NeKzor/sdp](https://github.com/NeKzor/sdp) (`src/messages.ts`, `src/types/*.ts`) | Portal 2 message-id table, most payload layouts, data and string table layouts | MIT; layouts re-derived, no code copied |
| [P2SR wiki, Demo](https://wiki.portal2.sr/Demo) | Playback and tooling | wiki |
| hl2sdk-csgo and saul/demofile `netmessages.proto` | Cross-check of message numbering | BSD |
| Local `~/src/cstrike15_src` (`demoformat.h`, `cl_demo.cpp`, `dt_common.h`) | Cross-check of the protocol 4 container and property flag order | Leaked Valve source: facts only, nothing copied. Its network layer is protobuf and does not apply |
| Retail `engine.so` in the Ghidra project `~/Downloads/portal2-steam2-research/ghidra/portal2_retail` | Ground truth where sdp and the fixtures left a question: the delta-bits reader and `RecvTable_MergeDeltas` (run `StringUsers.java` on the string in the function's error message) | local |
| The 81 demos | Oracle. Independent Python decoders (message walker, string tables, send tables, entity delta streams) were checked against the engine's reads | n/a |

Where sdp and the fixtures disagreed, the fixtures won (the `ServerInfo` field order
below).

## Where the dialect is selected

Everything keys on one fact: the demo header's `demoprotocol` is 4.
`CDemoFile::IsRetailDialect()` -> `IDemoPlayer::IsRetailDialect()` ->
`INetChannel::IsRetailDemoDialect()` (set by `CNetChan::ProcessPlayback` per
packet). The network protocol number cannot be used: retail's 2001 is not
comparable with the fork's. Live connections never see the dialect.

## 1. Container (`engine/demofile.cpp`, `public/demofile/demoformat.h`)

Header as protocol 3. Per command: type byte, tick `i32`, **slot byte**. Type 8 is
**custom data** (`i32` id, `i32` size, bytes: skipped) and 9 is **string tables**.
Packets carry **two** 76-byte view records (the first is played); then in and out
sequence and the size-prefixed data. 31 or 91 bytes after `dem_stop` (SAR) are
ignored.

## 2. Message ids (`common/protocol.h`, `CNetChan::_ProcessMessages`)

Retail uses the Alien Swarm numbering. Differences from this fork, applied while
reading a retail demo only:

| Message | Retail | Fork |
| --- | --- | --- |
| `net_SplitScreenUser` | 3 | 35 |
| `net_Tick` / `StringCmd` / `SetConVar` / `SignonState` | 4 / 5 / 6 / 7 | 3 / 4 / 5 / 6 |
| `svc_Print` | 16 | 7 |
| `svc_SplitScreen` | 22 | 34 |
| `svc_PaintMapData` | 33 | 33 is `svc_SetPauseTimed` |

`RetailDemoMessageToFork()` maps the first four rows and `svc_Print`. 22 and 33
have no counterpart: their length-prefixed bodies are skipped. Every other id is
the same.

## 3. Payload differences (`common/netmessages.cpp`)

| Message | Retail layout |
| --- | --- |
| `net_SignonState` | adds server player count `i32`, a network id list (`i32` count + bytes) and the map name (`i32` length + bytes); skipped |
| `svc_ServerInfo` | `i32` field **between the client CRC and max classes** (sdp lists it later), map CRC `i32` instead of the MD5, no replay bit |
| `svc_CreateStringTable` | data length 20 bits; two flag bits: bit 0 LZSS-compressed data, bit 1 a table of file names |
| `svc_UserMessage` | length 12 bits (fork 11); bodies are skipped |
| `svc_VoiceInit` | `i32` where the fork reads a sample rate |
| `svc_Prefetch` | 13 bits (fork 14) |
| `svc_TempEntities` | length 17 bits (fork: varint); bodies are skipped |
| `svc_CmdKeyValues` | body skipped (retail menu traffic) |

Sound blocks, user messages, entity messages, temp entities and key values are
length-prefixed and are **read past, not processed** (their ids and layouts belong to
the retail client). Game events are processed.

## 4. Tables and entities

* **String table updates** start with an "encoded using dictionaries" bit
  (`CNetworkStringTable::ParseUpdate(..., bRetailDemo)`). Dictionary-encoded
  updates (the shared string dictionary) are refused by name; none of the 81 demos
  used one.
* **Send props** (`RecvTable_ReadInfos`): type 5 bits, name, **19 flag bits** in
  retail order, then an **8-bit priority**. Retail flag bits from 10 up differ from the
  fork and add cell coordinates; `RetailPropFlagsToFork()` maps them. The cell flags
  (`SPROP_CELL_COORD*`, bits 20 to 22 in the fork) are decode only
  (`bf_read::ReadBitCellCoord`; positions use them).
* **Property order** is by priority ascending (`SendTable_SortByPriority`,
  `SendProp::GetPriority()`); `SPROP_CHANGES_OFTEN` counts as priority 64. With only
  default priorities this equals the previous behaviour.
* **Entity delta streams** (`CDeltaBitsReader(..., bRetail)`) start with a mode bit.
  In compact mode: 1 = next property, 01 + 3 bits = a small delta, 00 = the long
  form. The long form (also the whole of mode 0) is a 7-bit code (5-bit value, 2-bit
  tag) with 0, 2, 4 or 7 more bits; the value `0xfff` ends the stream. The
  merge writer emits mode 0 so merged baselines read back in the same dialect.
* **Entity headers** use the old `ReadUBitVar` (four value bits, then a two-bit tag).
* **Classes the client cannot create** (no client class, or no create function: 15
  on `sp_a1_intro3`): entity data is read and dropped
  (`CL_SkipStubEntityData`), the class is remembered per entity index so later
  updates stay aligned, and their send tables get a receive table with no
  properties.

## 5. Findings fixed on the way (not retail specific)

* `CL_FlushEntityPacket` called `delete` on a frame from the client frame pool
  (heap corruption on any flushed packet). It now calls `CClientFrameManager::FreeFrame`.
* `RecvTable_MergeDeltas` indexed past a table's properties on malformed streams; it
  now raises a named error.

## 6. Evidence

`corpus.portal2.demos.self-test` (25 checks) and `corpus.portal2.demos.all` (81
checks, 4 workers) in `quality/conformance.manifest.json`. A demo passes only if it
logs a start and an end in order, plays at least 80 percent of its recorded length
and the log holds no `Host_Error`, `Host_EndGame`, unknown net message, datatable
warning or script error. The suite runs with `r_core_world_strict 0` (a view the
render core cannot draw, such as a planar reflection that did not import, would
otherwise end the process); `--render-strict` turns it back on. The recorded game
state is not checked: entities of classes this client lacks are dropped, and
sound, effects, user messages and menu traffic are not replayed.

## 7. Open

* A retail demo that uses dictionary-encoded string tables.
* Replaying entity state for the 15 classes without client counterparts.
* Game events from the retail event list against the reconstructed client.
