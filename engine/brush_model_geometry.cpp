//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Brush and studio model geometry the server and the client both read
//			(RFC 0001 R12): brush model planes, surface centroids and a studio
//			model's illumination point. Moved out of gl_rsurf.cpp and
//			l_studio.cpp, which the dedicated product does not build.
//
//=============================================================================//

#include "quakedef.h"
#include "gl_model_private.h"
#include "gl_rsurf.h"
#include "l_studio.h"
#include "host.h"
#include "studio.h"
#include "iclientrenderable.h"
#include "mathlib/mathlib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns planes in brush models
//-----------------------------------------------------------------------------
int R_GetBrushModelPlaneCount( const model_t *model )
{
	return model->brush.nummodelsurfaces;
}

const cplane_t &R_GetBrushModelPlane( const model_t *model, int nIndex, Vector *pOrigin )
{
	SurfaceHandle_t surfID =
	    SurfaceHandleFromIndex( model->brush.firstmodelsurface, model->brush.pShared );
	surfID += nIndex;
	Assert( !( MSurf_Flags( surfID ) & SURFDRAW_NODRAW ) );

	if ( pOrigin )
	{
		int vertCount = MSurf_VertCount( surfID );
		if ( vertCount > 0 )
		{
			int nFirstVertex = model->brush.pShared->vertindices[MSurf_FirstVertIndex( surfID )];
			*pOrigin = model->brush.pShared->vertexes[nFirstVertex].position;
		}
		else
		{
			const cplane_t &plane = MSurf_Plane( surfID );
			VectorMultiply( plane.normal, plane.dist, *pOrigin );
		}
	}

	return MSurf_Plane( surfID );
}

//-----------------------------------------------------------------------------
// Computes the centroid of a surface
//-----------------------------------------------------------------------------
void Surf_ComputeCentroid( SurfaceHandle_t surfID, Vector *pVecCentroid )
{
	int nCount = MSurf_VertCount( surfID );
	int nFirstVertIndex = MSurf_FirstVertIndex( surfID );

	float flTotalArea = 0.0f;
	Vector vecNormal;
	pVecCentroid->Init( 0, 0, 0 );
	int vertIndex = host_state.worldbrush->vertindices[nFirstVertIndex];
	Vector vecApex = host_state.worldbrush->vertexes[vertIndex].position;
	for ( int v = 1; v < nCount - 1; ++v )
	{
		vertIndex = host_state.worldbrush->vertindices[nFirstVertIndex + v];
		Vector v1 = host_state.worldbrush->vertexes[vertIndex].position;
		vertIndex = host_state.worldbrush->vertindices[nFirstVertIndex + v + 1];
		Vector v2 = host_state.worldbrush->vertexes[vertIndex].position;
		CrossProduct( v2 - v1, v1 - vecApex, vecNormal );
		float flArea = vecNormal.Length();
		flTotalArea += flArea;
		*pVecCentroid += ( vecApex + v1 + v2 ) * flArea / 3.0f;
	}

	if ( flTotalArea )
	{
		*pVecCentroid /= flTotalArea;
	}
}

//-----------------------------------------------------------------------------
// A studio model's illumination point
//-----------------------------------------------------------------------------
void R_ComputeLightingOrigin( IClientRenderable *pRenderable, studiohdr_t *pStudioHdr,
    const matrix3x4_t &matrix, Vector &center )
{
	int nAttachmentIndex = pStudioHdr->IllumPositionAttachmentIndex();
	if ( nAttachmentIndex <= 0 )
	{
		VectorTransform( pStudioHdr->illumposition, matrix, center );
	}
	else
	{
		matrix3x4_t attachment;
		pRenderable->GetAttachment( nAttachmentIndex, attachment );
		VectorTransform( pStudioHdr->illumposition, attachment, center );
	}
}
