//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 execution (RFC 0016). The serial executor is the
//			oracle and low-capacity mode (RFC 0003): it creates the transients,
//			records every kept pass into one encoder in order, with the
//			compiled transitions before each, submits, and releases the
//			transients behind the submission's token.
//
//			The pooled executor records each pass on the job system and submits
//			the encoders in order; both executors may keep physical transients
//			between executions in a TransientPool.
//
//=============================================================================//

#ifndef RENDER_GRAPH_EXECUTOR_H
#define RENDER_GRAPH_EXECUTOR_H

#include "foundation/expected.h"
#include "jobsystem/graph_executor.h"
#include "render/device/device.h"
#include "render/graph/compiled_graph.h"

#include <vector>

namespace render::graph
{

// What a pass's execute function sees.
class RecordContext
{
public:
	RecordContext( device::CommandEncoder &encoder, const std::vector<device::TextureId> &textures,
	    const std::vector<device::BufferId> &buffers )
	    : m_Encoder( encoder ), m_Textures( textures ), m_Buffers( buffers )
	{
	}

	device::CommandEncoder &Encoder() { return m_Encoder; }
	device::TextureId Texture( ResourceRef resource ) const;
	device::BufferId Buffer( ResourceRef resource ) const;

private:
	device::CommandEncoder &m_Encoder;
	const std::vector<device::TextureId> &m_Textures;
	const std::vector<device::BufferId> &m_Buffers;
};

struct ExecuteResult
{
	device::CompletionToken token;
	std::uint32_t passes = 0;
	std::uint32_t transitions = 0;
	std::uint32_t transients = 0; // physical transients created by this execution
	std::uint32_t reused = 0;     // physical transients taken from a pool
	std::uint32_t encoders = 0;
};

// Physical transients kept between executions, keyed by shape and usages.
// A resource returns with the usage its execution left it in and the token
// of that submission. A later execution may take it at once: submissions on
// one queue run in order, and its first transition starts from that usage.
// A resource unused for idleExecutions executions is released behind its
// token; destruction releases the rest. Owned by the device's sequence.
class TransientPool
{
public:
	explicit TransientPool( device::IRenderDevice2 &device, std::uint32_t idleExecutions = 4 );
	~TransientPool();
	TransientPool( const TransientPool & ) = delete;
	TransientPool &operator=( const TransientPool & ) = delete;

	std::size_t Size() const { return m_Entries.size(); }
	device::IRenderDevice2 &Device() const { return m_Device; }

	struct Entry
	{
		PhysicalResource shape;
		device::TextureId texture;
		device::BufferId buffer;
		device::ResourceUsage usage = device::ResourceUsage::kUndefined;
		device::CompletionToken token;
		std::uint32_t idle = 0;
	};

	// For executors: takes a matching entry, if any.
	bool Take( const PhysicalResource &shape, Entry *out );
	// For executors: returns entries after a submission, and ages the rest.
	void Return( std::vector<Entry> &&entries );

private:
	device::IRenderDevice2 &m_Device;
	std::uint32_t m_IdleExecutions;
	std::vector<Entry> m_Entries;
};

// The serial executor: every kept pass recorded into one encoder, in order,
// with its transitions before it; one submission. The oracle and
// low-capacity mode (RFC 0003).
class SerialGraphExecutor
{
public:
	SerialGraphExecutor() = default;
	explicit SerialGraphExecutor( TransientPool &pool ) : m_Pool( &pool ) {}

	foundation::Expected<ExecuteResult, device::DeviceError> Execute( const CompiledGraph &graph,
	    device::IRenderDevice2 &device, const device::SubmitWaits &waits = {} );

private:
	TransientPool *m_Pool = nullptr;
};

// The pooled executor: each kept pass records into its own encoder as one job
// of a jobs.graph graph run by the injected executor (the engine's pool in
// products); the encoders are submitted together in pass order. Pass execute
// functions must therefore not share mutable state. The recorded command
// stream equals the serial executor's (render.graph.v1 G9).
class PooledGraphExecutor
{
public:
	explicit PooledGraphExecutor( jobsystem::IGraphExecutor &jobs, TransientPool *pool = nullptr )
	    : m_Jobs( jobs ), m_Pool( pool )
	{
	}

	foundation::Expected<ExecuteResult, device::DeviceError> Execute( const CompiledGraph &graph,
	    device::IRenderDevice2 &device, const device::SubmitWaits &waits = {} );

private:
	jobsystem::IGraphExecutor &m_Jobs;
	TransientPool *m_Pool;
};

} // namespace render::graph

#endif // RENDER_GRAPH_EXECUTOR_H
