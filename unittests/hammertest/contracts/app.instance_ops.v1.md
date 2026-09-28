# Contract: `app.instance_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/instance_ops.h`, `public/hammer/app/instance_fixup.h` ·
Impl: `hammer/core/app/ops/instance_ops.cpp`, `hammer/core/app/instance_fixup.cpp`
Conformance: `unittests/hammertest/app/test_instance_ops.cpp`
Migration: `R08-DOMAIN`
Depends on: `app.clipboard` (`Paste`), `app.prefab_ops`
(`TransformedFragment`), `ports.entity_catalog` (optional), `scene.map_queries`

## 1. Purpose, consumers, scope

func_instance collapse (legacy `CMapDoc::CollapseInstances`) with the
compiler's merge semantics (`utils/vbsp/map.cpp` `MergeInstance`,
`ReplaceInstancePair`; fgdlib `RemapNameField`), instance lookup and the
instance file name. `instance_fixup.h` holds the name and parameter rule as
string functions for every consumer. Consumers: the Instances menu, the
entity inspector ("open instance"), the command layer and MCP. File lookup
(the compiler's base-directory and instance-path search), material
replacement (absent from this tree's compiler), `func_instance_io_proxy`
I/O and `func_instance_parms` are out of scope. Required.

## 2. Accepted inputs

A func_instance entity id (class case-insensitive), its content as a
`MapFragment` (from `LoadFragment` on `InstanceFile`), and an optional
entity catalog. The instance's `origin`, `angles` (absent = zero),
`fixup_style` (absent/empty/0 prefix, 1 postfix, 2 none), `targetname` /
`name`, and `replace*` keys ("<variable> <value>").

## 3. Results and guarantees

- Per content entity, in the compiler's order: parameters substituted in key
  values, the classname and connection target/input/parameter (every
  case-insensitive occurrence, parameters in key order); then the name fixup
  of every name-typed key value and every connection target, except empty,
  `@` and `!` names (`!` kept deliberately; this tree's vbsp breaks them);
  then the transform (rotate by `angles`, translate by `origin`).
- Name-typed keys: `target_source`/`target_destination` keys of the class per
  the catalog; without a catalog or for an unknown class, `targetname`,
  `target`, `parentname`, `filtername`, `damagefilter`, `lightingorigin`.
  Names are fixed whether or not the instance defines them (compiler rule).
- Fixup name: `targetname`, else `name`, else `InstanceAuto<n>` with the
  smallest n no target entity name contains.
- Merged objects get fresh ids via `Paste` (overlay `sides` follow); solids
  keep texture alignment; overlays keep their basis; top-level content
  objects join the instance's group; the instance is removed. Empty content
  only removes it. Nested func_instances stay entities (their keys fixed,
  `file` verbatim).
- Refusals (`Rejected`, nothing staged): not a func_instance, an instance
  owning solids, malformed `origin`/`angles`, unknown `fixup_style`,
  degenerate transforms.

## 4. Ownership, threading

Pure functions over the edit; the catalog is borrowed for the call. Single
sequence.

## 5. Invariants

Every collapse passes `ValidateEdit` and commits; side VMF ids stay unique;
the content fragment is never modified.

## 6. Side effects and performance

Linear in content size times the parameter count; one scan of entity names
for an automatic fixup name.

## 7. Conformance suite and providers

`test_instance_ops.cpp` (56 checks, gcc and clang, `-Wall -Wextra -Werror`),
with `FakeEntityCatalog`: style parsing, global names, parameter order and
case, prefix/postfix/none collapse, conventional and catalog-typed keys,
rotation then translation of world solids, brush-entity solids, entities and
overlays, group membership, nested instances, fallback fixup names, empty
content, and negative checks that stage nothing. Seeded faults detected:
`!` names fixed, group membership dropped.
