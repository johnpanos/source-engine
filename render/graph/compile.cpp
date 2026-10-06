//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 compiler (RFC 0016); see compiled_graph.h.
//
//			Culling is conservative: a write never ends the life of what was
//			written before it (a render pass may load), so every earlier writer
//			of a needed resource is kept.
//
//			A transition is emitted when an access's usage differs from the
//			resource's current usage, and also between two writes in different
//			passes with the same usage (a write-after-write dependency with no
//			layout change).
//
//=============================================================================//

#include "render/graph/compiled_graph.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <utility>

namespace render::graph
{

namespace
{

foundation::Unexpected<GraphError> Fail(
    GraphStatus status, std::size_t pass, std::size_t resource )
{
	return foundation::MakeUnexpected( GraphError{
	    status, static_cast<std::uint32_t>( pass ), static_cast<std::uint32_t>( resource ) } );
}

// Two transients can share a physical resource when everything but their
// usages (which the resource takes the union of) and names matches.
bool SameShape( const PhysicalResource &slot, const ResourceDecl &decl )
{
	if ( slot.isTexture != decl.isTexture )
		return false;
	if ( decl.isTexture )
	{
		const device::TextureDesc &a = slot.texture;
		const device::TextureDesc &b = decl.texture;
		return a.dimension == b.dimension && a.format == b.format && a.width == b.width &&
		       a.height == b.height && a.depthOrLayers == b.depthOrLayers &&
		       a.mipLevels == b.mipLevels && a.sampleCount == b.sampleCount;
	}
	return slot.buffer.size == decl.buffer.size && slot.buffer.memory == decl.buffer.memory;
}

} // namespace

const char *DescribeGraphStatus( GraphStatus status )
{
	switch ( status )
	{
	case GraphStatus::kReadBeforeWrite:
		return "read before write";
	case GraphStatus::kConflictingAccess:
		return "conflicting access";
	case GraphStatus::kInvalidResource:
		return "invalid resource";
	case GraphStatus::kUsageKindMismatch:
		return "usage kind mismatch";
	case GraphStatus::kNoExecute:
		return "no execute";
	case GraphStatus::kQueueKindMismatch:
		return "queue kind mismatch";
	}
	return "unknown";
}

std::string GraphTrace::ToString() const
{
	std::ostringstream out;
	for ( std::uint32_t pass : kept )
		out << "keep " << pass << "\n";
	for ( std::uint32_t pass : culled )
		out << "cull " << pass << "\n";
	for ( const TraceTransition &t : transitions )
	{
		out << "transition pass="
		    << static_cast<std::int64_t>(
		           t.pass == UINT32_MAX ? -1 : static_cast<std::int64_t>( t.pass ) )
		    << " resource=" << t.resource << " " << static_cast<int>( t.before ) << "->"
		    << static_cast<int>( t.after ) << "\n";
	}
	for ( const TraceWait &w : waits )
		out << "wait pass=" << w.consumer << " for=" << w.producer << "\n";
	return out.str();
}

foundation::Expected<CompiledGraph, GraphError> CompileGraph(
    GraphBuilder &&builder, const CompileOptions &options )
{
	CompiledGraph graph;
	graph.resources = builder.Resources();
	graph.passes = builder.Passes();
	const std::size_t resourceCount = graph.resources.size();
	const std::size_t passCount = graph.passes.size();

	// Access shape.
	for ( std::size_t p = 0; p < passCount; ++p )
	{
		if ( graph.passes[p].queue == Queue::kAsyncCompute &&
		     graph.passes[p].kind != PassKind::kCompute )
			return Fail( GraphStatus::kQueueKindMismatch, p, 0 );
		if ( !options.asyncCompute )
			graph.passes[p].queue = Queue::kGraphics;
		const std::vector<Access> &accesses = graph.passes[p].accesses;
		for ( std::size_t a = 0; a < accesses.size(); ++a )
		{
			const Access &access = accesses[a];
			if ( !access.resource.IsValid() || access.resource.index >= resourceCount )
				return Fail( GraphStatus::kInvalidResource, p, access.resource.index );
			if ( access.write != device::IsWrite( access.usage ) ||
			     access.usage == device::ResourceUsage::kUndefined )
				return Fail( GraphStatus::kUsageKindMismatch, p, access.resource.index );
			for ( std::size_t b = 0; b < a; ++b )
			{
				if ( accesses[b].resource == access.resource && accesses[b].usage != access.usage )
					return Fail( GraphStatus::kConflictingAccess, p, access.resource.index );
			}
		}
	}

	// Transients are written before they are read, in declaration order.
	std::vector<bool> written( resourceCount, false );
	for ( std::size_t p = 0; p < passCount; ++p )
	{
		for ( const Access &access : graph.passes[p].accesses )
		{
			const std::uint32_t r = access.resource.index;
			if ( !access.write && !graph.resources[r].imported && !written[r] )
				return Fail( GraphStatus::kReadBeforeWrite, p, r );
		}
		for ( const Access &access : graph.passes[p].accesses )
		{
			if ( access.write )
				written[access.resource.index] = true;
		}
	}

	// Culling, backward: imported contents are needed.
	std::vector<bool> needed( resourceCount, false );
	for ( std::size_t r = 0; r < resourceCount; ++r )
		needed[r] = graph.resources[r].imported;
	std::vector<bool> keep( passCount, false );
	for ( std::size_t p = passCount; p-- > 0; )
	{
		const PassDecl &pass = graph.passes[p];
		bool kept = pass.sideEffect;
		for ( const Access &access : pass.accesses )
			kept |= access.write && needed[access.resource.index];
		if ( !kept )
			continue;
		keep[p] = true;
		for ( const Access &access : pass.accesses )
			needed[access.resource.index] = true;
	}
	for ( std::size_t p = 0; p < passCount; ++p )
	{
		if ( keep[p] && !graph.passes[p].execute )
			return Fail( GraphStatus::kNoExecute, p, 0 );
		( keep[p] ? graph.trace.kept : graph.trace.culled )
		    .push_back( static_cast<std::uint32_t>( p ) );
	}

	// Lifetimes of transients over kept passes.
	std::vector<std::int64_t> first( resourceCount, -1 );
	std::vector<std::int64_t> last( resourceCount, -1 );
	std::vector<device::UsageSet> usages( resourceCount );
	for ( std::size_t k = 0; k < graph.trace.kept.size(); ++k )
	{
		for ( const Access &access : graph.passes[graph.trace.kept[k]].accesses )
		{
			const std::uint32_t r = access.resource.index;
			if ( first[r] < 0 )
				first[r] = static_cast<std::int64_t>( k );
			last[r] = static_cast<std::int64_t>( k );
			usages[r].Add( access.usage );
		}
	}

	// Physical resources: each transient, in first-use order, takes the
	// lowest-numbered compatible physical resource free before it starts.
	graph.physicalOf.assign( resourceCount, -1 );
	std::vector<std::uint32_t> byFirstUse;
	for ( std::size_t r = 0; r < resourceCount; ++r )
	{
		if ( !graph.resources[r].imported && first[r] >= 0 )
			byFirstUse.push_back( static_cast<std::uint32_t>( r ) );
	}
	std::stable_sort( byFirstUse.begin(), byFirstUse.end(),
	    [&]( std::uint32_t a, std::uint32_t b )
	    {
		    return first[a] < first[b];
	    } );
	for ( std::uint32_t r : byFirstUse )
	{
		const ResourceDecl &decl = graph.resources[r];
		std::int32_t chosen = -1;
		for ( std::size_t p = 0; p < graph.physical.size() && chosen < 0; ++p )
		{
			const PhysicalResource &slot = graph.physical[p];
			if ( slot.last < first[r] && SameShape( slot, decl ) )
				chosen = static_cast<std::int32_t>( p );
		}
		if ( chosen < 0 )
		{
			PhysicalResource slot;
			slot.isTexture = decl.isTexture;
			slot.texture = decl.texture;
			slot.buffer = decl.buffer;
			slot.first = static_cast<std::uint32_t>( first[r] );
			graph.physical.push_back( slot );
			chosen = static_cast<std::int32_t>( graph.physical.size() - 1 );
		}
		PhysicalResource &slot = graph.physical[chosen];
		slot.members.push_back( r );
		slot.last = static_cast<std::uint32_t>( last[r] );
		for ( std::uint32_t u = 0; u < static_cast<std::uint32_t>( device::ResourceUsage::kCount );
		    ++u )
		{
			if ( usages[r].Has( static_cast<device::ResourceUsage>( u ) ) )
				slot.usages.Add( static_cast<device::ResourceUsage>( u ) );
		}
		graph.physicalOf[r] = chosen;
	}
	for ( const PhysicalResource &slot : graph.physical )
		graph.trace.aliasSets.push_back( slot.members );

	// Transitions, forward over kept passes. State is tracked per physical
	// resource for transients (so a reused resource starts from what its
	// previous member left) and per resource for imports.
	auto key = [&]( std::uint32_t r ) -> std::size_t
	{
		return graph.physicalOf[r] >= 0
		           ? resourceCount + static_cast<std::size_t>( graph.physicalOf[r] )
		           : r;
	};
	const std::size_t keyCount = resourceCount + graph.physical.size();
	std::vector<device::ResourceUsage> state( keyCount, device::ResourceUsage::kUndefined );
	std::vector<std::int64_t> lastWriter( keyCount, -1 );
	for ( std::size_t r = 0; r < resourceCount; ++r )
		state[r] = graph.resources[r].initialUsage;
	// Cross-queue ordering. Per state owner: the queue of its last access,
	// and per queue the latest reader since its last write. Per queue: the
	// latest pass on the other queue already known complete before the
	// queue's next pass (its waits so far, and what those producers knew).
	constexpr std::size_t kQueues = 2;
	auto queueOf = []( Queue queue )
	{
		return static_cast<std::size_t>( queue );
	};
	std::vector<std::int64_t> lastQueue( keyCount, -1 );
	std::vector<std::array<std::int64_t, kQueues>> lastReader( keyCount, { -1, -1 } );
	std::vector<std::size_t> compiledQueue;
	std::vector<std::array<std::int64_t, kQueues>> knownAfter; // per compiled pass
	std::array<std::int64_t, kQueues> lastOnQueue = { -1, -1 };
	for ( std::uint32_t p : graph.trace.kept )
	{
		CompiledPass compiled;
		compiled.declaration = p;
		compiled.queue = graph.passes[p].queue;
		const std::uint32_t index = static_cast<std::uint32_t>( graph.order.size() );
		const std::size_t q = queueOf( compiled.queue );
		const std::size_t other = 1 - q;
		// What this queue knows of the other before any new wait.
		std::array<std::int64_t, kQueues> known = { -1, -1 };
		if ( lastOnQueue[q] >= 0 )
			known = knownAfter[static_cast<std::size_t>( lastOnQueue[q] )];
		std::int64_t need = -1;
		for ( const Access &access : graph.passes[p].accesses )
		{
			const std::size_t k = key( access.resource.index );
			const std::int64_t writer = lastWriter[k];
			if ( writer >= 0 && compiledQueue[static_cast<std::size_t>( writer )] == other )
				need = std::max( need, writer );
			if ( access.write )
				need = std::max( need, lastReader[k][other] );
			if ( lastQueue[k] >= 0 && static_cast<std::size_t>( lastQueue[k] ) != q )
				compiled.acquires.push_back( access.resource );
		}
		if ( need > known[other] )
		{
			compiled.waitFor = static_cast<std::uint32_t>( need );
			graph.trace.waits.push_back( { index, compiled.waitFor } );
			const std::array<std::int64_t, kQueues> &producer =
			    knownAfter[static_cast<std::size_t>( need )];
			known[other] = need;
			known[q] = std::max( known[q], producer[q] );
		}
		known[q] = index;
		knownAfter.push_back( known );
		compiledQueue.push_back( q );
		lastOnQueue[q] = index;
		for ( const Access &access : graph.passes[p].accesses )
		{
			const std::size_t k = key( access.resource.index );
			lastQueue[k] = static_cast<std::int64_t>( q );
			if ( access.write )
				lastReader[k] = { -1, -1 };
			else
				lastReader[k][q] = index;
		}
		for ( const Access &access : graph.passes[p].accesses )
		{
			const std::uint32_t r = access.resource.index;
			const std::size_t k = key( r );
			const bool writeAfterWrite = access.write && state[k] == access.usage &&
			                             lastWriter[k] >= 0 && lastWriter[k] != index;
			if ( state[k] != access.usage || writeAfterWrite )
			{
				compiled.transitions.push_back( { access.resource, state[k], access.usage } );
				graph.trace.transitions.push_back( { index, r, state[k], access.usage } );
				state[k] = access.usage;
			}
			if ( access.write )
				lastWriter[k] = index;
		}
		graph.order.push_back( std::move( compiled ) );
	}
	for ( std::size_t r = 0; r < resourceCount; ++r )
	{
		const ResourceDecl &decl = graph.resources[r];
		const ResourceRef ref{ static_cast<std::uint32_t>( r ) };
		if ( decl.imported )
		{
			if ( decl.finalUsage != device::ResourceUsage::kUndefined &&
			     state[r] != decl.finalUsage )
			{
				graph.finalTransitions.push_back( { ref, state[r], decl.finalUsage } );
				graph.trace.transitions.push_back(
				    { UINT32_MAX, static_cast<std::uint32_t>( r ), state[r], decl.finalUsage } );
			}
			continue;
		}
		if ( first[r] >= 0 )
			graph.transients.push_back( { ref, static_cast<std::uint32_t>( first[r] ),
			    static_cast<std::uint32_t>( last[r] ), usages[r] } );
	}
	return graph;
}

} // namespace render::graph
