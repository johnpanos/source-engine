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
