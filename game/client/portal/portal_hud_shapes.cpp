//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The portal gun crosshair's shapes, rasterized at the screen's pixel
//			size (see portal_hud_shapes.h).
//
//			Not original Valve source; the repository's provenance and
//			distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_hud_shapes.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"
#include "cdll_util.h"
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// The left bracket in the units of retail's 44x64 bitmap: the ring between an
// upright ellipse and a smaller one turned 12.2 degrees, cut at its two ends,
// with a 0.75-unit outline centred just outside the edge and the fill inset
// inside it. The constants are the least-squares solution against both retail
// bitmaps (empty and full), to about 8 of 255 levels RMS; each edge alone fits
// an ellipse to 0.12 pixels. Portal and Portal 2 ship the same full bitmaps;
// Portal's empty one sits one unit to the right (DrawCell's offset).
//
// The last-placed marker (28x64) is an upright ellipse outline, centred on the
// edge: the least-squares solution against Portal's bitmap, to 12 of 255
// levels RMS (its DXT5 noise).
//-----------------------------------------------------------------------------
struct HudEllipse
{
	float cx, cy;    // centre
	float rx, ry;    // semi-axes before rotation
	float flDegrees; // rotation, clockwise on screen
};
struct HudCut
{
	float x, y;      // a point on the line
	float flDegrees; // direction of the outward normal, clockwise from +x
};

static const float kBracketUnitsWide = 44.0f;
static const float kBracketUnitsTall = 64.0f;
static const HudEllipse kBracketOuter = { 28.079f, 47.364f, 26.454f, 45.871f, -0.442f };
static const HudEllipse kBracketInner = { 27.471f, 46.308f, 21.157f, 38.837f, 12.201f };
static const HudCut kBracketCuts[2] = {
    { 42.512f, 12.0f, -1.471f }, // top end
    { 4.0f, 62.115f, 87.952f },  // bottom end
};
static const float kBracketOutlineUnits = 0.747f;
static const float kBracketOutlineOffsetUnits = 0.212f; // band centre, outside the edge
static const float kBracketFillInsetUnits = 1.443f;

static const float kLastPlacedUnitsWide = 28.0f;
static const float kLastPlacedUnitsTall = 64.0f;
static const HudEllipse kLastPlaced = { 14.5f, 32.0f, 3.9f, 6.4f, 0.0f };
static const float kLastPlacedOutlineUnits = 1.2f;

//-----------------------------------------------------------------------------
// Signed distance to an ellipse, to first order (the implicit function over its
// gradient), negative inside: the measure the constants were solved with.
//-----------------------------------------------------------------------------
static float EllipseDistance( const HudEllipse &e, float x, float y )
{
	float s, c;
	SinCos( DEG2RAD( e.flDegrees ), &s, &c );
	const float u = ( x - e.cx ) * c + ( y - e.cy ) * s;
	const float v = -( x - e.cx ) * s + ( y - e.cy ) * c;
	const float f = ( u * u ) / ( e.rx * e.rx ) + ( v * v ) / ( e.ry * e.ry ) - 1.0f;
	const float g =
	    2.0f * FastSqrt( Square( u / ( e.rx * e.rx ) ) + Square( v / ( e.ry * e.ry ) ) );
	return f / MAX( g, 1e-6f );
}

static float CutDistance( const HudCut &cut, float x, float y )
{
	float s, c;
	SinCos( DEG2RAD( cut.flDegrees ), &s, &c );
	return ( x - cut.x ) * c + ( y - cut.y ) * s;
}

// Signed distance in units to the left bracket's edge, negative inside.
static float BracketDistance( float x, float y )
{
	float d =
	    MAX( EllipseDistance( kBracketOuter, x, y ), -EllipseDistance( kBracketInner, x, y ) );
	for ( int i = 0; i < ARRAYSIZE( kBracketCuts ); ++i )
		d = MAX( d, CutDistance( kBracketCuts[i], x, y ) );
	return d;
}

static unsigned char Coverage( float flValue )
{
	return (unsigned char)( clamp( flValue, 0.0f, 1.0f ) * 255.0f + 0.5f );
}

CPortalHudShapes::CPortalHudShapes()
{
	m_flPixelsPerUnit = 0.0f;
	m_flCellScale = 0.0f;
	for ( int i = 0; i < SHAPE_COUNT; ++i )
	{
		m_Images[i].nTexture = -1;
		m_Images[i].nWide = m_Images[i].nTall = 0;
		m_Images[i].flOriginX = m_Images[i].flOriginY = 0.0f;
		m_Images[i].flMaxS = m_Images[i].flMaxT = 1.0f;
	}
	m_nWhiteTexture = -1;
}

CPortalHudShapes::~CPortalHudShapes()
{
	if ( !vgui::surface() )
		return;
	for ( int i = 0; i < SHAPE_COUNT; ++i )
	{
		if ( m_Images[i].nTexture != -1 )
			vgui::surface()->DeleteTextureByID( m_Images[i].nTexture );
	}
	if ( m_nWhiteTexture != -1 )
		vgui::surface()->DeleteTextureByID( m_nWhiteTexture );
}

float CPortalHudShapes::CellUnitsWide( Shape eShape ) const
{
	return eShape == LAST_PLACED ? kLastPlacedUnitsWide : kBracketUnitsWide;
}

float CPortalHudShapes::CellUnitsTall( Shape eShape ) const
{
	return eShape == LAST_PLACED ? kLastPlacedUnitsTall : kBracketUnitsTall;
}

void CPortalHudShapes::Update( float flCellScale )
{
	float flUnitsPerPixelX, flUnitsPerPixelY;
	GetVGuiUnitsPerPixel( flUnitsPerPixelX, flUnitsPerPixelY );
	const float flScale = flUnitsPerPixelY > 0.0f ? 1.0f / flUnitsPerPixelY : 1.0f;
	if ( m_flPixelsPerUnit == flScale && m_flCellScale == flCellScale && m_nWhiteTexture != -1 )
		return;
	m_flPixelsPerUnit = flScale;
	m_flCellScale = flCellScale;

	if ( m_nWhiteTexture == -1 )
	{
		unsigned char white[4 * 4 * 4];
		V_memset( white, 255, sizeof( white ) );
		m_nWhiteTexture = vgui::surface()->CreateNewTextureID( true );
		vgui::surface()->DrawSetTextureRGBA( m_nWhiteTexture, white, 4, 4, 0, true );
	}

	for ( int i = 0; i < SHAPE_COUNT; ++i )
		Rasterize( (Shape)i, flScale * flCellScale );
}

//-----------------------------------------------------------------------------
// Purpose: Rasterizes one shape at flShapeScale pixels per cell unit. Coverage
// comes from the signed distance to the shape's edge, evaluated per screen
// pixel, so the edges are antialiased at the size they are drawn.
//-----------------------------------------------------------------------------
void CPortalHudShapes::Rasterize( Shape eShape, float flShapeScale )
{
	Image &image = m_Images[eShape];
	const float flCellWide = CellUnitsWide( eShape );
	const float flCellTall = CellUnitsTall( eShape );

	// One transparent pixel of border so edge texels never clamp onto ink.
	image.nWide = (int)ceilf( flCellWide * flShapeScale ) + 2;
	image.nTall = (int)ceilf( flCellTall * flShapeScale ) + 2;
	image.flOriginX = 0.5f * ( image.nWide - flCellWide * flShapeScale );
	image.flOriginY = 0.5f * ( image.nTall - flCellTall * flShapeScale );

	// The surface stores procedural textures at power-of-two sizes and maps
	// texture coordinates over the whole of them; upload the padded image.
	int nTextureWide = 1, nTextureTall = 1;
	while ( nTextureWide < image.nWide )
		nTextureWide <<= 1;
	while ( nTextureTall < image.nTall )
		nTextureTall <<= 1;
	image.flMaxS = (float)image.nWide / nTextureWide;
	image.flMaxT = (float)image.nTall / nTextureTall;

	CUtlVector<unsigned char> bits;
	bits.SetCount( nTextureWide * nTextureTall * 4 );
	V_memset( bits.Base(), 0, bits.Count() );
	for ( int y = 0; y < image.nTall; ++y )
	{
		for ( int x = 0; x < image.nWide; ++x )
		{
			const float flUnitX = ( x + 0.5f - image.flOriginX ) / flShapeScale;
			const float flUnitY = ( y + 0.5f - image.flOriginY ) / flShapeScale;

			// Signed distance in pixels at the pixel centre; coverage is a
			// one-pixel ramp across it.
			float flCoverage;
			if ( eShape == LAST_PLACED )
			{
				const float flSigned =
				    flShapeScale * EllipseDistance( kLastPlaced, flUnitX, flUnitY );
				flCoverage =
				    clamp( 0.5f * kLastPlacedOutlineUnits * flShapeScale - fabsf( flSigned ) + 0.5f,
				        0.0f, 1.0f );
			}
			else
			{
				const float flSigned = flShapeScale * BracketDistance( flUnitX, flUnitY );
				if ( eShape == BRACKET_OUTLINE )
				{
					const float flBandCentre = kBracketOutlineOffsetUnits * flShapeScale;
					flCoverage = clamp( 0.5f * kBracketOutlineUnits * flShapeScale -
					                        fabsf( flSigned - flBandCentre ) + 0.5f,
					    0.0f, 1.0f );
				}
				else
				{
					flCoverage = clamp(
					    -( flSigned + kBracketFillInsetUnits * flShapeScale ) + 0.5f, 0.0f, 1.0f );
				}
			}

			// The art is cropped to its cell; the ring's far side reaches past it.
			const float flOutsideCell = flShapeScale * MAX( MAX( -flUnitX, flUnitX - flCellWide ),
			                                               MAX( -flUnitY, flUnitY - flCellTall ) );
			const float flCellCoverage = clamp( 0.5f - flOutsideCell, 0.0f, 1.0f );

			const int n = ( y * nTextureWide + x ) * 4;
			bits[n + 0] = bits[n + 1] = bits[n + 2] = 255;
			bits[n + 3] = Coverage( flCellCoverage * flCoverage );
		}
	}

	// A procedural texture keeps its first size, so a new size needs a new one.
	if ( image.nTexture != -1 )
		vgui::surface()->DeleteTextureByID( image.nTexture );
	image.nTexture = vgui::surface()->CreateNewTextureID( true );
	vgui::surface()->DrawSetTextureRGBA(
	    image.nTexture, bits.Base(), nTextureWide, nTextureTall, 0, true );
}

//-----------------------------------------------------------------------------
// Purpose: Draws an image one texel per pixel at a pixel position, so the
// shapes keep the sharpness they were rasterized with.
//-----------------------------------------------------------------------------
void CPortalHudShapes::DrawImage(
    const Image &image, float flLeftPx, float flTopPx, bool bRotate180, const Color &clr )
{
	const float flUnits = 1.0f / m_flPixelsPerUnit;
	const float x0 = flLeftPx * flUnits, y0 = flTopPx * flUnits;
	const float x1 = ( flLeftPx + image.nWide ) * flUnits, y1 = ( flTopPx + image.nTall ) * flUnits;
	const float s0 = bRotate180 ? image.flMaxS : 0.0f, s1 = image.flMaxS - s0;
	const float t0 = bRotate180 ? image.flMaxT : 0.0f, t1 = image.flMaxT - t0;
	vgui::Vertex_t verts[4];
	verts[0].Init( Vector2D( x0, y0 ), Vector2D( s0, t0 ) );
	verts[1].Init( Vector2D( x1, y0 ), Vector2D( s1, t0 ) );
	verts[2].Init( Vector2D( x1, y1 ), Vector2D( s1, t1 ) );
	verts[3].Init( Vector2D( x0, y1 ), Vector2D( s0, t1 ) );
	vgui::surface()->DrawSetColor( clr );
	vgui::surface()->DrawSetTexture( image.nTexture );
	vgui::surface()->DrawTexturedPolygon( 4, verts );
}

void CPortalHudShapes::DrawCentred(
    Shape eShape, bool bRotate180, float x, float y, const Color &clr )
{
	const Image &image = m_Images[eShape];
	const float flLeft = floorf( x * m_flPixelsPerUnit - 0.5f * image.nWide + 0.5f );
	const float flTop = floorf( y * m_flPixelsPerUnit - 0.5f * image.nTall + 0.5f );
	DrawImage( image, flLeft, flTop, bRotate180, clr );
}

void CPortalHudShapes::DrawCell(
    Shape eShape, bool bRotate180, float x, float y, const Color &clr, float flOffsetUnits )
{
	const Image &image = m_Images[eShape];
	// The offset is along the cell's x axis, so it turns with the cell.
	const float flOffset = ( bRotate180 ? -flOffsetUnits : flOffsetUnits ) * m_flCellScale;
	const float flLeft = floorf( ( x + flOffset ) * m_flPixelsPerUnit - image.flOriginX + 0.5f );
	const float flTop = floorf( y * m_flPixelsPerUnit - image.flOriginY + 0.5f );
	DrawImage( image, flLeft, flTop, bRotate180, clr );
}

void CPortalHudShapes::DrawPixels( float flLeftPx, float flTopPx, int nPixels, const Color &clr )
{
	Image white;
	white.nTexture = m_nWhiteTexture;
	white.nWide = white.nTall = nPixels;
	white.flOriginX = white.flOriginY = 0.0f;
	white.flMaxS = white.flMaxT = 1.0f;
	DrawImage( white, flLeftPx, flTopPx, false, clr );
}
