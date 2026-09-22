# Contract: `world.map_container` v1

Owner: [RFC 0008](../../../RFC/0008-canonical-world-data-and-runtime-formats.md), phase F1.
Interface: [`public/mapcontainer/map_container.h`](../../../public/mapcontainer/map_container.h).
On-disk layout: [`public/mapcontainer/map_container_format.h`](../../../public/mapcontainer/map_container_format.h).
Shared suite: [`test_map_container.cpp`](../test_map_container.cpp) (`world.map-container`).

`IMapContainer` tells a consumer where a map lump is and what it is. It does
not own I/O. Consumers read lump bytes through their own path (the engine
filesystem, lump override files) and pass them back to `VerifyContent`.

## Providers

| Provider | Kind | Hashes |
| --- | --- | --- |
| Legacy VBSP reader | `LegacyVbsp` | none; `VerifyContent` always succeeds |
| BSP2 reader | `Bsp2` | BLAKE2b-128 per lump and over the directory |

## Obligations (every provider)

1. `LegacyVersion` and `MapRevision` equal the legacy header's `version` and
   `mapRevision`. A BSP2 container answers from `LHDR`.
2. `FindLegacyLump(i)` succeeds for every `0 <= i < 64` and fails for other
   indices or a null output. For each lump:
   - `fourcc` is `Lnnn`.
   - `version` and `legacyUncompressedSize` equal the legacy header fields,
     including for empty lumps.
   - `legacyOrigin` is the lump's legacy file offset. Game-lump dictionaries
     are rebased with it.
   - `storedSize` is the legacy length.
   - The bytes at `offset` equal the legacy lump bytes.
3. `FindLump(Lnnn)` agrees with `FindLegacyLump`. `LumpAt` enumerates every
   legacy lump that has data, and fails past `LumpCount`.
4. `VerifyContent` accepts the stored bytes. A provider with hashes rejects
   altered bytes (`content-hash-mismatch`) and a wrong size (`size-mismatch`).
5. `OpenMapContainer` either succeeds with a non-null container or returns a
   structured error with a null container. No malformed input may crash, read
   out of bounds or allocate without bound. Lump count is capped at 65536, and
   every size is checked against the actual file size first.

## BSP2 validation rules

The opener rejects each of the following with its named error. Each has a
negative fixture in the shared suite.

- Magic, container version, header flags, header/entry size, reserved fields
  and hash algorithm.
- Directory bounds and directory hash.
- Per-entry checks:
  - known flags only;
  - alignment is a power of two and at least 16;
  - offset is aligned;
  - payload lies between the header and the directory;
  - stored size equals uncompressed size when uncompressed.
- Overlapping payloads and duplicate 4CCs.
- An unknown lump flagged required fails with `unknown-required-lump`, unless
  the caller declares its 4CC. Legacy-only tools may opt out with
  `allowUnknownRequired`. An unknown optional lump is skipped whatever its
  encoding.
- Compression other than none on a known or required lump
  (`unsupported-compression`, until zstd is pinned).
- Legacy payload checks:
  - any `Lnnn` requires `LHDR`;
  - `LHDR` must describe a valid legacy file;
  - the `LHDR` revision must equal the container revision;
  - every nonempty legacy lump must be carried with its legacy length and
    version.
- Export only: an `LGAP` record may not overlap the legacy header or any
  legacy lump.

## Lossless carriage

`ConvertLegacyToBsp2` followed by `ExportLegacyFromBsp2` returns the original
legacy file byte-for-byte for any valid legacy file, and conversion is
deterministic. Zero-length lumps may have arbitrary offsets. Nonzero gap and
trailing bytes are carried in `LGAP`. Export drops lumps outside the legacy
payload, because its output is a derived legacy artifact.

## Deliberately bad providers

The shared suite must report violations for:

- a provider that returns the container offset as `legacyOrigin`;
- a provider that loses the versions of empty lumps;
- a provider whose `VerifyContent` accepts anything.
