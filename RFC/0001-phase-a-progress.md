# RFC 0001 Phase A progress

Updated: 2026-09-21

Phase A covers the dynamic-module-loader retirement inventory and freeze. This
file is the durable progress record; detailed current sites live in
`architecture/loader_inventory.json` and the enforcement snapshot lives in
`architecture/baseline.json`.

## Status

| Work item | Status | Evidence |
| --- | --- | --- |
| Classify every static load site | Complete for the initial tree | 301 loader-surface records in `architecture/loader_inventory.json`; verified by `archlint inventory --verify` |
| Reject new `Sys_LoadModule`, `Sys_GetFactory`, filesystem `LoadModule`, and `CDllDemandLoader` dependencies | Complete | `ARCH101`–`ARCH104`; 683 exact legacy fingerprints including the factory-type freeze in `architecture/baseline.json` |
| Prohibit new `CreateInterfaceFn` boundaries outside the legacy ABI package | Complete | `ARCH105`; legacy paths declared in `architecture/modules.json` |
| Run the freeze in CI | Complete | Architecture job in `.github/workflows/tests.yml` |
| Instrument load attempts and unloads with requester, resolved path, entry/interface, result, and lifetime | Not started | Requires an ABI-preserving telemetry context around Tier 1 and filesystem-mediated loads |

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

Add a structured loader telemetry sink and a scoped request context. The public
legacy symbols remain ABI-compatible; source call sites supply requester data,
filesystem resolution reports the final path, interface lookup reports the
entry point or version, and unload records elapsed lifetime plus provider
result. Tests must cover failed loads, failed lookups, duplicate native handles,
and shutdown after partial startup.
