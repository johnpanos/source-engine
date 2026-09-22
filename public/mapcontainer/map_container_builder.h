//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BSP2 writing and legacy conversion for tools (RFC 0008, phase F1).
//
// Strict C++20. Builders produce whole files in memory; callers own I/O.
// Legacy VBSP -> BSP2 -> legacy VBSP is byte-identical by construction: every
// legacy lump, the legacy header and any nonzero gap bytes are carried.
//
//=============================================================================//

#ifndef MAPCONTAINER_MAP_CONTAINER_BUILDER_H
#define MAPCONTAINER_MAP_CONTAINER_BUILDER_H

#include "foundation/expected.h"
#include "mapcontainer/map_container.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace mapcontainer
{

struct ContainerDeleter
{
	void operator()( IMapContainer *pContainer ) const noexcept { DestroyMapContainer( pContainer ); }
};
using ContainerPtr = std::unique_ptr<IMapContainer, ContainerDeleter>;

// Byte source over caller-owned memory; the span must outlive the source.
class MemoryByteSource final : public IMapByteSource
{
public:
	explicit MemoryByteSource( std::span<const std::byte> bytes ) noexcept : m_Bytes( bytes ) {}
	uint64_t Size() const override { return m_Bytes.size(); }
	bool ReadAt( uint64_t offset, void *pDest, size_t size ) override;

private:
	std::span<const std::byte> m_Bytes;
};

[[nodiscard]] foundation::Expected<ContainerPtr, MapContainerStatus> OpenMemoryContainer(
    MemoryByteSource &source, bool verifyContent, bool allowUnknownRequired = false );

struct Bsp2LumpInput
{
	uint32_t fourcc = 0;
	uint32_t version = 0;
	uint32_t flags = 0; // kBsp2FlagRequired; compression must be none for now
	uint32_t alignment = kBsp2MinAlignment;
	std::span<const std::byte> data;
};

// Writes a BSP2 container. Lumps are laid out in the given order; 4CCs must be
// unique and alignments powers of two >= kBsp2MinAlignment.
[[nodiscard]] foundation::Expected<std::vector<std::byte>, MapContainerStatus> WriteBsp2(
    int32_t mapRevision, std::span<const Bsp2LumpInput> lumps );

// Legacy VBSP file (any version) -> BSP2 carrying it losslessly. Additional
// lumps (for example new render lumps) are appended after the legacy payload.
[[nodiscard]] foundation::Expected<std::vector<std::byte>, MapContainerStatus> ConvertLegacyToBsp2(
    std::span<const std::byte> legacyFile, std::span<const Bsp2LumpInput> extraLumps = {} );

// BSP2 carrying a legacy payload -> the legacy VBSP file, byte-identical to
// the one it was converted from. Verifies every content hash first. Lumps
// outside the legacy payload are dropped, including required ones: the result
// is a derived legacy artifact (RFC 0008 "legacy BSP v21 export").
[[nodiscard]] foundation::Expected<std::vector<std::byte>, MapContainerStatus> ExportLegacyFromBsp2(
    std::span<const std::byte> bsp2File );

using ContentHash = std::array<uint8_t, kBsp2HashSize>;
[[nodiscard]] ContentHash HashContent( std::span<const std::byte> bytes ) noexcept;

} // namespace mapcontainer

#endif // MAPCONTAINER_MAP_CONTAINER_BUILDER_H
