//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 fixtures shared by the graph suites (RFC 0016
//			K2): seeded random graphs over any device, and an independent
//			reference model of culling, lifetimes, aliasing and transitions.
//			The model shares no code with the compiler: culling is a fixpoint
//			instead of one backward sweep, and transitions are computed per
//			resource (or per physical resource) instead of per pass.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_GRAPH_FIXTURES_H
#define RENDERTEST_CORE_GRAPH_FIXTURES_H

#include "render/device/device.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"

#include <algorithm>
#include <functional>
#include <cstdint>
#include <random>
#include <tuple>
#include <vector>

namespace rendertest::graph
{

using namespace render;
using namespace render::graph;
using device::ResourceUsage;

inline void Noop( RecordContext & )
{
}

inline device::TextureDesc Color( std::uint32_t size = 8 )
{
	device::TextureDesc desc;
	desc.format = device::Format::kRGBA8Unorm;
	desc.width = size;
	desc.height = size;
	return desc;
}

inline device::UsageSet Everything()
{
	device::UsageSet everything;
	for ( int u = 1; u < static_cast<int>( ResourceUsage::kCount ); ++u )
	{
		if ( static_cast<ResourceUsage>( u ) != ResourceUsage::kPresent )
			everything.Add( static_cast<ResourceUsage>( u ) );
	}
	return everything;
}

// The usages the random graphs use, which imported resources are created
// with (a real color texture cannot take depth usages).
inline device::UsageSet RandomTextureUsages()
{
	return { ResourceUsage::kSampled, ResourceUsage::kCopySource, ResourceUsage::kColorAttachment,
	    ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite };
}

inline device::UsageSet RandomBufferUsages()
{
	return { ResourceUsage::kVertex, ResourceUsage::kUniform, ResourceUsage::kCopySource,
	    ResourceUsage::kStorageRead, ResourceUsage::kCopyDestination,
	    ResourceUsage::kStorageWrite };
}

// Makes each pass's execute function from its accesses (the Vulkan lane
// records real work for them); empty uses the one `execute` for every pass.
using MakeExecuteFn =
    std::function<ExecuteFn( const std::vector<Access> &, const std::vector<ResourceDecl> & )>;

struct RandomGraph
{
	GraphBuilder builder;
	std::vector<ResourceDecl> resources;
	std::vector<PassDecl> passes;
};

// Imports are created on device (with the usages above); transients
// come in two texture shapes and one buffer shape, so aliasing happens.
inline RandomGraph MakeRandomGraph( std::uint32_t seed, device::IRenderDevice2 &device,
    ExecuteFn execute = Noop, const MakeExecuteFn &makeExecute = {} )
{
	const ResourceUsage textureReads[] = { ResourceUsage::kSampled, ResourceUsage::kCopySource };
	const ResourceUsage textureWrites[] = { ResourceUsage::kColorAttachment,
	    ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite };
	const ResourceUsage bufferReads[] = { ResourceUsage::kVertex, ResourceUsage::kUniform,
	    ResourceUsage::kCopySource, ResourceUsage::kStorageRead };
	const ResourceUsage bufferWrites[] = {
	    ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite };
	std::mt19937 random( seed );
	auto pick = [&]( int n )
	{
		return static_cast<int>( random() % static_cast<std::uint32_t>( n ) );
	};
	RandomGraph out;
	const int resourceCount = 2 + pick( 8 );
	std::vector<bool> isTexture;
	std::vector<bool> imported;
	for ( int r = 0; r < resourceCount; ++r )
	{
		const bool texture = pick( 3 ) != 0;
		const bool import = pick( 4 ) == 0;
		isTexture.push_back( texture );
		imported.push_back( import );
		if ( texture )
		{
			device::TextureDesc desc = Color( pick( 2 ) ? 8 : 16 );
			if ( import )
			{
				desc.usages = RandomTextureUsages();
				out.builder.ImportTexture( "import", device.CreateTexture( desc ).Value(), desc,
				    ResourceUsage::kUndefined,
				    pick( 2 ) ? ResourceUsage::kSampled : ResourceUsage::kUndefined );
			}
			else
			{
				out.builder.CreateTexture( "transient", desc );
			}
		}
		else
		{
			device::BufferDesc desc;
			desc.size = 256;
			if ( import )
			{
				desc.usages = RandomBufferUsages();
				out.builder.ImportBuffer( "import", device.CreateBuffer( desc ).Value(), desc,
				    ResourceUsage::kUndefined, ResourceUsage::kUndefined );
			}
			else
			{
				out.builder.CreateBuffer( "transient", desc );
			}
		}
	}
	std::vector<bool> written( resourceCount, false );
	const int passCount = 1 + pick( 12 );
	for ( int p = 0; p < passCount; ++p )
	{
		PassBuilder pass = out.builder.AddPass( "pass", PassKind::kRender );
		std::vector<bool> used( resourceCount, false );
		const int accesses = 1 + pick( 3 );
		for ( int a = 0; a < accesses; ++a )
		{
			const int r = pick( resourceCount );
			if ( used[r] )
				continue;
			used[r] = true;
			const bool canRead = imported[r] || written[r];
			const bool write = !canRead || pick( 2 ) == 0;
			const ResourceRef ref{ static_cast<std::uint32_t>( r ) };
			if ( isTexture[r] )
			{
				if ( write )
					pass.Write( ref, textureWrites[pick( 3 )] );
				else
					pass.Read( ref, textureReads[pick( 2 )] );
			}
			else
			{
				if ( write )
					pass.Write( ref, bufferWrites[pick( 2 )] );
				else
					pass.Read( ref, bufferReads[pick( 4 )] );
			}
			written[r] = written[r] || write;
		}
		if ( pick( 4 ) == 0 )
			pass.SideEffect();
		if ( makeExecute )
			pass.Execute(
			    makeExecute( out.builder.Passes().back().accesses, out.builder.Resources() ) );
		else
			pass.Execute( execute );
	}
	out.resources = out.builder.Resources();
	out.passes = out.builder.Passes();
	return out;
}

struct Model
{
	std::vector<std::uint32_t> kept;
	std::vector<TraceTransition> transitions; // sorted by (pass, resource)
	std::vector<std::vector<std::uint32_t>> aliasSets;
};

inline bool SameShape( const ResourceDecl &a, const ResourceDecl &b )
{
	if ( a.isTexture != b.isTexture )
		return false;
	if ( a.isTexture )
		return a.texture.format == b.texture.format && a.texture.width == b.texture.width &&
		       a.texture.height == b.texture.height && a.texture.dimension == b.texture.dimension &&
		       a.texture.depthOrLayers == b.texture.depthOrLayers &&
		       a.texture.mipLevels == b.texture.mipLevels &&
		       a.texture.sampleCount == b.texture.sampleCount;
	return a.buffer.size == b.buffer.size && a.buffer.memory == b.buffer.memory;
}

inline Model Reference(
    const std::vector<ResourceDecl> &resources, const std::vector<PassDecl> &passes )
{
	// Culling as a fixpoint.
	std::vector<bool> keep( passes.size(), false );
	for ( std::size_t p = 0; p < passes.size(); ++p )
	{
		keep[p] = passes[p].sideEffect;
		for ( const Access &a : passes[p].accesses )
			keep[p] = keep[p] || ( a.write && resources[a.resource.index].imported );
	}
	for ( bool changed = true; changed; )
	{
		changed = false;
		for ( std::size_t p = 0; p < passes.size(); ++p )
		{
			if ( keep[p] )
				continue;
			for ( const Access &a : passes[p].accesses )
			{
				if ( !a.write )
					continue;
				for ( std::size_t q = p + 1; q < passes.size() && !keep[p]; ++q )
				{
					if ( !keep[q] )
						continue;
					for ( const Access &b : passes[q].accesses )
						keep[p] = keep[p] || b.resource == a.resource;
				}
			}
			changed |= keep[p];
		}
	}
	Model model;
	for ( std::size_t p = 0; p < passes.size(); ++p )
	{
		if ( keep[p] )
			model.kept.push_back( static_cast<std::uint32_t>( p ) );
	}

	// Lifetimes, then slots: a transient reuses the first earlier slot of
	// its shape whose members have all ended before it starts.
	const std::size_t n = resources.size();
	std::vector<int> firstUse( n, -1 );
	std::vector<int> lastUse( n, -1 );
	for ( std::size_t k = 0; k < model.kept.size(); ++k )
	{
		for ( const Access &a : passes[model.kept[k]].accesses )
		{
			const std::uint32_t r = a.resource.index;
			if ( firstUse[r] < 0 )
				firstUse[r] = static_cast<int>( k );
			lastUse[r] = static_cast<int>( k );
		}
	}
	std::vector<std::uint32_t> transients;
	for ( std::uint32_t r = 0; r < n; ++r )
	{
		if ( !resources[r].imported && firstUse[r] >= 0 )
			transients.push_back( r );
	}
	std::stable_sort( transients.begin(), transients.end(),
	    [&]( std::uint32_t a, std::uint32_t b )
	    {
		    return firstUse[a] < firstUse[b];
	    } );
	std::vector<int> slotEnd;
	std::vector<int> slotOf( n, -1 );
	for ( std::uint32_t r : transients )
	{
		int chosen = -1;
		for ( std::size_t s = 0; s < model.aliasSets.size() && chosen < 0; ++s )
		{
			if ( slotEnd[s] < firstUse[r] &&
			     SameShape( resources[model.aliasSets[s][0]], resources[r] ) )
				chosen = static_cast<int>( s );
		}
		if ( chosen < 0 )
		{
			model.aliasSets.push_back( {} );
			slotEnd.push_back( -1 );
			chosen = static_cast<int>( model.aliasSets.size() - 1 );
		}
		model.aliasSets[chosen].push_back( r );
		slotEnd[chosen] = lastUse[r];
		slotOf[r] = chosen;
	}

	// Transitions per state owner: an import alone, or a slot's members in turn.
	std::vector<std::vector<std::uint32_t>> owners;
	for ( std::uint32_t r = 0; r < n; ++r )
	{
		if ( resources[r].imported )
			owners.push_back( { r } );
	}
	for ( const std::vector<std::uint32_t> &members : model.aliasSets )
		owners.push_back( members );
	for ( const std::vector<std::uint32_t> &members : owners )
	{
		ResourceUsage usage = resources[members[0]].initialUsage;
		bool writtenBefore = false;
		for ( std::size_t k = 0; k < model.kept.size(); ++k )
		{
			bool wroteHere = false;
			for ( const Access &a : passes[model.kept[k]].accesses )
			{
				if ( std::find( members.begin(), members.end(), a.resource.index ) ==
				     members.end() )
					continue;
				if ( usage != a.usage || ( a.write && writtenBefore ) )
					model.transitions.push_back(
					    { static_cast<std::uint32_t>( k ), a.resource.index, usage, a.usage } );
				usage = a.usage;
				wroteHere = wroteHere || a.write;
			}
			writtenBefore = writtenBefore || wroteHere;
		}
		const ResourceDecl &decl = resources[members[0]];
		if ( decl.imported && decl.finalUsage != ResourceUsage::kUndefined &&
		     usage != decl.finalUsage )
			model.transitions.push_back( { UINT32_MAX, members[0], usage, decl.finalUsage } );
	}
	std::stable_sort( model.transitions.begin(), model.transitions.end(),
	    []( const TraceTransition &a, const TraceTransition &b )
	    {
		    return std::make_tuple( a.pass, a.resource ) < std::make_tuple( b.pass, b.resource );
	    } );
	return model;
}

// The compiler's trace in the model's order.
inline std::vector<TraceTransition> SortedTransitions( const CompiledGraph &graph )
{
	std::vector<TraceTransition> sorted = graph.trace.transitions;
	std::stable_sort( sorted.begin(), sorted.end(),
	    []( const TraceTransition &a, const TraceTransition &b )
	    {
		    return std::make_tuple( a.pass, a.resource ) < std::make_tuple( b.pass, b.resource );
	    } );
	return sorted;
}

} // namespace rendertest::graph

#endif // RENDERTEST_CORE_GRAPH_FIXTURES_H
