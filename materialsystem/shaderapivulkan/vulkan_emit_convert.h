//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The chunked form of emit's per-vertex conversion (R32, RFC 0003).
//
//          CEmptyMesh::EmitToNativeQueue converts a draw's unique vertices
//          into the frame's vertex stream. A serial step first gives every
//          unique vertex its slot (first-use order) and writes the indices;
//          after it, each slot's record depends only on its vertex and the
//          draw's state, so slots can be converted in any order and on any
//          thread. This header splits the slots into fixed chunks for a
//          synchronous batch (jobsystem/parallel_batch.h):
//
//          - chunk c converts slots [c * size, min((c + 1) * size, count))
//            (size kEmitConvertChunk unless an oracle asks for smaller chunks)
//            and writes only those records and its own entry of the per-chunk
//            bone maximum;
//          - the caller reduces the chunk maxima serially after the batch
//            returns (the draw's maxBone, which keys the reuse cache).
//
//          Publication: the caller writes every input (mesh bytes, constants,
//          bones, the slot list) before starting the batch and reads the
//          records and maxima after it returns. The batch's fork is the
//          happens-before edge for inputs and its join the edge for outputs
//          (parallel_batch.h: "Return publishes all callback writes"); no
//          chunk reads another chunk's output.
//
//          The fault hooks exist for the oracles' negative controls: a chunk
//          offset bug (records shifted by one slot, overlapping the next
//          chunk) and an unsynchronized shared bone maximum (a data race).
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_EMIT_CONVERT_H
#define SHADERAPIVULKAN_VULKAN_EMIT_CONVERT_H

#ifdef _WIN32
#pragma once
#endif

#include <cstdint>

namespace render_vulkan
{

enum
{
	// Slots per chunk: enough work per item (about 5-12 us at 40-90 ns a
	// vertex) that claiming items costs little, small enough that a
	// 2048-vertex draw has 16 items to balance across the pool.
	kEmitConvertChunk = 128
};

enum class EmitConvertFault
{
	None,
	ChunkOffset,   // each chunk converts its range shifted by one slot
	SharedMaxBone, // chunks raise one shared maximum without synchronization
};

// ConvertRange: void( uint32_t begin, uint32_t end, int &maxBone ), converting
// slots [begin, end) into their records and raising maxBone to the highest
// bone they read. It must write nothing else that another chunk reads or
// writes.
template <typename ConvertRange> class CEmitConvertBatch
{
public:
	// chunkMaxBone must hold ChunkCount( count, chunkSize ) entries and outlive
	// the batch. chunkSize 0 means kEmitConvertChunk.
	CEmitConvertBatch( const ConvertRange &convert, uint32_t count, int *chunkMaxBone,
	    uint32_t chunkSize = 0, EmitConvertFault fault = EmitConvertFault::None )
	    : m_convert( convert ), m_count( count ), m_chunkSize( ChunkSize( chunkSize ) ),
	      m_chunkMaxBone( chunkMaxBone ), m_fault( fault ), m_sharedMaxBone( -1 )
	{
	}

	static uint32_t ChunkSize( uint32_t chunkSize )
	{
		return chunkSize ? chunkSize : static_cast<uint32_t>( kEmitConvertChunk );
	}
	static unsigned ChunkCount( uint32_t count, uint32_t chunkSize = 0 )
	{
		const uint64_t size = ChunkSize( chunkSize );
		return static_cast<unsigned>( ( uint64_t( count ) + size - 1 ) / size );
	}
	unsigned Chunks() const { return ChunkCount( m_count, m_chunkSize ); }

	// The batch's process callback (BatchDesc::process), context = this.
	static void Process( void *context, unsigned chunk )
	{
		CEmitConvertBatch &self = *static_cast<CEmitConvertBatch *>( context );
		uint32_t begin = static_cast<uint32_t>( chunk ) * self.m_chunkSize;
		uint32_t end = begin + self.m_chunkSize;
		if ( self.m_fault == EmitConvertFault::ChunkOffset )
		{
			++begin;
			++end;
		}
		if ( end > self.m_count )
			end = self.m_count;
		int maxBone = -1;
		if ( begin < end )
			self.m_convert( begin, end, maxBone );
		if ( self.m_fault == EmitConvertFault::SharedMaxBone )
		{
			// The defect: a read-modify-write of one shared value, from
			// every chunk at once.
			self.m_sharedMaxBone = maxBone > self.m_sharedMaxBone ? maxBone : self.m_sharedMaxBone;
			self.m_chunkMaxBone[chunk] = -1;
			return;
		}
		self.m_chunkMaxBone[chunk] = maxBone;
	}

	// After the batch returned: the highest bone any chunk read, or -1.
	int ReduceMaxBone() const
	{
		int maxBone = m_sharedMaxBone;
		const unsigned chunks = Chunks();
		for ( unsigned chunk = 0; chunk < chunks; ++chunk )
			if ( m_chunkMaxBone[chunk] > maxBone )
				maxBone = m_chunkMaxBone[chunk];
		return maxBone;
	}

private:
	const ConvertRange &m_convert;
	const uint32_t m_count;
	const uint32_t m_chunkSize;
	int *const m_chunkMaxBone;
	const EmitConvertFault m_fault;
	int m_sharedMaxBone; // written by chunks only under EmitConvertFault::SharedMaxBone
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_EMIT_CONVERT_H
