# Contract: `app.portal2_workflow.v1`

Module: `hammer.app` (integrates `hammer.formats`, `hammer.scene`)
Conformance: `unittests/hammertest/app/test_portal2_workflow.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

A cross-layer workflow oracle on real Portal 2 maps: the VMF codec, the
session and the operation families together, the way a UI drives them.
Required.

## 2. Accepted inputs

The vendored fixtures in `unittests/hammertest/fixtures/portal2/` (provenance
in `provenance.json`).

## 3. Results and guarantees

- Each map opens through the codec into a session and validates.
- Select all, move everything, and undo restores the decoded content exactly.
- The map check, a copied brush entity pasted with an offset, and a group
  delete keep the document valid.
- Saving the edited map and reopening it encodes identically.
- Every whole-map step finishes within an interactive budget (2 s), so a
  quadratic query path on maps of this size fails the suite.

## 4. Ownership, threading

Single sequence; the in-memory file store stands in for disk.

## 5. Invariants

`MapDocument::Validate()` is empty after every step.

## 6. Side effects and performance

Measured on the development host (2026-09-28): sp_a2_trust_fling opens in
0.14 s, moves in 0.08 s, checks in 0.30 s and saves in 0.05 s.

## 7. Conformance suite and providers

`test_portal2_workflow.cpp` (43 checks, gcc and clang).
