//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Paint maps (see paint.h and public/engine/ienginepaint.h).
//
// Ported from the CS:GO-era engine's paint.cpp, which is what Portal 2's
// engine ships (its ConVars and svc_PaintmapData are in the retail
// engine.so). Differences:
//   - The pages live here, not behind the material system's paint map
//     manager, and are laid out after this engine's lightmap allocation.
//   - The paint is dropped with the map's sort infos, so no call can index a
//     page of another map.
//   - Painted surfaces do not mark their collision brushes with
//     CONTENTS_BRUSH_PAINT: this engine's contents have no free bit there, and
//     the Portal 2 game code queries all brushes for paint contacts.
//   - Surface, page and save data indices are bounds checked.
//
//=============================================================================//

#include "quakedef.h"
#include "paint.h"
#include "gl_model_private.h"
#include "gl_matsysiface.h"
#include "cmodel_engine.h"
#include "con_nprint.h"
#include "materialsystem/imaterialsystem.h"
#include "engine/ienginepaint.h"
#include "tier1/interface.h"
#include "vstdlib/random.h"
#include "bspflags.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar paint_max_surface_border_alpha(
    "paint_max_surface_border_alpha", "0.7f", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar paint_alpha_offset_enabled(
    "paint_alpha_offset_enabled", "1", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar paintsplat_bias( "paintsplat_bias", "0.1f", FCVAR_REPLICATED | FCVAR_CHEAT,
    "Change bias value for computing circle buffer" );
ConVar paintsplat_noise_enabled( "paintsplat_noise_enabled", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar paintsplat_max_alpha_noise( "paintsplat_max_alpha_noise", "0.1f",
    FCVAR_REPLICATED | FCVAR_CHEAT, "Max noise value of circle alpha" );
ConVar paint_min_valid_alpha_value(
    "paint_min_valid_alpha_value", "0.7f", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar debug_paint_alpha( "debug_paint_alpha", "0", FCVAR_DEVELOPMENTONLY );
ConVar paint_debug_trace( "paint_debug_trace", "0", FCVAR_CHEAT,
    "Print each paint map query: 1 traces, 2 traces and paint calls" );

CPaintmapDataManager g_PaintManager;

unsigned char PaintByte::Make( unsigned char power, float alpha )
{
	unsigned char nAlpha = static_cast<unsigned char>( alpha * ALPHA_BITS );
	unsigned char nColor = power << NUM_ALPHA_BITS;
	return nColor | nAlpha;
}

static const unsigned char BARE_LUXEL = ENGINE_PAINT_NO_POWER << PaintByte::NUM_ALPHA_BITS;

//-----------------------------------------------------------------------------
// Surface geometry in paint page space
//-----------------------------------------------------------------------------
struct PaintSurfaceSpan_t
{
	int sOffset;
	int tOffset;
	int sMax; // extent - 1: the last luxel's offset from the first
	int tMax;
};

static PaintSurfaceSpan_t SurfaceSpan( SurfaceHandle_t surfID )
{
	PaintSurfaceSpan_t span;
	span.sOffset = MSurf_OffsetIntoLightmapPage( surfID )[0];
	span.tOffset = MSurf_OffsetIntoLightmapPage( surfID )[1];
	span.sMax = MSurf_LightmapExtents( surfID )[0];
	span.tMax = MSurf_LightmapExtents( surfID )[1];
	return span;
}

static int SurfaceLightmapPage( SurfaceHandle_t surfID )
{
	if ( !materialSortInfoArray )
		return -1;
	return materialSortInfoArray[MSurf_MaterialSortID( surfID )].lightmapPageID;
}

//-----------------------------------------------------------------------------
// CPaintPage
//-----------------------------------------------------------------------------
CPaintPage::CPaintPage() : m_nWidth( 0 ), m_nHeight( 0 ), m_pData( NULL ), m_bAllDirty( false )
{
}

CPaintPage::~CPaintPage()
{
	delete[] m_pData;
}

void CPaintPage::Init( int width, int height )
{
	delete[] m_pData;
	m_nWidth = width;
	m_nHeight = height;
	m_pData = new unsigned char[width * height];
	Clear( BARE_LUXEL );
}

void CPaintPage::Clear( unsigned char value )
{
	V_memset( m_pData, value, m_nWidth * m_nHeight );
	MarkAllDirty();
}

void CPaintPage::AddDirtyRect( const Rect_t &rect )
{
	if ( !m_bAllDirty )
		m_DirtyRects.AddToTail( rect );
}

void CPaintPage::MarkAllDirty()
{
	m_bAllDirty = true;
	m_DirtyRects.RemoveAll();
}

bool CPaintPage::TakeDirtyRect( Rect_t &rect )
{
	if ( m_bAllDirty )
	{
		rect.x = rect.y = 0;
		rect.width = m_nWidth;
		rect.height = m_nHeight;
	}
	else if ( m_DirtyRects.Count() )
	{
		int minX = m_nWidth, minY = m_nHeight, maxX = 0, maxY = 0;
		for ( int i = 0; i < m_DirtyRects.Count(); ++i )
		{
			const Rect_t &r = m_DirtyRects[i];
			minX = MIN( minX, r.x );
			minY = MIN( minY, r.y );
			maxX = MAX( maxX, r.x + r.width );
			maxY = MAX( maxY, r.y + r.height );
		}
		rect.x = clamp( minX, 0, m_nWidth );
		rect.y = clamp( minY, 0, m_nHeight );
		rect.width = clamp( maxX, 0, m_nWidth ) - rect.x;
		rect.height = clamp( maxY, 0, m_nHeight ) - rect.y;
	}
	else
	{
		return false;
	}
	m_bAllDirty = false;
	m_DirtyRects.RemoveAll();
	return rect.width > 0 && rect.height > 0;
}

enum PaintChangeFlags_t
{
	TEXEL_CHANGED = 0x1,
	PAINT_POWER_CHANGED = 0x2,
};

static unsigned char BlendColor( unsigned char colorIndex, unsigned char nPrePixel, float flAlpha,
    float flPaintCoatPercent, float flMaxAlpha )
{
	// Erasing stomps the luxel; its power stays so the edge blend keeps the color.
	if ( colorIndex == ENGINE_PAINT_NO_POWER )
	{
		return PaintByte::Make(
		    paint_alpha_offset_enabled.GetBool() ? PaintByte::Power( nPrePixel ) : colorIndex,
		    0.0f );
	}

	float flNewAlpha =
	    clamp( PaintByte::Alpha( nPrePixel ) + ( flPaintCoatPercent * flAlpha ), 0.0f, flMaxAlpha );
	return PaintByte::Make( colorIndex, flNewAlpha );
}

static float ComputeCircleAlpha( const PaintRect_t &paintRect, int x, int y )
{
	float flPixelDist = Vector2D( x - paintRect.uvCenter.x, y - paintRect.uvCenter.y ).Length();
	float flRadiusRatio =
	    clamp( paintRect.flCenterAlpha + flPixelDist / paintRect.flCircleRadius, 0.0f, 1.0f );
	float flAlpha = 1.0f - Bias( flRadiusRatio, paintsplat_bias.GetFloat() );

	if ( paintsplat_noise_enabled.GetBool() )
	{
		float flNoise = paintsplat_max_alpha_noise.GetFloat();
		flAlpha += RandomFloat( -flNoise, flNoise );
		flAlpha = clamp( flAlpha, 0.0f, 1.0f );
	}

	return flAlpha;
}

unsigned int CPaintPage::BlendLuxel(
    const PaintRect_t &paintRect, int x, int y, float flNewAlpha, float flMaxAlpha )
{
	unsigned int nChangeFlags = 0;
	unsigned char nPrePixel = GetPixel( x, y );
	unsigned char nPostPixel = BlendColor(
	    paintRect.colorIndex, nPrePixel, flNewAlpha, paintRect.flPaintCoatPercent, flMaxAlpha );

	if ( nPrePixel != nPostPixel )
	{
		SetPixel( x, y, nPostPixel );
		nChangeFlags |= TEXEL_CHANGED;
		if ( PaintByte::Power( nPrePixel ) != PaintByte::Power( nPostPixel ) )
		{
			nChangeFlags |= PAINT_POWER_CHANGED;
		}
	}

	return nChangeFlags;
}

// Gives the luxels around an edge luxel the new color (at zero alpha), so the
// shader's bilinear fetch blends toward the paint color, not the old one.
unsigned int CPaintPage::AddSurroundingAlpha( const PaintRect_t &paintRect, int x, int y )
{
	const PaintSurfaceSpan_t span = SurfaceSpan( paintRect.surfID );
	const bool bLeft = x > span.sOffset;
	const bool bRight = x < span.sOffset + span.sMax;
	const bool bUp = y > span.tOffset;
	const bool bDown = y < span.tOffset + span.tMax;

	unsigned int nChangeFlags = 0;
	for ( int dy = -1; dy <= 1; ++dy )
	{
		if ( ( dy < 0 && !bUp ) || ( dy > 0 && !bDown ) )
			continue;
		for ( int dx = -1; dx <= 1; ++dx )
		{
			if ( ( dx == 0 && dy == 0 ) || ( dx < 0 && !bLeft ) || ( dx > 0 && !bRight ) )
				continue;
			nChangeFlags |= BlendLuxel( paintRect, x + dx, y + dy, 0.0f );
		}
	}

	return nChangeFlags;
}

unsigned int CPaintPage::DrawLine( const PaintRect_t &paintRect, int x1, int x2, int y )
{
	unsigned int nChangeFlags = 0;
	const PaintSurfaceSpan_t span = SurfaceSpan( paintRect.surfID );

	int start = x1;
	int end = x2;

	// Border luxels stay below paint_max_surface_border_alpha so the shader
	// won't fetch the color of the neighbouring surface in the page.
	float flMaxAlpha = 1.0f;
	if ( y == span.tOffset || y == span.tOffset + span.tMax )
	{
		flMaxAlpha = paint_max_surface_border_alpha.GetFloat();
	}
	else
	{
		if ( x1 == span.sOffset )
		{
			float flAlpha = ( paintRect.colorIndex == ENGINE_PAINT_NO_POWER )
			                    ? 1.0f
			                    : ComputeCircleAlpha( paintRect, x1, y );
			nChangeFlags |=
			    BlendLuxel( paintRect, x1, y, flAlpha, paint_max_surface_border_alpha.GetFloat() );
			++start;
		}
		if ( x2 == span.sOffset + span.sMax )
		{
			float flAlpha = ( paintRect.colorIndex == ENGINE_PAINT_NO_POWER )
			                    ? 1.0f
			                    : ComputeCircleAlpha( paintRect, x2, y );
			nChangeFlags |=
			    BlendLuxel( paintRect, x2, y, flAlpha, paint_max_surface_border_alpha.GetFloat() );
			--end;
		}
	}

	for ( int x = start; x <= end; ++x )
	{
		float flAlpha = ( paintRect.colorIndex == ENGINE_PAINT_NO_POWER )
		                    ? 1.0f
		                    : ComputeCircleAlpha( paintRect, x, y );
		nChangeFlags |= BlendLuxel( paintRect, x, y, flAlpha, flMaxAlpha );
	}

	// Erasing adds no alpha around the edge.
	if ( paint_alpha_offset_enabled.GetBool() && paintRect.colorIndex != ENGINE_PAINT_NO_POWER )
	{
		nChangeFlags |= AddSurroundingAlpha( paintRect, x1, y );
		if ( x1 != x2 )
		{
			nChangeFlags |= AddSurroundingAlpha( paintRect, x2, y );
		}
	}

	return nChangeFlags;
}

unsigned int CPaintPage::Draw2Lines( const PaintRect_t &paintRect, float x, float y )
{
	const Rect_t &rect = paintRect.rect;
	int minX = rect.x;
	int minY = rect.y;
	int maxX = rect.x + rect.width - 1;
	int maxY = rect.y + rect.height - 1;

	const Vector2D &uvCenter = paintRect.uvCenter;

	int x1 = MAX( (int)( uvCenter.x - x - 0.5f ), minX );
	int y1 = MAX( (int)( uvCenter.y - y - 0.5f ), minY );
	int x2 = MIN( (int)( uvCenter.x + x + 0.5f ), maxX );
	int y2 = MIN( (int)( uvCenter.y + y + 0.5f ), maxY );

	if ( x1 > maxX || x2 < minX )
		return 0;

	unsigned int nChangeFlags = 0;
	if ( minY <= y1 && y1 <= maxY )
	{
		nChangeFlags |= DrawLine( paintRect, x1, x2, y1 );
	}

	if ( y1 != y2 && minY <= y2 && y2 <= maxY )
	{
		nChangeFlags |= DrawLine( paintRect, x1, x2, y2 );
	}

	return nChangeFlags;
}

unsigned int CPaintPage::Draw4Lines( const PaintRect_t &paintRect, float x, float y )
{
	unsigned int nChangeFlags = Draw2Lines( paintRect, x, y );
	if ( x != y )
	{
		nChangeFlags |= Draw2Lines( paintRect, y, x );
	}
	return nChangeFlags;
}

// Midpoint circle: scan lines between the octant points.
unsigned int CPaintPage::DrawCircle( const PaintRect_t &paintRect )
{
	float radius = paintRect.flCircleRadius;
	float error = -radius;
	float x = radius;
	float y = 0;

	unsigned int nChangeFlags = 0;

	while ( x >= y )
	{
		nChangeFlags |= Draw4Lines( paintRect, x, y );

		error += y;
		++y;
		error += y;

		if ( error >= 0 )
		{
			--x;
			error -= x;
			error -= x;
		}
	}

	return nChangeFlags;
}

bool CPaintPage::Paint( const PaintRect_t &paintRect )
{
	unsigned int nChangeFlags = DrawCircle( paintRect );
	if ( nChangeFlags & TEXEL_CHANGED )
	{
		Rect_t rect = paintRect.rect;
		// The edge alpha reaches one luxel beyond the circle.
		if ( paint_alpha_offset_enabled.GetBool() && paintRect.colorIndex != ENGINE_PAINT_NO_POWER )
		{
			const PaintSurfaceSpan_t span = SurfaceSpan( paintRect.surfID );
			if ( paintRect.rect.x > span.sOffset )
			{
				--rect.x;
				++rect.width;
			}
			if ( paintRect.rect.y > span.tOffset )
			{
				--rect.y;
				++rect.height;
			}
			if ( paintRect.rect.x + paintRect.rect.width - 1 < span.sOffset + span.sMax )
			{
				++rect.width;
			}
			if ( paintRect.rect.y + paintRect.rect.height - 1 < span.tOffset + span.tMax )
			{
				++rect.height;
			}
		}
		AddDirtyRect( rect );
	}
	return ( nChangeFlags & PAINT_POWER_CHANGED ) != 0;
}

void CPaintPage::GetPixels( const Rect_t &rect, CUtlVector<unsigned char> &surfColors ) const
{
	for ( int y = 0; y < rect.height; ++y )
	{
		for ( int x = 0; x < rect.width; ++x )
		{
			unsigned char packedColor = GetPixel( rect.x + x, rect.y + y );
			float flAlpha = PaintByte::Alpha( packedColor );

			if ( flAlpha > paint_min_valid_alpha_value.GetFloat() )
				surfColors.AddToTail( PaintByte::Power( packedColor ) );

			if ( debug_paint_alpha.GetBool() )
			{
				Con_NPrintf( y * rect.width + x, "(%d, %d), Alpha: %f\n", x, y, flAlpha );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Finding the surfaces a sphere touches
//-----------------------------------------------------------------------------
struct paintinfo_t
{
	Vector m_vPosition;
	VPlane m_plane; // the plane of the surface closest to the sphere's center
	worldbrushdata_t *m_pBrush;
	float m_flSize;            // sphere radius
	float m_flCurrentDistance; // squared distance to the closest surface so far
	CUtlVector<SurfaceHandle_t> m_aApplySurfs;
};

static Vector FindClosestPointToTriangle(
    const Vector &p, const Vector &a, const Vector &b, const Vector &c )
{
	Vector ab = b - a;
	Vector ac = c - a;
	Vector bc = c - b;

	float projABnom = DotProduct( p - a, ab ), projABdenom = DotProduct( p - b, a - b );
	float projACnom = DotProduct( p - a, ac ), projACdenom = DotProduct( p - c, a - c );

	if ( projABnom <= 0.0f && projACnom <= 0.0f )
		return a;

	float projBCnom = DotProduct( p - b, bc ), projBCdenom = DotProduct( p - c, b - c );

	if ( projABdenom <= 0.0f && projBCnom <= 0.0f )
		return b;

	if ( projACdenom <= 0.0f && projBCdenom <= 0.0f )
		return c;

	Vector n = CrossProduct( ab, ac );

	float tpC = DotProduct( n, CrossProduct( a - p, b - p ) );
	if ( tpC <= 0.0f && projABnom >= 0.0f && projABdenom >= 0.0f )
		return a + projABnom / ( projABnom + projABdenom ) * ab;

	float tpA = DotProduct( n, CrossProduct( b - p, c - p ) );
	if ( tpA <= 0.0f && projBCnom >= 0.0f && projBCdenom >= 0.0f )
		return b + projBCnom / ( projBCnom + projBCdenom ) * bc;

	float tpB = DotProduct( n, CrossProduct( c - p, a - p ) );
	if ( tpB <= 0.0f && projACnom >= 0.0f && projACdenom >= 0.0f )
		return a + projACnom / ( projACnom + projACdenom ) * ac;

	// Inside the triangle: barycentric coordinates.
	float u = tpA / ( tpA + tpB + tpC );
	float v = tpB / ( tpA + tpB + tpC );
	float w = 1.0f - u - v;
	return u * a + v * b + w * c;
}

static bool IsSphereIntersectForwardFacingTriangle( const Vector &vCenter, const VPlane &plane,
    float flRadius, const Vector &a, const Vector &b, const Vector &c, Vector &vClosestPoint )
{
	vClosestPoint = FindClosestPointToTriangle( vCenter, a, b, c );
	float flDistFromPlane = DotProduct( vClosestPoint, plane.m_Normal ) - plane.m_Dist;
	if ( flDistFromPlane < 0.0f )
	{
		vClosestPoint = vClosestPoint + ( flDistFromPlane + 0.1f ) * plane.m_Normal;
	}

	Vector vDist = vCenter - vClosestPoint;
	return DotProduct( vDist, vDist ) < Square( flRadius );
}

static const Vector &SurfaceVertex( worldbrushdata_t *pBrush, SurfaceHandle_t surfID, int i )
{
	return pBrush->vertexes[pBrush->vertindices[MSurf_FirstVertIndex( surfID ) + i]].position;
}

static void R_AddPaintToSurface( SurfaceHandle_t surfID, paintinfo_t *paintinfo )
{
	// SURF_NOPAINT is SURF_NODECALS in Portal 2's bspflags.h.
	if ( ( MSurf_TexInfo( surfID )->flags & SURF_NODECALS ) ||
	     ( MSurf_Flags( surfID ) & SURFDRAW_TRANS ) )
		return;

	if ( SurfaceHasDispInfo( surfID ) )
		return;

	if ( paintinfo->m_aApplySurfs.Find( surfID ) != -1 )
		return;

	// Fullbright surfaces have no paint page.
	if ( SurfaceLightmapPage( surfID ) < 0 )
		return;

	VPlane plane = MSurf_GetForwardFacingPlane( surfID );
	float flDistFromPlane = DotProduct( paintinfo->m_vPosition, plane.m_Normal ) - plane.m_Dist;

	// Behind the plane, or too far in front of it.
	if ( flDistFromPlane > paintinfo->m_flSize || flDistFromPlane < 0.0f )
		return;

	int numVert = MSurf_VertCount( surfID );
	const Vector &vOrigin = SurfaceVertex( paintinfo->m_pBrush, surfID, 0 );
	bool bIntersect = false;
	Vector vClosestPoint;
	for ( int v = 1; v < numVert - 1; ++v )
	{
		const Vector &v1 = SurfaceVertex( paintinfo->m_pBrush, surfID, v );
		const Vector &v2 = SurfaceVertex( paintinfo->m_pBrush, surfID, v + 1 );

		Vector vIntersectPoint;
		if ( IsSphereIntersectForwardFacingTriangle( paintinfo->m_vPosition, plane,
		         paintinfo->m_flSize, vOrigin, v1, v2, vIntersectPoint ) )
		{
			if ( !bIntersect || ( vIntersectPoint - paintinfo->m_vPosition ).LengthSqr() <
			                        ( vClosestPoint - paintinfo->m_vPosition ).LengthSqr() )
			{
				vClosestPoint = vIntersectPoint;
			}
			bIntersect = true;
		}
	}

	if ( bIntersect )
	{
		float flDistFromClosestPointSquare = ( vClosestPoint - paintinfo->m_vPosition ).LengthSqr();
		if ( flDistFromClosestPointSquare < paintinfo->m_flCurrentDistance )
		{
			paintinfo->m_flCurrentDistance = flDistFromClosestPointSquare;
			paintinfo->m_plane = plane;
		}

		paintinfo->m_aApplySurfs.AddToTail( surfID );
	}
}

static void R_PaintNode( mnode_t *node, paintinfo_t *paintinfo )
{
	while ( node )
	{
		if ( node->contents >= 0 )
		{
			mleaf_t *pLeaf = (mleaf_t *)node;
			SurfaceHandle_t *pHandle = &paintinfo->m_pBrush->marksurfaces[pLeaf->firstmarksurface];
			for ( int i = 0; i < pLeaf->nummarksurfaces; i++ )
			{
				R_AddPaintToSurface( pHandle[i], paintinfo );
			}
			return;
		}

		cplane_t *splitplane = node->plane;
		float dist = DotProduct( paintinfo->m_vPosition, splitplane->normal ) - splitplane->dist;

		if ( dist > paintinfo->m_flSize )
		{
			node = node->children[0];
		}
		else if ( dist < -paintinfo->m_flSize )
		{
			node = node->children[1];
		}
		else
		{
			R_PaintNode( node->children[0], paintinfo );
			node = node->children[1];
		}
	}
}

static bool IsSurfaceInFrontOfPlane(
    worldbrushdata_t *pBrush, SurfaceHandle_t surfID, const VPlane &plane )
{
	// Coplanar with the main surface.
	VPlane trianglePlane = MSurf_GetForwardFacingPlane( surfID );
	if ( AlmostEqual( DotProduct( plane.m_Normal, trianglePlane.m_Normal ), 1.0f ) &&
	     AlmostEqual( plane.m_Dist, trianglePlane.m_Dist ) )
		return true;

	if ( MSurf_VertCount( surfID ) < 3 )
		return false;

	// Ignore it when the center of its first triangle is behind the main plane.
	const Vector &vOrigin = SurfaceVertex( pBrush, surfID, 0 );
	const Vector &v1 = SurfaceVertex( pBrush, surfID, 1 );
	const Vector &v2 = SurfaceVertex( pBrush, surfID, 2 );
	Vector vCenter = vOrigin + 0.25f * ( ( v1 - vOrigin ) + ( v2 - vOrigin ) );
	vCenter += 0.1f * trianglePlane.m_Normal; // off the plane, for float error
	return plane.DistTo( vCenter ) >= 0.0f;
}

static void CollectSurfaces(
    const model_t *pModel, const Vector &vPosition, float flSphereRadius, paintinfo_t &paintinfo )
{
	paintinfo.m_vPosition = vPosition;
	paintinfo.m_flSize = flSphereRadius;
	paintinfo.m_flCurrentDistance = FLT_MAX;
	paintinfo.m_pBrush = pModel->brush.pShared;
	R_PaintNode( paintinfo.m_pBrush->nodes + pModel->brush.firstnode, &paintinfo );
}

static void ProjectPointOntoSurfaceTexture(
    SurfaceHandle_t surfID, const Vector &vPoint, Vector2D &uv )
{
	mtexinfo_t *pTexInfo = MSurf_TexInfo( surfID );

	uv.x = DotProduct( vPoint, pTexInfo->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D() ) +
	       pTexInfo->lightmapVecsLuxelsPerWorldUnits[0][3];
	uv.x -= MSurf_LightmapMins( surfID )[0];
	uv.x += 0.5f;

	uv.y = DotProduct( vPoint, pTexInfo->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D() ) +
	       pTexInfo->lightmapVecsLuxelsPerWorldUnits[1][3];
	uv.y -= MSurf_LightmapMins( surfID )[1];
	uv.y += 0.5f;

	// To luxels in the page.
	uv.x += MSurf_OffsetIntoLightmapPage( surfID )[0];
	uv.y += MSurf_OffsetIntoLightmapPage( surfID )[1];
}

static bool ComputePaintRect(
    SurfaceHandle_t surfID, const Vector &vPosition, float flSphereRadius, PaintRect_t &paintRect )
{
	VPlane forwardFacingPlane = MSurf_GetForwardFacingPlane( surfID );
	float distFromPlane = forwardFacingPlane.DistTo( vPosition );
	if ( distFromPlane >= flSphereRadius )
		return false;

	float circleRadius =
	    FastSqrt( flSphereRadius * flSphereRadius - distFromPlane * distFromPlane );

	Vector2D uvCenter, uvExtents;
	ProjectPointOntoSurfaceTexture( surfID, vPosition, uvCenter );
	mtexinfo_t *pTexInfo = MSurf_TexInfo( surfID );
	uvExtents.x = circleRadius * pTexInfo->lightmapVecsLuxelsPerWorldUnits[0].AsVector3D().Length();
	uvExtents.y = circleRadius * pTexInfo->lightmapVecsLuxelsPerWorldUnits[1].AsVector3D().Length();
	if ( uvExtents.x <= 0.0f || uvExtents.y <= 0.0f )
		return false;

	Vector2D uvMins = uvCenter - uvExtents;
	Vector2D uvMaxs = uvCenter + uvExtents;

	const PaintSurfaceSpan_t span = SurfaceSpan( surfID );
	if ( !( span.sOffset <= uvMaxs.x && uvMins.x <= span.sOffset + span.sMax &&
	         span.tOffset <= uvMaxs.y && uvMins.y <= span.tOffset + span.tMax ) )
		return false;

	float flRoundedCircleRadius = floor( MAX( uvExtents.x, uvExtents.y ) + 0.5f );
	int surfWidth = span.sMax + 1;
	int surfHeight = span.tMax + 1;

	paintRect.flCenterAlpha = distFromPlane / flSphereRadius;
	paintRect.flCircleRadius = flRoundedCircleRadius;
	paintRect.uvCenter = uvCenter;
	paintRect.surfID = surfID;

	int startX = MAX( (int)( uvCenter.x - flRoundedCircleRadius - 0.5f ), span.sOffset );
	int startY = MAX( (int)( uvCenter.y - flRoundedCircleRadius - 0.5f ), span.tOffset );
	int endX = MIN( (int)( uvCenter.x + flRoundedCircleRadius + 0.5f ), span.sOffset + surfWidth );
	int endY = MIN( (int)( uvCenter.y + flRoundedCircleRadius + 0.5f ), span.tOffset + surfHeight );

	paintRect.rect.x = startX;
	paintRect.rect.y = startY;
	paintRect.rect.width = endX - startX;
	paintRect.rect.height = endY - startY;

	return paintRect.rect.width > 0 && paintRect.rect.height > 0 && flRoundedCircleRadius > 0.0f;
}

//-----------------------------------------------------------------------------
// CPaintmapDataManager
//-----------------------------------------------------------------------------
CPaintmapDataManager::CPaintmapDataManager() : m_pWorld( NULL ), m_nGeneration( 0 )
{
}

CPaintmapDataManager::~CPaintmapDataManager()
{
	m_Pages.PurgeAndDeleteElements();
}

// "paintinmap" "1" in the worldspawn block of the map's entity lump.
static bool MapAllowsPaint()
{
	const char *pEntities = CM_EntityString();
	if ( !pEntities )
		return false;
	const char *pEnd = strchr( pEntities, '}' );
	const char *pKey = V_stristr( pEntities, "\"paintinmap\"" );
	if ( !pKey || ( pEnd && pKey > pEnd ) )
		return false;
	const char *pValue = strchr( pKey + V_strlen( "\"paintinmap\"" ), '"' );
	return pValue && ( !pEnd || pValue < pEnd ) && atoi( pValue + 1 ) != 0;
}

void CPaintmapDataManager::AllocatePaintmaps()
{
	DestroyPaintmaps();
	if ( !materialSortInfoArray || !host_state.worldbrush || !MapAllowsPaint() )
		return;

	int nPages = 0;
	int nSortIDs = materials->GetNumSortIDs();
	for ( int i = 0; i < nSortIDs; ++i )
	{
		nPages = MAX( nPages, materialSortInfoArray[i].lightmapPageID + 1 );
	}

	for ( int i = 0; i < nPages; ++i )
	{
		int width = 0, height = 0;
		materials->GetLightmapPageSize( i, &width, &height );
		CPaintPage *pPage = new CPaintPage;
		if ( width > 0 && height > 0 )
		{
			pPage->Init( width, height );
		}
		m_Pages.AddToTail( pPage );
	}

	m_pWorld = host_state.worldbrush;
	++m_nGeneration;
	DevMsg( "Paint: %d paint map page(s)\n", nPages );
}

void CPaintmapDataManager::DestroyPaintmaps()
{
	if ( m_Pages.Count() && m_pWorld )
	{
		for ( int i = 0; i < m_pWorld->numsurfaces; ++i )
		{
			MSurf_Flags( SurfaceHandleFromIndex( i, m_pWorld ) ) &= ~SURFDRAW_PAINTED;
		}
	}
	m_Pages.PurgeAndDeleteElements();
	m_pWorld = NULL;
	++m_nGeneration;
}

bool CPaintmapDataManager::SurfaceRect( SurfaceHandle_t surfID, int &nPage, Rect_t &rect ) const
{
	nPage = SurfaceLightmapPage( surfID );
	if ( nPage < 0 || nPage >= m_Pages.Count() || !m_Pages[nPage]->Data() )
		return false;
	const PaintSurfaceSpan_t span = SurfaceSpan( surfID );
	rect.x = span.sOffset;
	rect.y = span.tOffset;
	rect.width = span.sMax + 1;
	rect.height = span.tMax + 1;
	const CPaintPage *pPage = m_Pages[nPage];
	return rect.x >= 0 && rect.y >= 0 && rect.width > 0 && rect.height > 0 &&
	       rect.x + rect.width <= pPage->Width() && rect.y + rect.height <= pPage->Height();
}

bool CPaintmapDataManager::SpherePaint( const model_t *pModel, const Vector &vPosition,
    unsigned char colorIndex, float flSphereRadius, float flPaintCoatPercent )
{
	if ( !HasPaintmap() || !pModel || pModel->brush.pShared != m_pWorld ||
	     colorIndex > ENGINE_PAINT_NO_POWER )
		return false;

	paintinfo_t paintinfo;
	CollectSurfaces( pModel, vPosition, flSphereRadius, paintinfo );

	bool bChangedPaint = false;
	for ( int i = 0; i < paintinfo.m_aApplySurfs.Count(); ++i )
	{
		SurfaceHandle_t surfID = paintinfo.m_aApplySurfs[i];
		// Surfaces behind the main surface are not painted.
		if ( !IsSurfaceInFrontOfPlane( paintinfo.m_pBrush, surfID, paintinfo.m_plane ) )
			continue;

		int nPage;
		Rect_t surfRect;
		PaintRect_t paintRect;
		paintRect.colorIndex = colorIndex;
		paintRect.flPaintCoatPercent = flPaintCoatPercent;
		if ( !SurfaceRect( surfID, nPage, surfRect ) ||
		     !ComputePaintRect( surfID, vPosition, flSphereRadius, paintRect ) )
			continue;

		if ( m_Pages[nPage]->Paint( paintRect ) )
		{
			MSurf_Flags( surfID ) |= SURFDRAW_PAINTED;
			bChangedPaint = true;
		}
	}
	if ( paint_debug_trace.GetInt() >= 2 )
	{
		Msg( "paint (%.0f %.0f %.0f) power %d r%.0f: %d surfaces, %s\n", vPosition.x, vPosition.y,
		    vPosition.z, colorIndex, flSphereRadius, paintinfo.m_aApplySurfs.Count(),
		    bChangedPaint ? "changed" : "unchanged" );
	}
	return bChangedPaint;
}

void CPaintmapDataManager::SphereTrace( const model_t *pModel, const Vector &vPosition,
    const Vector &vContactNormal, float flSphereRadius, CUtlVector<unsigned char> &surfColors )
{
	surfColors.RemoveAll();
	if ( !HasPaintmap() || !pModel || pModel->brush.pShared != m_pWorld )
		return;

	paintinfo_t paintinfo;
	CollectSurfaces( pModel, vPosition, flSphereRadius, paintinfo );

	for ( int i = 0; i < paintinfo.m_aApplySurfs.Count(); ++i )
	{
		SurfaceHandle_t surfID = paintinfo.m_aApplySurfs[i];
		if ( !( MSurf_Flags( surfID ) & SURFDRAW_PAINTED ) )
			continue;

		// Only paint on surfaces facing the contact.
		if ( DotProduct( vContactNormal, MSurf_GetForwardFacingPlane( surfID ).m_Normal ) < 0.9f )
			continue;

		int nPage;
		Rect_t surfRect;
		PaintRect_t paintRect;
		if ( SurfaceRect( surfID, nPage, surfRect ) &&
		     ComputePaintRect( surfID, vPosition, flSphereRadius, paintRect ) )
		{
			m_Pages[nPage]->GetPixels( paintRect.rect, surfColors );
		}
	}

	if ( paint_debug_trace.GetBool() )
	{
		int nCounts[ENGINE_PAINT_POWER_COUNT + 1] = {};
		for ( int i = 0; i < surfColors.Count(); ++i )
			++nCounts[MIN( (int)surfColors[i], ENGINE_PAINT_POWER_COUNT )];
		Msg( "paint trace (%.0f %.0f %.0f) n(%.2f %.2f %.2f) r%.0f: %d surfaces, luxels %d %d %d "
		     "%d bare %d\n",
		    vPosition.x, vPosition.y, vPosition.z, vContactNormal.x, vContactNormal.y,
		    vContactNormal.z, flSphereRadius, paintinfo.m_aApplySurfs.Count(), nCounts[0],
		    nCounts[1], nCounts[2], nCounts[3], nCounts[ENGINE_PAINT_POWER_COUNT] );
	}
}

void CPaintmapDataManager::RemoveAllPaint()
{
	for ( int i = 0; i < m_Pages.Count(); ++i )
	{
		if ( m_Pages[i]->Data() )
			m_Pages[i]->Clear( BARE_LUXEL );
	}
	if ( m_Pages.Count() && m_pWorld )
	{
		for ( int i = 0; i < m_pWorld->numsurfaces; ++i )
		{
			MSurf_Flags( SurfaceHandleFromIndex( i, m_pWorld ) ) &= ~SURFDRAW_PAINTED;
		}
	}
}

void CPaintmapDataManager::RemovePaint( const model_t *pModel )
{
	if ( !HasPaintmap() || !pModel || pModel->brush.pShared != m_pWorld )
		return;

	for ( int i = 0; i < pModel->brush.nummodelsurfaces; ++i )
	{
		SurfaceHandle_t surfID =
		    SurfaceHandleFromIndex( pModel->brush.firstmodelsurface + i, m_pWorld );
		int nPage;
		Rect_t rect;
		if ( !SurfaceRect( surfID, nPage, rect ) )
			continue;
		CPaintPage *pPage = m_Pages[nPage];
		for ( int y = 0; y < rect.height; ++y )
		{
			V_memset( const_cast<unsigned char *>( pPage->Data() ) +
			              ( rect.y + y ) * pPage->Width() + rect.x,
			    BARE_LUXEL, rect.width );
		}
		pPage->AddDirtyRect( rect );
	}
}

void CPaintmapDataManager::PaintAllSurfaces( unsigned char colorIndex )
{
	if ( !HasPaintmap() || colorIndex > ENGINE_PAINT_NO_POWER )
		return;

	for ( int i = 0; i < m_Pages.Count(); ++i )
	{
		if ( m_Pages[i]->Data() )
			m_Pages[i]->Clear( PaintByte::Make( colorIndex, 1.0f ) );
	}
	for ( int i = 0; i < m_pWorld->numsurfaces; ++i )
	{
		MSurf_Flags( SurfaceHandleFromIndex( i, m_pWorld ) ) |= SURFDRAW_PAINTED;
	}
}

//-----------------------------------------------------------------------------
// Save data: per painted surface, its index, the RLE size in dwords and the
// RLE data of its luxels (rows of the surface's rectangle, padded to a dword).
// An RLE record is a negative count and one dword repeated -count times, or a
// positive count followed by that many literal dwords.
//-----------------------------------------------------------------------------
static void EncodeDataRLE( const uint32 *pBuffer, int nDwordCount, CUtlVector<uint32> &data )
{
	int i = 0;
	int nLiteralStart = 0;
	while ( i < nDwordCount )
	{
		// A run pays for itself from three equal dwords.
		int nRun = 1;
		while ( i + nRun < nDwordCount && pBuffer[i + nRun] == pBuffer[i] )
			++nRun;
		if ( nRun >= 3 )
		{
			if ( i > nLiteralStart )
			{
				data.AddToTail( uint32( i - nLiteralStart ) );
				data.AddMultipleToTail( i - nLiteralStart, pBuffer + nLiteralStart );
			}
			data.AddToTail( uint32( -nRun ) );
			data.AddToTail( pBuffer[i] );
			i += nRun;
			nLiteralStart = i;
		}
		else
		{
			i += nRun;
		}
	}
	if ( nDwordCount > nLiteralStart )
	{
		data.AddToTail( uint32( nDwordCount - nLiteralStart ) );
		data.AddMultipleToTail( nDwordCount - nLiteralStart, pBuffer + nLiteralStart );
	}
}

// Returns false when the records do not decode to exactly nDwordCount dwords.
static bool DecodeDataRLE( const uint32 *pRLE, int nRLECount, uint32 *pOutput, int nDwordCount )
{
	int nRead = 0;
	int nWritten = 0;
	while ( nRead < nRLECount )
	{
		int32 nCount = int32( pRLE[nRead++] );
		if ( nCount < 0 )
		{
			if ( nCount < -nDwordCount || nRead >= nRLECount || nWritten - nCount > nDwordCount )
				return false;
			uint32 nValue = pRLE[nRead++];
			for ( int i = 0; i < -nCount; ++i )
				pOutput[nWritten++] = nValue;
		}
		else
		{
			if ( nCount == 0 || nCount > nRLECount - nRead || nWritten + nCount > nDwordCount )
				return false;
			for ( int i = 0; i < nCount; ++i )
				pOutput[nWritten++] = pRLE[nRead++];
		}
	}
	return nWritten == nDwordCount;
}

static int PaddedByteCount( const Rect_t &rect )
{
	int nBytes = rect.width * rect.height;
	return nBytes + ( ( 4 - ( nBytes % 4 ) ) % 4 );
}

void CPaintmapDataManager::GetSurfacePaintData(
    SurfaceHandle_t surfID, CUtlVector<unsigned char> &data )
{
	int nPage;
	Rect_t rect;
	data.RemoveAll();
	if ( !SurfaceRect( surfID, nPage, rect ) )
		return;
	data.SetCount( PaddedByteCount( rect ) );
	V_memset( data.Base(), 0, data.Count() );
	const CPaintPage *pPage = m_Pages[nPage];
	for ( int y = 0; y < rect.height; ++y )
	{
		V_memcpy( data.Base() + y * rect.width,
		    pPage->Data() + ( rect.y + y ) * pPage->Width() + rect.x, rect.width );
	}
}

bool CPaintmapDataManager::SetSurfacePaintData(
    SurfaceHandle_t surfID, const unsigned char *pData, int nCount )
{
	int nPage;
	Rect_t rect;
	if ( !SurfaceRect( surfID, nPage, rect ) || nCount < rect.width * rect.height )
		return false;
	CPaintPage *pPage = m_Pages[nPage];
	for ( int y = 0; y < rect.height; ++y )
	{
		V_memcpy(
		    const_cast<unsigned char *>( pPage->Data() ) + ( rect.y + y ) * pPage->Width() + rect.x,
		    pData + y * rect.width, rect.width );
	}
	pPage->AddDirtyRect( rect );
	MSurf_Flags( surfID ) |= SURFDRAW_PAINTED;
	return true;
}

void CPaintmapDataManager::GetPaintmapDataRLE( CUtlVector<uint32> &data )
{
	data.RemoveAll();
	if ( !HasPaintmap() )
		return;

	CUtlVector<unsigned char> surfPaintData;
	CUtlVector<uint32> rleData;
	for ( int i = 0; i < m_pWorld->numsurfaces; ++i )
	{
		SurfaceHandle_t surfID = SurfaceHandleFromIndex( i, m_pWorld );
		if ( !( MSurf_Flags( surfID ) & SURFDRAW_PAINTED ) )
			continue;

		GetSurfacePaintData( surfID, surfPaintData );
		if ( !surfPaintData.Count() )
			continue;

		rleData.RemoveAll();
		EncodeDataRLE( (const uint32 *)surfPaintData.Base(), surfPaintData.Count() / 4, rleData );
		data.AddToTail( i );
		data.AddToTail( rleData.Count() );
		data.AddMultipleToTail( rleData.Count(), rleData.Base() );
	}
}

bool CPaintmapDataManager::LoadPaintmapDataRLE( const CUtlVector<uint32> &data )
{
	if ( !HasPaintmap() )
		return data.Count() == 0;

	CUtlVector<unsigned char> rawPaintData;
	int nRead = 0;
	while ( nRead < data.Count() )
	{
		if ( data.Count() - nRead < 2 )
			return false;
		uint32 nSurface = data[nRead++];
		uint32 nRLESize = data[nRead++];
		if ( nSurface >= (uint32)m_pWorld->numsurfaces ||
		     nRLESize > uint32( data.Count() - nRead ) )
		{
			Warning( "Paint: save data does not fit this map (surface %u)\n", nSurface );
			return false;
		}

		SurfaceHandle_t surfID = SurfaceHandleFromIndex( nSurface, m_pWorld );
		int nPage;
		Rect_t rect;
		if ( !SurfaceRect( surfID, nPage, rect ) )
		{
			Warning( "Paint: save data names surface %u, which has no paint\n", nSurface );
			return false;
		}
		rawPaintData.SetCount( PaddedByteCount( rect ) );
		if ( !DecodeDataRLE( data.Base() + nRead, nRLESize, (uint32 *)rawPaintData.Base(),
		         rawPaintData.Count() / 4 ) )
		{
			Warning( "Paint: save data for surface %u is corrupt\n", nSurface );
			return false;
		}
		nRead += nRLESize;
		SetSurfacePaintData( surfID, rawPaintData.Base(), rawPaintData.Count() );
	}
	return true;
}

//-----------------------------------------------------------------------------
// The engine interface the Portal 2 client and server connect to.
//-----------------------------------------------------------------------------
class CEnginePaint : public IEnginePaint
{
public:
	virtual bool HasPaintmap( void ) { return g_PaintManager.HasPaintmap(); }

	virtual bool SpherePaintSurface( const model_t *pModel, const Vector &vPosition,
	    unsigned char color, float flSphereRadius, float flPaintCoatPercent )
	{
		return g_PaintManager.SpherePaint(
		    pModel, vPosition, color, flSphereRadius, flPaintCoatPercent );
	}

	virtual void SphereTracePaintSurface( const model_t *pModel, const Vector &vPosition,
	    const Vector &vContactNormal, float flSphereRadius, CUtlVector<unsigned char> &surfColors )
	{
		g_PaintManager.SphereTrace( pModel, vPosition, vContactNormal, flSphereRadius, surfColors );
	}

	virtual void RemoveAllPaint( void ) { g_PaintManager.RemoveAllPaint(); }

	virtual void PaintAllSurfaces( unsigned char color )
	{
		g_PaintManager.PaintAllSurfaces( color );
	}

	virtual void RemovePaint( const model_t *pModel ) { g_PaintManager.RemovePaint( pModel ); }

	virtual void GetPaintmapDataRLE( CUtlVector<uint32> &data )
	{
		g_PaintManager.GetPaintmapDataRLE( data );
	}

	virtual void LoadPaintmapDataRLE( const CUtlVector<uint32> &data )
	{
		g_PaintManager.LoadPaintmapDataRLE( data );
	}
};

EXPOSE_SINGLE_INTERFACE( CEnginePaint, IEnginePaint, VENGINE_PAINT_INTERFACE_VERSION );

CON_COMMAND_F( paintmap_stats, "Prints the paint map pages and the painted surfaces", FCVAR_CHEAT )
{
	if ( !g_PaintManager.HasPaintmap() )
	{
		Msg( "Paint: no paint map\n" );
		return;
	}
	int nPainted = 0;
	for ( int i = 0; i < host_state.worldbrush->numsurfaces; ++i )
	{
		if ( MSurf_Flags( SurfaceHandleFromIndex( i ) ) & SURFDRAW_PAINTED )
			++nPainted;
	}
	int nCovered[ENGINE_PAINT_POWER_COUNT] = {};
	for ( int p = 0; p < g_PaintManager.PageCount(); ++p )
	{
		CPaintPage *pPage = g_PaintManager.Page( p );
		for ( int i = 0; i < pPage->Width() * pPage->Height(); ++i )
		{
			unsigned char b = pPage->Data()[i];
			if ( PaintByte::Alpha( b ) > paint_min_valid_alpha_value.GetFloat() &&
			     PaintByte::Power( b ) < ENGINE_PAINT_POWER_COUNT )
				++nCovered[PaintByte::Power( b )];
		}
	}
	// Powers in paint_enum.h order: bounce, reflect (stick), speed, portal.
	Msg( "Paint: %d page(s), %d painted surface(s); covered luxels per power %d %d %d %d\n",
	    g_PaintManager.PageCount(), nPainted, nCovered[0], nCovered[1], nCovered[2], nCovered[3] );
}
