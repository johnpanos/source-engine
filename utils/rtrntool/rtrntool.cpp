//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: rtrntool (RFC 0011 G4): runs the product's precomputed radiosity
//          solver (public/render/indirect_radiosity.h) on a baked map's PRBV
//          and RTRN, for the gate's convergence scenarios and CPU budgets.
//
//   rtrntool info <volume.prbv> <transfer.rtrn>
//   rtrntool converge <volume.prbv> <transfer.rtrn> [options]
//       Runs updates from a start state and prints, per update, the mean
//       indirect light (luminance) of the selected probes that gather patches
//       (absolute) or its change from the bake, then the frames until it came
//       and stayed within --tolerance of --expect (or of its settled value).
//         --absolute        measure from zero light (a cold start) instead of
//                           the bake's scalars
//         --set S=V         source S (index or name) at scalar V (repeatable)
//         --one-bounce      the one-bounce sensitivity defect
//         --iterations K    Jacobi iterations per update (default: the product's)
//         --frames N        updates to run (default 240)
//         --region X0 Y0 Z0 X1 Y1 Z1  probes inside this box (Source units)
//         --expect V        the expected mean indirect light
//         --tolerance T     relative tolerance (default 0.02)
//   rtrntool bench <volume.prbv> <transfer.rtrn> [--set S=V ...] [--frames N]
//       Per-update CPU time (update plus composition) on one thread while
//       converging from the bake to the set scalars: median and maximum.
//
// Output is one JSON document on stdout.
//
//===========================================================================//

#include "render/indirect_radiosity.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace
{
using namespace indirect_light;

std::vector<unsigned char> Load( const char *path )
{
	std::vector<unsigned char> bytes;
	if ( std::FILE *file = std::fopen( path, "rb" ) )
	{
		unsigned char block[65536];
		size_t read;
		while ( ( read = std::fread( block, 1, sizeof( block ), file ) ) > 0 )
			bytes.insert( bytes.end(), block, block + read );
		std::fclose( file );
	}
	return bytes;
}

struct Options
{
	bool absolute = false;
	bool oneBounce = false;
	int iterations = -1;
	int frames = 240;
	bool region = false;
	float box[6] = {};
	bool expect = false;
	float expected = 0.0f;
	float tolerance = 0.02f;
	std::vector<std::pair<std::string, float>> sets;
};

// Probe positions (Source units, relocated) and activity, in PRBV order.
void ProbePositions(
    const Volume &volume, std::vector<float> *positions, std::vector<bool> *active )
{
	const mapcontainer::ProbeVolumeLayout &layout = volume.layout;
	for ( uint32_t g = 0; g < layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
		const uint32_t row = grid.tilesPerRow * mapcontainer::kProbeVisibilityTile;
		for ( uint32_t i = 0; i < grid.probeCount; ++i )
		{
			const uint32_t index[3] = { i % grid.dims[0], ( i / grid.dims[0] ) % grid.dims[1],
			    i / ( grid.dims[0] * grid.dims[1] ) };
			const unsigned char *state =
			    volume.bytes.data() + layout.atlasOffset +
			    ( uint64_t( grid.stateOrigin[1] + i / row ) * layout.atlasWidth +
			        grid.stateOrigin[0] + i % row ) *
			        8;
			uint16_t half[4];
			std::memcpy( half, state, sizeof( half ) );
			for ( int k = 0; k < 3; ++k )
				positions->push_back( grid.origin[k] + float( index[k] ) * grid.spacing[k] +
				                      mapcontainer::HalfToFloat( half[k] ) );
			active->push_back( mapcontainer::HalfToFloat( half[3] ) >= 0.5f );
		}
	}
}

int SourceIndex( const Transfer &transfer, const std::string &name )
{
	const auto view = transfer.View();
	for ( uint32_t s = 0; s < transfer.layout.sourceCount; ++s )
		if ( name == view.Source( s ).name )
			return int( s );
	char *end = nullptr;
	const long index = std::strtol( name.c_str(), &end, 10 );
	return end && *end == 0 && index >= 0 && index < long( transfer.layout.sourceCount )
	           ? int( index )
	           : -1;
}

std::vector<float> Scalars( const Transfer &transfer, const Options &options, bool *ok )
{
	std::vector<float> scalars( transfer.layout.sourceCount, 1.0f );
	for ( const auto &set : options.sets )
	{
		const int s = SourceIndex( transfer, set.first );
		if ( s < 0 )
		{
			std::fprintf( stderr, "rtrntool: unknown source '%s'\n", set.first.c_str() );
			*ok = false;
		}
		else
			scalars[size_t( s )] = set.second;
	}
	return scalars;
}

int Info( const Volume &volume, const Transfer &transfer )
{
	const auto view = transfer.View();
	const auto &t = transfer.layout;
	std::printf(
	    "{\"patches\": %u, \"probes\": %u, \"transfer_links\": %u, \"injection_links\": "
	    "%u, \"gather_links\": %u, \"rtrn_bytes\": %zu, \"prbv_bytes\": %zu, \"sources\": [",
	    t.patchCount, t.probeCount, t.transferLinks, t.injectionLinks, t.gatherLinks,
	    transfer.bytes.size(), volume.bytes.size() );
	for ( uint32_t s = 0; s < t.sourceCount; ++s )
		std::printf( "%s{\"name\": \"%s\", \"kind\": %u, \"style\": %d}", s ? ", " : "",
		    view.Source( s ).name, view.Source( s ).kind, view.Source( s ).style );
	std::printf( "]}\n" );
	return 0;
}

int Converge(
    const Volume &volume, const std::shared_ptr<const Transfer> &transfer, const Options &options )
{
	bool ok = true;
	const std::vector<float> scalars = Scalars( *transfer, options, &ok );
	if ( !ok )
		return 2;
	RadiosityOptions solverOptions;
	solverOptions.oneBounce = options.oneBounce;
	if ( options.iterations > 0 )
		solverOptions.iterationsPerUpdate = uint32_t( options.iterations );
	RadiositySolver solver;
	solver.Init( transfer,
	    std::vector<float>( transfer->layout.sourceCount, options.absolute ? 0.0f : 1.0f ),
	    solverOptions );
	solver.SetScalars( scalars );
	std::vector<float> positions;
	std::vector<bool> active;
	ProbePositions( volume, &positions, &active );
	const auto view = transfer->View();
	std::vector<uint32_t> selected;
	for ( uint32_t i = 0; i < transfer->layout.probeCount; ++i )
	{
		const float *p = &positions[size_t( i ) * 3];
		const bool inside =
		    !options.region ||
		    ( p[0] >= options.box[0] && p[1] >= options.box[1] && p[2] >= options.box[2] &&
		        p[0] <= options.box[3] && p[1] <= options.box[4] && p[2] <= options.box[5] );
		if ( active[i] && inside && view.GatherBegin( i ) != view.GatherEnd( i ) )
			selected.push_back( i );
	}
	// Absolute: the probes' indirect light; otherwise its change from the bake.
	std::vector<double> means;
	for ( int frame = 0; frame < options.frames; ++frame )
	{
		solver.Update( nullptr );
		double sum = 0.0;
		for ( uint32_t i : selected )
		{
			const float *indirect = solver.ProbeIndirect( i );
			for ( uint32_t t = 0; t < RadiositySolver::kTexels; ++t )
				sum += 0.2126 * indirect[t * 3] + 0.7152 * indirect[t * 3 + 1] +
				       0.0722 * indirect[t * 3 + 2];
		}
		means.push_back( selected.empty()
		                     ? 0.0
		                     : sum / ( double( selected.size() ) * RadiositySolver::kTexels ) );
	}
	const double settled = means.empty() ? 0.0 : means.back();
	const double target = options.expect ? options.expected : settled;
	int reached = -1;
	for ( int frame = int( means.size() ) - 1; frame >= 0; --frame )
	{
		// The first frame after which every frame stays within tolerance.
		if ( std::fabs( means[size_t( frame )] - target ) >
		     options.tolerance * std::fabs( target ) )
			break;
		reached = frame + 1;
	}
	std::printf( "{\"mode\": \"%s\", \"one_bounce\": %s, \"iterations_per_update\": %u, "
	             "\"probes\": %zu, \"frames\": %d, \"settled\": %.6f, \"target\": %.6f, "
	             "\"tolerance\": %g, \"frames_to_tolerance\": %d, \"declared_convergence_frames\": "
	             "%u, \"means\": [",
	    options.absolute ? "absolute" : "change", options.oneBounce ? "true" : "false",
	    solverOptions.iterationsPerUpdate, selected.size(), options.frames, settled, target,
	    options.tolerance, reached, RadiosityProducer().Caps().convergenceFrames );
	for ( size_t k = 0; k < means.size() && k < 64; ++k )
		std::printf( "%s%.6f", k ? ", " : "", means[k] );
	std::printf( "]}\n" );
	return 0;
}

int Bench(
    const Volume &volume, const std::shared_ptr<const Transfer> &transfer, const Options &options )
{
	bool ok = true;
	const std::vector<float> scalars = Scalars( *transfer, options, &ok );
	if ( !ok )
		return 2;
	std::vector<double> samples;
	for ( int round = 0; round < 5; ++round )
	{
		RadiositySolver solver;
		solver.Init( transfer, std::vector<float>( transfer->layout.sourceCount, 1.0f ),
		    RadiosityOptions() );
		solver.SetScalars( scalars );
		for ( int frame = 0; frame < options.frames && solver.Dirty(); ++frame )
		{
			const auto start = std::chrono::steady_clock::now();
			solver.Update( nullptr );
			const auto volumeOut = solver.Compose( volume, nullptr );
			samples.push_back( std::chrono::duration<double, std::milli>(
			    std::chrono::steady_clock::now() - start )
			        .count() );
			(void)volumeOut;
		}
	}
	std::sort( samples.begin(), samples.end() );
	const double median = samples.empty() ? 0.0 : samples[samples.size() / 2];
	const double worst = samples.empty() ? 0.0 : samples.back();
	std::printf( "{\"updates\": %zu, \"median_ms\": %.4f, \"max_ms\": %.4f, \"patches\": %u, "
	             "\"transfer_links\": %u, \"gather_links\": %u}\n",
	    samples.size(), median, worst, transfer->layout.patchCount, transfer->layout.transferLinks,
	    transfer->layout.gatherLinks );
	return 0;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc < 4 )
	{
		std::fprintf( stderr, "usage: rtrntool info|converge|bench <volume.prbv> <transfer.rtrn> "
		                      "[options]\n" );
		return 2;
	}
	const std::string command = argv[1];
	const auto volume = Volume::FromBytes( Load( argv[2] ) );
	if ( !volume )
	{
		std::fprintf( stderr, "rtrntool: invalid PRBV %s\n", argv[2] );
		return 2;
	}
	const auto transfer = Transfer::FromBytes( Load( argv[3] ), *volume );
	if ( !transfer )
	{
		std::fprintf(
		    stderr, "rtrntool: invalid RTRN %s (or not baked for %s)\n", argv[3], argv[2] );
		return 2;
	}
	Options options;
	for ( int i = 4; i < argc; ++i )
	{
		const std::string arg = argv[i];
		if ( arg == "--absolute" )
			options.absolute = true;
		else if ( arg == "--one-bounce" )
			options.oneBounce = true;
		else if ( arg == "--iterations" && i + 1 < argc )
			options.iterations = std::atoi( argv[++i] );
		else if ( arg == "--frames" && i + 1 < argc )
			options.frames = std::atoi( argv[++i] );
		else if ( arg == "--expect" && i + 1 < argc )
		{
			options.expect = true;
			options.expected = float( std::atof( argv[++i] ) );
		}
		else if ( arg == "--tolerance" && i + 1 < argc )
			options.tolerance = float( std::atof( argv[++i] ) );
		else if ( arg == "--region" && i + 6 < argc )
		{
			options.region = true;
			for ( float &v : options.box )
				v = float( std::atof( argv[++i] ) );
		}
		else if ( arg == "--set" && i + 1 < argc )
		{
			const std::string value = argv[++i];
			const size_t equals = value.find( '=' );
			if ( equals == std::string::npos )
			{
				std::fprintf( stderr, "rtrntool: --set wants SOURCE=VALUE\n" );
				return 2;
			}
			options.sets.emplace_back(
			    value.substr( 0, equals ), float( std::atof( value.c_str() + equals + 1 ) ) );
		}
		else
		{
			std::fprintf( stderr, "rtrntool: unknown option %s\n", arg.c_str() );
			return 2;
		}
	}
	if ( command == "info" )
		return Info( *volume, *transfer );
	if ( command == "converge" )
		return Converge( *volume, transfer, options );
	if ( command == "bench" )
		return Bench( *volume, transfer, options );
	std::fprintf( stderr, "rtrntool: unknown command %s\n", command.c_str() );
	return 2;
}
