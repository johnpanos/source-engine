//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Per-frame cost counters for native Vulkan frame-pacing measurement.
//
//          A frame hitch is only fixable once it has a name. Each operation that
//          can block the frame being built (a pipeline compile, a submit that
//          waits for the queue, a device wait, a buffer reallocation, a texture
//          upload, a query wait) records its count and wall time into the
//          current FrameCost. The context emits one line per presented frame
//          when a stats sink is open (tools/quality/frame_pacing.py reads it),
//          and otherwise only accumulates, which costs two clock reads per
//          instrumented operation.
//
//          mesh_draw and emit run while the engine builds the frame (before
//          Present), so they are part of a frame's "engine" time, not its
//          "backend" time.
//
//          Kinds nest where the operations do: a texture creation that submits
//          a layout transition counts under both texture_create and
//          single_submit, and emit_convert (the per-vertex conversion) is part
//          of emit. Sums across kinds are therefore not meaningful; each kind
//          answers "how much of this frame did X take".
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_FRAME_STATS_H
#define SHADERAPIVULKAN_VULKAN_FRAME_STATS_H

#ifdef _WIN32
#pragma once
#endif

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>

namespace render_vulkan
{

enum FrameCostKind
{
	kCostPipelineCreate, // vkCreateGraphicsPipelines for a state first seen
	kCostSingleSubmit,   // a one-off command buffer submitted and waited for
	kCostDeviceWaitIdle, // vkDeviceWaitIdle outside teardown
	kCostTextureCreate,  // image, memory, views and descriptor sets
	kCostTextureUpload,  // staging copy of caller texels into an image
	kCostBufferGrow,     // a per-frame buffer reallocated to a larger size
	kCostQueryWait,      // an occlusion query read that waited for the GPU
	kCostFenceWait,      // BeginFrame waiting for a frame slot or image
	kCostAcquire,        // vkAcquireNextImageKHR
	kCostRecord,         // replaying the frame's stream into the command buffer
	kCostSubmit,         // vkQueueSubmit of the frame
	kCostPresent,        // vkQueuePresentKHR
	kCostMeshDraw,       // IMesh::Draw: the material's shader passes and their emits
	kCostEmit,           // converting a pass's mesh into the frame's vertex stream
	kCostEmitReuse,      // a pass that reused geometry already in the stream (count only)
	kCostRenderPass,     // a scene render pass begun; a tiled GPU stores and reloads
	                     // the target at each one (count only)
	kCostTargetCopy,     // a render-target copy recorded (count only)
	kCostEmitConvert,    // emit's per-vertex conversion of a draw's unique vertices
	                     // (count: draws; time from EmitConvertStats' nanoseconds)
	kFrameCostKinds
};

// Stable names: the frame-stats line format (frame_pacing.py) keys on them.
inline const char *FrameCostName( int kind )
{
	static const char *const kNames[kFrameCostKinds] = { "pipeline_create", "single_submit",
	    "device_wait_idle", "texture_create", "texture_upload", "buffer_grow", "query_wait",
	    "fence_wait", "acquire", "record", "submit", "present", "mesh_draw", "emit", "emit_reuse",
	    "render_pass", "target_copy", "emit_convert" };
	return kind >= 0 && kind < kFrameCostKinds ? kNames[kind] : "unknown";
}

inline uint64_t FrameClockMicros()
{
	return static_cast<uint64_t>( std::chrono::duration_cast<std::chrono::microseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	                                  .count() );
}

// CPU time consumed by the calling thread, in microseconds (0 where the
// platform has no per-thread clock). A frame's wall interval minus the render
// thread's CPU time is time it waited or was preempted: on a loaded host the
// wall interval alone cannot tell a slower build from a busier machine.
inline uint64_t ThreadCpuMicros()
{
#if defined( CLOCK_THREAD_CPUTIME_ID )
	timespec now;
	if ( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &now ) != 0 )
		return 0;
	return static_cast<uint64_t>( now.tv_sec ) * 1000000u +
	       static_cast<uint64_t>( now.tv_nsec ) / 1000u;
#else
	return 0;
#endif
}

inline uint64_t FrameClockNanos()
{
	return static_cast<uint64_t>( std::chrono::duration_cast<std::chrono::nanoseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	        .count() );
}

// Emit's vertex conversion by draw size: how many unique vertices each
// converted draw had, and what converting them cost. The buckets answer how
// much of the conversion is in draws large enough to split across workers.
// Bucket b holds draws of [kEmitConvertBucketMin[b], kEmitConvertBucketMin[b + 1])
// unique vertices; the last is open.
enum
{
	kEmitConvertBuckets = 7
};
static const uint32_t kEmitConvertBucketMin[kEmitConvertBuckets] = {
    1, 64, 256, 1024, 2048, 4096, 8192 };

struct EmitConvertStats
{
	uint32_t draws[kEmitConvertBuckets];
	uint64_t vertices[kEmitConvertBuckets];
	uint64_t ns[kEmitConvertBuckets];
	uint64_t skinnedVertices; // vertices of draws with bone weights (CPU skinning)
	uint32_t pooledDraws;     // draws converted by a pooled batch (the rest, serially)
	uint64_t totalNs;

	static int Bucket( uint32_t vertexCount )
	{
		int bucket = 0;
		while (
		    bucket + 1 < kEmitConvertBuckets && vertexCount >= kEmitConvertBucketMin[bucket + 1] )
			++bucket;
		return bucket;
	}
};

struct FrameCost
{
	uint32_t count[kFrameCostKinds];
	uint64_t us[kFrameCostKinds];
	uint64_t uploadBytes;
	EmitConvertStats convert;

	FrameCost() { Reset(); }
	void Reset()
	{
		std::memset( count, 0, sizeof( count ) );
		std::memset( us, 0, sizeof( us ) );
		uploadBytes = 0;
		std::memset( &convert, 0, sizeof( convert ) );
	}
	void Add( FrameCostKind kind, uint64_t micros )
	{
		++count[kind];
		us[kind] += micros;
	}
	// One draw's conversion. Kept in nanoseconds (a small draw converts in a
	// few microseconds); the emit_convert kind reports the total in
	// microseconds.
	void AddConvert( uint32_t vertexCount, uint64_t nanos, bool skinned, bool pooled )
	{
		if ( vertexCount == 0 )
			return;
		const int bucket = EmitConvertStats::Bucket( vertexCount );
		++convert.draws[bucket];
		convert.vertices[bucket] += vertexCount;
		convert.ns[bucket] += nanos;
		if ( skinned )
			convert.skinnedVertices += vertexCount;
		if ( pooled )
			++convert.pooledDraws;
		convert.totalNs += nanos;
		++count[kCostEmitConvert];
		us[kCostEmitConvert] = convert.totalNs / 1000u;
	}
};

// Adds the scope's wall time to one kind of the frame's cost.
class CFrameCostScope
{
public:
	CFrameCostScope( FrameCost &cost, FrameCostKind kind )
	    : m_cost( cost ), m_kind( kind ), m_start( FrameClockMicros() )
	{
	}
	~CFrameCostScope() { m_cost.Add( m_kind, FrameClockMicros() - m_start ); }

	CFrameCostScope( const CFrameCostScope & ) = delete;
	CFrameCostScope &operator=( const CFrameCostScope & ) = delete;

private:
	FrameCost &m_cost;
	FrameCostKind m_kind;
	uint64_t m_start;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_FRAME_STATS_H
