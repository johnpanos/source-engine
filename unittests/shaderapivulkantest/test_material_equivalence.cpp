//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Material equivalence oracle for the native Vulkan backend (roadmap
//          R32 -> R36). Where test_material_facing_render.cpp proves the
//          material interfaces *reach* the GPU, this suite asserts the native
//          UnlitGeneric path produces the *same result the D3D9 shader defines*.
//
//          The reference is the actual Source shader source:
//            stdshaders/unlitgeneric_ps2x.fxc:
//              result = i.vColor0 * tex2D( TextureSampler, i.vTexCoord0 )
//            stdshaders/unlitgeneric_vs20.fxc:
//              o.vColor     = cModulationColor                     (c47)
//              o.vTexCoord0 = mul( v.vTexCoord0, cBaseTextureTransform )  (c48-c49)
//              o.vProjPos   = mul( float4(pos,1), cModelViewProj )        (c4-c7)
//          i.e. the shaded pixel is
//              cModulationColor * baseTexture( cBaseTextureTransform * uv ).
//
//          Every constant here is set at its REAL Source register number
//          (common_vs_fxc.h), so a pass proves the backend honors the D3D9
//          constant contract -- the substitutability the material-system
//          generalization depends on -- not a bespoke convention. Negative
//          controls prove the oracle detects a backend that ignores the
//          modulation, samples the wrong texel, or honors the wrong register.
//
//          Checks are chosen to be swapchain-format agnostic (sRGB vs UNORM):
//          per-channel 0/1 modulation masks and blue/yellow texels map cleanly
//          through either encoding, and the one magnitude check asserts only
//          monotonicity (0.5x is strictly darker than 1x), never an absolute
//          midtone. This keeps the equivalence claim honest across devices.
//
//          Requires a Vulkan device and a display; otherwise reports an explicit
//          skip (exit 77) rather than a false pass.
//
//=============================================================================//

#include "bitmap/imageformat.h"
#include "materialsystem/imesh.h"
#include "materialsystem/ishadersystem_declarations.h"
#include "render/legacy_shader_provider.h"
#include "shaderapi/ishaderapi.h"
#include "shaderapi/IShaderDevice.h"
#include "shaderapi/ishadershadow.h"
#include "vulkan_device.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>

extern "C" render_vulkan::CVulkanContext *ShaderBackend_NativeVulkanContext();

namespace
{
// Source vertex-shader constant registers, from the enum the shaders' dynamic
// state writes through. An earlier revision hand-copied the vs_1_1 numbers
// (c37/c38) from common_vs_fxc.h; backend and oracle then agreed with each other
// and both disagreed with every real vs_2_0 shader.
constexpr int kRegModelViewProj = VERTEX_SHADER_MODELVIEWPROJ;              // c4-c7
constexpr int kRegModulationColor = VERTEX_SHADER_MODULATION_COLOR;         // c47
constexpr int kRegBaseTexTransform = VERTEX_SHADER_SHADER_SPECIFIC_CONST_0; // c48-c49

const float kIdentity4x4[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

// A full-screen quad with UV 0..1 across it, so the left quarter samples u~0.25
// and the right quarter samples u~0.75.
void LockFullScreenQuad( IMesh *mesh )
{
	MeshDesc_t desc;
	mesh->LockMesh( 6, 0, desc );
	const float quad[6][5] = {
	    // Clockwise on screen: D3D's front face under the default CCW culling.
	    { -1.0f, -1.0f, 0.5f, 0.0f, 0.0f },
	    { 1.0f, 1.0f, 0.5f, 1.0f, 1.0f },
	    { 1.0f, -1.0f, 0.5f, 1.0f, 0.0f },
	    { -1.0f, -1.0f, 0.5f, 0.0f, 0.0f },
	    { -1.0f, 1.0f, 0.5f, 0.0f, 1.0f },
	    { 1.0f, 1.0f, 0.5f, 1.0f, 1.0f },
	};
	for ( int i = 0; i < 6; ++i )
	{
		float *p =
		    reinterpret_cast<float *>( reinterpret_cast<unsigned char *>( desc.m_pPosition ) +
		                               static_cast<size_t>( i ) * desc.m_VertexSize_Position );
		p[0] = quad[i][0];
		p[1] = quad[i][1];
		p[2] = quad[i][2];
		float *uv =
		    reinterpret_cast<float *>( reinterpret_cast<unsigned char *>( desc.m_pTexCoord[0] ) +
		                               static_cast<size_t>( i ) * desc.m_VertexSize_TexCoord[0] );
		uv[0] = quad[i][3];
		uv[1] = quad[i][4];
	}
	mesh->UnlockMesh( 6, 0, desc );
}
} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL_Init(video) failed: %s\n", SDL_GetError() );
		return 77;
	}
	SDL_Window *window =
	    SDL_CreateWindow( "material-equivalence-vulkan", 256, 256, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: SDL_CreateWindow(vulkan) failed: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}

	render::LegacyShaderServices services;
	if ( !NativeVulkanShaderBackend_Create( &services ) || !services.IsComplete() )
	{
		std::fprintf(
		    stderr, "FAIL: NativeVulkanShaderBackend_Create did not return complete services\n" );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 1;
	}

	int checks = 0, failures = 0;
	auto check = [&]( bool ok, const char *what )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			std::fprintf( stderr, "FAIL: %s\n", what );
		}
	};

	ShaderDeviceInfo_t modeInfo;
	services.manager->SetMode( window, 0, modeInfo );

	render_vulkan::CVulkanContext *ctx = ShaderBackend_NativeVulkanContext();
	if ( !ctx || !ctx->IsValid() )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context not brought up (no device?)\n" );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}

	IMesh *mesh =
	    services.device->CreateStaticMesh( VERTEX_POSITION | VERTEX_COLOR, "equiv", nullptr );
	check( mesh != nullptr, "device created a mesh" );
	if ( !mesh )
	{
		ctx->Shutdown();
		SDL_DestroyWindow( window );
		SDL_Quit();
		std::fprintf( stderr, "material equivalence: %d checks, %d failures\n", checks, failures );
		return 1;
	}

	// A 1x1 solid-white base texture: the modulation is then the entire output, so
	// output == cModulationColor exactly (the D3D9 result with a white $basetexture).
	ShaderAPITextureHandle_t whiteTex =
	    services.api->CreateTexture( 1, 1, 1, IMAGE_FORMAT_RGBA8888, 1, 1, 0, "white", "white" );
	services.api->ModifyTexture( whiteTex );
	unsigned char white[4] = { 255, 255, 255, 255 };
	services.api->TexImage2D(
	    0, 0, IMAGE_FORMAT_RGBA8888, 0, 1, 1, IMAGE_FORMAT_RGBA8888, false, white );

	// A 2-tone base texture: left half blue, right half yellow.
	ShaderAPITextureHandle_t toneTex =
	    services.api->CreateTexture( 2, 1, 1, IMAGE_FORMAT_RGBA8888, 1, 1, 0, "tone", "tone" );
	services.api->ModifyTexture( toneTex );
	unsigned char tone[2 * 4] = { 0, 0, 255, 255, 255, 255, 0, 255 };
	services.api->TexImage2D(
	    0, 0, IMAGE_FORMAT_RGBA8888, 0, 2, 1, IMAGE_FORMAT_RGBA8888, false, tone );

	// Helper: bind UnlitGeneric, set the constants (at their real Source registers),
	// draw the quad, and read back the pixel at fractional (fx, fy).
	auto drawUnlit = [&]( ShaderAPITextureHandle_t tex, const float mod[4], const float xform0[4],
	                     const float xform1[4], float fx, float fy, uint8_t out[4] ) -> bool
	{
		services.shadow->SetPixelShader( "unlitgeneric_ps20b", 0 );
		const StateSnapshot_t snap = services.api->TakeSnapshot();
		services.api->ClearColor4ub( 0, 0, 0, 255 );
		services.api->ClearBuffers( true, true, true, -1, -1 );
		services.api->BeginPass( snap );
		services.api->SetVertexShaderConstant( kRegModelViewProj, kIdentity4x4, 4, false );
		if ( mod )
			services.api->SetVertexShaderConstant( kRegModulationColor, mod, 1, false );
		if ( xform0 && xform1 )
		{
			float rows[8];
			for ( int i = 0; i < 4; ++i )
			{
				rows[i] = xform0[i];
				rows[4 + i] = xform1[i];
			}
			services.api->SetVertexShaderConstant( kRegBaseTexTransform, rows, 2, false );
		}
		services.api->BindTexture( SHADER_SAMPLER0, tex );
		LockFullScreenQuad( mesh );
		mesh->Draw();
		ctx->RequestCapture();
		services.device->Present();
		int w = 0, h = 0;
		const std::vector<uint8_t> &px = ctx->GetCapturedPixels( &w, &h );
		if ( w <= 0 || h <= 0 || px.empty() )
			return false;
		const int x = static_cast<int>( fx * w );
		const int y = static_cast<int>( fy * h );
		const uint8_t *c = &px[( static_cast<size_t>( y ) * w + x ) * 4];
		out[0] = c[0];
		out[1] = c[1];
		out[2] = c[2];
		out[3] = c[3];
		return true;
	};

	const float identityRow0[4] = { 1, 0, 0, 0 };
	const float identityRow1[4] = { 0, 1, 0, 0 };
	uint8_t c[4];

	// --- 1. Base texture sampling (white modulation, identity UV): output == texel.
	const float white4[4] = { 1, 1, 1, 1 };
	check( drawUnlit( toneTex, white4, identityRow0, identityRow1, 0.25f, 0.5f, c ) && c[0] <= 3 &&
	           c[1] <= 3 && c[2] >= 252,
	    "UnlitGeneric samples $basetexture: left half is blue" );
	check( drawUnlit( toneTex, white4, identityRow0, identityRow1, 0.75f, 0.5f, c ) &&
	           c[0] >= 252 && c[1] >= 252 && c[2] <= 3,
	    "UnlitGeneric samples $basetexture: right half is yellow" );

	// --- 2. cModulationColor per-channel multiply (D3D9 result = vColor0 * tex).
	//         White texture * (1,0,0,1) -> pure red; green and blue are masked out.
	const float modRed[4] = { 1, 0, 0, 1 };
	const bool gotRed = drawUnlit( whiteTex, modRed, identityRow0, identityRow1, 0.5f, 0.5f, c );
	check( gotRed && c[0] >= 252 && c[1] <= 3 && c[2] <= 3,
	    "cModulationColor (c47) masks channels: (1,0,0)*white -> red" );
	// Negative control: the oracle must reject the un-modulated (white) result.
	check( gotRed && !( c[0] >= 252 && c[1] >= 252 && c[2] >= 252 ),
	    "negative control: modulated output is NOT the raw white texture" );

	// --- 3. cModulationColor multiplies a real texture per region.
	//         2-tone * (0,1,1,1): left blue (0,0,1)*(0,1,1)=(0,0,1) stays blue;
	//         right yellow (1,1,0)*(0,1,1)=(0,1,0) becomes green.
	const float modKillRed[4] = { 0, 1, 1, 1 };
	check( drawUnlit( toneTex, modKillRed, identityRow0, identityRow1, 0.25f, 0.5f, c ) &&
	           c[0] <= 3 && c[1] <= 3 && c[2] >= 252,
	    "modulation * texture: left (blue) unchanged by killing red" );
	check( drawUnlit( toneTex, modKillRed, identityRow0, identityRow1, 0.75f, 0.5f, c ) &&
	           c[0] <= 3 && c[1] >= 252 && c[2] <= 3,
	    "modulation * texture: right (yellow) -> green when red is killed" );

	// --- 4. cModulationColor magnitude attenuates ($color/$alpha). Format-agnostic:
	//         assert only that 0.5x is strictly darker than 1x on a white texture.
	uint8_t full[4], half[4];
	const float modHalf[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	const bool gotFull =
	    drawUnlit( whiteTex, white4, identityRow0, identityRow1, 0.5f, 0.5f, full );
	const bool gotHalf =
	    drawUnlit( whiteTex, modHalf, identityRow0, identityRow1, 0.5f, 0.5f, half );
	check( gotFull && gotHalf && half[0] < full[0] - 8 && half[1] < full[1] - 8 &&
	           half[2] < full[2] - 8,
	    "cModulationColor magnitude: 0.5x is strictly darker than 1x" );

	// --- 5. cBaseTextureTransform (c48-c49) transforms the sampled coordinate.
	//         Map every u to 0.75 (row0 = (0,0,0,0.75)): the whole quad now samples
	//         the right (yellow) half, so the LEFT pixel reads yellow. The identity
	//         control (test 1) read blue there, so the transform is what moved it.
	const float uvToRight0[4] = { 0, 0, 0, 0.75f };
	const float uvKeepV1[4] = { 0, 1, 0, 0 };
	check( drawUnlit( toneTex, white4, uvToRight0, uvKeepV1, 0.25f, 0.5f, c ) && c[0] >= 252 &&
	           c[1] >= 252 && c[2] <= 3,
	    "cBaseTextureTransform (c48) remaps UV: left pixel now samples yellow" );

	// --- 6. Register fidelity negative control: modulation set at the WRONG
	//         register (c20, unused by UnlitGeneric) must NOT affect the output.
	//         Reset the real modulation to white first, then poke c20.
	services.api->SetVertexShaderConstant( kRegModulationColor, white4, 1, false );
	const float bogus[4] = { 1, 0, 0, 1 };
	services.api->SetVertexShaderConstant( 20, bogus, 1, false );
	check( drawUnlit( toneTex, nullptr, identityRow0, identityRow1, 0.75f, 0.5f, c ) &&
	           c[0] >= 252 && c[1] >= 252 && c[2] <= 3,
	    "register fidelity: modulation at c20 is ignored (right stays yellow)" );

	// --- 7. Blend modes ($translucent / $additive). The blend state is recorded on
	//         IShaderShadow (EnableBlending/BlendFunc) into the snapshot, then the
	//         backend selects the matching pipeline variant. A solid-blue base
	//         texture with white modulation gives a blue source; the modulation
	//         alpha is the source alpha the alpha blend uses. Checks compare the
	//         composited result against the D3D9-defined blend equations, at 0/1
	//         channels so they are swapchain-format agnostic.
	ShaderAPITextureHandle_t blueTex =
	    services.api->CreateTexture( 1, 1, 1, IMAGE_FORMAT_RGBA8888, 1, 1, 0, "blue", "blue" );
	services.api->ModifyTexture( blueTex );
	unsigned char blue[4] = { 0, 0, 255, 255 };
	services.api->TexImage2D(
	    0, 0, IMAGE_FORMAT_RGBA8888, 0, 1, 1, IMAGE_FORMAT_RGBA8888, false, blue );

	// Draw the blue quad over a chosen clear color under a chosen blend mode
	// (0 opaque / 1 alpha / 2 additive / 3 modulate / 4 alpha-additive), reading
	// back the center pixel.
	auto drawBlend = [&]( const float mod[4], unsigned char cr, unsigned char cg, unsigned char cb_,
	                     int blendMode, uint8_t out[4] ) -> bool
	{
		if ( blendMode == 0 )
		{
			services.shadow->EnableBlending( false );
		}
		else
		{
			services.shadow->EnableBlending( true );
			if ( blendMode == 2 )
				services.shadow->BlendFunc( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			else if ( blendMode == 3 )
				services.shadow->BlendFunc( SHADER_BLEND_ZERO, SHADER_BLEND_SRC_COLOR );
			else if ( blendMode == 4 )
				services.shadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
			else
				services.shadow->BlendFunc(
				    SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
		}
		services.shadow->SetPixelShader( "unlitgeneric_ps20b", 0 );
		const StateSnapshot_t snap = services.api->TakeSnapshot();
		services.api->ClearColor4ub( cr, cg, cb_, 255 );
		services.api->ClearBuffers( true, true, true, -1, -1 );
		services.api->BeginPass( snap );
		services.api->SetVertexShaderConstant( kRegModelViewProj, kIdentity4x4, 4, false );
		services.api->SetVertexShaderConstant( kRegModulationColor, mod, 1, false );
		services.api->BindTexture( SHADER_SAMPLER0, blueTex );
		LockFullScreenQuad( mesh );
		mesh->Draw();
		ctx->RequestCapture();
		services.device->Present();
		int w = 0, h = 0;
		const std::vector<uint8_t> &px = ctx->GetCapturedPixels( &w, &h );
		if ( w <= 0 || h <= 0 || px.empty() )
			return false;
		const uint8_t *p = &px[( static_cast<size_t>( h / 2 ) * w + w / 2 ) * 4];
		out[0] = p[0];
		out[1] = p[1];
		out[2] = p[2];
		out[3] = p[3];
		return true;
	};

	// Additive over a red clear: src(0,0,1) + dst(1,0,0) = magenta (1,0,1).
	check( drawBlend( white4, 255, 0, 0, 2, c ) && c[0] >= 252 && c[1] <= 3 && c[2] >= 252,
	    "$additive: blue + red clear = magenta (src + dst)" );
	// Opaque control, identical inputs: src replaces dst -> blue, not magenta.
	check( drawBlend( white4, 255, 0, 0, 0, c ) && c[0] <= 3 && c[1] <= 3 && c[2] >= 252,
	    "opaque control: same draw replaces the clear (blue, not magenta)" );
	// Translucent with source alpha 0: result is entirely the dst (green clear).
	const float modAlpha0[4] = { 1, 1, 1, 0 };
	check( drawBlend( modAlpha0, 0, 255, 0, 1, c ) && c[0] <= 3 && c[1] >= 252 && c[2] <= 3,
	    "$translucent alpha=0: fully transparent, clear (green) shows through" );
	// Translucent with source alpha 1: result is entirely the src (blue).
	check( drawBlend( white4, 0, 255, 0, 1, c ) && c[0] <= 3 && c[1] <= 3 && c[2] >= 252,
	    "$translucent alpha=1: fully opaque source (blue) over the clear" );
	// Modulate (decal shadows: ZERO, SRC_COLOR) over yellow: dst * src = black.
	// Alpha blending would give blue and additive white, so only the recorded
	// factors pass.
	check( drawBlend( white4, 255, 255, 0, 3, c ) && c[0] <= 3 && c[1] <= 3 && c[2] <= 3,
	    "modulate (zero, src_color): blue * yellow clear = black" );
	// Alpha-additive (SRC_ALPHA, ONE) with source alpha 0 adds nothing: the red
	// clear stays red, where plain additive (ONE, ONE) would give magenta.
	check( drawBlend( modAlpha0, 255, 0, 0, 4, c ) && c[0] >= 252 && c[1] <= 3 && c[2] <= 3,
	    "alpha-additive (src_alpha, one) alpha=0: clear (red) unchanged" );

	// Snapshot ids are 16 bits. Materials take far more snapshots than they have
	// distinct states; identical states must share an id, so a state taken after
	// thousands of snapshots still binds its own blend rather than an aliased one.
	services.shadow->EnableBlending( false );
	services.shadow->SetPixelShader( "unlitgeneric_ps20b", 0 );
	const StateSnapshot_t firstOpaque = services.api->TakeSnapshot();
	bool shared = true;
	for ( int i = 0; i < 3000; ++i )
	{
		if ( services.api->TakeSnapshot() != firstOpaque )
			shared = false;
	}
	check( shared, "identical shadow states share one snapshot id" );
	check( drawBlend( white4, 255, 0, 0, 2, c ) && c[0] >= 252 && c[1] <= 3 && c[2] >= 252,
	    "a state taken after 3000 snapshots keeps its own blend ($additive = magenta)" );

	// --- 8. $alphatest. A 2-tone texture with per-region alpha (left opaque, right
	//         alpha 0) drawn over a red clear with the GEQUAL alpha test at 0.5:
	//         the left texel (alpha 1 >= 0.5) survives, the right texel (alpha 0)
	//         is discarded so the clear shows through. The alpha-test-off control
	//         proves the discard is the alpha test, not the texture.
	ShaderAPITextureHandle_t alphaTex =
	    services.api->CreateTexture( 2, 1, 1, IMAGE_FORMAT_RGBA8888, 1, 1, 0, "at", "at" );
	services.api->ModifyTexture( alphaTex );
	unsigned char atTexels[2 * 4] = { 0, 0, 255, 255, 255, 255, 0, 0 }; // left blue a=1, right a=0
	services.api->TexImage2D(
	    0, 0, IMAGE_FORMAT_RGBA8888, 0, 2, 1, IMAGE_FORMAT_RGBA8888, false, atTexels );

	auto drawAlphaTest = [&]( bool alphaTestOn, float fx, uint8_t out[4] ) -> bool
	{
		services.shadow->EnableBlending( false );
		services.shadow->EnableAlphaTest( alphaTestOn );
		if ( alphaTestOn )
			services.shadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, 0.5f );
		services.shadow->SetPixelShader( "unlitgeneric_ps20b", 0 );
		const StateSnapshot_t snap = services.api->TakeSnapshot();
		services.api->ClearColor4ub( 255, 0, 0, 255 );
		services.api->ClearBuffers( true, true, true, -1, -1 );
		services.api->BeginPass( snap );
		services.api->SetVertexShaderConstant( kRegModelViewProj, kIdentity4x4, 4, false );
		services.api->SetVertexShaderConstant( kRegModulationColor, white4, 1, false );
		const float idRows[8] = { 1, 0, 0, 0, 0, 1, 0, 0 };
		services.api->SetVertexShaderConstant( kRegBaseTexTransform, idRows, 2, false );
		services.api->BindTexture( SHADER_SAMPLER0, alphaTex );
		LockFullScreenQuad( mesh );
		mesh->Draw();
		ctx->RequestCapture();
		services.device->Present();
		int w = 0, h = 0;
		const std::vector<uint8_t> &px = ctx->GetCapturedPixels( &w, &h );
		if ( w <= 0 || h <= 0 || px.empty() )
			return false;
		const int x = static_cast<int>( fx * w );
		const uint8_t *p = &px[( static_cast<size_t>( h / 2 ) * w + x ) * 4];
		out[0] = p[0];
		out[1] = p[1];
		out[2] = p[2];
		out[3] = p[3];
		return true;
	};

	// Alpha test on: right (alpha 0) is discarded -> red clear shows through.
	check( drawAlphaTest( true, 0.75f, c ) && c[0] >= 252 && c[1] <= 3 && c[2] <= 3,
	    "$alphatest: right texel (alpha 0) discarded, clear (red) shows through" );
	// Left (alpha 1) survives the alpha test -> blue.
	check( drawAlphaTest( true, 0.25f, c ) && c[0] <= 3 && c[1] <= 3 && c[2] >= 252,
	    "$alphatest: left texel (alpha 1) survives -> blue" );
	// Control, alpha test off: right texel is NOT discarded -> yellow, not red.
	check( drawAlphaTest( false, 0.75f, c ) && c[0] >= 252 && c[1] >= 252 && c[2] <= 3,
	    "alpha-test-off control: right texel is kept (yellow, not the clear)" );

	services.device->DestroyStaticMesh( mesh );
	ctx->Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();

	std::fprintf(
	    stderr, "material equivalence (UnlitGeneric): %d checks, %d failures\n", checks, failures );
	return failures == 0 ? 0 : 1;
}
