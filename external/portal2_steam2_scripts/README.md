# Portal 2 Steam2 script references

These are historical Portal 2 VScript (`.nut`), UI Lua, tool Python, and
`portal2.fgd` sources extracted from Steam2 depot `852` version `3` and depot
`841` versions `1`, `8`, `22`, and `24`. The version 1 blob CRC is `8f160ceb`.
Directory names preserve the original depot paths. The extract contains
143 files: 57 VScript files, 23 Lua files, one Python utility, and two FGDs
from `841_1`; 52 VScript files and one FGD from `852_3`; and seven distinct
later VScript/FGD variants. Every copied file was verified byte for byte
against the extracted depot.

The 841 version 8, 22, and 24 manifests contain 104, 106, and 107
VScript/FGD files. Seven unique path/content variants differ from the
installed retail files and are preserved here. Identical later variants
are represented once. The 14 UI Lua files in those versions and depot 852
version 3 were byte-identical to the version 1 UI Lua files, so they are
also retained only once.
The aggregate SHA-256 over sorted relative paths, NUL separators, and the
raw SHA-256 digest of each file is
`fd63e6b45ceecf832d72de2ee85265df817cc65ecfc08b2674cb9f32e8d07f5f`.

These files are reference versions and are not selected by `./play_p2`.
Some are byte-identical to files in the installed retail game; others differ
and may document earlier behavior. The installed Portal 2 content remains
the runtime authority. The repository's provenance and distribution warning
applies to this material.

Source metadata: [Steam2 depot 841](https://femtendo.github.io/steam2-catalog/#depot-841)
and [depot 852](https://femtendo.github.io/steam2-catalog/#depot-852).
