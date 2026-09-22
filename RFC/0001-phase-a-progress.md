# RFC 0001 Phase A progress

Updated: 2026-09-21

Phase A covers the dynamic-module-loader retirement inventory and freeze. This
file is the durable progress record; detailed current sites live in
`architecture/loader_inventory.json` and the enforcement snapshot lives in
`architecture/baseline.json`.

## Status

| Work item | Status | Evidence |
| --- | --- | --- |
| Classify every static load site | Initial snapshot complete; current inventory stale | Initial 301 loader-surface records in `architecture/loader_inventory.json`; telemetry changes need reviewed reclassification/verification |
| Reject new `Sys_LoadModule`, `Sys_GetFactory`, filesystem `LoadModule`, and `CDllDemandLoader` dependencies | Checker installed; current tree fails | `ARCH101`–`ARCH104`; initial 683 exact fingerprints including factory-type freeze; changes are not automatically grandfathered |
| Prohibit new `CreateInterfaceFn` boundaries outside the legacy ABI package | Complete | `ARCH105`; legacy paths declared in `architecture/modules.json` |
| Run the freeze in CI | Complete | Architecture job in `.github/workflows/tests.yml` |
| Instrument load attempts and unloads with requester, resolved path, entry/interface, result, and lifetime | Source implementation present; acceptance incomplete | `tier1/module_load_telemetry.cpp`, scoped request API, and loader adapters exist; conformance, ABI and lifecycle gates remain unverified |

Assessment at source revision `87955f67`: eight checker fixture tests pass;
`check --all` and `baseline --verify` report 10 new and 3 stale occurrences;
`inventory --verify` reports stale. These are recorded observations, not accepted
baseline changes. The Phase A gate is not complete.

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

Validate the telemetry sink, scoped request context, and loader adapters using
Q-FOUNDATION fixtures. Verify preserved ABI calls, requester/path/interface
observations, provider results, and unload ordering. Cover failed loads/lookups,
duplicate native handles, nested/concurrent requests, sink lifetime/reentrancy,
and shutdown after partial startup. Resolve every new/stale architecture
occurrence through reviewed classification or implementation correction, then
verify the inventory/baseline. Source presence alone does not establish ABI or
behavioral compatibility.
