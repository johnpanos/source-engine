//========= Copyright Valve Corporation, All rights reserved. ============//
#include "render_diagnostics.h"

int main( int argc, char **argv )
{
	renderdiagnostics::Capture disabled( nullptr, 1, 2, 3 );
	if ( disabled.Enabled() || disabled.WantsDrawState() )
		return 1;
	disabled.BeginFrame();
	disabled.EndFrame();
	if ( argc == 1 )
		return 0;
	renderdiagnostics::Capture capture( argv[1], 1, 2, 3 );
	if ( !capture.Enabled() || capture.WantsDrawState() )
		return 2;
	capture.BeginFrame();
	if ( !capture.WantsDrawState() )
		return 3;
	renderdiagnostics::DrawState draw;
	draw.material = "material\"\\\n\tname";
	draw.kind = "indexed";
	draw.primitives = 2;
	draw.viewportWidth = draw.targetWidth = 320;
	draw.viewportHeight = draw.targetHeight = 240;
	draw.targetFormat = 21;
	draw.target = 123;
	draw.vertexShader = 444;
	draw.pixelShader = 555;
	draw.textureMask = 3;
	draw.colorWrite = 15;
	draw.stateAvailable = true;
	capture.RequestShader( false, "quad_vs20", 0, 0, false );
	capture.RequestShader( true, "constant_ps20", 0, 0, false );
	capture.Draw( draw );
	capture.RequestShader( true, "missing_ps20b", 96, 2, true );
	capture.ShaderFailure( "missing_ps20b", 96, 96 );
	capture.Draw( draw );
	capture.Skipped( draw.material, "zero viewport\n" );
	capture.Present( 0 );
	capture.EndFrame();
	capture.BeginFrame();
	capture.Draw( draw );
	if ( capture.WantsDrawState() )
		return 4;
	capture.Draw( draw );
	capture.EndFrame();
	capture.Present( -1 );
	return 0;
}
