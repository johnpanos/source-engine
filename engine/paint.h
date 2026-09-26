//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Paint maps: Portal 2 gel on the luxels of lightmapped brush
//          surfaces (public/engine/ienginepaint.h). Ported from the CS:GO-era
//          engine's paint.cpp; the paint textures that engine's material
//          system kept are owned by the engine here (paint_render.cpp).
//
//=============================================================================//

#ifndef PAINT_H
#define PAINT_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/utlvector.h"
#include "mathlib/vector.h"
#include "mathlib/vector2d.h"
#include "surfacehandle.h"

struct model_t;
struct worldbrushdata_t;

// A paint map byte: power in the top three bits, alpha (0..31) in the rest.
namespace PaintByte
{
	const unsigned char NUM_ALPHA_BITS = 5;
	const unsigned char COLOR_BITS = 7 << NUM_ALPHA_BITS;
	const unsigned char ALPHA_BITS = COLOR_BITS ^ 0xFF;

	inline unsigned char Power( unsigned char b ) { return ( COLOR_BITS & b ) >> NUM_ALPHA_BITS; }
	inline float Alpha( unsigned char b ) { return ( ALPHA_BITS & b ) / float( ALPHA_BITS ); }
	unsigned char Make( unsigned char power, float alpha );
}

struct PaintRect_t
{
	Rect_t rect;
	Vector2D uvCenter;
	float flCenterAlpha;
	float flCircleRadius;
	float flPaintCoatPercent;
	unsigned char colorIndex;
	SurfaceHandle_t surfID;
};

// One lightmap page's paint: a byte per luxel, and the rectangles changed
// since the render cache last read them.
class CPaintPage
{
public:
	CPaintPage();
	~CPaintPage();

	void Init( int width, int height );

	int Width() const { return m_nWidth; }
	int Height() const { return m_nHeight; }
	const unsigned char *Data() const { return m_pData; }

	unsigned char GetPixel( int x, int y ) const { return m_pData[y * m_nWidth + x]; }
	void SetPixel( int x, int y, unsigned char value ) { m_pData[y * m_nWidth + x] = value; }

	void Clear( unsigned char value );

	// Returns true when a luxel's paint power changed.
	bool Paint( const PaintRect_t &paintRect );
	void GetPixels( const Rect_t &rect, CUtlVector<unsigned char> &surfColors ) const;

	// Change tracking for the render cache.
	void AddDirtyRect( const Rect_t &rect );
	void MarkAllDirty();
	bool IsDirty() const { return m_bAllDirty || m_DirtyRects.Count() > 0; }
	// The union of the changes since the last call; false when clean.
	bool TakeDirtyRect( Rect_t &rect );

private:
	CPaintPage( const CPaintPage & );
	CPaintPage &operator=( const CPaintPage & );

	unsigned int BlendLuxel( const PaintRect_t &paintRect, int x, int y, float flNewAlpha, float flMaxAlpha = 1.0f );
	unsigned int AddSurroundingAlpha( const PaintRect_t &paintRect, int x, int y );
	unsigned int DrawLine( const PaintRect_t &paintRect, int x1, int x2, int y );
	unsigned int Draw2Lines( const PaintRect_t &paintRect, float x, float y );
	unsigned int Draw4Lines( const PaintRect_t &paintRect, float x, float y );
	unsigned int DrawCircle( const PaintRect_t &paintRect );

	int m_nWidth;
	int m_nHeight;
	unsigned char *m_pData;
	bool m_bAllDirty;
	CUtlVector<Rect_t> m_DirtyRects;
};

class CPaintmapDataManager
{
public:
	CPaintmapDataManager();
	~CPaintmapDataManager();

	// Called once the map's lightmap pages and sort infos exist: allocates a
	// page of paint per lightmap page when the map allows paint.
	void AllocatePaintmaps();
	// Called with the sort infos: forgets the pages and the painted flags.
	void DestroyPaintmaps();

	bool HasPaintmap() const { return m_Pages.Count() > 0; }
	int PageCount() const { return m_Pages.Count(); }
	CPaintPage *Page( int nPage ) { return ( nPage >= 0 && nPage < m_Pages.Count() ) ? m_Pages[nPage] : NULL; }
	// Incremented whenever the pages are allocated or destroyed.
	int Generation() const { return m_nGeneration; }

	bool SpherePaint( const model_t *pModel, const Vector &vPosition, unsigned char colorIndex,
					  float flSphereRadius, float flPaintCoatPercent );
	void SphereTrace( const model_t *pModel, const Vector &vPosition, const Vector &vContactNormal,
					  float flSphereRadius, CUtlVector<unsigned char> &surfColors );
	void RemoveAllPaint();
	void RemovePaint( const model_t *pModel );
	void PaintAllSurfaces( unsigned char colorIndex );

	void GetPaintmapDataRLE( CUtlVector<uint32> &data );
	// Returns false (and stops) at the first record that does not fit this map.
	bool LoadPaintmapDataRLE( const CUtlVector<uint32> &data );

	// The page and in-page luxel rectangle of a lightmapped surface; false
	// for surfaces without a paint page.
	bool SurfaceRect( SurfaceHandle_t surfID, int &nPage, Rect_t &rect ) const;

private:
	void GetSurfacePaintData( SurfaceHandle_t surfID, CUtlVector<unsigned char> &data );
	bool SetSurfacePaintData( SurfaceHandle_t surfID, const unsigned char *pData, int nCount );

	CUtlVector<CPaintPage *> m_Pages;
	worldbrushdata_t *m_pWorld;
	int m_nGeneration;
};

extern CPaintmapDataManager g_PaintManager;

// Paint shared by the painted surfaces' render pass (paint_render.cpp).
void R_PaintLevelInit();
void R_PaintLevelShutdown();

#endif // PAINT_H
