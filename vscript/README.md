# vscript — Squirrel VScript provider

Provenance: `vscript.cpp` and everything under `languages/squirrel/` are
imported from the leaked 2017 CS:GO source tree (`cstrike15_src/vscript`,
from the "April 22nd 2020" archive). They are not original work of this
repository. The repository's provenance and distribution warning applies to
this directory; do not redistribute it on the assumption that it is freely
licensed. Squirrel 2.1 and SqPlus themselves carry the zlib-style notice in
`languages/squirrel/COPYRIGHT`; Valve's glue (`vscript.cpp`,
`languages/squirrel/vsquirrel/`) does not.

Imported subset (the POSIX file list of the CS:GO `vscript.vpc`, kept here for
reference): Squirrel core, sqstdlib (aux/blob/math/rex/stream/string), SqPlus
(without `SquirrelBindingsUtilsWin32`), the remote debugger (`sqdbg`), and
`vsquirrel` (the `IScriptVM` implementation). GameMonkey, Lua and Python are
not imported.

Local changes, marked in the sources:

- `vscript.cpp` includes only the Squirrel language header; the Lua branch is
  behind `VLUA_ENABLED` instead of a platform test.

`init.nut` is embedded at build time as `init_nut.h` (`g_Script_init`) by
`wscript`; the same bytes the original `texttoarray.pl` step produced.

The module exports `VScriptManager009` (`public/vscript/ivscript.h`, which is
identical to the CS:GO header). It is built only for the Portal 2 game build
(`--build-games=portal2`); the Portal 2 client and server load it when the
engine's app-system factory does not provide the interface.
