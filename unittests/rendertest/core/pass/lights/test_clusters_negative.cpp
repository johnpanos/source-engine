//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lights.clusters.sensitivity (RFC 0016 K7): the oracles of
//			render.lights.clusters against deliberately bad builders. Each is
//			the real builder with one seeded defect in its grid, its input or
//			its output, and each must be detected on the seeded scenes; the
//			real builder on the same scenes must not be.
//
//			slice-off-by-one      the assignment's slice boundaries shifted by one
//			range-squared         a sphere test of distance squared against range
//			y-flipped             rows mirrored (pixel row 0 read as clip -Y)
//			column-off-by-one     columns shifted by one
//			spot-half-angle-halved   the outer cone's half-angle halved
//			spot-half-angle-doubled  the outer cone's half-angle doubled (only
//			                         over-lists: caught by the false-positive ceiling)
//			near-plane-cull       lights whose center is nearer than the near plane dropped
//			unbounded-as-zero     radius 0 (unbounded) read as reaching nothing
//			overflow-silent       lights dropped for capacity without being counted
//
//=============================================================================//

#include "cluster_oracle.h"

#include "testing/checks.h"

#include <cstdio>
#include <string>

namespace
{

using namespace cluster_oracle;
namespace lights = render::pass::lights;

constexpr int kMaxScenes = 300;
constexpr int kOverflowScenes = 60;

BuildOutput BuildOn( const Scene &scene, const std::vector<RuntimeLight> &lights,
    void ( *tamperGrid )( ClusterGrid & ) = nullptr )
{
	BuildOutput out;
	auto grid = lights::CreateClusterGrid( scene.desc, scene.limits );
	if ( !grid )
		return out;
	out.grid = grid.Value();
	ClusterGrid used = grid.Value();
	if ( tamperGrid )
		tamperGrid( used );
	auto stats = lights::AssignLights( used, lights, out.lists );
	if ( !stats )
		return out;
	out.stats = stats.Value();
	out.ok = true;
	return out;
}

// Rebuilds the lists with each froxel taking the list of another (or none).
void Remap( BuildOutput &out,
    const std::function<std::int64_t( std::uint32_t x, std::uint32_t y, std::uint32_t k )> &source )
{
	const ClusterGrid &g = out.grid;
	ClusterLists remapped;
	remapped.froxels.resize( g.FroxelCount() );
	for ( std::uint32_t k = 0; k < g.slices; ++k )
		for ( std::uint32_t y = 0; y < g.tilesY; ++y )
			for ( std::uint32_t x = 0; x < g.tilesX; ++x )
			{
				const std::uint32_t f = g.FroxelIndex( x, y, k );
				const std::int64_t from = source( x, y, k );
				remapped.froxels[f].offset = std::uint32_t( remapped.lightIndices.size() );
				if ( from < 0 )
					continue;
				const auto &range = out.lists.froxels[std::size_t( from )];
				for ( std::uint32_t i = 0; i < range.count; ++i )
					remapped.lightIndices.push_back( out.lists.lightIndices[range.offset + i] );
				remapped.froxels[f].count = range.count;
			}
	out.lists = std::move( remapped );
	out.stats.assignments = std::uint32_t( out.lists.lightIndices.size() );
}

// Removes the lights a predicate names from every list.
void Drop( BuildOutput &out, const std::function<bool( std::uint32_t )> &dropped )
{
	ClusterLists kept;
	kept.froxels.resize( out.lists.froxels.size() );
	for ( std::size_t f = 0; f < out.lists.froxels.size(); ++f )
	{
		const auto &range = out.lists.froxels[f];
		kept.froxels[f].offset = std::uint32_t( kept.lightIndices.size() );
		for ( std::uint32_t i = 0; i < range.count; ++i )
		{
			const std::uint32_t light = out.lists.lightIndices[range.offset + i];
			if ( !dropped( light ) )
				kept.lightIndices.push_back( light );
		}
		kept.froxels[f].count = std::uint32_t( kept.lightIndices.size() ) - kept.froxels[f].offset;
	}
	out.lists = std::move( kept );
	out.stats.assignments = std::uint32_t( out.lists.lightIndices.size() );
}

std::vector<RuntimeLight> WithCones( const Scene &scene, double factor )
{
	std::vector<RuntimeLight> changed = scene.lights;
	for ( RuntimeLight &light : changed )
	{
		if ( light.shape == LightShape::Spot )
		{
			const double theta = std::acos( std::clamp<double>( light.outerCos, -1.0, 1.0 ) );
			light.outerCos = float( std::cos( std::min( kPi, theta * factor ) ) );
		}
	}
	return changed;
}

struct Defect
{
	const char *name;
	Builder build;
};

std::vector<Defect> Defects()
{
	std::vector<Defect> defects;
	defects.push_back( { "slice-off-by-one", []( const Scene &scene )
	    {
		    return BuildOn( scene, scene.lights,
		        []( ClusterGrid &grid )
		        {
			        for ( std::uint32_t k = 0; k < grid.slices; ++k )
				        grid.sliceDepths[k] = grid.sliceDepths[k + 1];
			        grid.sliceDepths[grid.slices] *= 2.0f;
		        } );
	    } } );
	defects.push_back( { "range-squared", []( const Scene &scene )
	    {
		    std::vector<RuntimeLight> changed = scene.lights;
		    for ( RuntimeLight &light : changed )
			    light.radius = std::sqrt( light.radius );
		    return BuildOn( scene, changed );
	    } } );
	defects.push_back( { "y-flipped", []( const Scene &scene )
	    {
		    BuildOutput out = BuildOn( scene, scene.lights );
		    if ( out.ok )
		    {
			    const ClusterGrid g = out.grid;
			    Remap( out,
			        [&]( std::uint32_t x, std::uint32_t y, std::uint32_t k )
			        {
				        return std::int64_t( g.FroxelIndex( x, g.tilesY - 1 - y, k ) );
			        } );
		    }
		    return out;
	    } } );
	defects.push_back( { "column-off-by-one", []( const Scene &scene )
	    {
		    BuildOutput out = BuildOn( scene, scene.lights );
		    if ( out.ok )
		    {
			    const ClusterGrid g = out.grid;
			    Remap( out,
			        [&]( std::uint32_t x, std::uint32_t y, std::uint32_t k )
			        {
				        return x + 1 < g.tilesX ? std::int64_t( g.FroxelIndex( x + 1, y, k ) ) : -1;
			        } );
		    }
		    return out;
	    } } );
	defects.push_back( { "spot-half-angle-halved", []( const Scene &scene )
	    {
		    return BuildOn( scene, WithCones( scene, 0.5 ) );
	    } } );
	defects.push_back( { "spot-half-angle-doubled", []( const Scene &scene )
	    {
		    return BuildOn( scene, WithCones( scene, 2.0 ) );
	    } } );
	defects.push_back( { "near-plane-cull", []( const Scene &scene )
	    {
		    BuildOutput out = BuildOn( scene, scene.lights );
		    if ( out.ok )
		    {
			    const RefGrid g = MakeRefGrid( scene.desc, scene.limits );
			    Drop( out,
			        [&]( std::uint32_t i )
			        {
				        return -ToView( g, scene.lights[i].position ).z < g.nearZ;
			        } );
		    }
		    return out;
	    } } );
	defects.push_back( { "unbounded-as-zero", []( const Scene &scene )
	    {
		    BuildOutput out = BuildOn( scene, scene.lights );
		    if ( out.ok )
		    {
			    Drop( out,
			        [&]( std::uint32_t i )
			        {
				        return scene.lights[i].radius == 0.0f;
			        } );
		    }
		    return out;
	    } } );
	defects.push_back( { "overflow-silent", []( const Scene &scene )
	    {
		    BuildOutput out = BuildOn( scene, scene.lights );
		    out.stats.lightsOverCapacity = 0;
		    out.stats.froxelsOverflowed = 0;
		    out.stats.assignmentsDropped = 0;
		    return out;
	    } } );
	return defects;
}

bool Detected( const Tally &t )
{
	return t.buildFailures || t.shapeErrors || t.falseNegatives || t.lookupOutside ||
	       t.lookupMisses || t.accountingErrors ||
	       t.PointFalsePositiveRate() > kPointFalsePositiveCeiling ||
	       t.SpotFalsePositiveRate() > kSpotFalsePositiveCeiling;
}

// Runs a builder over the seeded scenes, then the overflow scenes, stopping
// at the first detection. Returns the scenes it took, or 0 when undetected.
int Run( const Builder &build, Tally &tally )
{
	std::mt19937 random( 20260928u );
	for ( int s = 0; s < kMaxScenes; ++s )
	{
		const Scene scene = MakeScene( random );
		CheckScene( scene, build( scene ), random, tally );
		// The false-positive ceilings judge a rate: only after enough scenes.
		const bool found = tally.buildFailures || tally.shapeErrors || tally.falseNegatives ||
		                   tally.lookupOutside || tally.lookupMisses ||
		                   ( s >= 50 && Detected( tally ) );
		if ( found )
			return s + 1;
	}
	for ( int s = 0; s < kOverflowScenes; ++s )
	{
		CheckOverflow( MakeScene( random, 600 ), build, random, tally );
		if ( tally.accountingErrors || tally.buildFailures )
			return kMaxScenes + s + 1;
	}
	return 0;
}

} // namespace

int main()
{
	testing::Checks checks;

	Tally control;
	const int controlDetected = Run( RealBuild, control );
	std::printf( "INFO control: %llu scenes, false positives points %.4f spots %.4f\n",
	    static_cast<unsigned long long>( control.scenes ), control.PointFalsePositiveRate(),
	    control.SpotFalsePositiveRate() );
	if ( !control.first.empty() )
		std::printf( "INFO control finding: %s\n", control.first.c_str() );
	checks.Equal( controlDetected, 0, "the real builder passes every oracle on the seeded scenes" );

	int detected = 0;
	const std::vector<Defect> defects = Defects();
	for ( const Defect &defect : defects )
	{
		Tally tally;
		const int scenes = Run( defect.build, tally );
		std::printf( "INFO %s: %s after %d scenes (%s; false positives points %.4f spots %.4f)\n",
		    defect.name, scenes ? "detected" : "NOT detected", scenes,
		    tally.first.empty() ? "false-positive ceiling" : tally.first.c_str(),
		    tally.PointFalsePositiveRate(), tally.SpotFalsePositiveRate() );
		const std::string what = std::string( "seeded defect detected: " ) + defect.name;
		detected += checks.That( scenes != 0, what ) ? 1 : 0;
	}
	std::printf( "INFO %d of %zu seeded defects detected\n", detected, defects.size() );
	return checks.Report();
}
