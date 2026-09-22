//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal placement carving: clipping world/wall brush and static-prop
//          polyhedrons against the portal planes. Shared by the portal
//          simulator (client and server) and its conformance suite.
//
//=============================================================================//
#ifndef PORTAL_CARVE_H
#define PORTAL_CARVE_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/polyhedron.h"
#include "tier1/utlvector.h"
#include "vstdlib/jobgraph_parallel.h"
#include "portal_shareddefs.h"

#define PORTAL_WALL_FARDIST 200.0f
#define PORTAL_WALL_TUBE_DEPTH 1.0f
#define PORTAL_WALL_TUBE_OFFSET 0.01f
#define PORTAL_WALL_MIN_THICKNESS 0.1f
#define PORTAL_POLYHEDRON_CUT_EPSILON (1.0f/1099511627776.0f) //    1 / (1<<40)
#define PORTAL_WORLD_WALL_HALF_SEPARATION_AMOUNT 0.1f //separating the world collision from wall collision by a small amount gets rid of extremely thin erroneous collision at the separating plane

#define PORTAL_HOLE_HALF_HEIGHT (PORTAL_HALF_HEIGHT + 0.1f)
#define PORTAL_HOLE_HALF_WIDTH (PORTAL_HALF_WIDTH + 0.1f)

// Every clip is independent: it reads one immutable input polyhedron and the
// shared plane array, and returns a new heap polyhedron (never the shared
// temporary polyhedron). Results are appended in input order on the calling
// thread, so the produced lists are identical in every mode. Serial graph and
// pooled graph modes borrow the inputs, planes and output slots until return.
namespace PortalCarve
{

enum Mode_t
{
	MODE_LEGACY = 0,		// the original in-place loop
	MODE_SERIAL_GRAPH = 1,	// deterministic job graph, ascending order
	MODE_POOLED_GRAPH = 2,	// job graph on the engine pool
};

struct ClipItem_t
{
	const CPolyhedron *pInput;
	CPolyhedron *pResult;
};

struct ClipContext_t
{
	const float *pPlanes;
	int iPlaneCount;
	float fEpsilon;

	void Process( ClipItem_t &item )
	{
		item.pResult = ClipPolyhedron( item.pInput, pPlanes, iPlaneCount, fEpsilon, false );
	}
};

// Writes ClipPolyhedron( pInputs[i] ) (NULL inputs and fully clipped inputs
// yield NULL) to pResults[i]. False means the batch could not be constructed
// and nothing was clipped.
inline bool ClipBatch( const CPolyhedron *const *pInputs, int iCount, const float *pPlanes,
	int iPlaneCount, float fEpsilon, CPolyhedron **pResults, int iMode, IThreadPool *pPool )
{
	if ( iCount <= 0 )
		return true;
	if ( iMode == MODE_LEGACY )
	{
		for ( int i = 0; i != iCount; ++i )
			pResults[i] = ClipPolyhedron( pInputs[i], pPlanes, iPlaneCount, fEpsilon, false );
		return true;
	}
	CUtlVector<ClipItem_t> items;
	items.SetCount( iCount );
	for ( int i = 0; i != iCount; ++i )
	{
		items[i].pInput = pInputs[i];
		items[i].pResult = NULL;
	}
	ClipContext_t context = { pPlanes, iPlaneCount, fEpsilon };
	const jobsystem::BatchMode mode =
		iMode == MODE_SERIAL_GRAPH ? jobsystem::BatchMode::Serial : jobsystem::BatchMode::Parallel;
	if ( !JobGraphParallelProcess( "PortalCarve::ClipBatch", items.Base(), unsigned( iCount ),
			 &context, &ClipContext_t::Process,
			 static_cast<void ( ClipContext_t::* )()>( NULL ),
			 static_cast<void ( ClipContext_t::* )()>( NULL ), INT_MAX, pPool, mode ) )
		return false;
	for ( int i = 0; i != iCount; ++i )
		pResults[i] = items[i].pResult;
	return true;
}

// Appends every non-NULL clip result to pOut, in input order.
inline bool ClipInOrder( const CPolyhedron *const *pInputs, int iCount, const float *pPlanes,
	int iPlaneCount, float fEpsilon, CUtlVector<CPolyhedron *> *pOut, int iMode,
	IThreadPool *pPool )
{
	if ( pOut == NULL || pInputs == NULL || iCount <= 0 )
		return true;
	CUtlVector<CPolyhedron *> results;
	results.SetCount( iCount );
	if ( !ClipBatch( pInputs, iCount, pPlanes, iPlaneCount, fEpsilon, results.Base(), iMode,
			 pPool ) )
		return false;
	for ( int i = 0; i != iCount; ++i )
	{
		if ( results[i] )
			pOut->AddToTail( results[i] );
	}
	return true;
}

// Static props: pPieces holds each group's (prop's) pieces consecutively;
// pGroupSizes[g] pieces belong to group g. Appends each group's non-NULL clips
// in order and stores the number appended in pAppended[g].
inline bool ClipGroupsInOrder( const CPolyhedron *const *pPieces, const int *pGroupSizes,
	int iGroupCount, const float *pPlanes, int iPlaneCount, float fEpsilon,
	CUtlVector<CPolyhedron *> *pOut, int *pAppended, int iMode, IThreadPool *pPool )
{
	int iPieceCount = 0;
	for ( int g = 0; g != iGroupCount; ++g )
		iPieceCount += pGroupSizes[g];
	CUtlVector<CPolyhedron *> results;
	results.SetCount( iPieceCount );
	if ( !ClipBatch( pPieces, iPieceCount, pPlanes, iPlaneCount, fEpsilon, results.Base(), iMode,
			 pPool ) )
		return false;
	for ( int g = 0, iPiece = 0; g != iGroupCount; ++g )
	{
		pAppended[g] = 0;
		for ( int j = 0; j != pGroupSizes[g]; ++j, ++iPiece )
		{
			if ( results[iPiece] )
			{
				pOut->AddToTail( results[iPiece] );
				++pAppended[g];
			}
		}
	}
	return true;
}

struct Placement_t
{
	Vector ptCenter;
	Vector vForward;
	Vector vRight;
	Vector vUp;
};

// World-space bounds for gathering the wall brushes around a placement.
inline void ComputeWallBrushBounds( const Placement_t &placement, Vector *pMins, Vector *pMaxs )
{
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
	*pMins = vAABBMins;
	*pMaxs = vAABBMaxs;
}

// The "(Holy) Wall": the wall around the portal hole plus the thin tube lining
// it. pWallBrushes are the polyhedrons of the brushes inside the wall bounds
// (NULL entries allowed), consulted only when simulating vphysics. Appends to
// TubeOut and BrushesOut. False means a clip batch could not be constructed.
inline bool CarveWall( const Placement_t &placement, const float fWallClipPlane_Forward[4],
	bool bSimulatingVPhysics, const CPolyhedron *const *pWallBrushes, int iWallBrushCount,
	CUtlVector<CPolyhedron *> &TubeOut, CUtlVector<CPolyhedron *> &BrushesOut, int iMode,
	IThreadPool *pPool )
{
	Vector vBackward = -placement.vForward;
	Vector vLeft = -placement.vRight;
	Vector vDown = -placement.vUp;

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


	CUtlVector<CPolyhedron *> WallBrushPolyhedrons_ClippedToWall;
	CPolyhedron **pWallClippedPolyhedrons = NULL;
	int iWallClippedPolyhedronCount = 0;
	bool bOk = true;
	if( bSimulatingVPhysics ) //if not simulating vphysics, we skip making the entire wall, and just create the minimal tube instead
	{
		if( iWallBrushCount != 0 )
			bOk = ClipInOrder( pWallBrushes, iWallBrushCount, fPlanes, 1, PORTAL_POLYHEDRON_CUT_EPSILON, &WallBrushPolyhedrons_ClippedToWall, iMode, pPool );

		if( bOk && WallBrushPolyhedrons_ClippedToWall.Count() != 0 )
		{
			// Uses the shared temporary polyhedron; stays on this thread.
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
	if( bOk )
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

		bOk = ClipInOrder( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut, iMode, pPool );
	}

	//lower wall
	if( bOk )
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

		bOk = ClipInOrder( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut, iMode, pPool );
	}

	//left wall
	if( bOk )
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

		bOk = ClipInOrder( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut, iMode, pPool );
	}

	//right wall
	if( bOk )
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

		bOk = ClipInOrder( pWallClippedPolyhedrons, iWallClippedPolyhedronCount, fSidePlanesOnly, 4, PORTAL_POLYHEDRON_CUT_EPSILON, &BrushesOut, iMode, pPool );
	}

	for( int i = WallBrushPolyhedrons_ClippedToWall.Count(); --i >= 0; )
		WallBrushPolyhedrons_ClippedToWall[i]->Release();

	WallBrushPolyhedrons_ClippedToWall.RemoveAll();
	return bOk;
}

} // namespace PortalCarve

#endif // PORTAL_CARVE_H
