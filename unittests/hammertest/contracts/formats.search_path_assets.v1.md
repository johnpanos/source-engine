# Contract: `formats.search_path_assets.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/search_path_assets.h`
Impl: `hammer/core/formats/search_path_assets.cpp`
Conformance: `unittests/hammertest/formats/test_search_path_assets.cpp`
Migration: `HAM-ASSET-001`

An ordered composite `IAssetSource` over several providers. A Source game mounts
many archives plus loose files; an asset resolves from the first provider that
has it. This is the mount behavior a game relies on — a `.vmt` in one VPK naming
a `.vtf` in another.

## 1. Purpose, consumers

Compose VPK archives (and any other `IAssetSource`) into one search path.
Consumers: the material catalog and the GTK shell's asset mount. C++ stdlib and
the `IAssetSource` port only. Required for multi-archive mounts.

## 2. Accepted inputs

- `AddProvider( provider )`: appends to the search order (lowest priority so
  far). A null provider is ignored. Providers are borrowed, not owned.
- `HasAsset` / `ReadAsset` / `ListAssets`: canonical paths, as for any
  `IAssetSource`.

## 3. Results, error taxonomy

- `HasAsset` / `ReadAsset` return the FIRST provider's hit (shadowing). Absent in
  every provider is a clean miss (false).
- `ListAssets` is the de-duplicated union across providers in search order (the
  first occurrence of each path is kept).

## 4. Ownership, threading

- Borrows its providers (they must outlive the search path). Reads are const; not
  internally synchronized. No globals.
