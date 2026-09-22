//========= Copyright Valve Corporation, All rights reserved. ============//
#include "render_diagnostics.h"

void TraceNativeDraw( IDirect3DDevice9 *device, const char *material, const char *kind,
    int primitives, HRESULT result )
{
	renderdiagnostics::Capture &capture = renderdiagnostics::Current();
	if ( !capture.Enabled() )
		return;
	renderdiagnostics::DrawState state;
	state.material = material;
	state.kind = kind;
	state.primitives = primitives;
	state.result = result;
	if ( capture.WantsDrawState() )
	{
		state.stateAvailable = true;
		auto query = [&]( HRESULT status )
		{
			if ( FAILED( status ) )
				++state.queryErrors;
			return SUCCEEDED( status );
		};
		D3DVIEWPORT9 viewport = {};
		query( device->GetViewport( &viewport ) );
		state.viewportWidth = viewport.Width;
		state.viewportHeight = viewport.Height;
		IDirect3DSurface9 *target = nullptr;
		if ( query( device->GetRenderTarget( 0, &target ) ) && target )
		{
			D3DSURFACE_DESC description = {};
			query( target->GetDesc( &description ) );
			state.target = reinterpret_cast<std::uintptr_t>( target );
			state.targetWidth = description.Width;
			state.targetHeight = description.Height;
			state.targetFormat = description.Format;
			target->Release();
		}
		IDirect3DVertexShader9 *vertex = nullptr;
		query( device->GetVertexShader( &vertex ) );
		state.vertexShader = reinterpret_cast<std::uintptr_t>( vertex );
		if ( vertex )
			vertex->Release();
		IDirect3DPixelShader9 *pixel = nullptr;
		query( device->GetPixelShader( &pixel ) );
		state.pixelShader = reinterpret_cast<std::uintptr_t>( pixel );
		if ( pixel )
			pixel->Release();
		for ( unsigned sampler = 0; sampler < 16; ++sampler )
		{
			IDirect3DBaseTexture9 *texture = nullptr;
			query( device->GetTexture( sampler, &texture ) );
			if ( texture )
			{
				state.textureMask |= 1u << sampler;
				texture->Release();
			}
		}
		query( device->GetRenderState( D3DRS_ZENABLE, &state.depthTest ) );
		query( device->GetRenderState( D3DRS_ZWRITEENABLE, &state.depthWrite ) );
		query( device->GetRenderState( D3DRS_COLORWRITEENABLE, &state.colorWrite ) );
		query( device->GetRenderState( D3DRS_CULLMODE, &state.cull ) );
		query( device->GetRenderState( D3DRS_ALPHABLENDENABLE, &state.blend ) );
	}
	capture.Draw( state );
}
