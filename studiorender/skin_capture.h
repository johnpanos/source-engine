//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: skinning corpus capture for RFC 0016 K6 (render.skinning.corpus).
//
//			Off unless the environment names a file in SOURCE_SKIN_CAPTURE.
//			Then every mesh the software skinning path processes is appended:
//			its pose-to-world palette, and per vertex the skinning input (the
//			flexed position, normal and tangent when the vertex is flexed),
//			its bone weights, and the legacy output. The GPU kernel is judged
//			against that output. SOURCE_SKIN_CAPTURE_MESHES caps the number
//			of meshes (default 4000); the file is closed when the cap is hit.
//
//			Format (little-endian): "SKCAP001", then per mesh "MESH", uint32
//			vertex count, uint32 bone count, bone count x 12 floats (3x4
//			row-major), then per vertex: in position[3], normal[3],
//			tangent[4], uint8 numbones, uint8 bone[3], float weight[3],
//			uint32 flags (1 flexed, 2 has tangent), out position[3],
//			normal[3], tangent[4].
//
//=============================================================================//

#ifndef STUDIORENDER_SKIN_CAPTURE_H
#define STUDIORENDER_SKIN_CAPTURE_H

#include "mathlib/mathlib.h"
#include "studio.h"

class CSkinCapture
{
public:
	// The sink, or NULL when capture is off (checked once).
	static CSkinCapture *Get();

	void BeginMesh( int numVertices, int numBones, const matrix3x4_t *pPoseToWorld );
	void Vertex( const Vector &inPos, const Vector &inNormal, const Vector4D *pInTangent,
	    const mstudioboneweight_t &weights, bool bFlexed, const Vector &outPos,
	    const Vector &outNormal, const Vector4D &outTangent );
	void EndMesh();

private:
	CSkinCapture( FILE *pFile, int nMaxMeshes ) : m_pFile( pFile ), m_nMaxMeshes( nMaxMeshes ) {}
	FILE *m_pFile;
	int m_nMaxMeshes;
	int m_nMeshes = 0;
	int m_nPending = 0; // vertices still owed by the open mesh
	bool m_bOpen = false;
};

#endif // STUDIORENDER_SKIN_CAPTURE_H
