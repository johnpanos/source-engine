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
