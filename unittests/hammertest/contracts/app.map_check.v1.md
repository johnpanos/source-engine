# Contract: `app.map_check.v1`

Module: `hammer.app`
Header: `public/hammer/app/map_check.h` · Impl: `hammer/core/app/map_check.cpp`
Conformance: `unittests/hammertest/app/test_map_check.cpp`
Migration: `R08-DOMAIN`
Depends on: `ports.entity_catalog`, `ports.material_info` (both optional),
`app.visgroup_ops`, `scene.map_queries`, `scene.solid_geometry`

## 1. Purpose, consumers, scope

Check for Problems (legacy `CMapCheckDlg`): `CheckMap` scans a document and
lists problems; `FixProblem` and `FixAll` are the exact fixes. Consumers: the
problems presenter, the command layer, and a pre-build check. Required.

## 2. Accepted inputs

Any `DocumentReader`, including documents that break `MapDocument::Validate`
(a loaded map may). The catalog and materials ports are optional; checks that
need them are skipped without them.

## 3. Results and guarantees

- Codes, severities and fixes are listed in the header: no player start
  (`info_player_*`), empty class, unknown class, empty brush entity, point
  class owning solids, keys and outputs naming no entity (special targets
  `!self !activator !caller !player !pvsplayer !speechtarget !picker` valid;
  `NameMatches` wildcards), undeclared outputs and inputs, keys outside the
  schema (allowlist `origin angles angle targetname spawnflags`;
  `multi_manager` skipped), repeated keys, open solids, duplicate or
  non-bounding sides, missing materials, duplicate side ids, duplicate VMF ids
  within a kind, undefined visgroup ids, objects hidden with no defined
  covering visgroup, empty groups, objects beyond +/-16384.
- Duplicate targetnames are legal in Source and not reported.
- One problem per object and code; order by code, then first object id, then
  message; map-wide problems first within their code.
- `FixProblem` re-derives the problem from the edit and changes only what it
  names; a problem that no longer applies is `Nothing`; codes without a fix
  are `Rejected` and stage nothing. `FixAll` repeats until no fixable problem
  remains (at most 32 passes); nothing fixable is `Nothing`.

## 4. Ownership, threading

Pure functions; no state; ports are borrowed for the call. Single sequence.

## 5. Invariants

Fixes are idempotent and leave an edit that passes `ValidateEdit`. After
`FixAll`, `CheckMap` reports no fixable problem.

## 6. Side effects and performance

`CheckMap` is O(entities x (keys + connections) x entities) for name lookups
and O(solids x sides^2) for plane checks; it allocates only its result.

## 7. Conformance suite and providers

`test_map_check.cpp` (179 checks, gcc and clang, `-Wall -Wextra -Werror`)
with the in-memory catalog and materials fakes: a clean map with every
accepted special case reports nothing; each of the 20 codes has a seeded
fixture on which exactly that code fires and a near-miss fixture on which it
does not; fixes apply, validate and are idempotent; unfixable codes refuse;
`FixAll` over all fixable seeds; ordering and determinism. Seeded checker
mutants (case-sensitive duplicate keys, `>=` bounds, specials not honored,
non-bounding sides or duplicate normals ignored, dangling ids counted as
coverage, unknown-class inputs judged, materials ignored) are each detected.
