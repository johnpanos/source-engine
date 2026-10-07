//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The portal gun crosshair's shapes (the brackets and the last-placed
//			marker of sprites/hud/portal_crosshairs), rasterized at the
//			screen's pixel size so they stay sharp at any resolution and UI
//			scale. Shared by the Portal and Portal 2 crosshair elements.
//
//			Not original Valve source; the repository's provenance and
//			distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL_HUD_SHAPES_H
#define PORTAL_HUD_SHAPES_H
#ifdef _WIN32
#pragma once
#endif

class Color;

class CPortalHudShapes
{
public:
	// Each shape is one cell of the retail sheet, in that bitmap's units: the
	// left bracket (44x64) and the last-placed marker (28x64). The right
	// bracket is the left one turned 180 degrees about the cell centre.
	enum Shape
	{
		BRACKET_OUTLINE,
		BRACKET_FILL,
		LAST_PLACED,
		SHAPE_COUNT
	};

	CPortalHudShapes();
	~CPortalHudShapes();

	// Rasterizes the shapes for the current UI scale with a cell unit drawn
	// as flCellScale UI units; does nothing when neither has changed.
	void Update( float flCellScale );
	// The shapes are rebuilt on the next Update (a new video mode).
	void Invalidate() { m_flPixelsPerUnit = 0.0f; }

	float PixelsPerUnit() const { return m_flPixelsPerUnit; }
	float CellUnitsWide( Shape eShape ) const;
	float CellUnitsTall( Shape eShape ) const;

	// Draws a shape's cell centred on ( x, y ), or with its top-left corner at
	// ( x, y ), in UI units. flOffsetUnits moves the shape along the cell's x
	// axis (before the 180-degree turn), in cell units.
	void DrawCentred( Shape eShape, bool bRotate180, float x, float y, const Color &clr );
	void DrawCell( Shape eShape, bool bRotate180, float x, float y, const Color &clr,
	    float flOffsetUnits = 0.0f );
	// A square of whole pixels, nPixels wide, with its top-left at a pixel.
	void DrawPixels( float flLeftPx, float flTopPx, int nPixels, const Color &clr );

private:
	struct Image
	{
		int nTexture;
		int nWide; // image size in pixels, with one transparent pixel of border
		int nTall;
		float flOriginX; // the cell's top-left in the image, in pixels
		float flOriginY;
		float flMaxS; // the image's extent in its power-of-two texture
		float flMaxT;
	};

	void Rasterize( Shape eShape, float flShapeScale );
	void DrawImage(
	    const Image &image, float flLeftPx, float flTopPx, bool bRotate180, const Color &clr );

	float m_flPixelsPerUnit; // the UI scale the shapes were rasterized for; 0 before the first
	float m_flCellScale;
	Image m_Images[SHAPE_COUNT];
	int m_nWhiteTexture;
};

#endif // PORTAL_HUD_SHAPES_H
