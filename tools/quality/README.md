# Shared conformance runner (`tools/quality`)

This is the RFC 0005 **Q1** shared runner (AGENTS.md roadmap row **R02**): one
host-side orchestrator that builds, runs, and evidences every headless
conformance suite declared in the authoritative manifest. It is the connective
tissue for achieving parity across the RFCs — a domain adds **one manifest row**
when it lands a conformance suite, and every suite is then built with the correct
sources, run under a bounded timeout, classified by outcome, and recorded in
immutable evidence. No suite can silently drift out of the gate.

It is intentionally dependency-free (Python 3 standard library only), matching
the existing `tools/archlint` and `tools/stylelint` checkers.

## What a conformance suite is

Per the AGENTS.md "DRY and Liskov Substitution Principle" section: a conformance
suite is the **one shared behavioral oracle** for a replaceable contract. Each
suite is a standalone C++ program (its own `main`, exit 0 = pass) that encodes a
contract's clauses. Two `kind`s are recognized today:

- **`positive`** — asserts the real provider satisfies the contract.
- **`sensitivity`** — runs the contract predicate against a deliberately-broken
  provider and passes only when the oracle **detects** the violation, proving the
  suite is not vacuous (RFC 0005: "prove test sensitivity with negative fixtures").

## Authorities (no duplicated facts)

| Fact | Owner |
| --- | --- |
| Which suites exist, sources, expected outcome | `quality/conformance.manifest.json` (`conformance-manifest/v1`) |
| Toolchain, C++ standard, flags, include roots, timeout | `quality/profiles/<id>.json` (`conformance-profile/v1`) |
| Run results | generated `quality-results/*.json` (`conformance-evidence/v1`, git-ignored) |

The runner references profiles by id; it never copies toolchain facts. Domain
contract records and their suites stay with the domain (e.g.
`unittests/hammertest/`); the manifest only points at them.

## Commands

```sh
# List every declared suite.
python3 tools/quality/conformance.py list

# Build + run every suite, write evidence to quality-results/<timestamp>.json.
python3 tools/quality/conformance.py check

# Filter by suite id / domain / RFC / profile (selectors AND together).
python3 tools/quality/conformance.py check --rfc 0002 --domain Q-EDITOR
python3 tools/quality/conformance.py check --suite jobsystem.scheduler

# Pick the compiler; '-' skips writing evidence (useful in scripts/CI smoke).
python3 tools/quality/conformance.py check --cxx clang++ --out -
```

A `check` run **fails** (non-zero exit) on any of: a suite whose observed outcome
differs from its declared `expect`, zero suites discovered, an unmatched required
selector, a missing source file, a compile error, a crash (signal), a timeout, or
a reconciliation mismatch. This is the RFC 0005 runner contract.

## Evidence

Each `check` writes a `conformance-evidence/v1` JSON file identifying the source
revision and dirty digest, the compiler and its version, each profile's flags,
the expected vs. executed suite ids, per-suite outcome / exit code / signal /
duration / first divergence, and an exact rebuild command (`repro`) per suite.
Changing a contract, comparator, provider, or build setting invalidates prior
evidence — regenerate it.

## Self-tests (proving the runner is not vacuous)

The runner is tested with negative fixtures before it can be trusted as a gate
(RFC 0005: "Test the runner with fixtures that discover no tests, deliberately
fail, crash, hang, omit a result…"). The fixtures live in
`tools/quality/tests/fixtures/` and the suite drives every outcome class:

```sh
python3 -m unittest discover -s tools/quality/tests -v
```

## Adding a suite

1. Land the contract record and the standalone conformance `.cpp` in the domain
   (see `unittests/hammertest/` for the pattern; include a `sensitivity` suite
   with a deliberately-broken provider).
2. Add one row to `quality/conformance.manifest.json` (`id`, `domain`, `rfc`,
   `migration`, `contract`, `kind`, `profile`, `sources`, `expect: pass`; add
   `extra_flags` such as `-pthread` if needed).
3. Run `python3 tools/quality/conformance.py check --suite <id>` and confirm it
   passes under every required compiler.

## Scope / non-claims

This installs the shared runner and its negative self-tests. It does **not** by
itself certify any domain gate, establish native/GPU/device coverage, or wire a
required CI lane; those remain their own gates. See `RFC/0005-progress.md`.

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
