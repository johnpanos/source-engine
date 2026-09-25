//========= Portal 2 port ======================================================//
//
// Purpose: Blobulator tiler implementation (see public/blobulator/Implicit/ImpTiler.h).
//
// Clean-room implementation of the blobulator interface the Portal 2 paint
// blob renderer uses. It is not Valve's blobulator library.
//
//=============================================================================//

#include "blobulator/Implicit/ImpTiler.h"
#include "blobulator/Implicit/ImpRenderer.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Grid layout
//-----------------------------------------------------------------------------
// Corners per tile edge; neighbouring tiles duplicate their shared face so each
// tile polygonizes independently.
static const int NC = ImpTiler::TILE_CELLS + 1;
static const int CORNERS_PER_TILE = NC * NC * NC;

// Regula falsi steps that move an edge vertex onto the true surface: until the
// field is within VERTEX_REFINE_TOLERANCE (relative) of the threshold, at most
// VERTEX_REFINE_MAX_STEPS. A root near one end of a long edge can need more
// than four steps on the steep kernel.
static const int VERTEX_REFINE_MAX_STEPS = 16;
static const float VERTEX_REFINE_TOLERANCE = 1e-5f;

struct ImpTileBlock_t
{
	int bx, by, bz;
	float corners[CORNERS_PER_TILE]; // field value at each corner
	SmartArray<int, false, 16>
	    particles; // particles whose field reaches the tile, in insertion order
};

static inline int CornerIndex( int x, int y, int z )
{
	return ( z * NC + y ) * NC + x;
}

static inline int FloorDiv( int a, int b )
{
	return ( a >= 0 ) ? ( a / b ) : -( ( -a + b - 1 ) / b );
}

// Field kernel k(x) = (1-x)^12 on the squared distance in cutoff units; the
// high exponent makes nearby blobs bridge softly instead of fusing into one
// large mass (a Gaussian-like falloff that still ends at the cutoff).
static inline float Kernel( float x, float *pDerivFactor )
{
	float y = 1.0f - x;
	float y2 = y * y;
	float y4 = y2 * y2;
	float y8 = y4 * y4;
	float y11 = y8 * y2 * y;
	*pDerivFactor = -12.0f * y11; // dk/dx
	return y11 * y;
}

// Cube corners (bit 0 = x, bit 1 = y, bit 2 = z) and the six tetrahedra of the
// Kuhn split along the 0-7 diagonal. Every grid cube uses the same split, so
// the face diagonals of neighbouring cubes agree.
static const int s_CubeCorner[8][3] = {
    { 0, 0, 0 },
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 1, 1, 0 },
    { 0, 0, 1 },
    { 1, 0, 1 },
    { 0, 1, 1 },
    { 1, 1, 1 },
};

static const int s_Tetrahedra[6][4] = {
    { 0, 1, 3, 7 },
    { 0, 3, 2, 7 },
    { 0, 2, 6, 7 },
    { 0, 6, 4, 7 },
    { 0, 4, 5, 7 },
    { 0, 5, 1, 7 },
};

// Edge directions from the lower end of an edge (every tetrahedron edge has
// non-negative offsets from one of its ends).
static int EdgeDirection( int dx, int dy, int dz )
{
	static const int s_Dir[2][2][2] = {
	    { { -1, 2 }, { 1, 5 } }, // dx = 0: (0,0,1) (0,1,0) (0,1,1)
	    { { 0, 4 }, { 3, 6 } },  // dx = 1: (1,0,0) (1,0,1) (1,1,0) (1,1,1)
	};
	return s_Dir[dx][dy][dz];
}

static const int s_DirOffset[7][3] = {
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 },
    { 1, 1, 0 },
    { 1, 0, 1 },
    { 0, 1, 1 },
    { 1, 1, 1 },
};

//-----------------------------------------------------------------------------
// ImpTiler
//-----------------------------------------------------------------------------
ImpTiler::ImpTiler()
    : m_flCubeWidth( 1.0f ), m_flRenderR( 1.3f ), m_flCutoffR( 5.5f ), m_flThreshold( 0.0f ),
      m_nTileIndexToDraw( -1 ), m_ppRenderContext( NULL ), m_nLastTile( -1 )
{
	UpdateThreshold();
}

ImpTiler::~ImpTiler()
{
	ReleaseFrame();
	for ( int i = 0; i < m_FreeTiles.size; ++i )
		delete m_FreeTiles[i];
	m_FreeTiles.size = 0;
}

void ImpTiler::SetCubeWidth( float flCubeWidth )
{
	m_flCubeWidth = MAX( flCubeWidth, 0.01f );
}

void ImpTiler::SetRenderRadius( float flRenderRadius )
{
	m_flRenderR = MAX( flRenderRadius, 0.001f );
	UpdateThreshold();
}

void ImpTiler::SetCutoffRadius( float flCutoffRadius )
{
	m_flCutoffR = MAX( flCutoffRadius, 0.001f );
	UpdateThreshold();
}

void ImpTiler::UpdateThreshold()
{
	// A lone particle's surface is at the render radius; keep it inside the cutoff.
	float flRatio = MIN( m_flRenderR / m_flCutoffR, 0.95f );
	float flDeriv;
	m_flThreshold = Kernel( flRatio * flRatio, &flDeriv );
}

void ImpTiler::beginFrame( const Point3D &offset, bool bNoMargin, bool bDeferDraw )
{
	ReleaseFrame();
	m_Offset = offset;
}

void ImpTiler::insertParticle( const ImpParticleWithFourInterpolants *pParticle )
{
	Particle_t &particle = m_Particles.pushAutoSize();
	particle.center = pParticle->center.AsVector();
	particle.scale = pParticle->scale;
	particle.color = pParticle->interpolants1.AsVector();
	particle.tangent = pParticle->interpolants3.AsVector();
}

void ImpTiler::insertParticle( const ImpParticleWithOneInterpolant *pParticle )
{
	Particle_t &particle = m_Particles.pushAutoSize();
	particle.center = pParticle->center.AsVector();
	particle.scale = pParticle->scale;
	particle.color = pParticle->interpolants1.AsVector();
	particle.tangent.Init( 1.0f, 0.0f, 0.0f );
}

Point3D ImpTiler::getTileOffset( int nTile ) const
{
	const ImpTileBlock_t *pTile = m_Tiles[nTile];
	float flHalf = 0.5f * TILE_CELLS;
	return Point3D( m_Offset[0] + ( pTile->bx * TILE_CELLS + flHalf ) * m_flCubeWidth,
	    m_Offset[1] + ( pTile->by * TILE_CELLS + flHalf ) * m_flCubeWidth,
	    m_Offset[2] + ( pTile->bz * TILE_CELLS + flHalf ) * m_flCubeWidth );
}

Point3D ImpTiler::getRenderDim() const
{
	float flSize = TILE_CELLS * m_flCubeWidth;
	return Point3D( flSize, flSize, flSize );
}

ImpTileBlock_t *ImpTiler::FindOrCreateTile( int bx, int by, int bz )
{
	if ( m_nLastTile >= 0 )
	{
		ImpTileBlock_t *pLast = m_Tiles[m_nLastTile];
		if ( pLast->bx == bx && pLast->by == by && pLast->bz == bz )
			return pLast;
	}

	for ( int i = 0; i < m_Tiles.size; ++i )
	{
		ImpTileBlock_t *pTile = m_Tiles[i];
		if ( pTile->bx == bx && pTile->by == by && pTile->bz == bz )
		{
			m_nLastTile = i;
			return pTile;
		}
	}

	ImpTileBlock_t *pTile = ( m_FreeTiles.size > 0 ) ? m_FreeTiles.pop() : new ImpTileBlock_t;
	pTile->bx = bx;
	pTile->by = by;
	pTile->bz = bz;
	memset( pTile->corners, 0, sizeof( pTile->corners ) );
	pTile->particles.size = 0;
	m_nLastTile = m_Tiles.size;
	m_Tiles.pushAutoSize( pTile );
	return pTile;
}

void ImpTiler::ScatterParticle( int nParticle )
{
	const Particle_t &particle = m_Particles[nParticle];
	if ( particle.scale <= 0.0f )
		return;

	const float flR = m_flCutoffR * particle.scale;
	const float flInvR2 = 1.0f / ( flR * flR );
	const float flInvCube = 1.0f / m_flCubeWidth;

	// Particle position in grid units.
	Vector g( ( particle.center.x - m_Offset[0] ) * flInvCube,
	    ( particle.center.y - m_Offset[1] ) * flInvCube,
	    ( particle.center.z - m_Offset[2] ) * flInvCube );
	float flRCells = flR * flInvCube;

	int lo[3], hi[3];
	for ( int i = 0; i < 3; ++i )
	{
		lo[i] = (int)floorf( g[i] - flRCells );
		hi[i] = (int)ceilf( g[i] + flRCells );
	}

	// Tiles whose corner range [b*T, b*T + T] overlaps [lo, hi].
	int blo[3], bhi[3];
	for ( int i = 0; i < 3; ++i )
	{
		blo[i] = FloorDiv( lo[i] - 1, TILE_CELLS );
		bhi[i] = FloorDiv( hi[i], TILE_CELLS );
	}

	for ( int bz = blo[2]; bz <= bhi[2]; ++bz )
		for ( int by = blo[1]; by <= bhi[1]; ++by )
			for ( int bx = blo[0]; bx <= bhi[0]; ++bx )
			{
				int base[3] = { bx * TILE_CELLS, by * TILE_CELLS, bz * TILE_CELLS };

				// Skip tiles the field does not reach (sphere against the tile box).
				float flDistSq = 0.0f;
				for ( int i = 0; i < 3; ++i )
				{
					float flNearest =
					    clamp( g[i], (float)base[i], (float)( base[i] + TILE_CELLS ) );
					float d = ( g[i] - flNearest ) * m_flCubeWidth;
					flDistSq += d * d;
				}
				if ( flDistSq >= flR * flR )
					continue;

				ImpTileBlock_t *pTile = FindOrCreateTile( bx, by, bz );
				pTile->particles.pushAutoSize( nParticle );

				int x0 = MAX( lo[0], base[0] ), x1 = MIN( hi[0], base[0] + TILE_CELLS );
				int y0 = MAX( lo[1], base[1] ), y1 = MIN( hi[1], base[1] + TILE_CELLS );
				int z0 = MAX( lo[2], base[2] ), z1 = MIN( hi[2], base[2] + TILE_CELLS );
				for ( int z = z0; z <= z1; ++z )
				{
					float dz = ( z - g.z ) * m_flCubeWidth;
					for ( int y = y0; y <= y1; ++y )
					{
						float dy = ( y - g.y ) * m_flCubeWidth;
						float flYZ = ( dy * dy + dz * dz ) * flInvR2;
						if ( flYZ >= 1.0f )
							continue;

						float *pRow = &pTile->corners[CornerIndex( 0, y - base[1], z - base[2] )];
						for ( int x = x0; x <= x1; ++x )
						{
							float dx = ( x - g.x ) * m_flCubeWidth;
							float flX = flYZ + dx * dx * flInvR2;
							if ( flX >= 1.0f )
								continue;

							float flDeriv;
							pRow[x - base[0]] += Kernel( flX, &flDeriv );
						}
					}
				}
			}
}

// Field value, gradient, and field-weighted color and tangent sums at a point
// from the given particles. Callers pass the particles of a tile in insertion
// order (possibly without ones that cannot reach the point): every particle
// that reaches a point on a face shared by two tiles is in both tiles' lists,
// and particles that do not reach it add nothing, so both tiles compute the
// same value.
void ImpTiler::EvaluateAt( const int *pParticles, int nParticles, const Vector &vecPoint,
    float *pValue, Vector *pGrad, Vector *pColor, Vector *pTangent ) const
{
	float flValue = 0.0f;
	Vector vecGrad( 0.0f, 0.0f, 0.0f ), vecColor( 0.0f, 0.0f, 0.0f ),
	    vecTangent( 0.0f, 0.0f, 0.0f );
	for ( int i = 0; i < nParticles; ++i )
	{
		const Particle_t &particle = m_Particles[pParticles[i]];
		float flR = m_flCutoffR * particle.scale;
		float flInvR2 = 1.0f / ( flR * flR );
		Vector d = vecPoint - particle.center;
		float flX = d.LengthSqr() * flInvR2;
		if ( flX >= 1.0f )
			continue;

		float flDeriv;
		float k = Kernel( flX, &flDeriv );
		flValue += k;
		if ( pGrad )
		{
			vecGrad += d * ( flDeriv * 2.0f * flInvR2 );
			vecColor += particle.color * k;
			vecTangent += particle.tangent * k;
		}
	}

	*pValue = flValue;
	if ( pGrad )
	{
		*pGrad = vecGrad;
		*pColor = vecColor;
		*pTangent = vecTangent;
	}
}

float ImpTiler::EvaluateField( const Vector &vecPoint ) const
{
	float flSum = 0.0f;
	for ( int i = 0; i < m_Particles.size; ++i )
	{
		const Particle_t &particle = m_Particles[i];
		if ( particle.scale <= 0.0f )
			continue;
		float flR = m_flCutoffR * particle.scale;
		float flX = ( vecPoint - particle.center ).LengthSqr() / ( flR * flR );
		if ( flX < 1.0f )
		{
			float flDeriv;
			flSum += Kernel( flX, &flDeriv );
		}
	}
	return flSum;
}

// Returns the vertex on the edge from corner (x,y,z) of the tile in direction nDir.
int ImpTiler::EdgeVertex( ImpTileBlock_t *pTile, int x, int y, int z, int nDir, int *pEdgeCache )
{
	int nCornerA = CornerIndex( x, y, z );
	int &nCached = pEdgeCache[nCornerA * 7 + nDir];
	if ( nCached >= 0 )
		return nCached;

	const int *off = s_DirOffset[nDir];
	float fa = pTile->corners[nCornerA];
	float fb = pTile->corners[CornerIndex( x + off[0], y + off[1], z + off[2] )];

	int gx = pTile->bx * TILE_CELLS + x, gy = pTile->by * TILE_CELLS + y,
	    gz = pTile->bz * TILE_CELLS + z;
	Vector vecA( m_Offset[0] + gx * m_flCubeWidth, m_Offset[1] + gy * m_flCubeWidth,
	    m_Offset[2] + gz * m_flCubeWidth );
	Vector vecEdge( off[0] * m_flCubeWidth, off[1] * m_flCubeWidth, off[2] * m_flCubeWidth );

	// The tile's particles that can reach this edge (edge length <= sqrt(3) cells).
	int nearParticles[256];
	int nNear = 0;
	const int *pParticles = pTile->particles.a;
	int nParticles = pTile->particles.size;
	{
		Vector vecMid = vecA + vecEdge * 0.5f;
		float flHalfEdge = 0.5f * vecEdge.Length();
		for ( int i = 0; i < pTile->particles.size && nNear >= 0; ++i )
		{
			const Particle_t &particle = m_Particles[pTile->particles.a[i]];
			float flReach = m_flCutoffR * particle.scale + flHalfEdge;
			if ( ( vecMid - particle.center ).LengthSqr() < flReach * flReach )
			{
				if ( nNear == ARRAYSIZE( nearParticles ) )
					nNear = -1; // too many: use the whole tile list
				else
					nearParticles[nNear++] = pTile->particles.a[i];
			}
		}
		if ( nNear >= 0 )
		{
			pParticles = nearParticles;
			nParticles = nNear;
		}
	}

	// Illinois regula falsi on the true field along the edge. The ends
	// straddle the threshold (the corner values are the same field, summed on
	// the grid); halving a retained end's value keeps the convex kernel from
	// stalling on one side.
	float t0 = 0.0f, t1 = 1.0f, f0 = fa - m_flThreshold, f1 = fb - m_flThreshold;
	float t = ( fabsf( f1 - f0 ) > 1e-12f ) ? f0 / ( f0 - f1 ) : 0.5f;
	int nLastSide = 0;
	const float flTolerance = VERTEX_REFINE_TOLERANCE * m_flThreshold;
	for ( int nStep = 0; nStep < VERTEX_REFINE_MAX_STEPS; ++nStep )
	{
		float flValue;
		EvaluateAt( pParticles, nParticles, vecA + vecEdge * t, &flValue, NULL, NULL, NULL );
		float f = flValue - m_flThreshold;
		if ( fabsf( f ) <= flTolerance )
			break;
		if ( ( f > 0.0f ) == ( f0 > 0.0f ) )
		{
			t0 = t;
			f0 = f;
			if ( nLastSide == 0 )
				f1 *= 0.5f;
			nLastSide = 0;
		}
		else
		{
			t1 = t;
			f1 = f;
			if ( nLastSide == 1 )
				f0 *= 0.5f;
			nLastSide = 1;
		}
		t = ( fabsf( f1 - f0 ) > 1e-12f ) ? t0 + ( t1 - t0 ) * f0 / ( f0 - f1 )
		                                  : 0.5f * ( t0 + t1 );
	}
	t = clamp( t, 0.0f, 1.0f );

	Vertex_t &vert = m_Vertices.pushAutoSize();
	vert.pos = vecA + vecEdge * t;

	float flWeight;
	Vector vecGrad, vecColor, vecTangent;
	EvaluateAt( pParticles, nParticles, vert.pos, &flWeight, &vecGrad, &vecColor, &vecTangent );

	// The field falls off outwards: the normal is -grad f.
	vert.normal = -vecGrad;
	if ( VectorNormalize( vert.normal ) <= 0.0f )
		vert.normal.Init( 0.0f, 0.0f, 1.0f );

	vert.color = ( flWeight > 0.0f ) ? vecColor * ( 1.0f / flWeight ) : Vector( 1.0f, 1.0f, 1.0f );

	// Tangent frame: the blended particle tangent made orthogonal to the normal.
	// Normalize before projecting and fall back to a fixed axis when the unit
	// tangent is within ~0.6 degrees of the normal: VectorNormalize adds
	// FLT_EPSILON to the length, so a shorter projection would not come out unit.
	float flTangentLength = VectorNormalize( vecTangent );
	vecTangent -= vert.normal * DotProduct( vecTangent, vert.normal );
	if ( flTangentLength <= 0.0f || VectorNormalize( vecTangent ) <= 1e-2f )
	{
		Vector vecUp = ( fabsf( vert.normal.z ) < 0.9f ) ? Vector( 0.0f, 0.0f, 1.0f )
		                                                 : Vector( 1.0f, 0.0f, 0.0f );
		vecTangent = CrossProduct( vecUp, vert.normal );
		VectorNormalize( vecTangent );
	}
	vert.tangentS = vecTangent;
	vert.tangentT = CrossProduct( vert.normal, vecTangent );

	// Texture coordinates follow the tangent frame, one repeat per four render radii.
	float flUVScale = 1.0f / ( 4.0f * m_flRenderR );
	vert.uv[0] = DotProduct( vert.pos, vert.tangentS ) * flUVScale;
	vert.uv[1] = DotProduct( vert.pos, vert.tangentT ) * flUVScale;

	nCached = m_Vertices.size - 1;
	return nCached;
}

// vecOut: a direction from the tetrahedron's inside corners to its outside
// corners; the triangle is wound counter-clockwise around it.
void ImpTiler::EmitTriangle( int a, int b, int c, const Vector &vecOut )
{
	if ( a == b || b == c || a == c )
		return; // collapsed onto a shared vertex

	const Vertex_t &va = m_Vertices[a];
	const Vertex_t &vb = m_Vertices[b];
	const Vertex_t &vc = m_Vertices[c];

	Vector vecFace = CrossProduct( vb.pos - va.pos, vc.pos - va.pos );
	if ( DotProduct( vecFace, vecOut ) < 0.0f )
	{
		int tmp = b;
		b = c;
		c = tmp;
	}

	m_Indices.pushAutoSize( a );
	m_Indices.pushAutoSize( b );
	m_Indices.pushAutoSize( c );
}

void ImpTiler::PolygonizeTile( ImpTileBlock_t *pTile )
{
	const float flThreshold = m_flThreshold;

	// Corner inside flags; a tile entirely inside or outside has no surface.
	unsigned char inside[CORNERS_PER_TILE];
	int nInsideCorners = 0;
	for ( int i = 0; i < CORNERS_PER_TILE; ++i )
	{
		inside[i] = ( pTile->corners[i] > flThreshold ) ? 1 : 0;
		nInsideCorners += inside[i];
	}
	if ( nInsideCorners == 0 || nInsideCorners == CORNERS_PER_TILE )
		return;

	m_EdgeCache.resize( CORNERS_PER_TILE * 7 );
	int *pEdgeCache = m_EdgeCache.a;
	memset( pEdgeCache, 0xff, CORNERS_PER_TILE * 7 * sizeof( int ) );

	static const int s_CornerOffset[8] = {
	    0,
	    1,
	    NC,
	    NC + 1,
	    NC * NC,
	    NC * NC + 1,
	    NC * NC + NC,
	    NC * NC + NC + 1,
	};

	for ( int z = 0; z < TILE_CELLS; ++z )
		for ( int y = 0; y < TILE_CELLS; ++y )
			for ( int x = 0; x < TILE_CELLS; ++x )
			{
				const int nBase = CornerIndex( x, y, z );
				int nMask = 0;
				for ( int c = 0; c < 8; ++c )
					nMask |= inside[nBase + s_CornerOffset[c]] << c;
				if ( nMask == 0 || nMask == 0xff )
					continue;

				// Vertex on the edge between cube corners c0 and c1.
				auto edge = [&]( int c0, int c1 ) -> int
				{
					const int *p0 = s_CubeCorner[c0];
					const int *p1 = s_CubeCorner[c1];
					if ( p1[0] < p0[0] || p1[1] < p0[1] || p1[2] < p0[2] )
					{
						const int *tmp = p0;
						p0 = p1;
						p1 = tmp;
					}
					int nDir = EdgeDirection( p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] );
					return EdgeVertex( pTile, x + p0[0], y + p0[1], z + p0[2], nDir, pEdgeCache );
				};

				for ( int t = 0; t < 6; ++t )
				{
					const int *tet = s_Tetrahedra[t];
					int in[4], outside[4];
					int nIn = 0, nOut = 0;
					for ( int v = 0; v < 4; ++v )
					{
						if ( nMask & ( 1 << tet[v] ) )
							in[nIn++] = tet[v];
						else
							outside[nOut++] = tet[v];
					}
					if ( nIn == 0 || nOut == 0 )
						continue;

					// From the inside corners' centroid to the outside corners' centroid.
					Vector vecOut( 0.0f, 0.0f, 0.0f );
					for ( int v = 0; v < nOut; ++v )
						vecOut += Vector( s_CubeCorner[outside[v]][0], s_CubeCorner[outside[v]][1],
						              s_CubeCorner[outside[v]][2] ) *
						          ( 1.0f / nOut );
					for ( int v = 0; v < nIn; ++v )
						vecOut -= Vector( s_CubeCorner[in[v]][0], s_CubeCorner[in[v]][1],
						              s_CubeCorner[in[v]][2] ) *
						          ( 1.0f / nIn );

					if ( nIn == 1 )
					{
						EmitTriangle( edge( in[0], outside[0] ), edge( in[0], outside[1] ),
						    edge( in[0], outside[2] ), vecOut );
					}
					else if ( nIn == 3 )
					{
						EmitTriangle( edge( outside[0], in[0] ), edge( outside[0], in[1] ),
						    edge( outside[0], in[2] ), vecOut );
					}
					else
					{
						int e00 = edge( in[0], outside[0] );
						int e01 = edge( in[0], outside[1] );
						int e10 = edge( in[1], outside[0] );
						int e11 = edge( in[1], outside[1] );
						EmitTriangle( e00, e01, e11, vecOut );
						EmitTriangle( e00, e11, e10, vecOut );
					}
				}
			}
}

void ImpTiler::BuildSurface( const Point3D *pEye )
{
	m_Vertices.size = 0;
	m_Indices.size = 0;
	m_TileRanges.size = 0;

	for ( int i = 0; i < m_Particles.size; ++i )
		ScatterParticle( i );

	for ( int i = 0; i < m_Tiles.size; ++i )
	{
		TileRange_t &range = m_TileRanges.pushAutoSize();
		range.nFirstIndex = m_Indices.size;
		if ( m_nTileIndexToDraw < 0 || m_nTileIndexToDraw == i )
			PolygonizeTile( m_Tiles[i] );
		range.nIndexCount = m_Indices.size - range.nFirstIndex;
		range.flSortDist = pEye ? getTileOffset( i ).lengthSq( *pEye ) : 0.0f;
	}

	if ( pEye )
	{
		// back to front
		struct C
		{
			static bool IsLessThan( const TileRange_t &a, const TileRange_t &b )
			{
				return a.flSortDist > b.flSortDist;
			}
		};
		m_TileRanges.sort<C>();
	}
}

void ImpTiler::drawSurface( bool bDeferDraw )
{
	BuildSurface( NULL );
}

void ImpTiler::drawSurfaceSorted( const Point3D &eye, bool bDeferDraw )
{
	BuildSurface( &eye );
}

void ImpTiler::DrawTriangles()
{
	if ( m_Indices.size == 0 )
		return;

	if ( !m_ppRenderContext || !*m_ppRenderContext )
	{
		Warning( "[Blobulator] endFrame() without a render context; the surface was not drawn.\n" );
		return;
	}

	IMatRenderContext *pRenderContext = *m_ppRenderContext;
	IMaterial *pMaterial = pRenderContext->GetCurrentMaterial();
	int nMaxVerts =
	    MIN( pMaterial ? pRenderContext->GetMaxVerticesToRender( pMaterial ) : 32767, 32767 );
	int nMaxIndices = MIN( pRenderContext->GetMaxIndicesToRender(), 32767 * 3 );
	nMaxIndices -= nMaxIndices % 3;
	if ( nMaxVerts < 3 || nMaxIndices < 3 )
		return;

	IMesh *pMesh = pRenderContext->GetDynamicMesh( true );

	// Batch the triangles in draw order, remapping vertices per batch.
	SmartArray<int, false, 16> remap;
	remap.resize( m_Vertices.size );
	memset( remap.a, 0xff, m_Vertices.size * sizeof( int ) );
	SmartArray<int, false, 16> batchVerts;
	SmartArray<unsigned short, false, 16> batchIndices;

	auto flush = [&]()
	{
		if ( batchIndices.size == 0 )
			return;

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, batchVerts.size, batchIndices.size );
		for ( int i = 0; i < batchVerts.size; ++i )
		{
			const Vertex_t &vert = m_Vertices[batchVerts[i]];
			meshBuilder.Position3fv( vert.pos.Base() );
			meshBuilder.Normal3fv( vert.normal.Base() );
			meshBuilder.Color3fv( vert.color.Base() );
			meshBuilder.TexCoord2f( 0, vert.uv[0], vert.uv[1] );
			meshBuilder.TangentS3fv( vert.tangentS.Base() );
			meshBuilder.TangentT3fv( vert.tangentT.Base() );
			meshBuilder.AdvanceVertex();
		}
		for ( int i = 0; i < batchIndices.size; ++i )
			meshBuilder.FastIndex( batchIndices[i] );
		meshBuilder.End();
		pMesh->Draw();

		for ( int i = 0; i < batchVerts.size; ++i )
			remap[batchVerts[i]] = -1;
		batchVerts.size = 0;
		batchIndices.size = 0;
	};

	for ( int r = 0; r < m_TileRanges.size; ++r )
	{
		const TileRange_t &range = m_TileRanges[r];
		for ( int i = range.nFirstIndex; i < range.nFirstIndex + range.nIndexCount; i += 3 )
		{
			if ( batchVerts.size + 3 > nMaxVerts || batchIndices.size + 3 > nMaxIndices )
				flush();

			for ( int k = 0; k < 3; ++k )
			{
				int nVert = m_Indices[i + k];
				if ( remap[nVert] < 0 )
				{
					remap[nVert] = batchVerts.size;
					batchVerts.pushAutoSize( nVert );
				}
				batchIndices.pushAutoSize( (unsigned short)remap[nVert] );
			}
		}
	}
	flush();
}

void ImpTiler::endFrame( bool bDeferDraw )
{
	DrawTriangles();
	ReleaseFrame();
}

void ImpTiler::ReleaseFrame()
{
	for ( int i = 0; i < m_Tiles.size; ++i )
		m_FreeTiles.pushAutoSize( m_Tiles[i] );
	m_Tiles.size = 0;
	m_nLastTile = -1;
	m_Particles.size = 0;
	m_Vertices.size = 0;
	m_Indices.size = 0;
	m_TileRanges.size = 0;
	m_ppRenderContext = NULL;
}

//-----------------------------------------------------------------------------
// Factories
//-----------------------------------------------------------------------------
static ImpRendererFactory s_ImpRendererFactory;
ImpRendererFactory *ImpRendererFactory::s_pInstance = &s_ImpRendererFactory;

static ImpTilerFactory s_ImpTilerFactory;
ImpTilerFactory *ImpTilerFactory::factory = &s_ImpTilerFactory;

ImpTilerFactory::~ImpTilerFactory()
{
	for ( int i = 0; i < m_Tilers.size; ++i )
		delete m_Tilers[i];
	m_Tilers.size = 0;
}

ImpTiler *ImpTilerFactory::getTiler()
{
	ImpTiler *pTiler;
	{
		AUTO_LOCK( m_Mutex );
		pTiler = ( m_Tilers.size > 0 ) ? m_Tilers.pop() : NULL;
	}
	if ( !pTiler )
		pTiler = new ImpTiler;

	ImpRendererFactory::s_pInstance->OnTileRendererAcquired();
	return pTiler;
}

void ImpTilerFactory::returnTiler( ImpTiler *pTiler )
{
	if ( !pTiler )
		return;

	ImpRendererFactory::s_pInstance->OnTileRendererReleased();

	AUTO_LOCK( m_Mutex );
	m_Tilers.pushAutoSize( pTiler );
}
