//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Host frame graph over FrameCoordinator + DeterministicExecutor
//          (RFC 0003, R10).
//
//=============================================================================//

#include "jobsystem/serial_frame_graph.h"

#include <atomic>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

#include "jobsystem/frame_graph.h"
#include "jobsystem/graph_executor.h"

namespace jobsystem
{

struct SerialFrameGraph::Impl
{
	struct ShapeEntry
	{
		const char *name;
		bool ( *run )( void *, int );
	};

	bool SameShape( const FramePhaseDesc *phases, unsigned count ) const
	{
		if ( !sealed || shape.size() != count )
			return false;
		for ( unsigned i = 0; i < count; ++i )
		{
			// Names are compared by content so an equal static table from
			// another translation unit does not force a rebuild.
			if ( shape[i].run != phases[i].run ||
			     std::strcmp( shape[i].name, phases[i].name ) != 0 )
				return false;
		}
		return true;
	}

	bool Rebuild( const FramePhaseDesc *phases, unsigned count )
	{
		sealed.reset();
		shape.clear();

		FrameCoordinator coordinator( FrameContext{} );
		for ( unsigned i = 0; i < count; ++i )
		{
			coordinator.AddLegacyPhase( phases[i].name,
			    [this, i]( JobRunContext & )
			    {
				    RunPhase( i );
			    } );
		}
		Expected<SealedGraph, GraphError> built = coordinator.Build( false );
		if ( !built )
			return false;

		sealed.emplace( std::move( built ).Value() );
		shape.reserve( count );
		for ( unsigned i = 0; i < count; ++i )
			shape.push_back( ShapeEntry{ phases[i].name, phases[i].run } );
		++seals;
		return true;
	}

	void RunPhase( unsigned i )
	{
		const FramePhaseDesc &phase = current[i];
		++executed;
		if ( !phase.run( phase.context, phase.arg ) )
			stop.store( true, std::memory_order_relaxed );
	}

	std::vector<ShapeEntry> shape;
	std::optional<SealedGraph> sealed;
	DeterministicExecutor executor;
	const FramePhaseDesc *current = nullptr;
	std::atomic<bool> stop{ false };
	unsigned executed = 0;
	unsigned seals = 0;
	bool running = false;
};

SerialFrameGraph::SerialFrameGraph() : m_impl( new Impl )
{
}

SerialFrameGraph::~SerialFrameGraph()
{
	delete m_impl;
}

SerialFrameRun SerialFrameGraph::Run( const FramePhaseDesc *phases, unsigned count )
{
	SerialFrameRun result{ false, false, 0u };
	if ( m_impl->running || ( count && !phases ) )
		return result;
	for ( unsigned i = 0; i < count; ++i )
	{
		if ( !phases[i].name || !phases[i].name[0] || !phases[i].run )
			return result;
	}

	if ( !m_impl->SameShape( phases, count ) && !m_impl->Rebuild( phases, count ) )
		return result;

	m_impl->current = phases;
	m_impl->executed = 0;
	m_impl->stop.store( false, std::memory_order_relaxed );
	m_impl->running = true;

	// The deterministic executor runs in the sealed topological order, which
	// for one sequence lane is registration order; cancellation is checked
	// before each job starts.
	RunOptions options;
	options.cancel = &m_impl->stop;
	m_impl->executor.Execute( *m_impl->sealed, options );

	m_impl->running = false;
	m_impl->current = nullptr;
	result.valid = true;
	result.stopped = m_impl->stop.load( std::memory_order_relaxed );
	result.executed = m_impl->executed;
	return result;
}

unsigned SerialFrameGraph::SealCount() const
{
	return m_impl->seals;
}

} // namespace jobsystem
