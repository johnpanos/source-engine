//========= Portal 2 port ======================================================//
//
// Purpose: Blobulator tiler: builds and draws the isosurface of a set of
//          metaball particles.
//
// Clean-room implementation of the blobulator interface the Portal 2 paint
// blob renderer (game/client/portal2/c_paintblob_render.cpp) uses. It is not
// Valve's blobulator library, whose source is unavailable; it keeps the
// retail design (cubic tiles of a regular grid, a particle field cut off at
// the cutoff radius whose isosurface lies at the render radius of a lone
// particle) with its own field function and polygonizer:
//
//   field    f(p) = sum_i k( |p - c_i|^2 / ( cutoffR * s_i )^2 ),
//            k(x) = ( 1 - x )^12 for x < 1, else 0
//            (s_i = the particle's field scale, ImpParticle::scale)
//   surface  f = k( ( renderR / cutoffR )^2 ), so a lone particle of scale s
//            is a sphere of radius s * renderR
//   mesh     marching tetrahedra (6 per grid cube, shared main diagonal)
//            on a sparse grid of tiles; vertices are shared along edges and
//            refined onto the true surface along their edge (regula falsi)
//   normal   -grad f (analytic, at the refined vertex); color, tangent frame
//            and texture coordinates blend the particles' interpolants by
//            field weight
// Tiles evaluate only the particles whose field reaches them; a vertex on a
// face shared by two tiles is computed identically by both, so the surface is
// closed (vertices on tile seams are duplicated, not shared).
//
// Coordinates are in the space of the particle centers; the caller's model
// matrix places them in the world. Particles are copied on insert, so the
// caller's particle array may change once drawSurface() returns.
//
// Frame protocol (one tiler per draw, from ImpTilerFactory):
//   beginFrame -> insertParticle* -> drawSurface[Sorted] -> endFrame
// drawSurface builds the triangles; endFrame draws them with the render
// context given by SetRenderContext() (which may be set after drawSurface,
// on the material thread, for a deferred two-pass draw).
//
//=============================================================================//

#ifndef BLOBULATOR_IMPTILER_H
#define BLOBULATOR_IMPTILER_H
#ifdef _WIN32
#pragma once
#endif

#include "blobulator/Point3D.h"
#include "blobulator/SmartArray.h"
#include "blobulator/Implicit/ImpParticle.h"
#include "tier0/threadtools.h"

class IMatRenderContext;
struct ImpTileBlock_t;

class ImpTiler
{
public:
	// Cells per tile edge.
	enum
	{
		TILE_CELLS = 16
	};

	struct Vertex_t
	{
		Vector pos;
		Vector normal;
		Vector color;
		Vector tangentS;
		Vector tangentT;
		float uv[2];
	};

	ImpTiler();
	~ImpTiler();

	// Grid cell size, the surface radius of a lone unit-scale particle, and the
	// radius its field reaches. A render radius at or beyond the cutoff radius
	// is clamped inside it.
	void SetCubeWidth( float flCubeWidth );
	void SetRenderRadius( float flRenderRadius );
	void SetCutoffRadius( float flCutoffRadius );
	float GetCubeWidth() const { return m_flCubeWidth; }
	float GetRenderRadius() const { return m_flRenderR; }
	float GetCutoffRadius() const { return m_flCutoffR; }

	// -1 draws every tile; otherwise only tile nIndex (debugging).
	void setTileIndexToDraw( int nIndex ) { m_nTileIndexToDraw = nIndex; }

	// The context endFrame() draws with (the address of the caller's context
	// pointer, which must stay valid until endFrame); its bound material is used.
	void SetRenderContext( IMatRenderContext **ppRenderContext )
	{
		m_ppRenderContext = ppRenderContext;
	}

	// offset: origin of the grid. bNoMargin and bDeferDraw are accepted for the
	// retail signature; this tiler has no margin cells and always draws in
	// endFrame().
	void beginFrame( const Point3D &offset, bool bNoMargin, bool bDeferDraw );
	void insertParticle( const ImpParticleWithFourInterpolants *pParticle );
	void insertParticle( const ImpParticleWithOneInterpolant *pParticle );

	// Build the surface. The sorted version orders tiles back to front from eye.
	void drawSurface( bool bDeferDraw );
	void drawSurfaceSorted( const Point3D &eye, bool bDeferDraw );

	// Draw what drawSurface built, then release the frame's data.
	void endFrame( bool bDeferDraw );

	int getNoTiles() const { return m_Tiles.size; }
	Point3D getTileOffset( int nTile ) const; // center of the tile
	Point3D getRenderDim() const;             // size of a tile

	// The surface built by drawSurface(), for tests and tools.
	int GetVertexCount() const { return m_Vertices.size; }
	int GetTriangleCount() const { return m_Indices.size / 3; }
	const Vertex_t &GetVertex( int i ) const { return m_Vertices[i]; }
	const int *GetTriangle( int i ) const { return &m_Indices.a[3 * i]; }
	// Field value at a point (the surface is where it equals GetThreshold()).
	float EvaluateField( const Vector &vecPoint ) const;
	float GetThreshold() const { return m_flThreshold; }

private:
	struct Particle_t
	{
		Vector center;
		float scale;
		Vector color;
		Vector tangent;
	};

	struct TileRange_t
	{
		int nFirstIndex;
		int nIndexCount;
		float flSortDist;
	};

	void UpdateThreshold();
	ImpTileBlock_t *FindOrCreateTile( int bx, int by, int bz );
	void ScatterParticle( int nParticle );
	void EvaluateAt( const int *pParticles, int nParticles, const Vector &vecPoint, float *pValue,
	    Vector *pGrad, Vector *pColor, Vector *pTangent ) const;
	void PolygonizeTile( ImpTileBlock_t *pTile );
	int EdgeVertex( ImpTileBlock_t *pTile, int x, int y, int z, int nDir, int *pEdgeCache );
	void EmitTriangle( int a, int b, int c, const Vector &vecOut );
	void BuildSurface( const Point3D *pEye );
	void DrawTriangles();
	void ReleaseFrame();

	float m_flCubeWidth;
	float m_flRenderR;
	float m_flCutoffR;
	float m_flThreshold;
	int m_nTileIndexToDraw;
	IMatRenderContext **m_ppRenderContext;
	Point3D m_Offset;

	SmartArray<Particle_t, false, 16> m_Particles;
	SmartArray<ImpTileBlock_t *, false, 16> m_Tiles;
	SmartArray<ImpTileBlock_t *, false, 16> m_FreeTiles;
	SmartArray<Vertex_t, false, 16> m_Vertices;
	SmartArray<int, false, 16> m_Indices;
	SmartArray<TileRange_t, false, 16> m_TileRanges; // draw order
	SmartArray<int, false, 16> m_EdgeCache;
	int m_nLastTile;

	ImpTiler( const ImpTiler & );
	ImpTiler &operator=( const ImpTiler & );
};

// Pool of tilers; getTiler() and returnTiler() may run on different threads
// (a deferred draw returns its tiler on the material thread).
class ImpTilerFactory
{
public:
	static ImpTilerFactory *factory;

	ImpTiler *getTiler();
	void returnTiler( ImpTiler *pTiler );

	ImpTilerFactory() {}
	~ImpTilerFactory();

private:
	SmartArray<ImpTiler *, false, 16> m_Tilers;
	CThreadFastMutex m_Mutex;
};

#endif // BLOBULATOR_IMPTILER_H
