# Retail Portal 2 decompilation — session documents

Status: active (2026-10-04). This folder records every session's findings,
commands and status for the retail Portal 2 `server.so` / `client.so`
decompilation campaign.

`game/shared/portal2/BEHAVIOR_GAPS.md` and `game/shared/portal2/MISSING.md`
remain the authorities for what is missing in our tree. This folder never
redefines a gap; it records what the retail binaries actually contain and the
evidence for each verdict. Where this folder and a gap row disagree, the
disagreement is called out and `BEHAVIOR_GAPS.md` is updated in the same
change.

## Contents

| Document | Purpose |
| --- | --- |
| [log.md](log.md) | Append-only dated session log: commands run, results, decisions |
| [findings/datadesc-layout.md](findings/datadesc-layout.md) | Retail `typedescription_t` / `datamap_t` layout, string search method, tool inventory and their known pitfalls |
| [findings/g07-g14-base-game-keys.md](findings/g07-g14-base-game-keys.md) | Base-game keys and inputs (G07–G14): retail entries, owners, handlers |
| [findings/g15-g21-portal2-keys.md](findings/g15-g21-portal2-keys.md) | Portal 2 entity keys and inputs (G15, G17–G21) |
| [findings/not-a-gap.md](findings/not-a-gap.md) | Retail keys that do not exist, or that we already implement, so the row is not a gap |

## Binaries

| Binary | Path | Size | sha256 |
| --- | --- | --- | --- |
| `server.so` | `/home/john/.local/share/Steam/steamapps/common/Portal 2/portal2/bin/linux32/server.so` | 19,081,676 | `cf5326a75931256500afc870af767c5a4bf2fc9e41ced337171e1e13cd96b6f0` |
| `client.so` | `/home/john/.local/share/Steam/steamapps/common/Portal 2/portal2/bin/linux32/client.so` | 26,277,100 | (recorded on import) |

Both are stripped. `server.so` carries a `.gnu_debuglink` to
`server.so.dbg`, which does not exist. The retail FGD is
`/home/john/.local/share/Steam/steamapps/common/Portal 2/bin/portal2.fgd`;
the era FGD copy used for cross-checks is
`external/portal2_steam2_scripts/841_24/bin/portal2.fgd`.

## Ghidra

- Ghidra 12.0.4 at `/home/john/ghidra_12.0.4_PUBLIC`.
- Headless launcher `support/analyzeHeadless6g` (6G MAXMEM; 125 GB RAM /
  32 cores available).
- Persistent project: `/home/john/Downloads/portal2-steam2-research/ghidra/portal2_retail`,
  program `/server.so`, imported and fully analyzed once. `client.so` is not
  yet imported (G27 work still uses the volatile
  `/tmp/codex-portal2-ghidra/` project).
- Scripts live in `tools/portal2/dsym/` and are run with:

```sh
/home/john/ghidra_12.0.4_PUBLIC/support/analyzeHeadless6g \
  . portal2_retail -process server.so -noanalysis \
  -scriptPath /home/john/src/source-engine/tools/portal2/dsym \
  -postScript <Script>.java <args...> > out/<name>.log 2>&1
```

  run from `/home/john/Downloads/portal2-steam2-research/ghidra`. Output files
  are written relative to that directory (`out/…`). Scripts call
  `createFunction`, so `-noanalysis` is correct and `-readOnly` must not be
  passed. Note that `-postScript` may be repeated; the argument list must be
  expanded as separate shell words (zsh does not word-split an unquoted
  variable — use an explicit command line, not `"$ARGS"`).

## Address mapping (memorise this before reading any number)

| Region | ELF vaddr | Ghidra address | File offset |
| --- | --- | --- | --- |
| `.text` | `0x00385720`–`0x00d068d7` | **Ghidra = vaddr + 0x10000** | file off == vaddr |
| `.rodata` | — | Ghidra `0x00cf7000…` | file off == vaddr |
| `.data` | — | Ghidra `0x011861e0…` | file off = vaddr − 0x1000, so **entry file offset = Ghidra − 0x11000** |

Raw-byte and struct-dump scripts in `findings/` therefore use
`file_offset = ghidra_address - 0x11000` for `.data` and the addresses
directly for `.text` code pointers.

## Tool inventory (`tools/portal2/dsym/`)

| Script | Purpose | Known limits |
| --- | --- | --- |
| `DatadescLookup.java` | String name → data references → decode the 0x40-byte `typedescription_t`, decompile `inputFunc`, resolve the table's static-initializer writer | `findDatamap()` always returns null: retail `datamap_t` lives in `.bss` and is filled at runtime. Tail-merged string suffixes are handled (`merged into …`) |
| `FindOwner.java` | Class attribution: walk back 0x40-byte steps from an entry, keep candidates with code references, decompile the writer, regex `"C…"` class names | Needs the correct window (`0x1000` typical, up to `0x8000` for large tables); prints nothing when no code-referenced table start exists in range |
| `StringUsers.java` | Users of a string literal | Misses tail-merged suffixes; prefer `DatadescLookup` |
| `DumpRange.java`, `Probe.java`, `DerefDecomp.java`, `ProgramInfo.java` | Range dumps, single-address probes, pointer→decompile, program facts | |
| `VtableSlot.java`, `Callers.java`, `DecompByName.java` | Vtable slot, callers, name lookup | `getReferencesTo` is unreliable for code references to string literals (see below) |
| `function_gap.py`, `dwarf_skeleton.py` | Standalone analysis helpers | |

**Pitfall:** Ghidra's `getReferencesTo` misses code references to string
literals (e.g. `drawinfastreflection` is compared inside `FUN_0046f0e0` yet no
reference is reported). Use a raw 4-byte little-endian pointer search over
`.text` for code usage, and `getReferencesTo` only for `.data` addresses.

**Pitfall:** static `.data` arrays are relocated with `R_386_RELATIVE`, so
Ghidra applies the values but creates no references. `getReferencesTo` finds
nothing; scan memory dwords instead.

## Status

- `server.so` imported, analyzed, and queried for all gap keys G07–G21.
- Class attribution for every confirmed entry complete except
  `PaintPower`'s second owner (see `findings/g15-g21-portal2-keys.md`).
- `client.so` not imported into the persistent project (G27).
- No gap row has been changed yet; this folder is the evidence for the next
  `BEHAVIOR_GAPS.md` update.
