# RFC 0001 Phase A progress

Updated: 2026-09-21

Phase A covers the dynamic-module-loader retirement inventory and freeze. This
file is the durable progress record; detailed current sites live in
`architecture/loader_inventory.json` and the enforcement snapshot lives in
`architecture/baseline.json`.

## Status

| Work item | Status | Evidence |
| --- | --- | --- |
| Classify every static load site | Current | 326 loader-surface records in `architecture/loader_inventory.json`: 120 first-party composition, 21 retained extension, 22 optional provider, 142 tool indirection, and 21 native symbol probe |
| Reject new `Sys_LoadModule`, `Sys_GetFactory`, filesystem `LoadModule`, and `CDllDemandLoader` dependencies | Current | `ARCH101`–`ARCH104`; `check --all` and `baseline --verify` pass against the reviewed exact fingerprints |
| Prohibit new `CreateInterfaceFn` boundaries outside the legacy ABI package | Complete | `ARCH105`; legacy paths declared in `architecture/modules.json` |
| Run the freeze in CI | Complete | Architecture job in `.github/workflows/tests.yml` |
| Instrument load attempts and unloads with requester, resolved path, entry/interface, result, and lifetime | Tier 1 and filesystem-mediated paths verified; direct native paths remain | `tier1/module_load_telemetry.cpp`, scoped request API, loader adapters, and seven fixture cases cover success/failure, entry/interface lookup, rollback, lifetime, duplicate handles, nested/concurrent context, and reentrant sink removal; the 86 direct native-loader records still need runtime routing |

Assessment at source revision `a4f6f95f` plus the recorded Phase A changes: all
26 architecture-checker tests pass; `check --all`, `baseline --verify`, and
`inventory --verify` pass. A release game composition completed all 2,203 build
tasks. The isolated Tier 1 test module passes with `-moduleloadtelemetry` and
emits correlated provider results, resolved paths, entry/interface requests,
load IDs, unload results, and lifetimes. The full legacy unit-test composition
is not used as evidence because its pre-existing `TSList`/`TSQueue` stress tests
crash before a complete run. Phase A remains open until direct native-loader
paths and frozen-ABI coverage meet the same runtime gate.

Shared runner/evidence and loader tests are specified by
[RFC 0005](0005-quality-and-correctness-harnesses.md); implementation work is
tracked as R01/R07 in [AGENTS.md](../AGENTS.md).

## Commands

```text
python3 tools/archlint/archlint.py check --changed
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
python3 -m unittest discover -s tools/archlint/tests -v
```

Baseline or inventory rewrites are intentional review actions. After reviewing
the diff and its classification, use the corresponding `--write` command.

## Next increment

Route the 86 recorded direct native-loader sites through telemetry without
introducing a Tier 0 to Tier 1 dependency, including the standalone launchers.
Add frozen-header ABI fixtures and process-shutdown reporting for intentionally
retained handles. Re-run the isolated fixture, product build, and architecture
gates after that routing. Source presence alone does not establish ABI or
behavioral compatibility.
