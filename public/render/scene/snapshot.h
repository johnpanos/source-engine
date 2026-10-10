//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1 snapshots (RFC 0016): the scene as of one commit,
//			immutable. A frame renders one snapshot while the next frame's
//			changes commit.
//
//			The instance table is persistent: fixed blocks of kBlock slots
//			behind shared pointers. A snapshot is a list of block pointers, so
//			a commit copies the blocks its changes touch and shares the rest
//			with the previous snapshot: its cost follows the change set and
//			the block count, not the instance count. An instance keeps its
//			slot until it is removed; a removed instance leaves an empty slot
//			(no id, empty bounds), which no view keeps.
//
//=============================================================================//

#ifndef RENDER_SCENE_SNAPSHOT_H
#define RENDER_SCENE_SNAPSHOT_H

#include "render/scene/objects.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace render::scene
{

class InstanceTable
{
public:
	static constexpr std::size_t kBlock = 256;

	std::size_t size() const { return m_Size; } // slots, live or empty
	const MeshInstance &operator[]( std::size_t slot ) const
	{
		return ( *m_Blocks[slot / kBlock] )[slot % kBlock];
	}
	// A write copies the block first when a snapshot shares it.
	MeshInstance &operator[]( std::size_t slot )
	{
		auto &block = m_Blocks[slot / kBlock];
		if ( block.use_count() != 1 )
			block = std::make_shared<Block>( *block );
		return ( *block )[slot % kBlock];
	}
	void resize( std::size_t count ) // appends empty slots
	{
		while ( m_Blocks.size() * kBlock < count )
			m_Blocks.push_back( std::make_shared<Block>() );
		m_Size = count;
	}

private:
	using Block = std::array<MeshInstance, kBlock>;
	std::vector<std::shared_ptr<Block>> m_Blocks;
	std::size_t m_Size = 0;
};

struct SceneSnapshot
{
	std::uint64_t revision = 0;
	InstanceTable instances; // by slot; a removed instance leaves an empty slot
};

} // namespace render::scene

#endif // RENDER_SCENE_SNAPSHOT_H
