# Retail `typedescription_t` / `datamap_t` — layout and search method

Evidence: retail `server.so` (sha256 `cf5326a7…b6f0`), Ghidra project
`portal2_retail`. This is the method document; per-gap results live in the
`g*.md` files. `game/shared/portal2/BEHAVIOR_GAPS.md` remains the gap
authority.

## Where the data lives

| Object | Region | Notes |
| --- | --- | --- |
| `typedescription_t` arrays | `.data` (Ghidra `0x011861e0…`) | contiguous, stride `0x40`, terminated by a zeroed entry |
| `datamap_t` (className, base-map pointer, field table) | `.bss`, filled at runtime | **not** readable from the file — see below |
| string literals | `.rodata` (Ghidra `0x00cf7000…`) | tail-merged |
| `inputFunc` handlers, table writers | `.text` | Ghidra = vaddr + 0x10000 |

Retail `datamap_t` is in `.bss` and constructed by each class's static
initializer, so reading it out of the binary always yields null. This is why
`DatadescLookup.findDatamap()` was validated as a failure and replaced by
`FindOwner.java`.

## `typedescription_t`, stride 0x40 (retail)

| Offset | Size | Field |
| --- | --- | --- |
| `+0x00` | int | `fieldType` (`fieldtype_t`) |
| `+0x04` | ptr | `fieldName` (member-name string) |
| `+0x08` | int | `fieldOffset` — **single int**; our `public/datamap.h` declares `fieldOffset[2]` |
| `+0x0c` | u16 | `fieldSize` |
| `+0x0e` | s16 | `flags` (`FTYPEDESC_*`) |
| `+0x10` | ptr | `externalName` — the map key / input / output name |
| `+0x14` | ptr | `pSaveRestoreOps` |
| `+0x18` | ptr | `inputFunc` (inputs) |
| `+0x1c` | ptr | `td` (embedded `datamap_t *`) |
| `+0x20` | int | `fieldSizeInBytes` |
| `+0x24` … `+0x3c` | | unused / padding in the retail binary |

## `fieldtype_t` — identical to ours

`VOID 0, FLOAT 1, STRING 2, VECTOR 3, QUAT 4, INT 5, BOOL 6, SHORT 7,
CHAR 8, COLOR32 9, EMBEDDED 10, CUSTOM 11` (our
`public/datamap.h` `fieldtype_t`).

## `FTYPEDESC_*` flags — corrected reading

Read from our `public/datamap.h` (the retail binary uses the same values; an
earlier session assumed SAVE=0x0001, which is wrong):

| Flag | Value |
| --- | --- |
| `FTYPEDESC_GLOBAL` | `0x0001` |
| `FTYPEDESC_SAVE` | `0x0002` |
| `FTYPEDESC_KEY` | `0x0004` |
| `FTYPEDESC_INPUT` | `0x0008` |
| `FTYPEDESC_OUTPUT` | `0x0010` |
| `FTYPEDESC_FUNCTIONTABLE` | `0x0020` |
| `FTYPEDESC_PTR` | `0x0040` |

Observed encodings in retail entries:

| Observed | Meaning | Kind |
| --- | --- | --- |
| `0x0006` | SAVE\|KEY | map keyfield (`DEFINE_KEYFIELD`) |
| `0x0008` | INPUT | input (`DEFINE_INPUTFUNC`) |
| `0x0016` | OUTPUT\|SAVE\|KEY | output (`DEFINE_OUTPUT`) |
| `0x000e` | SAVE\|KEY\|INPUT | keyfield also exposed as an input |
| `0x0002` | SAVE | plain `DEFINE_FIELD`, not settable from a map |

## Two ways a datadesc array is reachable

1. **Static `.data` array.** Entries are relocated by `R_386_RELATIVE`.
   Ghidra applies the resulting values but creates *no* references, so
   `getReferencesTo(entryAddr)` finds nothing and
   `getReferencesTo(stringAddr)` usually finds only the `.data` pointer.
   Decode the entries directly (`index = (entryAddr − arrayStart) / 0x40`),
   and locate `arrayStart` by scanning backwards for a code-referenced
   address (this is what `FindOwner.java` does).
2. **Static-initializer-written table.** The `datamap_t` (and sometimes
   handler pointers) are zero in the file and written by a function that runs
   at load. `getReferencesTo(slotAddr)` returns the writer; decompiling it
   yields `*(int*)(DAT_… + off) = …` assignments and the `className = "C…"`
   string, which is the class attribution.

`FindOwner.java` walks back from an entry in `0x40` steps, keeps each
candidate start that has code references, decompiles the nearest writer, and
regex-extracts `assigned class names: [C…]`. Window sizing matters: `0x1000`
covers normal classes, `0x8000` was needed for `CParticleSystem`'s table.

## String search rules

- Tail merging: search for `name\0` **without** requiring a preceding NUL.
  `DatadescLookup` reports this (`merged into "allowfunnel" @ … suffix +0`).
- A key name string existing is *not* evidence that it is read: check for
  pointers. A pointer in `.rodata`/`.data` only (one hit) means the string is
  owned by the datadesc entry; a pointer in `.text` means a `KeyValue`-style
  code path also reads it.
- `.text` pointer search must be a raw 4-byte little-endian scan:
  `getReferencesTo` misses code references to string literals
  (`drawinfastreflection` in `FUN_0046f0e0` is the confirmed false negative).

## What "retail implements this key" means

All three of:

1. a `typedescription_t` entry (or a `KeyValue` compare in `.text`) whose
   `externalName` is the key, on a table whose owner is the FGD entity or one
   of its base classes;
2. a member field that stores it, with a type consistent with the FGD's
   declared type;
3. — where the key exists only in maps and FGD, not in a table — a note that
   retail ignores it (see `not-a-gap.md` for `target_size` and
   `playspawnsound`).
