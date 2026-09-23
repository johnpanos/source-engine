# Portal 2 shared source import

This directory contains the 70 `.cpp` and `.h` files moved from the local
`portal` source drop. Their bytes match the corresponding files in the local
`cstrike15_src/game/shared/portal` archive. The files retain their original
copyright headers and line endings. The aggregate SHA-256 of sorted file names,
NUL separators, and each file's SHA-256 digest is
`efd90b1a1bf85ff619401357da49b1266853e898e429eb83c827ffab32259b6f`.

This is the shared-code cohort for a **separate Portal 2 client/server target**.
The existing `portal` target selects `game/shared/portal` explicitly through
`game/client/client_portal.vpc` and `game/server/server_portal.vpc`. Its source
selection must remain independent of these files.

The import is not yet a buildable Portal 2 target. A scan of the imported
`#include` directives found 52 header names absent from this checkout and the
source drop, including `paint_enum.h`, `c_paint_stream.h`,
`portal_grabcontroller_shared.h`, and `prop_weightedcube.h`. The client/server
Portal 2 code, project source lists, engine API adaptations, content/runtime
profile, and game-specific conformance runs must be added and verified before
Waf can offer `--build-games=portal2`. Do not add these files to the Portal 1
VPC lists or label the source import a working Portal 2 build.

The repository's provenance and distribution warning in the root README also
applies to this import.
