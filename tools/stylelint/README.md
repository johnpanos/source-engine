# Source style checker

This is the installed mechanical-style slice of RFC 0006 / roadmap R04.
It is read-only: checks print diagnostics (and optionally a proposed diff), never
rewrite sources or update a baseline. It does not prove C++ correctness, ownership,
DRY, Liskov substitution, lock freedom, or CPU/GPU fence behavior. Those require
the contract review and harnesses specified in RFCs 0005 and 0006.

## Setup and commands

Python 3.10+ and Git are required. Install the pinned formatter in an isolated
environment, or use an existing binary with exactly the version in
`requirements.txt` (currently 22.1.8):

```sh
python3 -m venv build/stylelint-venv
build/stylelint-venv/bin/python -m pip install -r tools/stylelint/requirements.txt
export CLANG_FORMAT="$PWD/build/stylelint-venv/bin/clang-format"
python3 -m unittest discover -s tools/stylelint/tests -v
python3 tools/stylelint/stylelint.py --changed --diff
```

Run from the repository root. `--clang-format /path/to/clang-format` overrides
`CLANG_FORMAT`; otherwise the executable is resolved from `PATH`.

```sh
# All branch changes since its merge-base with the target, plus local edits:
python3 tools/stylelint/stylelint.py --changed --base origin/master --diff
# Whole-file check, useful for a new or deliberately migrated file:
python3 tools/stylelint/stylelint.py public/example.h --diff
# Optional legacy-debt audit, NOT the normal adoption gate:
python3 tools/stylelint/stylelint.py --all
```

Exit codes: **0** passes (or explicitly reports not-applicable when there are no
eligible files), **1** style violations, **2** invalid invocation, revision, file,
encoding, configuration, or missing/wrong/failing formatter. Formatter version
and configuration are validated even for documentation-only changes. Missing
tools are not skips. A not-applicable style diff certifies no runtime tests.

## Enforced policy

[`.clang-format`](../../.clang-format) is the single mechanical-format authority:
tabs for indentation, width four, Allman braces, 100-column target, pointer and
reference sigils next to the declarator, and Source-style spaces inside nonempty
parentheses. Existing include order and comments are preserved. No braces,
qualifiers, or parentheses are added/removed as a semantic modernization.
The `c++20` setting controls formatting, **not** compiler flags or ABI policy;
legacy targets and C17 dependencies retain their declared build settings.

- `STYLE001`: formatting differs from the pinned policy. clang-format supplies
  byte-offset replacements; the checker handles Unicode and CRLF without writing.
- `STYLE002`: an include directive follows a literal `memdbgon.h` include.
  Comments, ordinary/raw strings, and escaped newlines are handled lexically.
  Includes inside conditional branches, macro-based includes, `include_next`,
  and `import` are conservatively treated as subsequent includes. The check does
  not expand macros to discover aliases for `memdbgon.h`, or evaluate preprocessor
  conditions; keep this special include literal and lexically last.

Naming consistency, suitable use of results/smart pointers, interface shape, and
synchronization proofs remain review obligations. Do not introduce simplistic
token bans on `new`, raw pointers, inheritance, or atomics: context matters.
The formatter's `clang-format off/on` markers may be used for genuinely
format-sensitive tables/macros only with an adjacent explanation and review.
They do not exempt the include-order check.

## Incremental adoption and scope

`--changed` compares the current working tree against HEAD by default, including
staged, unstaged, and non-ignored untracked files. `--base REF` uses
`merge-base(REF, HEAD)`; an invalid/missing history fails instead of skipping.
Existing files pass changed line ranges to clang-format, which can expand a
range to the surrounding syntactic construct. Deletions check the surviving
boundary; mode-only changes do not trigger whole-file formatting. New and renamed
files are checked in full. An unchanged pre-existing late include is tolerated
only when the identical violating line maps to the base; added/modified violations
and moving `memdbgon.h` before a previously valid include fail. There is no mutable
formatting debt baseline to regenerate.

Whole-file checks (`--all` or explicit paths) enforce both rules without that
legacy allowance. `--all` is expected to find historical debt; do not reformat
the engine merely to make it pass. Review proposed patches, especially macros,
before deliberately formatting a whole file with the pinned formatter.

First-party extensions: `.c`, `.cc`, `.cpp`, `.cxx`, `.h`, `.hh`, `.hpp`, `.inl`,
`.m`, `.mm`. Vendor exclusions are owned by `VENDOR_ROOTS` in the checker:
`thirdparty/`, `external/`, `ivp/`, `lib/`, `common/protobuf-2.3.0/`, `common/lzma/`,
`utils/lzma/`, `utils/jpeglib/`, `utils/bzip2/`. Data fixtures whose exact text is
part of an oracle are excluded by `DATA_FIXTURE_ROOTS`: `quality/fixtures/corpus/`
(seeded constructs and line-numbered expectations of the corpus runner class).
Other fixtures stay eligible. Git-ignored untracked build outputs
and submodule contents are not enumerated. Symlinked source paths are rejected,
not followed. New and modified text must be UTF-8. Under `--changed`, unchanged
historical comment/literal lines may retain non-UTF-8 bytes when those exact lines
exist in the Git comparison baseline. This is a byte-preserving allowance, not
encoding detection or conversion: current and baseline lexical views use
surrogate escaping, clang-format receives original bytes, and redirected proposed
diffs retain those bytes. Invalid code tokens, new/modified non-UTF-8 text, missing
baselines and malformed formatter XML still fail. Whole-file audits have no
encoding-baseline allowance. These style exclusions do not waive
architecture or dependency requirements. Keep generated sources in ignored build
outputs or document/review any additional narrowly scoped exclusion.

## CI and acceptance evidence

[Source style](../../.github/workflows/style.yml) installs the exact pin, runs the
checker tests, and gates each PR and push to `master` against the relevant base.
It checks the PR head with full history; a new `master` creation uses a full audit
instead of silently checking nothing. Repository administrators must configure
the `Source style / style` check as required in branch protection if merges must
be blocked; adding a workflow cannot set that repository policy.

The suite covers formatter golden/idempotence tests, C++20 parsing, include-order
negative fixtures, unchanged debt versus new violations, scoped Git selections,
new/renamed/deleted/mode-only files, Unicode/CRLF, invalid tool/revision/output,
read-only diagnostics, and explicit not-applicable reporting. No engine or
native-provider correctness claim follows from these tests. R04 remains partial.

Policy changes update the config, version pin if needed, fixtures, and this
document together. See the official
[clang-format option reference](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
for option semantics; the pinned executable, not rolling documentation defaults,
determines output.
