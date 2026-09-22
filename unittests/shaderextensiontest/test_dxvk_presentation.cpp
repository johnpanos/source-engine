//========= Copyright Valve Corporation, All rights reserved. ============//
// Native integration gate: must run on Wayland with a Vulkan adapter. A missing
// display/provider is a failure, never a skipped or software-certified result.
#include "../../materialsystem/shaderapidx9/dxvk_compat.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
int g_Checks;
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
void Frame( IDirect3DDevice9 &device, UINT width, UINT height, D3DCOLOR color )
{
	Check( SUCCEEDED( device.Clear( 0, NULL, D3DCLEAR_TARGET, color, 1, 0 ) ), "clear" );
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
			match = match && ( row[x] == color );
	}
	Check( match, "exact GPU pixels" );
	Check( SUCCEEDED( readback->UnlockRect() ), "readback unlock" );
	readback->Release();
	target->Release();
	Check( SUCCEEDED( device.Present( NULL, NULL, NULL, NULL ) ), "present" );
}
}

int main()
{
	Check( SDL_Init( SDL_INIT_VIDEO ), "SDL video initialization" );
	const char *driver = SDL_GetCurrentVideoDriver();
	Check( driver && std::strcmp( driver, "wayland" ) == 0, "native Wayland driver" );
	SDL_Window *window = SDL_CreateWindow(
	    "Source Vulkan conformance", 320, 240, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE );
	Check( window != NULL, "Vulkan-capable SDL3 window" );
	IDirect3D9 *d3d = Direct3DCreate9( D3D_SDK_VERSION );
	Check( d3d != NULL, "linked DXVK Native provider" );
	D3DADAPTER_IDENTIFIER9 adapter;
	Check( SUCCEEDED( d3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &adapter ) ),
	    "adapter identity" );
	std::printf( "driver=%s adapter=%s vendor=%04x device=%04x\n", driver, adapter.Description,
	    adapter.VendorId, adapter.DeviceId );
	Check( adapter.VendorId != 0x10005 && !std::strstr( adapter.Description, "llvmpipe" ),
	    "hardware Vulkan adapter" );
	D3DPRESENT_PARAMETERS parameters = {};
	parameters.BackBufferWidth = 320;
	parameters.BackBufferHeight = 240;
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
	Frame( *device, 320, 240, D3DCOLOR_XRGB( 37, 83, 149 ) );
	Check( SDL_SetWindowSize( window, 400, 300 ), "window resize" );
	Check( SDL_SyncWindow( window ), "compositor resize acknowledgment" );
	parameters.BackBufferWidth = 400;
	parameters.BackBufferHeight = 300;
	Check( SUCCEEDED( device->Reset( &parameters ) ), "swapchain reset" );
	Frame( *device, 400, 300, D3DCOLOR_XRGB( 173, 59, 101 ) );
	Check( device->Release() == 0, "device release" );
	Check( d3d->Release() == 0, "provider release" );
	SDL_DestroyWindow( window );
	SDL_Quit();
	std::printf( "CHECKS %d\n", g_Checks );
	return 0;
}
