//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef SHADERAPIDX9_DXVK_COMPAT_H
#define SHADERAPIDX9_DXVK_COMPAT_H

// Private SDK bridge for the linked DXVK Native D3D9 provider.
#include "tier0/platform.h"
// Source's historical LP64 compatibility aliases differ from the D3D ABI.
// Rename only these SDK aliases rather than changing preserved public layouts.
#define ULONG DXVK_ULONG
#define HMODULE DXVK_HMODULE
#undef TRUE
#include <d3d9.h>
#define __D3DX9_H__
#define LF_FACESIZE 32
struct TEXTMETRICA;
struct TEXTMETRICW;
#include <d3dx9math.h>
#include <d3dx9core.h>
#include <d3dx9shader.h>
#undef HMODULE
#undef ULONG

#undef ZeroMemory
#include "bitmap/imageformat.h"
namespace ImageLoader
{
ImageFormat D3DFormatToImageFormat( D3DFORMAT format );
D3DFORMAT ImageFormatToD3DFormat( ImageFormat format );
}

typedef HWND VD3DHWND;
typedef HANDLE VD3DHANDLE;
bool GetClientRect( HWND window, RECT *rect );
#define GLMPRINTF( args )
#define GLMPRINTSTR( args )
#define GLMPRINTTEXT( args )

#endif
