//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's suite dispatch; see suites.h.
//
//=============================================================================//

#include "suites.h"

#include <cstdio>
#include <string>

namespace render::lab
{

int RunSuite( int argc, char **argv )
{
	if ( argc < 1 )
	{
		std::fprintf( stderr, "render_lab suite: name a suite (debug-views, lighting-controls)\n" );
		return 2;
	}
	const std::string name = argv[0];
	if ( name == "debug-views" )
		return RunDebugViewsSuite( argc - 1, argv + 1 );
	if ( name == "lighting-controls" )
		return RunLightingControlsSuite( argc - 1, argv + 1 );
	if ( name == "area-lights" )
		return RunAreaLightsSuite( argc - 1, argv + 1 );
	if ( name == "lightmap-basis" )
		return RunLightmapBasisSuite( argc - 1, argv + 1 );
	if ( name == "probe-volume" )
		return RunProbeVolumeSuite( argc - 1, argv + 1 );
	if ( name == "reflection-probes" )
		return RunReflectionProbesSuite( argc - 1, argv + 1 );
	if ( name == "volumetric" )
		return RunVolumetricSuite( argc - 1, argv + 1 );
	if ( name == "clustered-lights" )
		return RunClusteredLightsSuite( argc - 1, argv + 1 );
	if ( name == "map-terms" )
		return RunMapTermsSuite( argc - 1, argv + 1 );
	if ( name == "ssr" )
		return RunSsrSuite( argc - 1, argv + 1 );
	if ( name == "shadowed-lights" )
		return RunShadowedLightsSuite( argc - 1, argv + 1 );
	if ( name == "bounce" )
		return RunBounceSuite( argc - 1, argv + 1 );
	if ( name == "gtao" )
		return RunGtaoSuite( argc - 1, argv + 1 );
	if ( name == "panel" )
		return RunPanelSuite( argc - 1, argv + 1 );
	if ( name == "posed-model" )
		return RunPosedModelSuite( argc - 1, argv + 1 );
	if ( name == "view-state" )
		return RunViewStateSuite( argc - 1, argv + 1 );
	if ( name == "model-selection" )
		return RunModelSelectionSuite( argc - 1, argv + 1 );
	std::fprintf( stderr, "render_lab suite: no suite %s\n", name.c_str() );
	return 2;
}

} // namespace render::lab
