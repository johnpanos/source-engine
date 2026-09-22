# Contract: `formats.vpk_archive.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/vpk_archive.h`
Impl: `hammer/core/formats/vpk_archive.cpp`
Conformance: `unittests/hammertest/formats/test_vpk_archive.cpp` (+ `_negative`)
Migration: `HAM-ASSET-001`

A dependency-free reader for Valve Pack (VPK) archives, versions 1 and 2. It
parses the directory tree of a `_dir.vpk` and resolves a file's bytes on demand,
implementing `hammer::ports::IAssetSource`. It replaces the engine-coupled
`vpklib` `CPackedStore` (tier2 + `IBaseFileSystem`) for the strict editor core.

## 1. Purpose, consumers

Mount a shipped VPK and read assets out of it by canonical path. Consumers: the
material catalog and the GTK shell's asset mount. Depends only on the C++ stdlib
and the `IByteStore` ranged-read port. Required for VPK-backed content.

## 2. Accepted inputs

- `Open( store, dirVpkPath, error )`: the path of a `_dir.vpk` directory file,
  read through an `IByteStore`. Data-archive siblings (`_000.vpk`, ...) are
  derived from the base name. Version 1 (12-byte header) and 2 (28-byte header).
- `ReadAsset` / `HasAsset` / `Find`: a canonical path (lower-case, forward slash,
  extension included; root files carry no directory component).
- `ListAssets( prefix, extensionWithDot, out )`.

## 3. Results, error taxonomy

- `Open` returns the archive, or `nullptr` with a diagnostic `error` on: absent
  or too-small directory file, bad signature, unsupported version, a tree longer
  than the file, an unterminated extension/dir/file list, a preload run past the
  tree, or a bad `0xffff` file-record terminator.
- `ReadAsset` assembles a file's inline preload plus its chunk (from the `_dir`
  data section when `archiveIndex == 0x7fff`, else from `_NNN.vpk`). Returns
  false on an unknown path or when a required data archive / byte range is
  missing. A short read is a failure, never a truncation.
- Signature and MD5 sections (v2) are not verified: this is a read path, not a
  trust boundary.

## 4. Ownership, threading

- Borrows the `IByteStore` (must outlive the archive). Reads are const; not
  internally synchronized. No globals, no dialog or engine I/O.
