# Contract: `formats.keyvalues.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/keyvalues.h`
Impl: `hammer/core/formats/keyvalues.cpp`
Conformance: `unittests/hammertest/formats/test_keyvalues.cpp` (+ `_negative`)
Migration: `HAM-CORPUS-001` (and the H2 persistence slice)

The VMF/keyvalues document model, parser, writer, and versioned semantic
comparator. This is the codec + comparator foundation the RFC persistence slice
and characterization corpus build on. It does not select files, show dialogs, or
mutate any live document.

## 1. Purpose, consumers

Read VMF/keyvalues text into a tree, write it back, and compare two documents
semantically. Consumers: the persistence import/export path and the round-trip
corpus. Required.

## 2. Accepted inputs

- `ParseKeyValues( text )`: VMF/keyvalues text — named blocks, quoted `"key"
  "value"` pairs, nested `{ }` blocks, `//` line comments. Bare block names and
  quoted names both accepted.
- `WriteKeyValues( root )`: a document whose `children` are the top-level blocks.
- `CompareKeyValues( a, b )`: two parsed documents.

## 3. Results, error taxonomy

- `ParseKeyValues` returns `ok == true` with the tree, or `ok == false` with a
  human-readable `error` and 1-based `errorLine`. A malformed document (unclosed
  block, unterminated quote) fails; the partial tree is not published.
- `WriteKeyValues` returns serialized text with tab indentation.
- `CompareKeyValues` returns `equal`, or `equal == false` with a `firstDivergence`
  path. It never discards a field to force a match.

## 4. Ownership, threading

- Pure value transforms over owned trees. No globals, no file or dialog I/O, not
  internally synchronized.

## 5. Invariants and declared normalization

- **No data dropped**: unknown blocks and keys are preserved verbatim through
  parse+write, so an unrecognized VMF chunk round-trips losslessly.
- **Round-trip stable**: `ParseKeyValues(WriteKeyValues(d))` is semantically equal
  to `d`.
- **Comparator normalization (explicit)**: block name must match; key/value pairs
  compared as a multiset (order-insensitive, duplicates significant, empty values
  significant); child blocks compared IN ORDER and recursively.

## 6. Side effects and performance

- None beyond returned values; linear in input size.

## 7. Conformance suite and providers

- `test_keyvalues.cpp`: parse structure, round-trip + unknown-chunk preservation,
  malformed-input diagnostics, comparator normalization (key-order-insensitive,
  changed-value detected, duplicate-key significant).
- `test_keyvalues_negative.cpp`: a permissive always-equal comparator must be
  **detected** by the data-loss predicate (changed value and dropped block) —
  proving the comparator cannot certify data preservation vacuously.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
