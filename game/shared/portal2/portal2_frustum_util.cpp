//========= Portal 2 port ======================================================//
//
// Purpose: See portal2_frustum_util.h. The algorithm is the CS:GO base game's
//          (game/shared/util_shared.cpp); its debug-overlay tracing is omitted.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_frustum_util.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int UTIL_CalcFrustumThroughConvexPolygon( const Vector *pPolyVertices, int iPolyVertCount, const Vector &vFrustumOrigin, const VPlane *pInputFrustumPlanes, int iInputFrustumPlanes, VPlane *pOutputFrustumPlanes, int iMaxOutputPlanes, int iPreserveCount )
{
	Assert( iPreserveCount <= iMaxOutputPlanes );
	Assert( iPreserveCount <= iInputFrustumPlanes );
	if( iPolyVertCount < 3 )
		return 0;


	int iMaxComplexity = iMaxOutputPlanes - iPreserveCount;
	
	Vector *pClippedVerts;
	int iClippedVertCount;
	if( iInputFrustumPlanes > 0 )
	{
		//clip the polygon by the input frustum
		int iAllocSize = iPolyVertCount + iInputFrustumPlanes;

		Vector *pWorkVerts[2];
		pWorkVerts[0] = (Vector *)stackalloc( sizeof( Vector ) * iAllocSize * 2 ); //possible to add 1 point per cut, iPolyVertCount starting points, iInputFrustumPlaneCount cuts
		pWorkVerts[1] = pWorkVerts[0] + iAllocSize;

		//clip by first plane and put output into pInVerts
		iClippedVertCount = ClipPolyToPlane( (Vector *)pPolyVertices, iPolyVertCount, pWorkVerts[0], pInputFrustumPlanes[0].m_Normal, pInputFrustumPlanes[0].m_Dist, 0.01f );

		//clip by other planes and flipflop in and out pointers
		for( int i = 1; i != iInputFrustumPlanes; ++i )
		{
			if( iClippedVertCount < 3 )
				return 0; //nothing left in the frustum

			iClippedVertCount = ClipPolyToPlane( pWorkVerts[(i & 1) ^ 1], iClippedVertCount, pWorkVerts[i & 1], pInputFrustumPlanes[i].m_Normal, pInputFrustumPlanes[i].m_Dist, 0.01f );
		}

		if( iClippedVertCount < 3 )
			return 0; //nothing left in the frustum

		pClippedVerts = pWorkVerts[(iInputFrustumPlanes & 1) ^ 1];
	}
	else
	{
		//no input frustum
		if( iPolyVertCount > iMaxComplexity )
		{
			//we'll need to reduce our output frustum, copy the input polygon
			pClippedVerts = (Vector *)stackalloc( sizeof( Vector ) * iPolyVertCount );
			memcpy( pClippedVerts, pPolyVertices, sizeof( Vector ) * iPolyVertCount );
		}
		else
		{
			//we won't need to simplify the polygon to reduce output planes, just point at the input polygon
			pClippedVerts = (Vector *)pPolyVertices;
		}
		iClippedVertCount = iPolyVertCount;
	}


	Assert( iClippedVertCount <= (iPolyVertCount + iInputFrustumPlanes) );

	if( iClippedVertCount > iMaxComplexity )
	{
		float *fLineLengthSqr = (float *)stackalloc( sizeof( float ) * iClippedVertCount );

		for( int i = 0; i != (iClippedVertCount - 1); ++i )
		{
			fLineLengthSqr[i] = (pClippedVerts[i + 1] - pClippedVerts[i]).LengthSqr();
		}
		fLineLengthSqr[(iClippedVertCount - 1)] = (pClippedVerts[0] - pClippedVerts[(iClippedVertCount - 1)]).LengthSqr(); //wrap around


		while( iClippedVertCount > iMaxComplexity ) //vert count == number of planes we need to bound the polygon
		{
			//we have too many verts to represent this accurately in the output frustum plane count
			//so, we're going to eliminate the smallest sides one at a time and bridge the surrounding sides until we're down to iMaxComplexity
			float fMinSide = fLineLengthSqr[0];
			int iMinSideFirstPoint = 0;
			int iOldVertCount = iClippedVertCount;
			--iClippedVertCount; //we're going to decrement this sometime in this block, it makes math easier to do it now

			for( int i = 1; i != iOldVertCount; ++i )
			{
				if( fLineLengthSqr[i] < fMinSide )
				{
					fMinSide = fLineLengthSqr[i];
					iMinSideFirstPoint = i;
				}
			}

			int i1, i2, i3, i4;
			i1 = (iMinSideFirstPoint + iClippedVertCount)%(iOldVertCount); //-1 with a wrap
			i2 = iMinSideFirstPoint;
			i3 = (iMinSideFirstPoint + 1)%(iOldVertCount);
			i4 = (iMinSideFirstPoint + 2)%(iOldVertCount);

			Vector *p1, *p2, *p3, *p4;
			p1 = &pClippedVerts[i1];
			p2 = &pClippedVerts[i2];
			p3 = &pClippedVerts[i3]; //this is the one we'll actually be dropping in the merge
			p4 = &pClippedVerts[i4];


			//now we know the two points that we have to merge to one, project and make a merged point from the surrounding lines
			//if( fMinSide >= 0.1f ) //only worth doing the math if it's actually going to be accurate and make a difference
			{
				//http://mathworld.wolfram.com/Line-LineIntersection.html (20)
				Vector vA = *p2 - *p1;
				Vector vB = *p4 - *p3;
				Vector vC = *p3 - *p1;
				Vector vCxB = vC.Cross( vB );
				Vector vAxB = vA.Cross( vB );
				float fS = vCxB.Dot(vAxB)/vAxB.LengthSqr();

				*p2 = *p1 + (vA * fS);

				fLineLengthSqr[i1] = (*p2 - *p1).LengthSqr();
			}
			
			fLineLengthSqr[i2] = (*p4 - *p2).LengthSqr(); //must do this BEFORE possibly shifting points p4+ left

			if( i3 < i4 ) //not the last point in the array
			{
				int iElementShift = (iOldVertCount - i4);

				//eliminate p3, we merged p2+p3 and already stored the result in p2
				memmove( p3, p4, sizeof( Vector ) * iElementShift );
				memmove( &fLineLengthSqr[i3], &fLineLengthSqr[i4], sizeof( float ) * iElementShift );
			}
		}

	}

	//generate planes defined by each line around the convex and the frustum origin
	{
		int iFlipNormalsXOR = 0; //this algorithm was written assuming polygon vertices would be in a clockwise order from the perspective of vFrustumOrigin, some logic needs to flip if the inverse is true
		{
			Vector vLine1 = pPolyVertices[1] - pPolyVertices[0];
			Vector vLine2 = pPolyVertices[2] - pPolyVertices[1];
			Vector vFrontFace = vLine2.Cross( vLine1 );

			iFlipNormalsXOR = (vFrontFace.Dot( vFrustumOrigin - pPolyVertices[0] ) < 0.0f) ? 1 : 0; //this will assist in reversing the normal by flipping the cross product
		}		

		Vector vTemp[2];
		vTemp[0] = pClippedVerts[iClippedVertCount - 1] - vFrustumOrigin;
		for( int i = 0; i != iClippedVertCount; ++i )
		{
			int iIndexing = i & 1; //we can carry over the line computation from one iteration to the next, flip which order we look at the temps with
			vTemp[iIndexing ^ 1] = pClippedVerts[i] - vFrustumOrigin; 

			Vector vNormal = vTemp[iIndexing ^ iFlipNormalsXOR].Cross( vTemp[(iIndexing ^ iFlipNormalsXOR) ^ 1] ); //vLine1.Cross( vLine2 );
			vNormal.NormalizeInPlace();

			pOutputFrustumPlanes[i].Init( vNormal, vNormal.Dot( vFrustumOrigin ) );
		}
	}

	//preserve input planes on request
	if( iPreserveCount > 0 )
	{
		memcpy( &pOutputFrustumPlanes[iClippedVertCount], &pInputFrustumPlanes[iInputFrustumPlanes - iPreserveCount], sizeof( VPlane ) * iPreserveCount );
	}

	return (iClippedVertCount + iPreserveCount);
}
