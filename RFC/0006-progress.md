# RFC 0006 progress: per-target toolchain boundary

Updated: 2026-09-22
Portfolio row: R03 (M0), active.

## Bounded scope

Make the declared C++20 dialect the normal in-tree Waf build setting. Keep the
strict first-party targets separately selected, preserve Box3D's C17 flags and
the C++11 frozen external-consumer fixtures, and validate the final commands and
mixed-target ABI calls. The root build no longer accepts a private environment
override that silently puts unmigrated targets back on C++11.

## Current evidence and remaining gate

The installed Waf dialect tool and `quality/toolchain/policy.json` declare and
check the target settings. R01 captured ten Linux build trees and 18 successful
compiler probes under the C++20 opt-in; this established feasibility but did
not make normal builds use the declared policy. R03 remains active until fresh
normal-build evidence covers the installed product matrix, the final flag and
frozen-consumer checks pass, and supported non-Linux toolchains are assessed
against their declared profiles. Unavailable Apple runners remain explicit
verification gaps, not inferred support.
