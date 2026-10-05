# Session log — retail Portal 2 decomp

Append-only, newest last. Each entry records the day's goal, the commands that
produced the evidence, what was learned, and what remains open. Findings are
summarised here and written in full under `findings/`.

---

## 2026-10-03 — project import, tool inventory, batch lookups

**Goal.** Stand up a persistent Ghidra project over retail `server.so` and a
scripted route from a gap key name to its retail `typedescription_t` entry.

**Done.**

- Imported `server.so` into `portal2_retail` (`import_server.so.log`,
  `programinfo_server.txt`): stripped, image base 0x0, sections mapped as in
  README. `.gnu_debuglink` → `server.so.dbg`, absent, so no symbols.
- Address mapping derived and verified: Ghidra = ELF vaddr + 0x10000;
  `.text` file off == vaddr; `.data` file off = vaddr − 0x1000
  (entry file offset = Ghidra − 0x11000).
- Wrote `DatadescLookup.java`, `Probe.java`, `DumpRange.java`,
  `DerefDecomp.java`, `ProgramInfo.java`, `StringUsers.java`,
  `VtableSlot.java`, `Callers.java`, `DecompByName.java`.
- Ran batches A–C of gap key names → `ghidra/out/dd_batch_a.txt`,
  `dd_batch_b.txt`, `dd_batch_c.txt`, plus `dd_batch_d.txt`.
- Key methodological discovery: retail strings are tail-merged, so a search
  for `name\0` without a preceding NUL is required, and a lookup such as
  `allowfunnel` prints `merged into "allowfunnel"`.

**Open at end of day.** Class attribution for the located entries (Ghidra
creates no references for `.data` tables); `client.so` import.

---

## 2026-10-04 (session A) — layout, `FindOwner`, map evidence, FGD attribution

**Goal.** Fix the `typedescription_t` layout, attribute each entry to a class,
and cross-check the retail keys against the shipped FGD and the shipped maps.

**Done.**

1. **Layout fixed** (`findings/datadesc-layout.md`): retail
   `typedescription_t` is 0x40 bytes with a *single* `fieldOffset` int (our
   header has `fieldOffset[2]`), `fieldSize` u16 at +0x0c, `flags` s16 at
   +0x0e. `FTYPEDESC_*` values corrected from our `public/datamap.h`:
   `GLOBAL=0x0001 SAVE=0x0002 KEY=0x0004 INPUT=0x0008 OUTPUT=0x0010`.
   Retail encodings decoded: `0x0006` = SAVE|KEY (keyfield), `0x0008` =
   input, `0x0016` = OUTPUT|SAVE|KEY (output), `0x000e` = SAVE|KEY|INPUT.
2. **Two datadesc mechanisms confirmed**: static `.data` arrays (relocation
   only, no Ghidra refs) and handler slots zero in the file, written by
   static initializers. The second is found with `getReferencesTo(slotAddr)`
   and decompiled — this is how class attribution works.
3. **`DatadescLookup.findDatamap()` proven broken**: retail `datamap_t` is in
   `.bss` and filled at runtime, so class names never resolve from it.
   Replaced with `FindOwner.java` (writer-function decompile, `"C…"` regex).
   First write had to be corrected twice: `Long.decode(...).intValue()` for
   `getUnsignedValue` and no unimported `Memory` type.
4. **Attributions completed** for G07–G14, G17, G18, G21 (tables below in
   `findings/`), and the PaintPower entries were dumped as raw tables.
   One headless run failed because zsh does not word-split `"$ARGS"`; the
   `-postScript` list must be written out explicitly.
5. **Pointer-only-in-data check** over both binaries: `BombType`,
   `usesizelimit`, `LowerWeapon` do not exist at all; every other queried key
   has exactly one pointer, in `.data` (the datadesc entry), so retail has no
   `KeyValue` override for them. Exception: `drawinfastreflection`,
   `disableshadowdepth`, `shadowdepthnocache` have `.text` pointer hits
   (0x45f545/0x45f553, 0x45f56a/0x45f578, 0x45f58f/0x45f59d) inside
   `FUN_0046f0e0`, a `CBaseEntity::KeyValue`-style function — confirmed
   against `cstrike15_src/game/shared/baseentity_shared.cpp:361`
   (`bool CBaseEntity::KeyValue`), which handles exactly those three keys.
6. **FGD attribution** from the retail `portal2.fgd`: the entity blocks that
   declare each gap key (`env_portal_laser` L267, `info_placement_helper`
   L216, `npc_personality_core` L506, `prop_tractor_beam` L620,
   `prop_paint_bomb` L680, `prop_weighted_cube` L330,
   `prop_physics_paintable` L818, `prop_physics` L99).
7. **Map evidence**: new scanner `/tmp/opencode/mapkeys.py` (BSP header:
   magic@0, version@4 (=21), 64 lumps × 16 bytes from offset 8, lump 0 =
   entities text) over 106 loose BSPs in `portal2/maps`, 429 matches. Counts
   recorded in `findings/`.
8. **Our-tree cross-check** revealed three rows are not gaps at all:
   `mincpulevel`/`maxcpulevel`/`mingpulevel`/`maxgpulevel` are already
   handled in `game/shared/baseentity_shared.cpp:331-352` (PORTAL2-guarded
   `CBaseEntity::KeyValue`), and `PaintPower` is already a keyfield in
   `game/shared/portal2/prop_paint_power_user.h:126`. Recorded in
   `findings/not-a-gap.md`; `BEHAVIOR_GAPS.md` still needs the row update.

**Open at end of session.** Import `client.so`; vtable confirmation of
`FUN_0046f0e0` as `CBaseEntity::KeyValue` (currently by source comparison,
not by vtable); the second `PaintPower` owner; `BEHAVIOR_GAPS.md` state
updates.

---

## Reproduction cheat sheet

```sh
cd /home/john/Downloads/portal2-steam2-research/ghidra
HL=/home/john/ghidra_12.0.4_PUBLIC/support/analyzeHeadless6g
SRC=/home/john/src/source-engine/tools/portal2/dsym

# name → typedescription entry + handler decompile
$HL . portal2_retail -process server.so -noanalysis -scriptPath $SRC \
  -postScript DatadescLookup.java out/dd.txt allowfunnel trackspeed

# entry → owning class
$HL . portal2_retail -process server.so -noanalysis -scriptPath $SRC \
  -postScript FindOwner.java out/fo.txt 0x01206380 0x1000

# map usage of a key (self-contained; the session copy was /tmp/opencode/mapkeys.py)
python3 - <<'PY'
import struct, sys, glob, os, re, collections
MAPS = os.path.expanduser('~/.local/share/Steam/steamapps/common/Portal 2/portal2/maps')
def entity_lump(path):
    with open(path,'rb') as f:
        hdr = f.read(1036)
        if len(hdr) < 1036: return None
        ver = struct.unpack_from('<i', hdr, 4)[0]          # 21 for Portal 2
        ofs, ln = struct.unpack_from('<ii', hdr, 8)        # lump 0 = entities
        if ln <= 0 or ofs+ln > os.path.getsize(path): return None
        f.seek(ofs); return f.read(ln).decode('latin-1','replace')
def ents(text):
    out=[]
    for m in re.finditer(r'\{(.*?)\}', text, re.S):
        d = dict(re.findall(r'"([^"]*)"\s+"([^"]*)"', m.group(1)))
        if d: out.append(d)
    return out
KEYS = 'allowfunnel,playspawnsound,target_size'.split(',')
files = sorted(glob.glob(os.path.join(MAPS,'*.bsp')))
use = collections.defaultdict(collections.Counter); n = 0
for p in files:
    t = entity_lump(p)
    if not t: continue
    for e in ents(t):
        cn = e.get('classname','?')
        for k in KEYS:
            if k in e: use[k][cn] += 1; n += 1
print(f'files={len(files)} matches={n}')
for k in KEYS: print(f'== {k}: {dict(use[k].most_common(8))}')
PY

# input/output names live in values, so scan the raw BSP bytes instead
python3 - <<'PY'
import glob, os
MAPS = os.path.expanduser('~/.local/share/Steam/steamapps/common/Portal 2/portal2/maps')
keys = [b'Play Spawn Sound', b'playspawnsound', b'FadeAndKill', b'SetLightStyle',
        b'DestroyImmediately', b'StopPlayEndCap', b'GetSpeed', b'TeleportEntity',
        b'TeleportToCurrentPos', b'FadeReverse', b'BombType', b'usesizelimit', b'LowerWeapon']
files = sorted(glob.glob(os.path.join(MAPS, '*.bsp')))
for k in keys:
    print(f'{k.decode():22} maps={sum(1 for p in files if k in open(p,"rb").read())}')
PY

# binary spelling / existence check
python3 - <<'PY'
B = '/home/john/.local/share/Steam/steamapps/common/Portal 2/portal2/bin/linux32/'
for b in ('server.so','client.so'):
    d = open(B+b,'rb').read()
    for k in (b'BombType', b'usesizelimit', b'LowerWeapon', b'playspawnsound'):
        print(b, k.decode(), d.find(k))
PY
```

