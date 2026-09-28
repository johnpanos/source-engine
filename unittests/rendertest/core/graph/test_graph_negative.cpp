//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1.sensitivity (RFC 0016 K2): the five bad graphs.
//			Each defect is seeded into compiled graphs (every random graph
//			where it applies, 1,000 seeds) and ValidateCompiledGraph must
//			report it with its kind every time:
//
//			- a missing transition (one dropped, write-after-write included);
//			- overlapping live aliases (two transients that overlap put on one
//			  physical resource);
//			- a culled side-effect pass;
//			- a reordered dependent pass (two conflicting passes swapped);
//			- a read of an undefined version (the writer of a transient that a
//			  later pass reads removed).
//
//			The unseeded graphs validate (the control).
//
//=============================================================================//

#include "graph_fixtures.h"
#include "render/device/null/provider.h"
#include "render/graph/validate.h"
#include "testing/checks.h"

#include <cstdio>
#include <string>

namespace
{

namespace fixtures = rendertest::graph;
using namespace render::graph;

bool Reports( const CompiledGraph &graph, ViolationKind kind )
{
	for ( const GraphViolation &violation : ValidateCompiledGraph( graph ) )
	{
		if ( violation.kind == kind )
			return true;
	}
	return false;
}

bool Conflict( const CompiledGraph &graph, std::size_t a, std::size_t b )
{
	for ( const Access &x : graph.passes[graph.order[a].declaration].accesses )
	{
		for ( const Access &y : graph.passes[graph.order[b].declaration].accesses )
		{
			if ( ( x.write || y.write ) && x.resource == y.resource )
				return true;
		}
	}
	return false;
}

// Each mutation returns false when it does not apply to the graph.
bool DropTransition( CompiledGraph &graph )
{
	for ( CompiledPass &pass : graph.order )
	{
		if ( !pass.transitions.empty() )
		{
			pass.transitions.erase( pass.transitions.begin() );
			return true;
		}
	}
	return false;
}

bool OverlapAliases( CompiledGraph &graph )
{
	for ( std::size_t x = 0; x < graph.transients.size(); ++x )
	{
		for ( std::size_t y = 0; y < graph.transients.size(); ++y )
		{
			const Lifetime &a = graph.transients[x];
			const Lifetime &b = graph.transients[y];
			const std::int32_t pa = graph.physicalOf[a.resource.index];
			const std::int32_t pb = graph.physicalOf[b.resource.index];
			if ( x == y || pa == pb || a.first > b.last || b.first > a.last )
				continue;
			graph.physicalOf[b.resource.index] = pa;
			graph.physical[pa].members.push_back( b.resource.index );
			return true;
		}
	}
	return false;
}

bool CullSideEffect( CompiledGraph &graph )
{
	for ( auto it = graph.order.begin(); it != graph.order.end(); ++it )
	{
		if ( graph.passes[it->declaration].sideEffect )
		{
			graph.order.erase( it );
			return true;
		}
	}
	return false;
}

bool SwapDependents( CompiledGraph &graph )
{
	for ( std::size_t i = 0; i + 1 < graph.order.size(); ++i )
	{
		if ( Conflict( graph, i, i + 1 ) )
		{
			std::swap( graph.order[i], graph.order[i + 1] );
			return true;
		}
	}
	return false;
}

bool DropWriter( CompiledGraph &graph )
{
	// A transient read by some kept pass whose only earlier writer is one
	// pass: removing that pass leaves the read undefined.
	auto writes = [&]( std::size_t i, ResourceRef r )
	{
		for ( const Access &access : graph.passes[graph.order[i].declaration].accesses )
		{
			if ( access.write && access.resource == r )
				return true;
		}
		return false;
	};
	for ( std::size_t j = 0; j < graph.order.size(); ++j )
	{
		for ( const Access &read : graph.passes[graph.order[j].declaration].accesses )
		{
			if ( read.write || graph.resources[read.resource.index].imported )
				continue;
			std::size_t writers = 0;
			std::size_t writer = 0;
			for ( std::size_t i = 0; i < j; ++i )
			{
				if ( writes( i, read.resource ) )
				{
					++writers;
					writer = i;
				}
			}
			if ( writers == 1 )
			{
				graph.order.erase( graph.order.begin() + static_cast<std::ptrdiff_t>( writer ) );
				return true;
			}
		}
	}
	return false;
}

} // namespace

int main()
{
	testing::Checks checks;
	struct Mutation
	{
		const char *name;
		bool ( *apply )( CompiledGraph & );
		ViolationKind kind;
		int applied = 0;
		int detected = 0;
	};
	Mutation mutations[] = {
	    { "missing-transition", &DropTransition, ViolationKind::kMissingTransition },
	    { "overlapping-aliases", &OverlapAliases, ViolationKind::kOverlappingAliases },
	    { "culled-side-effect", &CullSideEffect, ViolationKind::kCulledSideEffect },
	    { "reordered-dependency", &SwapDependents, ViolationKind::kReorderedDependency },
	    { "undefined-read", &DropWriter, ViolationKind::kUndefinedRead },
	};
	int clean = 0;
	constexpr int kSeeds = 1000;
	for ( int seed = 0; seed < kSeeds; ++seed )
	{
		auto device = render::device::null::Create( {} ).Value();
		fixtures::RandomGraph random =
		    fixtures::MakeRandomGraph( static_cast<std::uint32_t>( seed ), *device );
		auto compiled = CompileGraph( std::move( random.builder ) );
		if ( !compiled )
			continue;
		clean += ValidateCompiledGraph( compiled.Value() ).empty();
		for ( Mutation &mutation : mutations )
		{
			CompiledGraph copy = compiled.Value();
			if ( !mutation.apply( copy ) )
				continue;
			++mutation.applied;
			mutation.detected += Reports( copy, mutation.kind );
		}
	}
	checks.Equal( clean, kSeeds, "control.unseeded-graphs-validate" );
	for ( const Mutation &mutation : mutations )
	{
		std::printf( "INFO graph sensitivity: %s seeded into %d graphs, detected %d\n",
		    mutation.name, mutation.applied, mutation.detected );
		checks.That( mutation.applied >= 50, std::string( "seeded " ) + mutation.name );
		checks.Equal(
		    mutation.detected, mutation.applied, std::string( "detects " ) + mutation.name );
	}
	return checks.Report();
}
