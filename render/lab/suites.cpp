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
	std::fprintf( stderr, "render_lab suite: no suite %s\n", name.c_str() );
	return 2;
}

} // namespace render::lab
