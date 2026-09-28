//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 compiled-graph validation (RFC 0016); see
//			validate.h.
//
//=============================================================================//

#include "render/graph/validate.h"

#include <algorithm>

namespace render::graph
{

const char *DescribeViolation( ViolationKind kind )
{
	switch ( kind )
	{
	case ViolationKind::kMissingTransition:
		return "missing transition";
	case ViolationKind::kOverlappingAliases:
		return "overlapping aliases";
	case ViolationKind::kCulledSideEffect:
		return "culled side effect";
	case ViolationKind::kReorderedDependency:
		return "reordered dependency";
	case ViolationKind::kUndefinedRead:
		return "undefined read";
	}
	return "unknown";
}

std::vector<GraphViolation> ValidateCompiledGraph( const CompiledGraph &graph )
{
	std::vector<GraphViolation> out;
	const std::size_t resourceCount = graph.resources.size();
	auto physicalOf = [&]( std::uint32_t r ) -> std::int32_t
	{
		return r < graph.physicalOf.size() ? graph.physicalOf[r] : -1;
	};
	auto key = [&]( std::uint32_t r ) -> std::size_t
	{
		const std::int32_t p = physicalOf( r );
		return p >= 0 ? resourceCount + static_cast<std::size_t>( p ) : r;
	};

	// Usages reached by the recorded transitions.
	std::vector<device::ResourceUsage> state(
	    resourceCount + graph.physical.size(), device::ResourceUsage::kUndefined );
	for ( std::size_t r = 0; r < resourceCount; ++r )
		state[r] = graph.resources[r].initialUsage;
	std::vector<bool> written( resourceCount, false );
	std::vector<std::int64_t> first( resourceCount, -1 );
	std::vector<std::int64_t> last( resourceCount, -1 );
	// The pass that last wrote each state owner: a write in a later pass
	// needs a transition even in the same usage (write after write).
	std::vector<std::int64_t> lastWriter( state.size(), -1 );
	for ( std::size_t i = 0; i < graph.order.size(); ++i )
	{
		const CompiledPass &compiled = graph.order[i];
		std::vector<bool> transitioned( state.size(), false );
		for ( const Transition &t : compiled.transitions )
		{
			const std::size_t k = key( t.resource.index );
			if ( t.before != device::ResourceUsage::kUndefined && t.before != state[k] )
				out.push_back(
				    { ViolationKind::kMissingTransition, compiled.declaration, t.resource.index } );
			state[k] = t.after;
			transitioned[k] = true;
		}
		for ( const Access &access : graph.passes[compiled.declaration].accesses )
		{
			const std::uint32_t r = access.resource.index;
			const std::size_t k = key( r );
			const bool unorderedWrite = access.write && lastWriter[k] >= 0 &&
			                            lastWriter[k] != static_cast<std::int64_t>( i ) &&
			                            !transitioned[k];
			if ( state[k] != access.usage || unorderedWrite )
				out.push_back( { ViolationKind::kMissingTransition, compiled.declaration, r } );
			if ( access.write )
				lastWriter[k] = static_cast<std::int64_t>( i );
			if ( !graph.resources[r].imported && !access.write && !written[r] )
				out.push_back( { ViolationKind::kUndefinedRead, compiled.declaration, r } );
			if ( access.write )
				written[r] = true;
			if ( first[r] < 0 )
				first[r] = static_cast<std::int64_t>( i );
			last[r] = static_cast<std::int64_t>( i );
		}
	}

	// Aliases: members of one physical resource never live at once.
	for ( std::size_t p = 0; p < graph.physical.size(); ++p )
	{
		std::vector<std::uint32_t> members;
		for ( std::size_t r = 0; r < resourceCount; ++r )
		{
			if ( physicalOf( static_cast<std::uint32_t>( r ) ) == static_cast<std::int32_t>( p ) &&
			     first[r] >= 0 )
				members.push_back( static_cast<std::uint32_t>( r ) );
		}
		for ( std::size_t a = 0; a < members.size(); ++a )
		{
			for ( std::size_t b = a + 1; b < members.size(); ++b )
			{
				const std::uint32_t x = members[a];
				const std::uint32_t y = members[b];
				if ( first[x] <= last[y] && first[y] <= last[x] )
					out.push_back( { ViolationKind::kOverlappingAliases, 0, y } );
			}
		}
	}

	// Side effects are kept.
	std::vector<std::int64_t> position( graph.passes.size(), -1 );
	for ( std::size_t i = 0; i < graph.order.size(); ++i )
		position[graph.order[i].declaration] = static_cast<std::int64_t>( i );
	for ( std::size_t p = 0; p < graph.passes.size(); ++p )
	{
		if ( graph.passes[p].sideEffect && position[p] < 0 )
			out.push_back(
			    { ViolationKind::kCulledSideEffect, static_cast<std::uint32_t>( p ), 0 } );
	}

	// Conflicting kept passes keep declaration order.
	for ( std::size_t a = 0; a < graph.passes.size(); ++a )
	{
		if ( position[a] < 0 )
			continue;
		for ( std::size_t b = a + 1; b < graph.passes.size(); ++b )
		{
			if ( position[b] < 0 || position[a] < position[b] )
				continue;
			bool conflict = false;
			for ( const Access &x : graph.passes[a].accesses )
			{
				for ( const Access &y : graph.passes[b].accesses )
					conflict |= key( x.resource.index ) == key( y.resource.index ) &&
					            ( x.write || y.write );
			}
			if ( conflict )
				out.push_back(
				    { ViolationKind::kReorderedDependency, static_cast<std::uint32_t>( b ), 0 } );
		}
	}
	return out;
}

} // namespace render::graph
