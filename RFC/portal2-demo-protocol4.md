# Portal 2 demo protocol 4: research and implementation plan

Status: research record (2026-10-09). The container format is implemented
(`cd4f2ba88`); the network layer is not. Purpose: play the 81 leaderboard demos in
`quality/fixtures/demos/portal2-board` through `tools/quality/portal2_demo_suite.py`.

## Sources and how they were used

| Source | Used for | Terms |
| --- | --- | --- |
| [NeKzor/sdp](https://github.com/NeKzor/sdp) (`src/messages.ts`, `src/types/*.ts`) | The Portal 2 message-id table, every payload layout below, data/string table and usercmd layouts | MIT. Layouts re-derived here, no code copied |
| [P2SR wiki, Demo](https://wiki.portal2.sr/Demo) | Demo playback and tooling | wiki |
| hl2sdk-csgo and saul/demofile `netmessages.proto` | Cross-check of the id numbering (`net_SplitScreenUser = 3`, `svc_Print = 16`) | BSD |
| Local `~/src/cstrike15_src` (`public/demofile/demoformat.h`, `engine/cl_demo.cpp`) | Cross-check of the protocol 4 container (`dem_customdata = 8`, `dem_stringtables = 9`, per-split `Split_t`) | Leaked Valve source: facts only, nothing copied |
| `~/Downloads/portal2-steam2-research/ghidra/portal2_retail.gpr` | Ground truth for any layout the above disagree on (retail `engine.so`) | local |
| The 81 demos themselves | Oracle: a standalone walk parses all 84 files to `dem_stop` | n/a |

CS:GO's wire is protobuf and is not a reference for payloads. Portal 2 is bit-packed.

## 1. Container (done, `cd4f2ba88`)

Header identical to protocol 3 (1072 bytes; `demoprotocol` 4, `networkprotocol` 2001).
Per command: type byte, tick `int32`, **slot byte**. Types 1-7 as protocol 3, **8 =
custom data** (`int32` id, `int32` size, bytes; skipped), **9 = string tables**.
Signon/packet: **two** 76-byte `Split_t` view records (first is played back), then
in/out sequence `int32` each, size `int32`, data. Console command, data tables and
string tables are `int32` size plus bytes; usercmd is `int32` command number, size,
bytes. 31 or 91 trailing bytes after `dem_stop` (SAR data) are ignored.

## 2. Message ids: retail Portal 2 vs this fork

Retail follows the Alien Swarm numbering. Ids not listed are **identical** in both
(8-15, 17-21, 23-31).

| Message | Retail id | Fork id |
| --- | --- | --- |
| `net_SplitScreenUser` | 3 | 35 |
| `net_Tick` | 4 | 3 |
| `net_StringCmd` | 5 | 4 |
| `net_SetConVar` | 6 | 5 |
| `net_SignonState` | 7 | 6 |
| `svc_Print` | 16 | 7 |
| `svc_SplitScreen` | 22 | 34 |
| `svc_CmdKeyValues` | 32 | 32 |
| `svc_PaintMapData` | 33 | (`svc_SetPauseTimed` is 33) |

`RFC/portal2-splitscreen-retail-engine.md` records that the fork cannot take retail
ids on its own wire, and that stays true. The demo reader needs a **translation
table applied while reading a protocol 4 demo's packets** (retail id to fork id)
and nothing else; the fork's live wire is unchanged. `net_File` (2) has no fork
message class; retail's `NetFile` has an extra bool when the demo protocol is 4.

## 3. Payloads that differ (retail dialect = demo protocol 4)

Verified by reading sdp's readers against `common/netmessages.cpp`. The fork's
readers are gated on `m_NetChannel->GetProtocolVersion()` /
`GetDemoProtocolVersion()`; both return 2001 for a retail demo, so the retail
dialect must key on the demo protocol (4), not the network protocol.

| Message | Retail layout | Fork reads |
| --- | --- | --- |
| `net_SignonState` | state `u8`, spawn count `i32`, **then** server player count `i32`, network-id count `i32` + bytes, map name length `i32` + string | state, spawn count only |
| `svc_ServerInfo` | protocol `i16`, server count `i32`, hltv, dedicated, client CRC `i32`, max classes `i16`, **map CRC `i32`**, slot `u8`, max clients `u8`, **unknown `i32`**, tick interval `f32`, OS char, four strings | MD5 (16 bytes) in place of the CRC when protocol above 17; no unknown `i32` |
| `svc_CreateStringTable` | name, max entries `i16`, entries `log2(max)+1` bits, data length **20 bits**, fixed-size flag (+12+4 bits), **flags 2 bits**, data | optional `:` prefix, **varint** length for protocol above 23, 1 compression bit |
| `svc_UserMessage` | type `u8`, length **12 bits** | `NETMSG_LENGTH_BITS` (check value) |
| `svc_SplitScreen` | 1 bit action, 11-bit data length, data | `splitscreenwire` (1+1+11 bits, own fields) |
| `net_SplitScreenUser` | 1 bit | 1 bit (same) |
| `svc_VoiceInit` | codec string, quality `u8`, `f32` when 255 | `i16` sample rate when 255 |
| Unchanged | `Tick` (`i32`, two `u16`), `StringCmd`, `SetConVar`, `Print`, `SendTable` header, `ClassInfo`, `SetPause`, `UpdateStringTable`, `Sounds`, `SetView`, `FixAngle`, `PacketEntities`, `TempEntities`, `Prefetch`, `GameEventList` | |

`SvcSounds` entry decoding is protocol 3 only in sdp (retail sound entries are not
decoded there); the fork's reader only skips the block, so it is unaffected.

Data tables (`dem_datatables`, and `svc_SendTable` bodies): retail `SendProp` is
type 5 bits, name, **flags 16 bits then an extra 11-bit field**, then the exclude
name or low/high/bits. The fork's send-table reader is the CS:GO-era one, so the
empty-name `missing SendTable ''` and `CreateDecoders failed` errors seen on the
first retail packet most likely come from this layout plus the misrouted ids, not
from a class mismatch. Expect a real class-set mismatch afterwards (retail server
classes against the reconstructed client); that is game-DLL work.

Usercmd (`dem_usercmd` payload): bit-packed with a presence bit per field (command
number, tick, three view angles, three moves, buttons, impulse, weapon select 11
+ optional subtype 6, mouse dx/dy `i16`). The engine hands it to
`DecodeUserCmdFromBuffer` in a 256-byte buffer; retail commands must fit.

## 4. Open questions (need the retail binary or a run)

1. `NETMSG_LENGTH_BITS` in the fork against retail's 12 for user messages.
2. Whether the fork's `SendProp` reader can take retail's extra 11-bit field
   behind a dialect switch or needs its own reader.
3. User message ids and layouts per game (retail Portal 2 table in sdp's
   `UserMessages.Portal2Engine`) against the reconstructed client DLL.
4. `svc_PaintMapData` (33): needs a handler or a skip.
5. Whether `svc_GameEventList`/`svc_GameEvent` descriptors match the client DLL.

## 5. Implementation order (each step has a gate)

1. **Dialect flag.** One owner: `CDemoPlayer` exposes `IsRetailDialect()` (demo
   protocol 4); message readers and the id table read it. Gate: unit suite that
   decodes captured retail packets (bit streams extracted from the fixtures) and
   proves each reader consumes exactly its bits.
2. **Id translation** in the demo packet path (section 2) and the payload
   dialect rows of section 3, smallest first: `SignonState`, `ServerInfo`,
   `CreateStringTable`, `UserMessage`, `VoiceInit`.
3. **Data tables** retail `SendProp`; gate: the first retail demo's signon
   completes with no `CreateDecoders` failure.
4. **Run the suite**: `corpus.portal2.demos.smoke`, then chapters, then all 81
   (`--workers 4`). A demo passes only if it plays at least 80 percent of its
   recorded length with no `Host_EndGame`, unknown net message or datatable
   error (already enforced by the suite).
5. Triage what remains per map (section 4 items 3 and 5, class mismatches).

Independent oracle: keep a Python walker (the one used for this research) that
decodes message boundaries of every fixture packet from sdp's layouts; the engine
and the walker must agree on message counts per packet. Never loosen the suite's
checks to turn a demo green.
