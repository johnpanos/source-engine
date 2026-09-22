//========= Copyright Valve Corporation, All rights reserved. ============//
// Native integration gate: must run on Wayland with a Vulkan adapter. A missing
// display/provider is a failure, never a skipped or software-certified result.
#include "../../materialsystem/shaderapidx9/dxvk_compat.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "sampled_quad_bytecode.h"

static_assert( !IsPlatformOpenGL(), "Native Vulkan must not inherit OpenGL capability policy" );

namespace
{
int g_Checks;
bool g_NegativePixelOracle;
int g_Frames;
int g_Resizes;
void Check( bool condition, const char *operation )
{
	++g_Checks;
	if ( !condition )
	{
		std::fprintf( stderr, "native presentation check %d failed: %s (%s)\n", g_Checks, operation,
		    SDL_GetError() );
		std::exit( 1 );
	}
}
void Readback( IDirect3DDevice9 &device, UINT width, UINT height, const D3DCOLOR colors[4] )
{
	IDirect3DSurface9 *target = NULL;
	Check( SUCCEEDED( device.GetRenderTarget( 0, &target ) ) && target, "render target" );
	D3DSURFACE_DESC description;
	Check( SUCCEEDED( target->GetDesc( &description ) ), "target description" );
	Check( description.Width == width && description.Height == height, "target dimensions" );
	IDirect3DSurface9 *readback = NULL;
	Check( SUCCEEDED( device.CreateOffscreenPlainSurface(
	           width, height, description.Format, D3DPOOL_SYSTEMMEM, &readback, NULL ) ) &&
	           readback,
	    "readback storage" );
	Check( SUCCEEDED( device.GetRenderTargetData( target, readback ) ), "GPU readback" );
	D3DLOCKED_RECT pixels;
	Check( SUCCEEDED( readback->LockRect( &pixels, NULL, D3DLOCK_READONLY ) ), "readback lock" );
	bool match = true;
	for ( UINT y = 0; y != height; ++y )
	{
		const DWORD *row = reinterpret_cast<const DWORD *>(
		    static_cast<const unsigned char *>( pixels.pBits ) + y * pixels.Pitch );
		for ( UINT x = 0; x != width; ++x )
		{
			D3DCOLOR expected =
			    colors[( y >= ( height + 1 ) / 2 ? 2 : 0 ) + ( x >= ( width + 1 ) / 2 ? 1 : 0 )];
			if ( g_NegativePixelOracle && x == 0 && y == 0 )
				expected ^= 1;
			if ( match && row[x] != expected )
			{
				std::fprintf( stderr,
				    "first pixel divergence x=%u y=%u expected=%08x actual=%08x\n", x, y, expected,
				    row[x] );
				match = false;
			}
		}
	}
	Check( match, "exact GPU pixels" );
	Check( SUCCEEDED( readback->UnlockRect() ), "readback unlock" );
	readback->Release();
	target->Release();
	Check( SUCCEEDED( device.Present( NULL, NULL, NULL, NULL ) ), "present" );
	++g_Frames;
}

void Frame( IDirect3DDevice9 &device, UINT width, UINT height, D3DCOLOR color )
{
	Check( SUCCEEDED( device.Clear( 0, NULL, D3DCLEAR_TARGET, color, 1, 0 ) ), "clear" );
	const D3DCOLOR colors[] = { color, color, color, color };
	Readback( device, width, height, colors );
}

void SampledFrame( IDirect3DDevice9 &device, UINT width, UINT height )
{
	const D3DCOLOR colors[] = { D3DCOLOR_XRGB( 37, 83, 149 ), D3DCOLOR_XRGB( 173, 59, 101 ),
	    D3DCOLOR_XRGB( 19, 193, 71 ), D3DCOLOR_XRGB( 229, 137, 43 ) };
	IDirect3DTexture9 *texture = NULL;
	Check( SUCCEEDED( device.CreateTexture(
	           2, 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, NULL ) ) &&
	           texture,
	    "sampled texture" );
	D3DLOCKED_RECT upload;
	Check( SUCCEEDED( texture->LockRect( 0, &upload, NULL, 0 ) ), "texture upload lock" );
	for ( UINT y = 0; y != 2; ++y )
		std::memcpy( static_cast<unsigned char *>( upload.pBits ) + y * upload.Pitch,
		    colors + 2 * y, 2 * sizeof( D3DCOLOR ) );
	Check( SUCCEEDED( texture->UnlockRect( 0 ) ), "texture upload unlock" );
	IDirect3DVertexShader9 *vertex = NULL;
	IDirect3DPixelShader9 *pixel = NULL;
	Check( SUCCEEDED( device.CreateVertexShader( kSampledQuadVertex, &vertex ) ) && vertex,
	    "compiled vertex shader" );
	Check( SUCCEEDED( device.CreatePixelShader( kSampledQuadPixel, &pixel ) ) && pixel,
	    "compiled pixel shader" );
	const D3DVERTEXELEMENT9 elements[] = {
	    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	    { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	    D3DDECL_END() };
	IDirect3DVertexDeclaration9 *declaration = NULL;
	Check( SUCCEEDED( device.CreateVertexDeclaration( elements, &declaration ) ) && declaration,
	    "vertex declaration" );
	Check( SUCCEEDED( device.SetVertexDeclaration( declaration ) ) &&
	           SUCCEEDED( device.SetVertexShader( vertex ) ) &&
	           SUCCEEDED( device.SetPixelShader( pixel ) ) &&
	           SUCCEEDED( device.SetTexture( 0, texture ) ),
	    "programmable pipeline binding" );
	// D3D9 samples integer pixel centers. Quarter-pixel alignment keeps every
	// sample strictly away from the 2x2 texel boundary even at odd dimensions.
	// UV=(pixel+0.25)/dimension gives an independent ceil(dimension/2) split;
	// half-pixel alignment would put the odd center exactly on a rounding tie.
	const float clipOffset[] = { -0.5f / width, 0.5f / height, 0, 0 };
	Check( SUCCEEDED( device.SetVertexShaderConstantF( 0, clipOffset, 1 ) ),
	    "vertex constant publication" );
	Check( SUCCEEDED( device.SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) ) &&
	           SUCCEEDED( device.SetRenderState( D3DRS_ZENABLE, FALSE ) ) &&
	           SUCCEEDED( device.SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ) ) &&
	           SUCCEEDED( device.SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE ) ),
	    "draw render states" );
	Check( SUCCEEDED( device.SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT ) ) &&
	           SUCCEEDED( device.SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT ) ) &&
	           SUCCEEDED( device.SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE ) ) &&
	           SUCCEEDED( device.SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ) ) &&
	           SUCCEEDED( device.SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ) ) &&
	           SUCCEEDED( device.SetSamplerState( 0, D3DSAMP_SRGBTEXTURE, FALSE ) ),
	    "point sample states" );
	const float vertices[][6] = { { -1, 1, 0, 1, 0, 0 }, { 1, 1, 0, 1, 1, 0 },
	    { -1, -1, 0, 1, 0, 1 }, { 1, -1, 0, 1, 1, 1 } };
	Check( SUCCEEDED( device.Clear( 0, NULL, D3DCLEAR_TARGET, 0, 1, 0 ) ), "draw clear" );
	Check( SUCCEEDED( device.BeginScene() ), "begin scene" );
	Check( SUCCEEDED(
	           device.DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vertices, sizeof( vertices[0] ) ) ),
	    "textured triangle draw" );
	Check( SUCCEEDED( device.EndScene() ), "end scene" );
	Readback( device, width, height, colors );
	Check( SUCCEEDED( device.SetTexture( 0, NULL ) ) &&
	           SUCCEEDED( device.SetVertexShader( NULL ) ) &&
	           SUCCEEDED( device.SetPixelShader( NULL ) ) &&
	           SUCCEEDED( device.SetVertexDeclaration( NULL ) ),
	    "draw resource unbind" );
	Check( texture->Release() == 0 && vertex->Release() == 0 && pixel->Release() == 0 &&
	           declaration->Release() == 0,
	    "draw resource release" );
}

void CoalescedDrag( SDL_Window *window, IDirect3DDevice9 &device,
    D3DPRESENT_PARAMETERS &parameters )
{
	const int requests = 48;
	int finalWidth = 0, finalHeight = 0;
	for ( int step = 0; step != requests; ++step )
	{
		finalWidth = 240 + ( step * 29 ) % 481;
		finalHeight = 180 + ( step * 17 ) % 361;
		Check( SDL_SetWindowSize( window, finalWidth, finalHeight ), "drag resize request" );
		// Keep full old-backbuffer frames flowing while configure events coalesce.
		SampledFrame( device, parameters.BackBufferWidth, parameters.BackBufferHeight );
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
			Check( event.type != SDL_EVENT_QUIT, "drag window remains open" );
	}
	Check( SDL_SyncWindow( window ), "drag compositor acknowledgment" );
	int logicalWidth = 0, logicalHeight = 0;
	int drawableWidth = 0, drawableHeight = 0;
	Check( SDL_GetWindowSize( window, &logicalWidth, &logicalHeight ), "drag logical size" );
	Check( logicalWidth == finalWidth && logicalHeight == finalHeight,
	    "drag converges to latest logical size" );
	Check( SDL_GetWindowSizeInPixels( window, &drawableWidth, &drawableHeight ) &&
	           drawableWidth > 0 && drawableHeight > 0,
	    "drag drawable size" );
	parameters.BackBufferWidth = drawableWidth;
	parameters.BackBufferHeight = drawableHeight;
	Check( SUCCEEDED( device.Reset( &parameters ) ), "one coalesced drag reset" );
	SampledFrame( device, drawableWidth, drawableHeight );
	std::printf( "DRAG requests=%d resets=1 frames=%d final_logical=%dx%d final_drawable=%dx%d\n",
	    requests, requests + 1, logicalWidth, logicalHeight, drawableWidth, drawableHeight );
}

void ResizeStress( SDL_Window *window, IDirect3DDevice9 &device, D3DPRESENT_PARAMETERS &parameters,
    bool negativeResizeOracle )
{
	for ( int step = 0; step != 64; ++step )
	{
		// Fixed workload and index identify the first divergence without randomness.
		// Both odd/even dimensions and changing aspect ratios exercise pixel sizing.
		const int width = 192 + ( step * 73 ) % 529;
		const int height = 144 + ( step * 47 ) % 389;
		std::printf( "resize=%d request=%dx%d\n", g_Resizes, width, height );
		Check( SDL_SetWindowSize( window, width, height ), "stress resize request" );

		// A native resize is asynchronous. Present the complete prior frame while
		// the compositor processes the new size; no clear-only frame is submitted.
		SampledFrame( device, parameters.BackBufferWidth, parameters.BackBufferHeight );
		Check( SDL_SyncWindow( window ), "stress compositor resize acknowledgment" );
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
			Check( event.type != SDL_EVENT_QUIT, "stress window remains open" );
		int logicalWidth = 0, logicalHeight = 0;
		int drawableWidth = 0, drawableHeight = 0;
		Check( SDL_GetWindowSize( window, &logicalWidth, &logicalHeight ), "stress logical size" );
		Check( logicalWidth == width && logicalHeight == height, "requested size converged" );
		Check( SDL_GetWindowSizeInPixels( window, &drawableWidth, &drawableHeight ),
		    "stress drawable size" );
		Check( drawableWidth > 0 && drawableHeight > 0, "nonempty drawable" );
		parameters.BackBufferWidth = drawableWidth;
		parameters.BackBufferHeight = drawableHeight;
		Check( SUCCEEDED( device.Reset( &parameters ) ), "stress swapchain reset" );
		D3DVIEWPORT9 viewport;
		Check( SUCCEEDED( device.GetViewport( &viewport ) ) &&
		           viewport.Width == static_cast<UINT>( drawableWidth ) &&
		           viewport.Height == static_cast<UINT>( drawableHeight ),
		    "reset viewport follows drawable pixels" );
		++g_Resizes;
		for ( int frame = 0; frame != 3; ++frame )
		{
			g_NegativePixelOracle = negativeResizeOracle && step == 2 && frame == 1;
			SampledFrame( device, drawableWidth, drawableHeight );
		}
	}
}

void RunWindow( bool stress, bool negativeResizeOracle )
{
	SDL_Window *window = SDL_CreateWindow(
	    "Source Vulkan conformance", 320, 240, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE );
	Check( window != NULL, "Vulkan-capable SDL3 window" );
	IDirect3D9 *d3d = Direct3DCreate9( D3D_SDK_VERSION );
	Check( d3d != NULL, "linked DXVK Native provider" );
	D3DADAPTER_IDENTIFIER9 adapter;
	Check( SUCCEEDED( d3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &adapter ) ),
	    "adapter identity" );
	std::printf( "driver=%s adapter=%s vendor=%04x device=%04x\n", SDL_GetCurrentVideoDriver(),
	    adapter.Description, adapter.VendorId, adapter.DeviceId );
	Check( adapter.VendorId != 0x10005 && !std::strstr( adapter.Description, "llvmpipe" ),
	    "hardware Vulkan adapter" );
	Check( SDL_SyncWindow( window ), "initial compositor acknowledgment" );
	int drawableWidth = 0, drawableHeight = 0;
	Check( SDL_GetWindowSizeInPixels( window, &drawableWidth, &drawableHeight ) &&
	           drawableWidth > 0 && drawableHeight > 0,
	    "initial drawable dimensions" );
	D3DPRESENT_PARAMETERS parameters = {};
	parameters.BackBufferWidth = drawableWidth;
	parameters.BackBufferHeight = drawableHeight;
	parameters.BackBufferFormat = D3DFMT_A8R8G8B8;
	parameters.BackBufferCount = 1;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = window;
	parameters.Windowed = TRUE;
	parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	IDirect3DDevice9 *device = NULL;
	Check( SUCCEEDED( d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
	           D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &parameters,
	           &device ) ) &&
	           device,
	    "native device" );
	Frame( *device, drawableWidth, drawableHeight, D3DCOLOR_XRGB( 37, 83, 149 ) );
	SampledFrame( *device, drawableWidth, drawableHeight );
	if ( stress )
	{
		CoalescedDrag( window, *device, parameters );
		ResizeStress( window, *device, parameters, negativeResizeOracle );
	}
	else
	{
		Check( SDL_SetWindowSize( window, 400, 300 ), "window resize" );
		Check( SDL_SyncWindow( window ), "compositor resize acknowledgment" );
		Check( SDL_GetWindowSizeInPixels( window, &drawableWidth, &drawableHeight ) &&
		           drawableWidth > 0 && drawableHeight > 0,
		    "resized drawable dimensions" );
		parameters.BackBufferWidth = drawableWidth;
		parameters.BackBufferHeight = drawableHeight;
		Check( SUCCEEDED( device->Reset( &parameters ) ), "swapchain reset" );
		Frame( *device, drawableWidth, drawableHeight, D3DCOLOR_XRGB( 173, 59, 101 ) );
		SampledFrame( *device, drawableWidth, drawableHeight );
	}
	Check( device->Release() == 0, "device release" );
	Check( d3d->Release() == 0, "provider release" );
	SDL_DestroyWindow( window );
}
}

int main( int argc, char **argv )
{
	const bool negative = argc == 2 && std::strcmp( argv[1], "--negative-pixel-oracle" ) == 0;
	const bool stress = argc == 2 && std::strcmp( argv[1], "--resize-stress" ) == 0;
	const bool negativeResize =
	    argc == 2 && std::strcmp( argv[1], "--negative-resize-pixel-oracle" ) == 0;
	Check( argc == 1 || negative || stress || negativeResize, "known test arguments" );
	g_NegativePixelOracle = negative;
	Check( SDL_Init( SDL_INIT_VIDEO ), "SDL video initialization" );
	const char *driver = SDL_GetCurrentVideoDriver();
	Check( driver && std::strcmp( driver, "wayland" ) == 0, "native Wayland driver" );
	const Uint64 started = SDL_GetTicks();
	const int lifetimes = stress || negativeResize ? 2 : 1;
	for ( int lifetime = 0; lifetime != lifetimes; ++lifetime )
		RunWindow( stress || negativeResize, negativeResize );
	const Uint64 elapsed = SDL_GetTicks() - started;
	SDL_Quit();
	std::printf( "PRESENTATION lifetimes=%d resizes=%d frames=%d elapsed_ms=%llu\n", lifetimes,
	    g_Resizes, g_Frames, static_cast<unsigned long long>( elapsed ) );
	std::printf( "CHECKS %d\n", g_Checks );
	return 0;
}
