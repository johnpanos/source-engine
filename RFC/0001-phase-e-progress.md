# RFC 0001 Phase E progress

Updated: 2026-09-25

Phase E replaces first-party launchable-DLL tool indirection with normal
executables or directly linked tool libraries. A retained process boundary must
preserve structured arguments, working directory, environment, output, exit
status, cancellation acknowledgement, and child cleanup. Perforce, database,
and proprietary integrations belong only to explicit tool products.

This record is a gate decision log, not a completion claim. R40 is active while
its hard prerequisites R11, R12, and R22 remain open. Bounded cohorts may land
ahead of those gates, but Phase E cannot close until the prerequisites and every
declared compiler workflow have their required evidence.

## Audited starting point

The initial audit found five launchable implementations, six filename-loading
wrappers, and the VMPI in-process debug path:

| Cohort | Legacy indirection | Required disposition | State |
| --- | --- | --- | --- |
| VTEX / vtexconv | Two wrappers load `vtex_dll` and request `ILaunchableDLL` | Directly linked executable entry; preserve `IVTex` temporarily for existing engine callers | active |
| VVIS | Wrapper and VMPI debug mode load `vvis_dll` | Normal executable; VMPI always uses its process path | active |
| VRAD | Wrapper loads `vrad_dll`; redirect and `-both` select/run variants | Normal executable plus process routing where isolation/version selection is required; preserve Hammer `IVRadDLL` until R22 migrates it | planned |
| shadercompile | Wrapper loads `shadercompile_dll` | Normal executable; preserve required remote compiler behavior through an explicit process workflow | active |
| texturecompile | Wrapper loads `texturecompile_dll` | Normal executable | active |
| StudioMDL / tool dictionaries | Generic app-system/module composition | Inventory named callers and migrate only after workflow fixtures exist | planned |
| Perforce / SQL providers | Generic loader use, including P4 in staging runtime code | Tool-only linked provider or named optional tool host; zero runtime-product dependencies | planned |

The State column follows the authoritative ledger,
[`architecture/tool_migrations.json`](../architecture/tool_migrations.json), as of
2026-09-25. The ledger also tracks `phase-e-vmpi-worker` (active),
`phase-e-tool-dictionaries` (planned) and `phase-e-external-vpc-binlaunch`
(blocked on an external VPC decision). No cohort is `verified` or `retired`,
and no cohort records acceptance evidence yet.

The Phase A loader inventory is a freeze, not a Phase E oracle: it classifies
tool sites broadly and does not reject `ILaunchableDLL`, app-system member loads,
or tool integrations leaking into runtime products. Phase E therefore owns a
separate reviewed migration ledger and strict retirement checks; it does not
silently reinterpret or regenerate the Phase A baseline.

## Active implementation slices

### Structured process contract

A strict, standard-library-only contract and shared conformance suite are being
installed before process callers migrate. The suite covers exact argument
boundaries (including empty and whitespace-containing arguments), working
directory, environment overrides, separate stdout/stderr, exit status, spawn
failure, timeout, acknowledged cancellation, and cleanup. Deliberately broken
providers must prove each clause is observable. Native providers and installed
tool workflows remain separate evidence.

Update (2026-09-25): the contract is installed as `platform.tool_process.v1`
(`public/platform/contracts/tool_process.h`, `platform::ToolProcessClient`).
`platform.tool_process` (49 checks) and its sensitivity suite (11 checks) pass
against the test backend in the 2026-09-23 Q-FOUNDATION run. A POSIX provider,
`platform/posix/tool_process_provider.cpp`, is in the tree, but no Waf target
builds it and no suite runs it. No tool caller uses the client yet.

### VTEX direct entry

`vtex_dll` retains `IVTex` only for current compatibility callers. The command
line path now exports a directly linked `VTex_RunCommandLine` function, and both
tool projects use a normal `main` that calls that symbol. The old wrappers and
their filename/interface lookup are retirement targets in this slice. A
separate host-tools Waf composition and fixture workflow are required before
this cohort is complete.

Update (2026-09-25): the wrappers and `public/ilaunchabledll.h` are deleted
(`66e2a40c`). The isolated `./waf configure --tools` product builds `vtex` and
`vtexconv`, and R01 records `build.tools` passing with gcc and clang. No
installed VTEX workflow or `IVTex` caller inventory is recorded.

### VVIS and VMPI

VVIS already contains its real executable `main`; its DLL adapter and wrapper
are being removed. VMPI's ordinary remote-worker path already creates a child
process from an argument vector and supports termination. Its optional
`-TryDLLMode` path is the in-process exception being retired; the remaining
process implementation still has to satisfy the new protocol tests.

Update (2026-09-25): the `utils/vvis_launcher` sources and `-TryDLLMode` are
gone. On Linux the `--tools` product builds VVIS and VRAD as normal
executables, and `vvis_host_smoke.py` and `vrad_host_smoke.py` pass on a
synthetic room (see
[the R48 record](0007-progress.md#r48-host-compiler-preparation-2026-09-23)).
Linux rejects `-mpi`. These synthetic smokes are not the installed-workflow,
legacy-output or process-protocol evidence the ledger requires. shadercompile and texturecompile now have normal
`main` entries but only VPC projects, no Waf target.

## Required evidence before closure

- R11, R12, and R22 are complete and their evidence is current.
- The Phase E migration ledger has no incomplete launchable-wrapper cohort and
  architecture lint rejects seeded wrapper/runtime-integration regressions.
- Host-tool profiles build into isolated outputs and do not enter client or
  dedicated runtime compositions.
- Native process providers pass the shared suite on every declared host profile.
- Installed VRAD, VVIS, VTEX/texturecompile, StudioMDL, and shadercompile
  workflows preserve argv, environment, working directory, exit status, output,
  cancellation, and their declared content semantics.
- Product link/load evidence demonstrates that Perforce, SQL/database, and
  proprietary tool integrations are absent from runtime engine compositions.
- Reviewed Phase A baseline/inventory reductions and exact reproduction
  commands are recorded; missing runners or fixtures remain unverified rather
  than being counted as passes.

## Current verification notes

At the audit start, architecture and checker unit tests passed, while
`inventory --verify` exposed committed native-telemetry drift in
`video/videoservices.cpp`. The missing adapter include has been restored without
blindly rewriting the inventory. Broader inventory staleness from independently
reviewed loader removals still requires a separate classification review.

The existing Waf game build is not a Phase E tools profile. A diagnostic
`vtex_dll` build reached unrelated pre-existing `vstdlib/jobthread.cpp` compile
errors before the tool link; the edited `vtex.cpp` translation unit itself
compiled using the generated command. This is not installed-tool evidence.

On 2026-09-25, `python3 tools/archlint/archlint.py tools --verify` reports the
ledger and launchable-DLL ratchet valid: 10 cohorts, no first-party wrapper
sites, and 2 preserved occurrences in the vendored
`external/vpc/public/ilaunchabledll.h`. The general architecture check is red
for unrelated reasons; see the [Phase A record](0001-phase-a-progress.md).

## Rollback

Each cohort remains independently revertible until its workflow passes. Do not
restore generic runtime discovery as rollback: restore the previous named tool
product or direct-library composition for that cohort. The preserved `IVTex`
and `IVRadDLL` compatibility APIs are not permission to reintroduce
`ILaunchableDLL` discovery.
