//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal placement carving conformance (RFC 0003, Q-JOBS). The
//          reference is the original CPortalSimulator::CreatePolyhedrons clip
//          code (commit 3d3e5e68); the candidate is game/shared/portal/
//          portal_carve.h, which the simulator now calls, run as the legacy
//          loop, the deterministic serial graph and the pooled graph on real
//          vstdlib thread pools. Every produced polyhedron is compared bit for
//          bit (vertices, lines, indices, polygon normals) and in list order.
//
//          Real mathlib clipping, no engine, content or display. Scenarios are
//          generated portal placements on tiled walls with floors, adjoining
//          walls, clutter, wedges, static-prop pieces and missing brushes.
//
//=============================================================================//

#include <cstdio>
#include <cstring>
#include <vector>

#include "mathlib/mathlib.h"
#include "mathlib/polyhedron.h"
#include "tier1/utlvector.h"
#include "vstdlib/jobthread.h"
#include "game/shared/portal/portal_carve.h"

static int s_checks = 0;
static int s_failures = 0;

static void Check( bool value, const char *what, const char *detail )
{
	++s_checks;
	if ( !value )
	{
		++s_failures;
		std::printf( "FAIL %s: %s\n", what, detail );
	}
}

// ---------------------------------------------------------------------------
// Scenario world: brush polyhedrons by id (NULL = cache miss), AABB query.
struct TestWorld
{
	std::vector<CPolyhedron *> brushes;
	std::vector<Vector> mins, maxs;

	int Add( CPolyhedron *p )
	{
		brushes.push_back( p );
		Vector lo( 1e30f, 1e30f, 1e30f ), hi( -1e30f, -1e30f, -1e30f );
		for ( int i = 0; p && i < p->iVertexCount; ++i )
		{
			const Vector &v = p->pVertices[i];
			lo.x = MIN( lo.x, v.x );
			lo.y = MIN( lo.y, v.y );
			lo.z = MIN( lo.z, v.z );
			hi.x = MAX( hi.x, v.x );
			hi.y = MAX( hi.y, v.y );
			hi.z = MAX( hi.z, v.z );
		}
		mins.push_back( lo );
		maxs.push_back( hi );
		return int( brushes.size() ) - 1;
	}
	const CPolyhedron *GetBrushPolyhedron( int id ) const
	{
		if ( id < 0 || id >= int( brushes.size() ) )
			return NULL;
		return brushes[id];
	}
	// Engine brush query stand-in. Cache misses (NULL) are always reported, as
	// a brush without cached collision still comes back from the engine.
	void GetBrushesInAABB( const Vector &lo, const Vector &hi, CUtlVector<int> *pOut ) const
	{
		for ( int i = 0; i < int( brushes.size() ); ++i )
		{
			if ( !brushes[i] || ( mins[i].x <= hi.x && maxs[i].x >= lo.x && mins[i].y <= hi.y &&
			                        maxs[i].y >= lo.y && mins[i].z <= hi.z && maxs[i].z >= lo.z ) )
				pOut->AddToTail( i );
		}
	}
	void Release()
	{
		for ( size_t i = 0; i < brushes.size(); ++i )
		{
			if ( brushes[i] )
				brushes[i]->Release();
		}
		brushes.clear();
		mins.clear();
		maxs.clear();
	}
};

// ---------------------------------------------------------------------------
// Reference: original helpers and clip loops (3d3e5e68), cache lookups bound
// to the scenario world. Kept byte-for-byte in the legacy layout so it can be
// diffed against the original source; formatting it would obscure that.
// clang-format off
static void OracleConvertBrushListToClippedPolyhedronList( const TestWorld &world, const int *pBrushes, int iBrushCount, const float *pOutwardFacingClipPlanes, int iClipPlaneCount, float fClipEpsilon, CUtlVector<CPolyhedron *> *pPolyhedronList )
{
	if( pPolyhedronList == NULL )
		return;

	if( (pBrushes == NULL) || (iBrushCount == 0) )
		return;

	for( int i = 0; i != iBrushCount; ++i )
	{
		CPolyhedron *pPolyhedron = ClipPolyhedron( world.GetBrushPolyhedron( pBrushes[i] ), pOutwardFacingClipPlanes, iClipPlaneCount, fClipEpsilon );
		if( pPolyhedron )
			pPolyhedronList->AddToTail( pPolyhedron );
	}
}

static void OracleClipPolyhedrons( CPolyhedron * const *pExistingPolyhedrons, int iPolyhedronCount, const float *pOutwardFacingClipPlanes, int iClipPlaneCount, float fClipEpsilon, CUtlVector<CPolyhedron *> *pPolyhedronList )
{
	if( pPolyhedronList == NULL )
		return;

	if( (pExistingPolyhedrons == NULL) || (iPolyhedronCount == 0) )
		return;

	for( int i = 0; i != iPolyhedronCount; ++i )
	{
		CPolyhedron *pPolyhedron = ClipPolyhedron( pExistingPolyhedrons[i], pOutwardFacingClipPlanes, iClipPlaneCount, fClipEpsilon );
		if( pPolyhedron )
			pPolyhedronList->AddToTail( pPolyhedron );
	}
}

// Original static-prop loop; props are visited from the end of the list.
static void OracleStaticProps( const std::vector< std::vector<CPolyhedron *> > &props, const float fWorldClipPlane_Reverse[4], CUtlVector<CPolyhedron *> &Polyhedrons, std::vector<int> &groupCounts )
{
	for( int i = int( props.size() ); --i >= 0; )
	{
		int iStartIndex = Polyhedrons.Count();
		for( int j = 0; j != int( props[i].size() ); ++j )
		{
			CPolyhedron *pPropPolyhedronPiece = props[i][j];
			if( pPropPolyhedronPiece )
			{
				CPolyhedron *pClippedPropPolyhedron = ClipPolyhedron( pPropPolyhedronPiece, fWorldClipPlane_Reverse, 1, 0.01f, false );
				if( pClippedPropPolyhedron )
					Polyhedrons.AddToTail( pClippedPropPolyhedron );
			}
		}
		groupCounts.push_back( Polyhedrons.Count() - iStartIndex );
	}
}

static void OracleWall( const TestWorld &world, const PortalCarve::Placement_t &placement, const float fWallClipPlane_Forward[4], bool bSimulatingVPhysics, CUtlVector<CPolyhedron *> &TubeOut, CUtlVector<CPolyhedron *> &BrushesOut )
{
	// Verbatim "(Holy) Wall" block of CreatePolyhedrons, extracted mechanically
	// with m_InternalData.Placement -> placement, the two output lists, the
	// vphysics flag and the brush query/helpers bound to this test world.
	//(Holy) Wall
	{
		Assert( TubeOut.Count() == 0 );
		Assert( BrushesOut.Count() == 0 );

		Vector vBackward = -placement.vForward;
		Vector vLeft = -placement.vRight;
		Vector vDown = -placement.vUp;

		Vector vOBBForward = -placement.vForward;
		Vector vOBBRight = -placement.vRight;
		Vector vOBBUp = placement.vUp;

		//scale the extents to usable sizes
		vOBBForward *= PORTAL_WALL_FARDIST / 2.0f;
		vOBBRight *= PORTAL_WALL_FARDIST * 2.0f;
		vOBBUp *= PORTAL_WALL_FARDIST * 2.0f;

		Vector ptOBBOrigin = placement.ptCenter;
		ptOBBOrigin -= vOBBRight / 2.0f;
		ptOBBOrigin -= vOBBUp / 2.0f;

		Vector vAABBMins, vAABBMaxs;
		vAABBMins = vAABBMaxs = ptOBBOrigin;

		for( int i = 1; i != 8; ++i )
		{
			Vector ptTest = ptOBBOrigin;
			if( i & (1 << 0) ) ptTest += vOBBForward;
			if( i & (1 << 1) ) ptTest += vOBBRight;
			if( i & (1 << 2) ) ptTest += vOBBUp;

			if( ptTest.x < vAABBMins.x ) vAABBMins.x = ptTest.x;
			if( ptTest.y < vAABBMins.y ) vAABBMins.y = ptTest.y;
			if( ptTest.z < vAABBMins.z ) vAABBMins.z = ptTest.z;
			if( ptTest.x > vAABBMaxs.x ) vAABBMaxs.x = ptTest.x;
			if( ptTest.y > vAABBMaxs.y ) vAABBMaxs.y = ptTest.y;
			if( ptTest.z > vAABBMaxs.z ) vAABBMaxs.z = ptTest.z;
		}


		float fPlanes[6 * 4];

		//first and second planes are always forward and backward planes
		fPlanes[(0*4) + 0] = fWallClipPlane_Forward[0];
		fPlanes[(0*4) + 1] = fWallClipPlane_Forward[1];
		fPlanes[(0*4) + 2] = fWallClipPlane_Forward[2];
		fPlanes[(0*4) + 3] = fWallClipPlane_Forward[3] - PORTAL_WALL_TUBE_OFFSET;

		fPlanes[(1*4) + 0] = vBackward.x;
		fPlanes[(1*4) + 1] = vBackward.y;
		fPlanes[(1*4) + 2] = vBackward.z;
		float fTubeDepthDist = vBackward.Dot( placement.ptCenter + (vBackward * (PORTAL_WALL_TUBE_DEPTH + PORTAL_WALL_TUBE_OFFSET)) );
		fPlanes[(1*4) + 3] = fTubeDepthDist;


		//the remaining planes will always have the same ordering of normals, with different distances plugged in for each convex we're creating
		//normal order is up, down, left, right

		fPlanes[(2*4) + 0] = placement.vUp.x;
		fPlanes[(2*4) + 1] = placement.vUp.y;
		fPlanes[(2*4) + 2] = placement.vUp.z;
		fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (placement.vUp * PORTAL_HOLE_HALF_HEIGHT) );

		fPlanes[(3*4) + 0] = vDown.x;
		fPlanes[(3*4) + 1] = vDown.y;
		fPlanes[(3*4) + 2] = vDown.z;
		fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + (vDown * PORTAL_HOLE_HALF_HEIGHT) );

		fPlanes[(4*4) + 0] = vLeft.x;
		fPlanes[(4*4) + 1] = vLeft.y;
		fPlanes[(4*4) + 2] = vLeft.z;
		fPlanes[(4*4) + 3] = vLeft.Dot( placement.ptCenter + (vLeft * PORTAL_HOLE_HALF_WIDTH) );

		fPlanes[(5*4) + 0] = placement.vRight.x;
		fPlanes[(5*4) + 1] = placement.vRight.y;
		fPlanes[(5*4) + 2] = placement.vRight.z;
		fPlanes[(5*4) + 3] = placement.vRight.Dot( placement.ptCenter + (placement.vRight * PORTAL_HOLE_HALF_WIDTH) );

		float *fSidePlanesOnly = &fPlanes[(2*4)];

		//these 2 get re-used a bit
		float fFarRightPlaneDistance = placement.vRight.Dot( placement.ptCenter + placement.vRight * (PORTAL_WALL_FARDIST * 10.0f) );
		float fFarLeftPlaneDistance = vLeft.Dot( placement.ptCenter + vLeft * (PORTAL_WALL_FARDIST * 10.0f) );


		CUtlVector<int> WallBrushes;
		CUtlVector<CPolyhedron *> WallBrushPolyhedrons_ClippedToWall;
		CPolyhedron **pWallClippedPolyhedrons = NULL;
		int iWallClippedPolyhedronCount = 0;
		if( bSimulatingVPhysics ) //if not simulating vphysics, we skip making the entire wall, and just create the minimal tube instead
		{
			world.GetBrushesInAABB( vAABBMins, vAABBMaxs, &WallBrushes );

			if( WallBrushes.Count() != 0 )
				OracleConvertBrushListToClippedPolyhedronList( world, WallBrushes.Base(), WallBrushes.Count(), fPlanes, 1, PORTAL_POLYHEDRON_CUT_EPSILON, &WallBrushPolyhedrons_ClippedToWall );
			
			if( WallBrushPolyhedrons_ClippedToWall.Count() != 0 )
			{
				for( int i = WallBrushPolyhedrons_ClippedToWall.Count(); --i >= 0; )
				{
					CPolyhedron *pPolyhedron = ClipPolyhedron( WallBrushPolyhedrons_ClippedToWall[i], fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, true );
					if( pPolyhedron )
					{
						//a chunk of this brush passes through the hole, not eligible to be removed from cutting
						pPolyhedron->Release();
					}
					else
					{
						//no part of this brush interacts with the hole, no point in cutting the brush any later
						BrushesOut.AddToTail( WallBrushPolyhedrons_ClippedToWall[i] );
						WallBrushPolyhedrons_ClippedToWall.FastRemove( i );
					}
				}

				if( WallBrushPolyhedrons_ClippedToWall.Count() != 0 ) //might have become 0 while removing uncut brushes
				{
					pWallClippedPolyhedrons = WallBrushPolyhedrons_ClippedToWall.Base();
					iWallClippedPolyhedronCount = WallBrushPolyhedrons_ClippedToWall.Count();
				}
			}
		}


		//upper wall
		{
			//minimal portion that extends into the hole space
			//fPlanes[(1*4) + 3] = fTubeDepthDist;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + placement.vUp * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + placement.vUp * PORTAL_HOLE_HALF_HEIGHT );
			fPlanes[(4*4) + 3] = vLeft.Dot( placement.ptCenter + vLeft * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS) );
			fPlanes[(5*4) + 3] = placement.vRight.Dot( placement.ptCenter + placement.vRight * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS) );

			CPolyhedron *pTubePolyhedron = GeneratePolyhedronFromPlanes( fPlanes, 6, PORTAL_POLYHEDRON_CUT_EPSILON );
			if( pTubePolyhedron )
				TubeOut.AddToTail( pTubePolyhedron );

			//general hole cut
			//fPlanes[(1*4) + 3] += 2000.0f;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + placement.vUp * (PORTAL_WALL_FARDIST * 10.0f) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + placement.vUp * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS) );
			fPlanes[(4*4) + 3] = fFarLeftPlaneDistance;
			fPlanes[(5*4) + 3] = fFarRightPlaneDistance;

			

			OracleClipPolyhedrons( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut );
		}

		//lower wall
		{
			//minimal portion that extends into the hole space
			//fPlanes[(1*4) + 3] = fTubeDepthDist;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (vDown * PORTAL_HOLE_HALF_HEIGHT) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + vDown * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS) );
			fPlanes[(4*4) + 3] = vLeft.Dot( placement.ptCenter + vLeft * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS) );
			fPlanes[(5*4) + 3] = placement.vRight.Dot( placement.ptCenter + placement.vRight * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS) );

			CPolyhedron *pTubePolyhedron = GeneratePolyhedronFromPlanes( fPlanes, 6, PORTAL_POLYHEDRON_CUT_EPSILON );
			if( pTubePolyhedron )
				TubeOut.AddToTail( pTubePolyhedron );

			//general hole cut
			//fPlanes[(1*4) + 3] += 2000.0f;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (vDown * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS)) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + (vDown * (PORTAL_WALL_FARDIST * 10.0f)) );
			fPlanes[(4*4) + 3] = fFarLeftPlaneDistance;
			fPlanes[(5*4) + 3] = fFarRightPlaneDistance;

			OracleClipPolyhedrons( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut );
		}

		//left wall
		{
			//minimal portion that extends into the hole space
			//fPlanes[(1*4) + 3] = fTubeDepthDist;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (placement.vUp * PORTAL_HOLE_HALF_HEIGHT) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + (vDown * PORTAL_HOLE_HALF_HEIGHT) );
			fPlanes[(4*4) + 3] = vLeft.Dot( placement.ptCenter + (vLeft * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS)) );
			fPlanes[(5*4) + 3] = placement.vRight.Dot( placement.ptCenter + (vLeft * PORTAL_HOLE_HALF_WIDTH) );

			CPolyhedron *pTubePolyhedron = GeneratePolyhedronFromPlanes( fPlanes, 6, PORTAL_POLYHEDRON_CUT_EPSILON );
			if( pTubePolyhedron )
				TubeOut.AddToTail( pTubePolyhedron );

			//general hole cut
			//fPlanes[(1*4) + 3] += 2000.0f;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (placement.vUp * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS)) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter - (placement.vUp * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS)) );
			fPlanes[(4*4) + 3] = fFarLeftPlaneDistance;
			fPlanes[(5*4) + 3] = placement.vRight.Dot( placement.ptCenter + (vLeft * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS)) );

			OracleClipPolyhedrons( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut );
		}

		//right wall
		{
			//minimal portion that extends into the hole space
			//fPlanes[(1*4) + 3] = fTubeDepthDist;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (placement.vUp * (PORTAL_HOLE_HALF_HEIGHT)) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + (vDown * (PORTAL_HOLE_HALF_HEIGHT)) );
			fPlanes[(4*4) + 3] = vLeft.Dot( placement.ptCenter + placement.vRight * PORTAL_HOLE_HALF_WIDTH );
			fPlanes[(5*4) + 3] = placement.vRight.Dot( placement.ptCenter + placement.vRight * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS) );

			CPolyhedron *pTubePolyhedron = GeneratePolyhedronFromPlanes( fPlanes, 6, PORTAL_POLYHEDRON_CUT_EPSILON );
			if( pTubePolyhedron )
				TubeOut.AddToTail( pTubePolyhedron );

			//general hole cut
			//fPlanes[(1*4) + 3] += 2000.0f;
			fPlanes[(2*4) + 3] = placement.vUp.Dot( placement.ptCenter + (placement.vUp * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS)) );
			fPlanes[(3*4) + 3] = vDown.Dot( placement.ptCenter + (vDown * (PORTAL_HOLE_HALF_HEIGHT + PORTAL_WALL_MIN_THICKNESS)) );
			fPlanes[(4*4) + 3] = vLeft.Dot( placement.ptCenter + placement.vRight * (PORTAL_HOLE_HALF_WIDTH + PORTAL_WALL_MIN_THICKNESS) );
			fPlanes[(5*4) + 3] = fFarRightPlaneDistance;

			OracleClipPolyhedrons( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut );
		}

		for( int i = WallBrushPolyhedrons_ClippedToWall.Count(); --i >= 0; )
			WallBrushPolyhedrons_ClippedToWall[i]->Release();

		WallBrushPolyhedrons_ClippedToWall.RemoveAll();
	}
}
// clang-format on

// ---------------------------------------------------------------------------
struct CarveOutput
{
	CUtlVector<CPolyhedron *> worldBrushes;
	CUtlVector<CPolyhedron *> props;
	std::vector<int> propGroups;
	CUtlVector<CPolyhedron *> tube;
	CUtlVector<CPolyhedron *> wall;

	void Release()
	{
		CUtlVector<CPolyhedron *> *lists[] = { &worldBrushes, &props, &tube, &wall };
		for ( int l = 0; l < 4; ++l )
		{
			for ( int i = 0; i < lists[l]->Count(); ++i )
				( *lists[l] )[i]->Release();
			lists[l]->RemoveAll();
		}
		propGroups.clear();
	}
};

struct Scenario
{
	TestWorld world;
	std::vector<int> worldBrushIds; // world (in front of the portal) query result
	std::vector<std::vector<CPolyhedron *>> props;
	PortalCarve::Placement_t placement;
	float fWorldClipPlane_Reverse[4];
	float fWallClipPlane_Forward[4];
	bool bSimulatingVPhysics;

	void Release()
	{
		world.Release();
		for ( size_t i = 0; i < props.size(); ++i )
		{
			for ( size_t j = 0; j < props[i].size(); ++j )
			{
				if ( props[i][j] )
					props[i][j]->Release();
			}
		}
		props.clear();
		worldBrushIds.clear();
	}
};

static void RunOracle( const Scenario &sc, CarveOutput &out )
{
	OracleConvertBrushListToClippedPolyhedronList( sc.world,
	    sc.worldBrushIds.empty() ? NULL : &sc.worldBrushIds[0], int( sc.worldBrushIds.size() ),
	    sc.fWorldClipPlane_Reverse, 1, PORTAL_POLYHEDRON_CUT_EPSILON, &out.worldBrushes );
	OracleStaticProps( sc.props, sc.fWorldClipPlane_Reverse, out.props, out.propGroups );
	OracleWall( sc.world, sc.placement, sc.fWallClipPlane_Forward, sc.bSimulatingVPhysics, out.tube,
	    out.wall );
}

// Mirrors the CreatePolyhedrons call sites that now use portal_carve.h.
static bool RunCandidate( const Scenario &sc, int iMode, IThreadPool *pPool, CarveOutput &out )
{
	CUtlVector<const CPolyhedron *> worldPolys;
	for ( size_t i = 0; i < sc.worldBrushIds.size(); ++i )
		worldPolys.AddToTail( sc.world.GetBrushPolyhedron( sc.worldBrushIds[i] ) );
	if ( !PortalCarve::ClipInOrder( worldPolys.Base(), worldPolys.Count(),
	         sc.fWorldClipPlane_Reverse, 1, PORTAL_POLYHEDRON_CUT_EPSILON, &out.worldBrushes, iMode,
	         pPool ) )
		return false;

	CUtlVector<const CPolyhedron *> pieces;
	CUtlVector<int> groupSizes;
	for ( int i = int( sc.props.size() ); --i >= 0; )
	{
		groupSizes.AddToTail( int( sc.props[i].size() ) );
		for ( size_t j = 0; j < sc.props[i].size(); ++j )
			pieces.AddToTail( sc.props[i][j] );
	}
	out.propGroups.assign( sc.props.size(), -1 );
	if ( !PortalCarve::ClipGroupsInOrder( pieces.Base(), groupSizes.Base(), groupSizes.Count(),
	         sc.fWorldClipPlane_Reverse, 1, 0.01f, &out.props,
	         out.propGroups.empty() ? NULL : &out.propGroups[0], iMode, pPool ) )
		return false;

	CUtlVector<const CPolyhedron *> wallPolys;
	if ( sc.bSimulatingVPhysics )
	{
		Vector lo, hi;
		PortalCarve::ComputeWallBrushBounds( sc.placement, &lo, &hi );
		CUtlVector<int> ids;
		sc.world.GetBrushesInAABB( lo, hi, &ids );
		for ( int i = 0; i < ids.Count(); ++i )
			wallPolys.AddToTail( sc.world.GetBrushPolyhedron( ids[i] ) );
	}
	return PortalCarve::CarveWall( sc.placement, sc.fWallClipPlane_Forward, sc.bSimulatingVPhysics,
	    wallPolys.Base(), wallPolys.Count(), out.tube, out.wall, iMode, pPool );
}

// Deliberate defects applied to a correct candidate's output; the comparator
// must reject each in at least one scenario.
enum Mutation_t
{
	MUTATE_NONE,
	MUTATE_SWAP_WALL_PIECES, // unordered merge of the hole cuts
	MUTATE_DROP_LAST_WORLD,  // lost result slot
	MUTATE_WORLD_EPSILON,    // wrong clip epsilon (plane handling drift)
	MUTATE_SHIFT_PROP_GROUP, // prop pieces attributed to the wrong prop
	MUTATE_COUNT
};

static const char *MutationName( int m )
{
	switch ( m )
	{
	case MUTATE_SWAP_WALL_PIECES:
		return "adjacent wall pieces swapped";
	case MUTATE_DROP_LAST_WORLD:
		return "last world brush result dropped";
	case MUTATE_WORLD_EPSILON:
		return "world clip epsilon changed";
	case MUTATE_SHIFT_PROP_GROUP:
		return "static-prop group boundary shifted";
	}
	return "none";
}

static bool RunMutant( const Scenario &sc, int mutation, CarveOutput &out );

// ---------------------------------------------------------------------------
static unsigned int s_rng = 1;
static unsigned int Rand()
{
	s_rng ^= s_rng << 13;
	s_rng ^= s_rng >> 17;
	s_rng ^= s_rng << 5;
	return s_rng;
}
static float RandRange( float lo, float hi )
{
	return lo + ( hi - lo ) * float( Rand() & 0xffff ) / 65535.0f;
}

// Oriented box (outward planes) with optional extra cut planes (wedges).
static CPolyhedron *MakeBrush( const Vector &c, const Vector &a0, const Vector &a1,
    const Vector &a2, const Vector &h, int extraCuts )
{
	float planes[9 * 4];
	const Vector *axes[3] = { &a0, &a1, &a2 };
	int n = 0;
	for ( int k = 0; k < 3; ++k )
	{
		const Vector &a = *axes[k];
		const float d = a.Dot( c );
		planes[n * 4 + 0] = a.x;
		planes[n * 4 + 1] = a.y;
		planes[n * 4 + 2] = a.z;
		planes[n * 4 + 3] = d + h[k];
		++n;
		planes[n * 4 + 0] = -a.x;
		planes[n * 4 + 1] = -a.y;
		planes[n * 4 + 2] = -a.z;
		planes[n * 4 + 3] = -d + h[k];
		++n;
	}
	for ( int k = 0; k < extraCuts; ++k )
	{
		Vector nrm( RandRange( -1, 1 ), RandRange( -1, 1 ), RandRange( -1, 1 ) );
		if ( VectorNormalize( nrm ) < 0.1f )
			nrm = a0;
		const float reach = fabsf( nrm.Dot( a0 ) ) * h.x + fabsf( nrm.Dot( a1 ) ) * h.y +
		                    fabsf( nrm.Dot( a2 ) ) * h.z;
		planes[n * 4 + 0] = nrm.x;
		planes[n * 4 + 1] = nrm.y;
		planes[n * 4 + 2] = nrm.z;
		planes[n * 4 + 3] = nrm.Dot( c ) + reach * RandRange( 0.2f, 0.9f );
		++n;
	}
	return GeneratePolyhedronFromPlanes( planes, n, PORTAL_POLYHEDRON_CUT_EPSILON );
}

static void BuildScenario( Scenario &sc, unsigned int seed )
{
	s_rng = seed * 2654435761u + 12345u;
	static const Vector kAxes[6] = { Vector( 1, 0, 0 ), Vector( -1, 0, 0 ), Vector( 0, 1, 0 ),
	    Vector( 0, -1, 0 ), Vector( 0, 0, 1 ), Vector( 0, 0, -1 ) };
	Vector f;
	const unsigned int kind = Rand() % 8;
	if ( kind < 6 )
		f = kAxes[kind]; // walls, floor and ceiling portals, as in Portal maps
	else
	{
		f = Vector(
		    RandRange( -1, 1 ), RandRange( -1, 1 ), RandRange( -0.7f, 0.7f ) ); // angled panels
		if ( VectorNormalize( f ) < 0.1f )
			f = Vector( 1, 0, 0 );
	}
	Vector upHint = fabsf( f.z ) > 0.9f ? Vector( 1, 0, 0 ) : Vector( 0, 0, 1 );
	Vector r = CrossProduct( f, upHint );
	VectorNormalize( r );
	Vector u = CrossProduct( r, f );
	VectorNormalize( u );
	// Source convention: right = forward x up; keep the basis right-handed.
	r = CrossProduct( f, u );
	VectorNormalize( r );

	const Vector center(
	    RandRange( -2000, 2000 ), RandRange( -2000, 2000 ), RandRange( -500, 500 ) );
	sc.placement.ptCenter = center;
	sc.placement.vForward = f;
	sc.placement.vRight = r;
	sc.placement.vUp = u;
	const float dist = f.Dot( center );
	sc.fWallClipPlane_Forward[0] = f.x;
	sc.fWallClipPlane_Forward[1] = f.y;
	sc.fWallClipPlane_Forward[2] = f.z;
	sc.fWallClipPlane_Forward[3] = dist;
	sc.fWorldClipPlane_Reverse[0] = -f.x;
	sc.fWorldClipPlane_Reverse[1] = -f.y;
	sc.fWorldClipPlane_Reverse[2] = -f.z;
	sc.fWorldClipPlane_Reverse[3] = -( dist + PORTAL_WORLD_WALL_HALF_SEPARATION_AMOUNT );
	sc.bSimulatingVPhysics = ( Rand() % 6 ) != 0;

	// The wall the portal sits on: tiles behind the surface, offset so the hole
	// straddles tile seams; a few tiles missing or slightly recessed.
	const float tile = ( Rand() & 1 ) ? 64.0f : 128.0f;
	const float thick = RandRange( 4.0f, 32.0f );
	const float offR = RandRange( 0.0f, tile ), offU = RandRange( 0.0f, tile );
	for ( float y = -448.0f; y < 448.0f; y += tile )
	{
		for ( float x = -448.0f; x < 448.0f; x += tile )
		{
			if ( Rand() % 17 == 0 )
				continue;
			const float recess = ( Rand() % 11 == 0 ) ? RandRange( 0.5f, 8.0f ) : 0.0f;
			const Vector c =
			    center + r * ( x + offR ) + u * ( y + offU ) - f * ( thick * 0.5f + recess );
			sc.world.Add(
			    MakeBrush( c, r, u, f, Vector( tile * 0.5f, tile * 0.5f, thick * 0.5f ), 0 ) );
		}
	}
	// Room shell: floor/ceiling/side walls meeting the portal wall, in world axes.
	for ( int k = 0; k < 4; ++k )
	{
		const Vector c = center + f * RandRange( 64, 256 ) + r * RandRange( -400, 400 ) +
		                 u * RandRange( -300, 300 );
		sc.world.Add( MakeBrush( c, Vector( 1, 0, 0 ), Vector( 0, 1, 0 ), Vector( 0, 0, 1 ),
		    Vector( RandRange( 8, 512 ), RandRange( 8, 512 ), RandRange( 8, 256 ) ), 0 ) );
	}
	// Clutter straddling and near the portal plane: rotated boxes and wedges.
	const int clutter = 6 + int( Rand() % 20 );
	for ( int k = 0; k < clutter; ++k )
	{
		Vector a0( RandRange( -1, 1 ), RandRange( -1, 1 ), RandRange( -1, 1 ) );
		if ( VectorNormalize( a0 ) < 0.1f )
			a0 = f;
		Vector a1 = CrossProduct( a0, Rand() & 1 ? f : u );
		if ( VectorNormalize( a1 ) < 0.1f )
			a1 = r;
		Vector a2 = CrossProduct( a0, a1 );
		VectorNormalize( a2 );
		const Vector c = center + f * RandRange( -64, 96 ) + r * RandRange( -160, 160 ) +
		                 u * RandRange( -200, 200 );
		sc.world.Add( MakeBrush( c, a0, a1, a2,
		    Vector( RandRange( 2, 96 ), RandRange( 2, 96 ), RandRange( 2, 96 ) ),
		    int( Rand() % 3 ) ) );
	}
	// Brushes without cached collision.
	for ( int k = int( Rand() % 3 ); k > 0; --k )
		sc.world.Add( NULL );

	// World query result for the front half-space (legacy bounds math is not
	// under test); a slice of all brushes including cache misses.
	for ( int i = 0; i < int( sc.world.brushes.size() ); ++i )
	{
		if ( Rand() % 4 != 0 )
			sc.worldBrushIds.push_back( i );
	}
	sc.worldBrushIds.push_back( int( sc.world.brushes.size() ) + 7 ); // invalid id

	// Static props: 0-7 props of 0-4 convex pieces (some missing), both sides.
	const int nProps = int( Rand() % 8 );
	for ( int p = 0; p < nProps; ++p )
	{
		std::vector<CPolyhedron *> pieces;
		const Vector base = center + f * RandRange( -48, 128 ) + r * RandRange( -150, 150 ) +
		                    u * RandRange( -150, 150 );
		const int nPieces = int( Rand() % 5 );
		for ( int j = 0; j < nPieces; ++j )
		{
			if ( Rand() % 9 == 0 )
			{
				pieces.push_back( NULL );
				continue;
			}
			const Vector c =
			    base + Vector( RandRange( -24, 24 ), RandRange( -24, 24 ), RandRange( -24, 24 ) );
			pieces.push_back( MakeBrush( c, Vector( 1, 0, 0 ), Vector( 0, 1, 0 ), Vector( 0, 0, 1 ),
			    Vector( RandRange( 2, 40 ), RandRange( 2, 40 ), RandRange( 2, 40 ) ),
			    int( Rand() % 3 ) ) );
		}
		sc.props.push_back( pieces );
	}
}

// Independent coverage probe: wall brushes (clipped to the wall plane) that do
// or do not reach into the portal hole, using the hole's four side planes.
static void CountWallBrushes( const Scenario &sc, int *pCrossing, int *pClear )
{
	const PortalCarve::Placement_t &p = sc.placement;
	Vector lo, hi;
	PortalCarve::ComputeWallBrushBounds( p, &lo, &hi );
	CUtlVector<int> ids;
	sc.world.GetBrushesInAABB( lo, hi, &ids );
	float wall[4] = { sc.fWallClipPlane_Forward[0], sc.fWallClipPlane_Forward[1],
	    sc.fWallClipPlane_Forward[2], sc.fWallClipPlane_Forward[3] - PORTAL_WALL_TUBE_OFFSET };
	const Vector sides[4] = { p.vUp, -p.vUp, -p.vRight, p.vRight };
	const float extent[4] = { PORTAL_HOLE_HALF_HEIGHT, PORTAL_HOLE_HALF_HEIGHT,
	    PORTAL_HOLE_HALF_WIDTH, PORTAL_HOLE_HALF_WIDTH };
	float hole[16];
	for ( int k = 0; k < 4; ++k )
	{
		hole[k * 4 + 0] = sides[k].x;
		hole[k * 4 + 1] = sides[k].y;
		hole[k * 4 + 2] = sides[k].z;
		hole[k * 4 + 3] = sides[k].Dot( p.ptCenter + sides[k] * extent[k] );
	}
	for ( int i = 0; i < ids.Count(); ++i )
	{
		CPolyhedron *pWall = ClipPolyhedron(
		    sc.world.GetBrushPolyhedron( ids[i] ), wall, 1, PORTAL_POLYHEDRON_CUT_EPSILON );
		if ( !pWall )
			continue;
		CPolyhedron *pHole = ClipPolyhedron( pWall, hole, 4, PORTAL_POLYHEDRON_CUT_EPSILON );
		( pHole ? *pCrossing : *pClear ) += 1;
		if ( pHole )
			pHole->Release();
		pWall->Release();
	}
}

static bool RunMutant( const Scenario &sc, int mutation, CarveOutput &out )
{
	if ( !RunCandidate( sc, PortalCarve::MODE_SERIAL_GRAPH, NULL, out ) )
		return false;
	switch ( mutation )
	{
	case MUTATE_SWAP_WALL_PIECES:
		if ( out.wall.Count() >= 2 )
		{
			CPolyhedron *p = out.wall[out.wall.Count() - 1];
			out.wall[out.wall.Count() - 1] = out.wall[out.wall.Count() - 2];
			out.wall[out.wall.Count() - 2] = p;
		}
		break;
	case MUTATE_DROP_LAST_WORLD:
		if ( out.worldBrushes.Count() )
		{
			out.worldBrushes.Tail()->Release();
			out.worldBrushes.RemoveMultipleFromTail( 1 );
		}
		break;
	case MUTATE_WORLD_EPSILON:
	{
		for ( int i = 0; i < out.worldBrushes.Count(); ++i )
			out.worldBrushes[i]->Release();
		out.worldBrushes.RemoveAll();
		CUtlVector<const CPolyhedron *> worldPolys;
		for ( size_t i = 0; i < sc.worldBrushIds.size(); ++i )
			worldPolys.AddToTail( sc.world.GetBrushPolyhedron( sc.worldBrushIds[i] ) );
		PortalCarve::ClipInOrder( worldPolys.Base(), worldPolys.Count(), sc.fWorldClipPlane_Reverse,
		    1, 0.01f, &out.worldBrushes, PortalCarve::MODE_LEGACY, NULL );
		break;
	}
	case MUTATE_SHIFT_PROP_GROUP:
		for ( size_t g = 0; g + 1 < out.propGroups.size(); ++g )
		{
			if ( out.propGroups[g] > 0 )
			{
				--out.propGroups[g];
				++out.propGroups[g + 1];
				break;
			}
		}
		break;
	}
	return true;
}

// ---------------------------------------------------------------------------
static bool SamePolyhedron( const CPolyhedron *a, const CPolyhedron *b )
{
	if ( a->iVertexCount != b->iVertexCount || a->iLineCount != b->iLineCount ||
	     a->iIndexCount != b->iIndexCount || a->iPolygonCount != b->iPolygonCount )
		return false;
	if ( memcmp( a->pVertices, b->pVertices, sizeof( Vector ) * a->iVertexCount ) != 0 )
		return false;
	if ( memcmp( a->pLines, b->pLines, sizeof( Polyhedron_IndexedLine_t ) * a->iLineCount ) != 0 )
		return false;
	for ( int i = 0; i < a->iIndexCount; ++i )
	{
		if ( a->pIndices[i].iLineIndex != b->pIndices[i].iLineIndex ||
		     a->pIndices[i].iEndPointIndex != b->pIndices[i].iEndPointIndex )
			return false;
	}
	for ( int i = 0; i < a->iPolygonCount; ++i )
	{
		if ( a->pPolygons[i].iFirstIndex != b->pPolygons[i].iFirstIndex ||
		     a->pPolygons[i].iIndexCount != b->pPolygons[i].iIndexCount ||
		     memcmp( &a->pPolygons[i].polyNormal, &b->pPolygons[i].polyNormal, sizeof( Vector ) ) !=
		         0 )
			return false;
	}
	return true;
}

static bool SameList( const CUtlVector<CPolyhedron *> &a, const CUtlVector<CPolyhedron *> &b,
    char *why, size_t whySize, const char *name )
{
	if ( a.Count() != b.Count() )
	{
		std::snprintf( why, whySize, "%s count %d/%d", name, a.Count(), b.Count() );
		return false;
	}
	for ( int i = 0; i < a.Count(); ++i )
	{
		if ( !SamePolyhedron( a[i], b[i] ) )
		{
			std::snprintf( why, whySize, "%s[%d] differs", name, i );
			return false;
		}
	}
	return true;
}

static bool SameOutput( const CarveOutput &a, const CarveOutput &b, char *why, size_t whySize )
{
	if ( !SameList( a.worldBrushes, b.worldBrushes, why, whySize, "world brushes" ) ||
	     !SameList( a.props, b.props, why, whySize, "static props" ) ||
	     !SameList( a.tube, b.tube, why, whySize, "wall tube" ) ||
	     !SameList( a.wall, b.wall, why, whySize, "wall brushes" ) )
		return false;
	if ( a.propGroups != b.propGroups )
	{
		std::snprintf( why, whySize, "static prop group counts differ" );
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
int main( int argc, char **argv )
{
	bool benchmark = argc > 1 && std::strcmp( argv[1], "--benchmark" ) == 0;
	MathLib_Init();

	static const int kWorkers[] = { 0, 1, 3 };
	IThreadPool *pools[3];
	for ( int i = 0; i < 3; ++i )
	{
		pools[i] = CreateThreadPool();
		ThreadPoolStartParams_t params;
		params.nThreads = kWorkers[i];
		Check( pools[i]->Start( params ), "pool start", "" );
	}

	struct Candidate
	{
		int mode;
		int pool;
		const char *name;
	};
	static const Candidate kCandidates[] = {
	    { PortalCarve::MODE_LEGACY, -1, "legacy-loop" },
	    { PortalCarve::MODE_SERIAL_GRAPH, 2, "serial-graph" },
	    { PortalCarve::MODE_POOLED_GRAPH, 0, "pooled-graph/0" },
	    { PortalCarve::MODE_POOLED_GRAPH, 1, "pooled-graph/1" },
	    { PortalCarve::MODE_POOLED_GRAPH, 2, "pooled-graph/3" },
	};

	const int nScenarios = 160;
	int wallCutByHole = 0, wallUncut = 0, worldOut = 0, propOut = 0, propEmptyGroup = 0,
	    noVPhysics = 0, polyhedra = 0;
	double tLegacy = 0, tPooled = 0;
	int detected[MUTATE_COUNT] = { 0 };
	for ( int s = 0; s < nScenarios; ++s )
	{
		Scenario sc;
		BuildScenario( sc, unsigned( s + 1 ) );
		CarveOutput ref;
		RunOracle( sc, ref );

		// Coverage of the paths whose ordering the migration touches.
		noVPhysics += sc.bSimulatingVPhysics ? 0 : 1;
		worldOut += ref.worldBrushes.Count() ? 1 : 0;
		propOut += ref.props.Count() ? 1 : 0;
		for ( size_t g = 0; g < ref.propGroups.size(); ++g )
			propEmptyGroup += ref.propGroups[g] == 0 ? 1 : 0;
		polyhedra +=
		    ref.worldBrushes.Count() + ref.props.Count() + ref.tube.Count() + ref.wall.Count();
		if ( sc.bSimulatingVPhysics )
		{
			int crossing = 0, clear = 0;
			CountWallBrushes( sc, &crossing, &clear );
			wallCutByHole += crossing ? 1 : 0;
			wallUncut += clear ? 1 : 0;
		}

		for ( int m = MUTATE_NONE + 1; m < MUTATE_COUNT; ++m )
		{
			CarveOutput bad;
			char why[128] = "";
			if ( RunMutant( sc, m, bad ) && !SameOutput( ref, bad, why, sizeof( why ) ) )
				++detected[m];
			bad.Release();
		}

		for ( size_t c = 0; c < sizeof( kCandidates ) / sizeof( kCandidates[0] ); ++c )
		{
			const Candidate &cand = kCandidates[c];
			CarveOutput out;
			const double t0 = Plat_FloatTime();
			const bool ok =
			    RunCandidate( sc, cand.mode, cand.pool < 0 ? NULL : pools[cand.pool], out );
			const double dt = Plat_FloatTime() - t0;
			if ( cand.mode == PortalCarve::MODE_LEGACY )
				tLegacy += dt;
			else if ( cand.pool == 2 && cand.mode == PortalCarve::MODE_POOLED_GRAPH )
				tPooled += dt;
			char what[96], why[128] = "";
			std::snprintf( what, sizeof( what ), "scenario %d %s", s + 1, cand.name );
			Check( ok, what, "batch construction failed" );
			Check( ok && SameOutput( ref, out, why, sizeof( why ) ), what, why );
			out.Release();
		}
		ref.Release();
		sc.Release();
	}

	char detail[256];
	std::snprintf( detail, sizeof( detail ),
	    "polyhedra=%d world=%d props=%d emptyPropGroups=%d uncutWall=%d holeCut=%d noVPhysics=%d",
	    polyhedra, worldOut, propOut, propEmptyGroup, wallUncut, wallCutByHole, noVPhysics );
	std::printf( "coverage: %s\n", detail );
	Check( worldOut > nScenarios / 2 && propOut > 10 && propEmptyGroup > 5 && wallCutByHole > 50 &&
	           wallUncut > 50 && noVPhysics > 5,
	    "scenarios cover world, static-prop, empty-group, uncut-wall, hole-cut and tube-only paths",
	    detail );
	for ( int m = MUTATE_NONE + 1; m < MUTATE_COUNT; ++m )
	{
		std::snprintf( detail, sizeof( detail ), "%s: detected in %d/%d scenarios",
		    MutationName( m ), detected[m], nScenarios );
		std::printf( "sensitivity: %s\n", detail );
		Check( detected[m] > 0, "comparator rejects deliberate defect", detail );
	}
	if ( benchmark )
		std::printf( "carve time: legacy %.3f ms, pooled(3 workers) %.3f ms over %d scenarios\n",
		    tLegacy * 1000.0, tPooled * 1000.0, nScenarios );

	for ( int i = 0; i < 3; ++i )
	{
		Check( pools[i]->Stop(), "pool stop", "" );
		DestroyThreadPool( pools[i] );
	}
	std::printf( "%d checks, %d failures\n", s_checks, s_failures );
	return s_failures ? 1 : 0;
}
