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
	draw.material = "caf\xc3\xa9 \xf0\x9f\x9a\x80 material\"\\\n\tname";
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
	capture.Skipped( "bad\xa9", "malformed byte" );
	capture.Skipped( "truncated\xe2\x82", "truncated UTF-8" );
	capture.Skipped( "overlong\xc0\xaf", "overlong UTF-8" );
	capture.Skipped( "surrogate\xed\xa0\x80", "invalid Unicode scalar" );
	capture.Skipped( "beyond\xf4\x90\x80\x80", "invalid Unicode scalar" );
	capture.Skipped( "literal\\udca9", "literal escape" );
	capture.EndFrame();
	capture.Present( -1 );
	return 0;
}
