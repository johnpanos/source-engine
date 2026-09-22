# Contract: `scene.scene_graph.v1`

Module: `hammer.scene` · Type: `hammer::scene::SceneGraph`
Header: `public/hammer/scene/scene_graph.h` · Impl: `hammer/core/scene/scene_graph.cpp`
Conformance: `unittests/hammertest/scene/test_scene_graph.cpp` (+ `_negative`)
Migration: `HAM-SCENE-001`

The RFC 0002 H1 scene mutation/query boundary. Nodes are addressed only by
`NodeHandle` (see `scene.handle_table.v1`); the graph never exposes raw pointers.

## 1. Purpose, consumers, required vs optional

Own the scene node hierarchy and the validated operations on it: create, query,
reparent, delete. Consumers: document/session, tools, viewport extraction (via
handles + queries). All clauses are **required**.

## 2. Accepted inputs

- `SceneGraph( documentId )`: non-zero document identity (see handle contract).
- Query/mutation methods accept any `NodeHandle`, including stale/foreign/null;
  these are rejected with an explicit outcome, never dereferenced.
- `SetParent( child, parent )`: `parent == kInvalidNodeHandle` means "make child
  a root".

## 3. Results, error taxonomy

- `CreateNode()` returns a valid root handle (or the invalid handle if the graph
  has no document identity).
- `SetParent` returns a `ReparentResult`: `kOk`, `kInvalidChild`, `kInvalidParent`,
  `kForeignDocument`, `kSelfParent`, or `kWouldCycle`. **Only `kOk` mutates**;
  every failure leaves the graph exactly as it was.
- `Remove` returns true and deletes the node **and its whole subtree atomically**
  (every affected handle becomes stale); false and no change for an invalid node.
- `GetParent` returns the parent handle, or the invalid handle for a root/invalid
  node. `Children` returns the direct children (empty for invalid). `IsAncestor(
  a, n )` is true iff `a == n` or `a` is above `n`.

## 4. Ownership, threading, ordering

- The graph owns its node records and the underlying handle table. Handles are
  values that borrow nothing. Not internally synchronized; distinct graphs (hence
  distinct documents) are fully independent. No globals.

## 5. Invariants and legal sequences

- **No cycles**: a reparent whose parent is the child or a descendant of the
  child is rejected (`kWouldCycle`); self-parenting is `kSelfParent`.
- **Valid references**: children/parent links only ever reference live nodes of
  the same document.
- **Atomic reparent**: validation happens before any mutation; a rejected
  reparent changes nothing (old parent/children lists intact).
- **Atomic delete**: removing a node frees its entire subtree; no dangling child
  link and no resolvable handle to a removed node remain.
- **Independent documents**: a handle issued by one document's graph never
  validates or mutates another's, even under slot-index reuse.

## 6. Side effects and performance

- No I/O, logging, or globals. Create/query are effectively O(1)/O(children);
  reparent is O(depth) for the cycle check; delete is O(subtree).

## 7. Conformance suite and providers

- `test_scene_graph.cpp`: tree build/query, cycle+self rejection with no change,
  atomic subtree delete + stale rejection, independent-document rejection.
- `test_scene_graph_negative.cpp`: a permissive (cycle-accepting) provider must be
  detected by the no-cycle predicate — proving the oracle is not vacuous.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
