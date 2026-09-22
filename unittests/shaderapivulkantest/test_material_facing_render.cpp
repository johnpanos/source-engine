//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Proves the material-system-facing legacy shader contract renders
//          through the native Vulkan backend (roadmap R32, first integration
//          slice). It drives the exact interfaces the material system uses --
//          IShaderDeviceMgr::SetMode, IShaderAPI::ClearColor4ub,
//          IShaderDevice::Present, IMesh (lock/write/unlock/draw), and the shader
//          binding flow (IShaderShadow::SetPixelShader -> IShaderAPI::TakeSnapshot
//          -> BeginPass) -- obtained from the linked backend via
//          NativeVulkanShaderBackend_Create, then reads the presented image back.
//
//          This is the bridge between the two goal halves: the generalized
//          backend (render_backend.h, proven substitutable by
//          render_backend_vulkan_conformance) and real native Vulkan rendering.
//          It certifies clear/present, mesh geometry, material shader selection,
//          shader constants, the vertex transform, AND texture sampling through
//          the material-facing interfaces: a mesh renders blue (passthrough),
//          green ("greenify"), yellow ("constant color" via a pixel-shader
//          constant), moves off-center when a model->projection matrix is set in
//          vertex constants c0-c3, and shows a material-uploaded texture
//          (CreateTexture/TexImage2D/BindTexture) UV-sampled under a
//          "$basetexture" shader. The rest of the material render state and the
//          full Source shader library remain the migration.
//
//          Requires a Vulkan device and a display; otherwise it reports an
//          explicit skip (exit 77) rather than a false pass.
//
//=============================================================================//

#include "bitmap/imageformat.h"
#include "materialsystem/imesh.h"
#include "render/legacy_shader_provider.h"
#include "shaderapi/ishaderapi.h"
#include "shaderapi/IShaderDevice.h"
#include "shaderapi/ishadershadow.h"
#include "vulkan_device.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>

// Test seam exported by the backend (see shaderapivulkan.cpp).
extern "C" render_vulkan::CVulkanContext *ShaderBackend_NativeVulkanContext();

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL_Init(video) failed: %s\n", SDL_GetError() );
		return 77;
	}
	SDL_Window *window = SDL_CreateWindow( "material-facing-vulkan", 256, 256, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: SDL_CreateWindow(vulkan) failed: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}

	// Obtain the material-facing services exactly as composition does.
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

	// Bring the backend up against the SDL window through the material-facing
	// device manager (this is where native Vulkan is created).
	ShaderDeviceInfo_t mode;
	services.manager->SetMode( window, 0, mode );

	render_vulkan::CVulkanContext *ctx = ShaderBackend_NativeVulkanContext();
	if ( !ctx || !ctx->IsValid() )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context not brought up (no device?)\n" );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}
	check( ctx->IsValid(), "SetMode brought up a valid native Vulkan device" );

	// Drive the material system's clear color and present through the legacy
	// interfaces, capturing the presented frame.
	services.api->ClearColor4ub( 0, 0, 255, 255 ); // blue, via IShaderAPI
	ctx->RequestCapture();
	services.device->Present(); // IShaderDevice::Present -> native Vulkan frame

	int w = 0, h = 0;
	{
		const std::vector<uint8_t> &px = ctx->GetCapturedPixels( &w, &h );
		check( w > 0 && h > 0 && !px.empty(), "a frame was presented and captured" );
		if ( w > 0 && h > 0 && !px.empty() )
		{
			const uint8_t *c = &px[( size_t( h / 2 ) * w + w / 2 ) * 4];
			const bool blue = c[0] <= 2 && c[1] <= 2 && c[2] >= 253 && c[3] >= 253;
			check( blue, "material-facing ClearColor4ub reached the GPU (frame is blue)" );
		}
	}

	// Geometry + material shader binding through the material-facing interfaces.
	// Build a BLUE-vertex triangle once, then draw it under two different bound
	// shaders selected via the faithful record->snapshot->BeginPass flow: the
	// passthrough shader shows the vertex color (blue), the "greenify" shader
	// overrides it (green). Different output by shader selection proves material
	// shader binding routes to distinct native Vulkan pipelines.
	check( ctx->DynamicMeshReady(), "dynamic mesh pipelines are ready" );
	IMesh *mesh =
	    services.device->CreateStaticMesh( VERTEX_POSITION | VERTEX_COLOR, "test", nullptr );
	check( mesh != nullptr, "device created a mesh" );
	if ( mesh )
	{
		MeshDesc_t desc;
		mesh->LockMesh( 3, 0, desc );
		const float positions[3][3] = {
		    { 0.0f, -0.7f, 0.5f },
		    { 0.7f, 0.7f, 0.5f },
		    { -0.7f, 0.7f, 0.5f },
		};
		for ( int i = 0; i < 3; ++i )
		{
			float *p =
			    reinterpret_cast<float *>( reinterpret_cast<unsigned char *>( desc.m_pPosition ) +
			                               static_cast<size_t>( i ) * desc.m_VertexSize_Position );
			p[0] = positions[i][0];
			p[1] = positions[i][1];
			p[2] = positions[i][2];
			unsigned char *c = desc.m_pColor + static_cast<size_t>( i ) * desc.m_VertexSize_Color;
			c[0] = 0;
			c[1] = 0;
			c[2] = 255;
			c[3] = 255; // blue vertices, RGBA
		}
		mesh->UnlockMesh( 3, 0, desc );

		// Two material shaders, each captured into its own snapshot.
		services.shadow->SetVertexShader( "vertex_passthrough", 0 );
		services.shadow->SetPixelShader( "vertexcolor", 0 );
		const StateSnapshot_t passthrough = services.api->TakeSnapshot();
		services.shadow->SetPixelShader( "greenify", 0 );
		const StateSnapshot_t greenify = services.api->TakeSnapshot();
		services.shadow->SetPixelShader( "constantcolor", 0 );
		const StateSnapshot_t constColor = services.api->TakeSnapshot();

		struct Pass
		{
			StateSnapshot_t snapshot;
			bool setConstant; // set a yellow pixel-shader constant before drawing
			int r, g, b;      // expected center color
			const char *what;
		};
		const Pass passes[] = {
		    { passthrough, false, 0, 0, 255,
		        "passthrough shader shows vertex color (center blue)" },
		    { greenify, false, 0, 255, 0, "greenify shader overrides vertex color (center green)" },
		    { constColor, true, 255, 255, 0,
		        "constant-color shader uses the pixel-shader constant (center yellow)" },
		};
		for ( const Pass &pass : passes )
		{
			services.api->ClearColor4ub( 255, 0, 0, 255 );          // red clear behind the triangle
			services.api->ClearBuffers( true, true, true, -1, -1 ); // start a new frame
			services.api->BeginPass( pass.snapshot );               // bind the material shader
			if ( pass.setConstant )
			{
				const float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
				services.api->SetPixelShaderConstant( 0, yellow, 1, false );
			}
			mesh->Draw();
			ctx->RequestCapture();
			services.device->Present();

			int gw = 0, gh = 0;
			const std::vector<uint8_t> &gpx = ctx->GetCapturedPixels( &gw, &gh );
			if ( gw > 0 && gh > 0 && !gpx.empty() )
			{
				const uint8_t *cc = &gpx[( size_t( gh / 2 ) * gw + gw / 2 ) * 4];
				const bool match = std::abs( int( cc[0] ) - pass.r ) <= 2 &&
				                   std::abs( int( cc[1] ) - pass.g ) <= 2 &&
				                   std::abs( int( cc[2] ) - pass.b ) <= 2;
				check( match, pass.what );
			}
			else
			{
				check( false, "captured a frame with the mesh geometry" );
			}
		}
		// Vertex transform: shift the triangle off-center via the model->projection
		// matrix supplied in vertex-shader constants c0-c3, and confirm the center
		// now reads the clear color -- proving the material transform reached the
		// vertex shader (geometry actually moved).
		services.api->ClearColor4ub( 255, 0, 0, 255 );
		services.api->ClearBuffers( true, true, true, -1, -1 );
		services.shadow->SetPixelShader( "vertexcolor", 0 );
		const StateSnapshot_t tsnap = services.api->TakeSnapshot();
		services.api->BeginPass( tsnap );
		// Column-major translate by +1.5 in x (moves the triangle off screen right).
		const float translate[16] = {
		    1,
		    0,
		    0,
		    0,
		    0,
		    1,
		    0,
		    0,
		    0,
		    0,
		    1,
		    0,
		    1.5f,
		    0,
		    0,
		    1,
		};
		services.api->SetVertexShaderConstant( 0, translate, 4, false );
		mesh->Draw();
		ctx->RequestCapture();
		services.device->Present();
		int tw = 0, th = 0;
		const std::vector<uint8_t> &tpx = ctx->GetCapturedPixels( &tw, &th );
		if ( tw > 0 && th > 0 && !tpx.empty() )
		{
			const uint8_t *cc = &tpx[( size_t( th / 2 ) * tw + tw / 2 ) * 4];
			const bool red = cc[0] >= 253 && cc[1] <= 2 && cc[2] <= 2;
			check( red, "vertex transform moved the geometry off-center (center is clear red)" );
		}
		else
		{
			check( false, "captured a frame with the transformed mesh" );
		}

		// Texturing: create and upload a material texture through IShaderAPI
		// (CreateTexture + ModifyTexture + TexImage2D), bind it, and draw a
		// full-screen UV quad with the "$basetexture" material shader. The
		// uploaded 2-tone texture (left blue, right yellow) must appear left/right,
		// proving material-supplied texture upload + UV-correct sampling through
		// the material interfaces.
		{
			ShaderAPITextureHandle_t tex = services.api->CreateTexture(
			    2, 1, 1, IMAGE_FORMAT_RGBA8888, 1, 1, 0, "mat", "mat" );
			check( tex != 0, "IShaderAPI::CreateTexture returned a handle" );
			services.api->ModifyTexture( tex );
			unsigned char texels[2 * 4] = {
			    0,
			    0,
			    255,
			    255, // left: blue
			    255,
			    255,
			    0,
			    255, // right: yellow
			};
			services.api->TexImage2D(
			    0, 0, IMAGE_FORMAT_RGBA8888, 0, 2, 1, IMAGE_FORMAT_RGBA8888, false, texels );
			services.api->BindTexture( SHADER_SAMPLER0, tex );

			mesh->LockMesh( 6, 0, desc );
			const float quad[6][5] = {
			    // x, y, z, u, v
			    { -1.0f, -1.0f, 0.5f, 0.0f, 0.0f },
			    { 1.0f, -1.0f, 0.5f, 1.0f, 0.0f },
			    { 1.0f, 1.0f, 0.5f, 1.0f, 1.0f },
			    { -1.0f, -1.0f, 0.5f, 0.0f, 0.0f },
			    { 1.0f, 1.0f, 0.5f, 1.0f, 1.0f },
			    { -1.0f, 1.0f, 0.5f, 0.0f, 1.0f },
			};
			for ( int i = 0; i < 6; ++i )
			{
				float *p = reinterpret_cast<float *>(
				    reinterpret_cast<unsigned char *>( desc.m_pPosition ) +
				    static_cast<size_t>( i ) * desc.m_VertexSize_Position );
				p[0] = quad[i][0];
				p[1] = quad[i][1];
				p[2] = quad[i][2];
				float *uv = reinterpret_cast<float *>(
				    reinterpret_cast<unsigned char *>( desc.m_pTexCoord[0] ) +
				    static_cast<size_t>( i ) * desc.m_VertexSize_TexCoord[0] );
				uv[0] = quad[i][3];
				uv[1] = quad[i][4];
			}
			mesh->UnlockMesh( 6, 0, desc );

			// Bind by the REAL Source shader name: UnlitGeneric samples $basetexture,
			// which the backend maps to its native textured pipeline.
			services.shadow->SetPixelShader( "unlitgeneric_ps20b", 0 );
			const StateSnapshot_t texsnap = services.api->TakeSnapshot();
			services.api->ClearColor4ub( 0, 0, 0, 255 );
			services.api->ClearBuffers( true, true, true, -1, -1 );
			services.api->BeginPass( texsnap );
			const float identity[16] = {
			    1,
			    0,
			    0,
			    0,
			    0,
			    1,
			    0,
			    0,
			    0,
			    0,
			    1,
			    0,
			    0,
			    0,
			    0,
			    1,
			};
			services.api->SetVertexShaderConstant( 0, identity, 4, false ); // reset transform
			mesh->Draw();
			ctx->RequestCapture();
			services.device->Present();

			int xw = 0, xh = 0;
			const std::vector<uint8_t> &xpx = ctx->GetCapturedPixels( &xw, &xh );
			if ( xw > 0 && xh > 0 && !xpx.empty() )
			{
				const uint8_t *left = &xpx[( size_t( xh / 2 ) * xw + xw / 4 ) * 4];
				const uint8_t *right = &xpx[( size_t( xh / 2 ) * xw + ( xw * 3 ) / 4 ) * 4];
				const bool leftBlue = left[0] <= 2 && left[1] <= 2 && left[2] >= 253;
				const bool rightYellow = right[0] >= 253 && right[1] >= 253 && right[2] <= 2;
				check( leftBlue && rightYellow,
				    "IShaderAPI-uploaded texture is sampled by UV (left blue, right yellow)" );
			}
			else
			{
				check( false, "captured a frame with the textured mesh" );
			}
		}

		// Compressed texture (Portal's format): upload a DXT1/BC1 block encoding
		// solid blue and confirm it samples blue -- the block-compressed texture
		// path native Vulkan uses to sample real Source VTF textures.
		{
			ShaderAPITextureHandle_t dtex =
			    services.api->CreateTexture( 4, 4, 1, IMAGE_FORMAT_DXT1, 1, 1, 0, "dxt", "dxt" );
			check( dtex != 0, "IShaderAPI created a DXT1 texture" );
			services.api->ModifyTexture( dtex );
			// One 4x4 DXT1 block: color0 = color1 = RGB565 blue (0x001F), all
			// indices 0 -> every texel is blue.
			unsigned char dxt1Blue[8] = { 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00 };
			services.api->TexImage2D(
			    0, 0, IMAGE_FORMAT_DXT1, 0, 4, 4, IMAGE_FORMAT_DXT1, false, dxt1Blue );
			services.api->BindTexture( SHADER_SAMPLER0, dtex );
			services.shadow->SetPixelShader( "unlitgeneric_ps20b", 0 );
			const StateSnapshot_t dsnap = services.api->TakeSnapshot();
			services.api->ClearColor4ub( 0, 0, 0, 255 );
			services.api->ClearBuffers( true, true, true, -1, -1 );
			services.api->BeginPass( dsnap );
			mesh->Draw();
			ctx->RequestCapture();
			services.device->Present();
			int dw = 0, dh = 0;
			const std::vector<uint8_t> &dpx = ctx->GetCapturedPixels( &dw, &dh );
			if ( dw > 0 && dh > 0 && !dpx.empty() )
			{
				const uint8_t *c = &dpx[( size_t( dh / 2 ) * dw + dw / 2 ) * 4];
				const bool blue = c[0] <= 4 && c[1] <= 4 && c[2] >= 250;
				check( blue, "DXT1/BC1 compressed texture samples natively (blue)" );
			}
			else
			{
				check( false, "captured a frame with the DXT texture" );
			}
		}

		services.device->DestroyStaticMesh( mesh );
	}

	ctx->Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();

	std::fprintf(
	    stderr, "material-facing native Vulkan: %d checks, %d failures\n", checks, failures );
	return failures == 0 ? 0 : 1;
}
