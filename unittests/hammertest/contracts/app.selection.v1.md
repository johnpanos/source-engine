# Contract: `app.selection.v1`

Module: `hammer.app`
Header: `public/hammer/app/selection.h` · Impl: `hammer/core/app/selection.cpp`
Conformance: `unittests/hammertest/app/test_selection.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The selection value and its one policy owner: what a hit selects per
granularity (legacy Groups, Objects, Solids modes), how modes combine, the
primary object, pruning, select-all and invert. Consumers: `EditSession`,
tools, presenters, commands. Required.

## 2. Accepted inputs

Any ids and faces; invalid and unknown ids are ignored.

## 3. Results and guarantees

- Objects and faces stay sorted and unique.
- The primary is the last id added; if removed, the largest remaining id.
- `Prune` drops objects, faces of dead solids and faces of removed sides.
- `SelectAll` excludes objects that are not visible unless asked.

## 4. Ownership, threading

Values; pure functions.

## 5. Invariants

`primary` is a member of `objects` or invalid.

## 6. Side effects and performance

None.

## 7. Conformance suite and providers

`test_selection.cpp` (24 checks, gcc and clang), with negative checks for
invalid and unknown ids, absent removals and removed sides.
