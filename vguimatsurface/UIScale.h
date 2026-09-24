//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: UI scale policy for the material system surface. The surface lays
//          out and paints VGUI in UI units; the UI scale is the number of back
//          buffer pixels per UI unit. Proportional layouts already follow the
//          screen height, so the scale affects them only through their font
//          raster size; pixel-authored UI (menus, dialogs, console, HUD icons)
//          grows with it.
//
//=============================================================================//

#ifndef VGUIMATSURFACE_UISCALE_H
#define VGUIMATSURFACE_UISCALE_H

namespace uiscale
{

struct Inputs
{
	// The window system's UI scale for the window's display (SDL_GetWindowDisplayScale);
	// <= 0 when unknown.
	float displayScale;
	// The user's override (ui_scale); <= 0 selects the display scale.
	float userScale;
	// The surface's pixel extent: the video mode's back buffer.
	int backBufferWide;
	int backBufferTall;
	// The window's size in pixels; <= 0 when unknown. A back buffer presented at
	// a different size (a lower resolution scaled to the window) changes how
	// large each back buffer pixel appears.
	int drawableWide;
	int drawableTall;
};

// The limits of any UI scale.
const float kMinScale = 0.5f;
const float kMaxScale = 4.0f;
// The smallest UI extent a scale may reduce the screen to (the Source UI
// minimum). A screen already smaller than this keeps a scale of at most 1.
const int kMinUIWide = 640;
const int kMinUITall = 480;

// Back buffer pixels per UI unit, quantized to hundredths.
float ComputeScale( const Inputs &inputs );

// The UI units that cover a pixel extent, rounded up so the UI reaches the
// last pixel. Exact at scale 1.
int UnitsCoveringPixels( int pixels, float scale );

// The UI unit that contains a pixel coordinate, and the first pixel inside a UI
// unit. For scales of at least 1, UnitAtPixel( PixelAtUnit( u ) ) == u.
int UnitAtPixel( int pixel, float scale );
int PixelAtUnit( int unit, float scale );

// A metric measured in pixels expressed in whole UI units, and a size in UI
// units expressed in whole pixels, rounded to nearest.
int PixelsToUnitsRounded( int pixels, float scale );
int UnitsToPixelsRounded( int units, float scale );

} // namespace uiscale

#endif // VGUIMATSURFACE_UISCALE_H
