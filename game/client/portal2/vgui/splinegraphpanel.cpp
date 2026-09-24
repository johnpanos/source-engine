//========= Portal 2 reconstruction ============================================//
//
// Purpose: Smoothed histogram panel used by the leaderboard graphs; see
//          splinegraphpanel.h.
//
//=============================================================================//

#include "cbase.h"
#include "splinegraphpanel.h"
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_splinegraph_bucket( "cl_splinegraph_bucket", "10", FCVAR_NONE, "Number of buckets the leaderboard graphs count scores into.", true, 2, true, 100 );

// Spline segments drawn between two bucket centers.
static const int SPLINEGRAPH_SUBDIVISIONS = 8;

SplineGraphPanel::SplineGraphPanel( vgui::Panel *pParent, const char *pName ) : BaseClass( pParent, pName )
{
	m_flMin = 0.0f;
	m_flMax = 0.0f;
	m_nWhiteTexture = -1;
	SetPaintBackgroundEnabled( true );
}

void SplineGraphPanel::SetData( const CUtlVector< float > &samples, float flMin, float flMax )
{
	m_Samples.CopyArray( samples.Base(), samples.Count() );
	m_flMin = flMin;
	m_flMax = flMax;
}

void SplineGraphPanel::ClearData()
{
	m_Samples.RemoveAll();
	m_flMin = m_flMax = 0.0f;
}

void SplineGraphPanel::DrawFilledQuad( float x0, float y0, float x1, float y1, float flBottom )
{
	vgui::Vertex_t verts[ 4 ] =
	{
		vgui::Vertex_t( Vector2D( x0, y0 ), Vector2D( 0.0f, 0.0f ) ),
		vgui::Vertex_t( Vector2D( x1, y1 ), Vector2D( 1.0f, 0.0f ) ),
		vgui::Vertex_t( Vector2D( x1, flBottom ), Vector2D( 1.0f, 1.0f ) ),
		vgui::Vertex_t( Vector2D( x0, flBottom ), Vector2D( 0.0f, 1.0f ) ),
	};
	vgui::surface()->DrawTexturedPolygon( 4, verts );
}

void SplineGraphPanel::Paint()
{
	if ( m_Samples.Count() == 0 )
		return;

	int nWide, nTall;
	GetSize( nWide, nTall );
	if ( nWide <= 0 || nTall <= 0 )
		return;

	// Count the samples into buckets across [ m_flMin, m_flMax ].
	const int nBuckets = cl_splinegraph_bucket.GetInt();
	CUtlVector< float > buckets;
	buckets.SetCount( nBuckets );
	for ( int i = 0; i < nBuckets; ++i )
		buckets[ i ] = 0.0f;

	float flRange = m_flMax - m_flMin;
	for ( int i = 0; i < m_Samples.Count(); ++i )
	{
		int nBucket = 0;
		if ( flRange > 0.0f )
			nBucket = (int)( ( m_Samples[ i ] - m_flMin ) / flRange * nBuckets );
		buckets[ clamp( nBucket, 0, nBuckets - 1 ) ] += 1.0f;
	}

	float flTallest = 0.0f;
	for ( int i = 0; i < nBuckets; ++i )
		flTallest = MAX( flTallest, buckets[ i ] );
	if ( flTallest <= 0.0f )
		return;

	if ( m_nWhiteTexture == -1 )
	{
		m_nWhiteTexture = vgui::surface()->DrawGetTextureId( "vgui/white" );
		if ( m_nWhiteTexture == -1 )
		{
			m_nWhiteTexture = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( m_nWhiteTexture, "vgui/white", true, true );
		}
	}
	vgui::surface()->DrawSetTexture( m_nWhiteTexture );
	vgui::surface()->DrawSetColor( GetFgColor() );

	// Heights of the bucket centers, with the ends held at zero so the curve
	// starts and finishes on the bottom edge.
	CUtlVector< Vector2D > points;
	points.AddToTail( Vector2D( 0.0f, (float)nTall ) );
	for ( int i = 0; i < nBuckets; ++i )
	{
		float x = ( i + 0.5f ) * nWide / nBuckets;
		float y = nTall - buckets[ i ] / flTallest * nTall;
		points.AddToTail( Vector2D( x, y ) );
	}
	points.AddToTail( Vector2D( (float)nWide, (float)nTall ) );

	const float flBottom = (float)nTall;
	for ( int nSeg = 0; nSeg < points.Count() - 1; ++nSeg )
	{
		const Vector2D &p0 = points[ MAX( nSeg - 1, 0 ) ];
		const Vector2D &p1 = points[ nSeg ];
		const Vector2D &p2 = points[ nSeg + 1 ];
		const Vector2D &p3 = points[ MIN( nSeg + 2, points.Count() - 1 ) ];

		Vector2D prev = p1;
		for ( int nStep = 1; nStep <= SPLINEGRAPH_SUBDIVISIONS; ++nStep )
		{
			float t = (float)nStep / SPLINEGRAPH_SUBDIVISIONS;
			float t2 = t * t;
			float t3 = t2 * t;

			// Catmull-Rom
			Vector2D cur;
			for ( int nAxis = 0; nAxis < 2; ++nAxis )
			{
				cur[ nAxis ] = 0.5f * ( ( 2.0f * p1[ nAxis ] ) +
										( -p0[ nAxis ] + p2[ nAxis ] ) * t +
										( 2.0f * p0[ nAxis ] - 5.0f * p1[ nAxis ] + 4.0f * p2[ nAxis ] - p3[ nAxis ] ) * t2 +
										( -p0[ nAxis ] + 3.0f * p1[ nAxis ] - 3.0f * p2[ nAxis ] + p3[ nAxis ] ) * t3 );
			}
			cur.y = clamp( cur.y, 0.0f, flBottom );

			DrawFilledQuad( prev.x, prev.y, cur.x, cur.y, flBottom );
			prev = cur;
		}
	}
}
