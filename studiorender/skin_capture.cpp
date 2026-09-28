//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: skinning corpus capture (RFC 0016 K6); see skin_capture.h.
//
//=============================================================================//

#include "skin_capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CSkinCapture *CSkinCapture::Get()
{
	static CSkinCapture *s_pCapture = []() -> CSkinCapture *
	{
		const char *pPath = getenv( "SOURCE_SKIN_CAPTURE" );
		if ( !pPath || !*pPath )
			return NULL;
		FILE *pFile = fopen( pPath, "wb" );
		if ( !pFile )
		{
			Warning( "skin capture: cannot open %s\n", pPath );
			return NULL;
		}
		fwrite( "SKCAP001", 1, 8, pFile );
		const char *pMax = getenv( "SOURCE_SKIN_CAPTURE_MESHES" );
		const int nMax = pMax ? atoi( pMax ) : 4000;
		Msg( "skin capture: writing up to %d meshes to %s\n", nMax, pPath );
		return new CSkinCapture( pFile, nMax > 0 ? nMax : 4000 );
	}();
	return ( s_pCapture && s_pCapture->m_pFile ) ? s_pCapture : NULL;
}

void CSkinCapture::BeginMesh( int numVertices, int numBones, const matrix3x4_t *pPoseToWorld )
{
	if ( !m_pFile || numVertices <= 0 || numBones <= 0 )
		return;
	const uint32 header[2] = { (uint32)numVertices, (uint32)numBones };
	fwrite( "MESH", 1, 4, m_pFile );
	fwrite( header, sizeof( header ), 1, m_pFile );
	for ( int b = 0; b < numBones; ++b )
		fwrite( pPoseToWorld[b].Base(), sizeof( float ), 12, m_pFile );
	m_nPending = numVertices;
	m_bOpen = true;
}

void CSkinCapture::Vertex( const Vector &inPos, const Vector &inNormal, const Vector4D *pInTangent,
    const mstudioboneweight_t &weights, bool bFlexed, const Vector &outPos, const Vector &outNormal,
    const Vector4D &outTangent )
{
	if ( !m_bOpen || m_nPending <= 0 )
		return;
	float in[10] = { inPos.x, inPos.y, inPos.z, inNormal.x, inNormal.y, inNormal.z, 0, 0, 0, 0 };
	if ( pInTangent )
	{
		in[6] = pInTangent->x;
		in[7] = pInTangent->y;
		in[8] = pInTangent->z;
		in[9] = pInTangent->w;
	}
	const uint8 bones[4] = { (uint8)weights.numbones, (uint8)weights.bone[0],
	    (uint8)weights.bone[1], (uint8)weights.bone[2] };
	const uint32 flags = ( bFlexed ? 1u : 0u ) | ( pInTangent ? 2u : 0u );
	const float out[10] = { outPos.x, outPos.y, outPos.z, outNormal.x, outNormal.y, outNormal.z,
	    outTangent.x, outTangent.y, outTangent.z, outTangent.w };
	fwrite( in, sizeof( in ), 1, m_pFile );
	fwrite( bones, sizeof( bones ), 1, m_pFile );
	fwrite( weights.weight, sizeof( float ), 3, m_pFile );
	fwrite( &flags, sizeof( flags ), 1, m_pFile );
	fwrite( out, sizeof( out ), 1, m_pFile );
	--m_nPending;
}

void CSkinCapture::EndMesh()
{
	if ( !m_bOpen )
		return;
	m_bOpen = false;
	if ( m_nPending != 0 )
		Warning( "skin capture: a mesh ended %d vertices short\n", m_nPending );
	if ( ++m_nMeshes >= m_nMaxMeshes )
	{
		fclose( m_pFile );
		m_pFile = NULL;
		Msg( "skin capture: %d meshes written\n", m_nMeshes );
	}
	else
	{
		fflush( m_pFile );
	}
}
