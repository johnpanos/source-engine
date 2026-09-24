# RFC 0011 progress: Runtime indirect lighting

Updated: 2026-09-24
Rows: RFC 0011 proposes R70–R78 (G0–G8). They are not yet ranked in
AGENTS.md. On 2026-09-24 the user directed implementation of G0 through G8 in
order; this file is the gate-decision record for that work.

Where this file disagrees with the versioned artifacts, the artifacts win:

- the conformance manifest rows named below;
- the fixture declarations under `quality/fixtures/gi/`;
- local evidence under `quality-results/rfc0011-*` (not versioned; the
  reproduction commands regenerate it).

## Gate status

| Gate | State | Summary |
| --- | --- | --- |
| G0 baseline, fixtures and runner | active | GPU runner class and profile done; fixtures, references, debug view, budgets and the leaf-ambient "before" capture in progress |
| G1 probe volume, baked producer | planned | — |
| G2 light set, separated bake, policy | planned | — |
| G3 producer contract and switching | planned | — |
| G4 precomputed radiosity | planned | — |
| G5 GPU compute foundation | planned | — |
| G6 SDF-traced producer | planned | — |
| G7 ray-query producer | planned | — |
| G8 product defaults and soak | planned | — |

## G0: Baseline, fixtures and runner

### G0.2 GPU runner profile (done 2026-09-24)

The conformance runner has a second runner class. A profile declares
`runner` (`headless`, the default, or `gpu`). An unselected
`conformance.py check` still runs only the headless class, so a GPU-less CI
host keeps its gate. `--runner gpu` (or naming a suite, domain, RFC or profile)
selects GPU suites. A class with no suites is a fatal selection error, like
any unmatched selector.

Profiles may now also declare:

- `link_flags`, appended to every suite's link step;
- `requires`, providers applied to every suite in the profile;
- `run_env` and `run_env_unset`, the environment suites and provider probes
  run under.

A new provider kind, `vulkan-device:gpu`, parses `vulkaninfo --summary` and
needs an integrated or discrete GPU; llvmpipe does not satisfy it. The
evidence records every device vulkaninfo reported under the profile's
environment. A negative row may name the defect it must be rejected for with
`expected_divergence`.

[`linux-native-vulkan-gpu`](../quality/profiles/linux-native-vulkan-gpu.json)
compiles the native device core with each suite under C++20 `-Werror`, links
the system SDL3 and Vulkan loader, and renders headless through SDL's
offscreen driver (`VK_EXT_headless_surface`), so no window reaches a desktop.
The existing standalone programs now run as manifest rows with `checks-v1`
records:

| Row | Checks |
| --- | --- |
| `render.world-pbr.native-pixels` | 53 |
| `render.model-pbr.native-pixels` | 34 |
| `render.pbr-direct.native-pixels` | 31 (converted to `checks-v1`) |
| `render.world-glass.native-pixels` | 36 |
| `render.gpu-runner.zero-checks` (sensitivity) | renders one frame, reports zero checks and exits 0; must fail with `zero checks executed` |

Evidence: `python3 tools/quality/conformance.py check --runner gpu` passed
5/5 on the Radeon 8060S (RADV, Mesa 26.2.2), evidence
`quality-results/conformance.20260924T214756Z.json`. The runner self-tests
(`python3 -m unittest tools/quality/tests/test_conformance.py`) pass 55 tests,
including new ones for class selection, profile environment, link flags,
providers, vulkaninfo parsing and `expected_divergence`. Two
enum/`sizeof` conditional expressions in `vulkan_device.cpp` were cast so the
core builds under `-Werror`.

Not covered: an Android GPU runner profile (the Fold7 runs through the APK
harnesses) and a hosted CI lane with a GPU.
