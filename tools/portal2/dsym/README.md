# Portal 2 dSYM guided recovery

`ExportMissingSourceTree.java` is a Ghidra headless post script. It reads a
newline separated list of source paths, finds DWARF line mappings for those
paths, and writes one decompiler reference file per mapped path. Run it on a
Mach-O binary with its matching `.dSYM` directory beside it. Confirm matching
UUIDs with `llvm-dwarfdump --uuid` first.

The Steam2 depot `841` version `1` and depot `852` version `3` client/server
imports were verified with Ghidra 12.0.4. Their debug data required an 8 GiB
Java heap; the default 2 GiB heap ran out of memory. After importing and analyzing the binary, process the
saved Ghidra project with `-noanalysis -readOnly -scriptPath` pointing at this
directory and `-postScript ExportMissingSourceTree.java <list.txt> <output>`.
The script arguments are the selected source path list and output directory.

The generated files are pseudocode. They are useful for reconstructing
behavior and checking symbol identities, but must be reviewed and adapted
against this fork's headers, ABI, and gameplay tests before use in a build.

## Declaration skeletons

`dwarf_skeleton.py` reads the same dSYM DWARF directly (pyelftools; no Ghidra)
and, for each requested `.cpp` compile unit, writes declaration skeletons for
the unit and for every header it uses that is absent from this checkout:
classes in declaration-line order with i386 member offsets, networked members
spelled as their `CNetworkVar` macros, method signatures, enums, file-scope
variables, and every defined function with its original line, parameter
names, locals, lexical blocks and inlined callees.

    python3 tools/portal2/dsym/dwarf_skeleton.py \
        --dsym <bundle>.dSYM/Contents/Resources/DWARF/server.dylib \
        --label 'Steam2 depot 852_3 server.dylib.dSYM' \
        --out external/portal2_steam2_decompiled/skeleton/852_3/server \
        game/server/portal2/paint_sphere.cpp ...

The committed skeletons under `external/portal2_steam2_decompiled/skeleton`
were generated this way for every covered path in both depots (the
`dsym_*` columns of `coverage.csv`). Like the pseudocode, they are
reconstruction aids, not source. `llvm-dwarfdump --debug-info` crashes
partway through these files; per-name lookups (`--name=`) still work.

## What is left: function gap and retail lookups

`function_gap.py` lists, per compile unit of one or more dSYMs, the
functions whose names the source the build compiles for that unit (from
`compile_commands.json`) never defines or calls, and units with no file in
the checkout. The 2010 builds predate retail, so each name is a lead: many
were renamed, inlined, moved or cut before ship.

    R=<research dir>/852_3/portal2/bin
    python3 tools/portal2/dsym/function_gap.py --build build-p2 --filter portal \
        --dsym 852_3:server=$R/server.dylib.dSYM/Contents/Resources/DWARF/server.dylib \
        --dsym 852_3:client=$R/client.dylib.dSYM/Contents/Resources/DWARF/client.dylib

Ghidra post scripts for following a lead (run with `-noanalysis -readOnly`
on an analyzed project; the first argument is always the output file):

- `DecompByName.java <out> <name substring>...` decompiles matching named
  functions (dSYM builds).
- `Callers.java <out> <name substring>...` lists each match's callers.
- `StringUsers.java <out> <exact string>...` decompiles every function that
  references a C string. On the stripped retail Linux binaries this finds
  code by its ConVar, message, file or datadesc names.
- `VtableSlot.java <out> <mangled RTTI name> <byte offset>...` finds a
  class's primary vtable through its typeinfo and decompiles the given
  slots. A datadesc think or input member pointer holding an odd value v is
  virtual, at byte offset v - 1.
- `DerefDecomp.java <out> [*]<address>...` decompiles at an address, or at
  the pointer stored there when it is prefixed with `*`.

Retail datadescs are built by static initializers: `StringUsers` on the
class name finds the initializer that stores each field name, and the
typedescription's `inputFunc` is at +0x18 (stride 0x40). Ghidra loads the
retail server at image base 0x10000.
