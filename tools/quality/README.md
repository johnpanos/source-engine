# Shared conformance runner (`tools/quality`)

This is the RFC 0005 **Q1** shared runner (AGENTS.md roadmap row **R02**): one
host-side orchestrator that builds, runs, and evidences every headless
conformance suite declared in the authoritative manifest. It is the connective
tissue for achieving parity across the RFCs — a domain adds **one manifest row**
when it lands a conformance suite, and every suite is then built with the correct
sources, run under bounded time and memory, classified by outcome, and recorded
in immutable evidence with full logs. No suite can silently drift out of the gate.

It is intentionally dependency-free (Python 3 standard library only), matching
the existing `tools/archlint` and `tools/stylelint` checkers.

## What a conformance suite is

Per the AGENTS.md "DRY and Liskov Substitution Principle" section: a conformance
suite is the **one shared behavioral oracle** for a replaceable contract. Each
suite is a standalone C++ program with its own `main` that encodes a contract's
clauses. Two `kind`s are recognized today:

- **`positive`** — asserts the real provider satisfies the contract.
- **`sensitivity`** — runs the contract predicate against a deliberately-broken
  provider and passes only when the oracle **detects** the violation, proving the
  suite is not vacuous (RFC 0005: "prove test sensitivity with negative fixtures").

### Result protocol (`checks-v1`)

An exit status alone cannot tell a complete run from one that returned before
its checks. Every suite whose manifest row expects `pass` therefore declares
`"result_protocol": "checks-v1"` (the manifest loader rejects one that does not)
and ends `main` with:

```cpp
#include "testing/conformance_result.h"
...
return testing::ReportConformance( checks, failures );
```

The header prints exactly one `CONFORMANCE <checks> <failures>` line and
returns the matching exit status. The runner accepts the run only when the
process exits 0 and stdout carries exactly one record with `checks > 0` and
`failures == 0` (and at least the row's optional `min_checks`). A missing
record (an early return or incomplete output), a duplicate record, zero checks
or a nonzero failure count all fail, even with exit status 0. Count every
evaluated check, including each detection condition of a sensitivity suite.
Never build checks on `assert()`: `NDEBUG` removes them, so the runner refuses
to build a suite whose `unittests/` sources call it (`invalid-oracle`).

Negative fixtures that expect `fail`, `crash`, `timeout`, `compile-error` or
`missing-source` need no record. `expected_signal` and `expected_diagnostic`
pin them to their specific defect.

## Authorities (no duplicated facts)

| Fact | Owner |
| --- | --- |
| Which suites exist, sources, expected outcome, required providers | `quality/conformance.manifest.json` (`conformance-manifest/v1`) |
| Toolchain, C++ standard, flags, include roots, timeout, memory cap | `quality/profiles/<id>.json` (`conformance-profile/v1`) |
| Result record format | `public/testing/conformance_result.h` + `classify_run()` in `conformance.py` |
| Run results | generated `quality-results/*.json` (`conformance-evidence/v2`, git-ignored) and the `<name>.logs/` directory beside it |

The runner references profiles by id; it never copies toolchain facts. Domain
contract records and their suites stay with the domain (e.g.
`unittests/hammertest/`); the manifest only points at them. `parity_wine.py`
runs the same manifest rows under Wine through the same `classify_run()`.

## Commands

```sh
# List every declared suite.
python3 tools/quality/conformance.py list

# Enumerate the selected suites, their profiles and provider availability
# before anything is built or run.
python3 tools/quality/conformance.py plan --domain Q-JOBS

# Build + run every suite; evidence goes to quality-results/conformance.<stamp>.json
# and full per-suite build/run logs to quality-results/conformance.<stamp>.logs/.
python3 tools/quality/conformance.py check

# Filter by suite id / domain / RFC / profile. Each selector field ANDs; every
# requested value must match at least one suite.
python3 tools/quality/conformance.py check --rfc 0002 --domain Q-EDITOR
python3 tools/quality/conformance.py check --suite jobsystem.scheduler

# Compiler, optimized NDEBUG build, repeats and seed.
python3 tools/quality/conformance.py check --cxx clang++ --config release
python3 tools/quality/conformance.py check --suite jobsystem.executorstress --repeat 5 --seed 7

# '-' skips the evidence file (logs stay under --build-dir); for scripts.
python3 tools/quality/conformance.py check --cxx clang++ --out -
```

A `check` run **fails** (non-zero exit) on any of the following:
- a suite whose observed outcome differs from its declared `expect` on any attempt
- zero suites discovered, or any requested selector value that matches nothing
- an unavailable compiler, or an unavailable provider named by a required suite's `requires`
- a missing source file, a compile error, a crash (signal) or a timeout
- a missing, duplicate, zero-check or failing result record
- an `assert()`-based test source
- a reconciliation mismatch between the enumerated and executed suites

This is the RFC 0005 runner contract. Exit status 2 marks a structural or
selection error; exit status 1 marks a failing run.

### Providers, skips and bounds

- `requires: ["executable:<name>", "env:<VAR>", "path:<repo-relative or absolute>"]`
  names the providers a suite needs. A required suite with a missing provider
  fails as `unavailable-provider`. A suite with `optional: true` is `skipped`
  instead. The skip reason is recorded, the run can still pass, and the suite is
  counted in `counts.skipped`. It is never counted as certified. An optional row
  must name the provider it may be skipped for.
- Each suite process runs in its own session with the profile's
  `timeout_seconds` and `memory_limit_mb` address-space cap (default 4096 MiB,
  not applied to sanitizer builds) and no core dumps. On timeout the whole
  process group is killed, and the output written so far is kept in the log.
- `--repeat N` runs each suite N times and keeps every attempt. The suite
  matches only if all attempts match. There is no retry-until-green mode.
  `CONFORMANCE_SEED`, `CONFORMANCE_ATTEMPT` and `CONFORMANCE_SUITE` are
  exported to every suite process.

## Evidence

Each `check` writes `conformance-evidence/v2` before the first suite and after
every suite. A runner that is interrupted therefore leaves `"decision":
"incomplete"`, never a pass. The final file identifies:
- **Source and host:** the source revision, dirty digest, submodule revisions,
  host OS/architecture and Python version.
- **Compiler:** its path, version and target triple.
- **Run parameters:** each profile's flags, the exact invocation, selectors,
  build configuration, seed and repeat count.
- **Suites:** the expected vs. executed suite ids and the manifest digest.
- **Per suite:** outcome, exit code, signal, check counts, duration, first
  divergence, an input digest of its declared sources and contract, an exact
  rebuild command (`repro`), every attempt, and the build/run log paths.

Changing a contract, comparator, provider or build setting invalidates prior
evidence, so regenerate it.

## Self-tests (proving the runner is not vacuous)

The runner is tested with negative fixtures before it can be trusted as a gate
(RFC 0005: "Test the runner with fixtures that discover no tests, deliberately
fail, crash, hang, omit a result, and request an unavailable required
provider"). The fixtures live in `tools/quality/tests/fixtures/`. The tests
cover:
- **Process outcomes:** pass, fail, crash, a hang (including a forked child that
  must not outlive the timeout) and a memory hog stopped by the cap.
- **Result records:** missing, duplicate and zero-check records, and `min_checks`.
- **Build and oracle problems:** `assert()`-only suites, a counted failure in the
  default and release configs, compile errors and missing sources.
- **Providers:** required and optional unavailable providers, and an
  unavailable compiler.
- **Selection:** zero discovery and partially unmatched selectors.
- **Runner behaviour:** seed/attempt export, retained intermittent failures,
  interrupted runs and evidence completeness.

```sh
python3 -m unittest tools/quality/tests/test_conformance.py -v
```

## Adding a suite

1. Land the contract record and the standalone conformance `.cpp` in the domain
   (see `unittests/hammertest/` for the pattern; include a `sensitivity` suite
   with a deliberately-broken provider). End `main` with
   `testing::ReportConformance( checks, failures )`.
2. Add one row to `quality/conformance.manifest.json` (`id`, `domain`, `rfc`,
   `migration`, `contract`, `kind`, `profile`, `result_protocol: checks-v1`,
   `sources`, `expect: pass`; add `extra_flags` such as `-pthread`, and
   `requires`/`optional` when the suite needs a host provider).
3. Run `python3 tools/quality/conformance.py check --suite <id>` under g++ and
   clang++, in both the default and `--config release` configurations.

## CI

[`.github/workflows/conformance.yml`](../../.github/workflows/conformance.yml)
first runs the runner self-tests. It then runs every manifest suite under
`{g++, clang++} × {default, release}` on `ubuntu-24.04` and uploads each job's
evidence and logs, even when a job fails. Making the check *required* is
repository-administrator policy.

## Scope / non-claims

This installs the shared runner, its result protocol and its negative
self-tests. It does **not** by itself certify any domain gate or establish
native/GPU/device coverage. Those remain their own gates. See
`RFC/0005-progress.md`.

## Baseline and profile inventory (R01 / RFC 0005 Q0)

`baseline.py` reproduces the repository baseline from one declaration,
[`quality/baseline.json`](../../quality/baseline.json) (`source-baseline/v1`):
the host tools and SDKs each profile needs, external content corpora, the
support matrix of declared product/OS/architecture profiles, every installed
check/build/run command with the outcome recorded at this revision, and the
identified baseline captures and budgets per harness domain. Profile facts stay
in the profile files it links; the tool reads their evidence status.

```sh
python3 tools/quality/baseline.py validate     # schema, references, linked records
python3 tools/quality/baseline.py probe        # tool and content availability here
python3 tools/quality/baseline.py audit        # probe + run every check, write evidence
python3 tools/quality/baseline.py audit --group static --group suites
python3 -m unittest tools/quality/tests/test_baseline.py -v
```

`audit` writes `baseline-evidence/v1` and per-check logs under
`quality-results/baseline/<timestamp>/`. It fails when a check's outcome
differs from its recorded baseline: a regression, a changed failure mode, or a
known failure that now passes (update the declaration after review; never to get
green). A check whose tool, content or build tree is missing is `unavailable`:
reported, never counted as a pass, and a failure under `--strict`. Groups:
`static`, `suites`, `build` (incremental Waf builds under each tree's private
`WAFLOCK`; `setup` records the configure line), `runtime` and `package`.
The current results and their owners are recorded in
[RFC 0005 progress](../../RFC/0005-progress.md#q0--r01-baseline-and-profile-inventory).

## BSP2 F1 content and native checks

`bsp2_f1_gate.py` runs the pinned Portal v20 and Portal 2 v21 corpora,
client/server VBSP-versus-BSP2 collision and visibility comparisons, a
SHA-pinned sparse file inspection/export above 4 GiB, and a clearly labeled
synthetic v19-header route check. It requires licensed content,
built Portal client and dedicated products, and `bsp2tool`. Missing inputs,
missing or changed maps, incomplete probes, failed launches and absent players
fail with evidence. Generated v19 and sparse Portal map copies are removed after
their checks.

```sh
python3 tools/quality/bsp2_f1_gate.py \
  --portal-runtime <portal-runtime> --portal2-runtime <portal2-runtime> \
  --dedicated-build <dedicated-build> --client-build <client-build> \
  --tool <bsp2tool> --out <fresh-evidence-directory>
```

The pinned map inventories are
[`bsp2-corpus-v20.json`](../../quality/fixtures/bsp2-corpus-v20.json) and
[`bsp2-corpus-v21.json`](../../quality/fixtures/bsp2-corpus-v21.json). The
[RFC 0008 progress record](../../RFC/0008-progress.md) gives the exact local
build commands, outcomes and remaining F1 gates. This command checks the Linux
content/native slice; it does not establish Portal 2 gameplay support or the
rest of RFC 0008.

## Native Portal product and render diagnostics

The [Portal profile](../../quality/product_profiles/README.md) adds an isolated
Waf/native runner alongside the headless suite runner. `portal_boot.py` stages
licensed content privately, verifies native dependency and shader artifacts,
requires actual SDL3/Wayland/Vulkan markers and mapped libraries, checks an active
map/player and fresh scene capture, and records clean shutdown. `--render-trace`
also requires a complete trace with no observed shader, draw or presentation
failures. A file's mere existence cannot satisfy that gate.

[Shader artifact tooling](shader_artifacts.md) compiles the source permutation
schema from a versioned workload. [Render diagnostics](render_trace.md) retain
per-material draw state and requested-versus-bound shader identities, with
independent analyzer and seeded-failure fixtures. Neither source-matched bytecode
nor successful GPU submission replaces native visual acceptance.

## Frame pacing (native Vulkan)

`frame_pacing.py` measures per-frame cost of a scripted Portal workload
(`quality/workloads/portal-frame-pacing-v1.json`) on the native Vulkan backend,
headless, and attributes hitches to named backend operations. Keep `--out`
short (the engine rejects command lines over 512 characters):

```sh
python3 tools/quality/frame_pacing.py --runtime <portal-runtime> --build build \
  --out /tmp/fp/run1 --cold-shader-cache
python3 tools/quality/frame_pacing.py --runtime <portal-runtime> --build <tree-a> \
  --ab-build <tree-b> --rounds 3 --out /tmp/fp/ab
python3 -m unittest tools/quality/tests/test_frame_pacing.py
```

Evidence and findings: `RFC/0001-native-vulkan-frame-pacing-progress.md`.
