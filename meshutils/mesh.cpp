//=========== Copyright © Valve Corporation, All rights reserved. ============//
//
// Purpose: Mesh class operations.
//
//===========================================================================//

#include "mesh.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlhash.h"

// simplest mesh type - array of vec3
static CMeshVertexAttribute g_PositionAttribute = {0,VERTEX_ELEMENT_POSITION};

// CMesh - utility mesh class implementation
CMesh::CMesh() : 
	m_pVerts(NULL), m_pAttributes(NULL), m_pIndices(NULL), m_nVertexCount(0), m_nVertexStrideFloats(0),
	m_nAttributeCount(0), m_nIndexCount(0), m_bAllocatedMeshData(false)
{
}

CMesh::~CMesh()
{
	FreeAllMemory();
}


// free anything we allocated
void CMesh::FreeAllMemory()
{
	if ( m_bAllocatedMeshData )
	{
		delete[] m_pVerts;
		delete[] m_pAttributes;
		delete[] m_pIndices;
	}
	m_pVerts = NULL;
	m_pAttributes = NULL;
	m_pIndices = NULL;
	m_bAllocatedMeshData = false;
	m_nAttributeCount = 0;
	m_nVertexStrideFloats = 0;
	m_nVertexCount = 0;
	m_nIndexCount = 0;
}

void CMesh::AllocateMesh( int nVertexCount, int nIndexCount, int nVertexStride, CMeshVertexAttribute *pAttributes, int nAtrributeCount )
{
	FreeAllMemory();
	if ( !pAttributes )
	{
		pAttributes = &g_PositionAttribute;
		nAtrributeCount = 1;
	}

	m_nVertexCount = nVertexCount;
	m_nVertexStrideFloats = nVertexStride;
	m_nIndexCount = nIndexCount;
	m_nAttributeCount = nAtrributeCount;

	// allocate the mesh data, mark as allocated so it will be freed on destruct
	m_bAllocatedMeshData = true;
	m_pVerts = new float[nVertexStride * nVertexCount];
	m_pIndices = new uint32[nIndexCount];
	m_pAttributes = new CMeshVertexAttribute[nAtrributeCount];

	for ( int i = 0; i < nAtrributeCount; i++ )
	{
		m_pAttributes[i] = pAttributes[i];
	}
}

void CMesh::AllocateAndCopyMesh( int nInputVertexCount, const float *pInputVerts, int nInputIndexCount, const uint32 *pInputIndices, int nVertexStride, CMeshVertexAttribute *pAttributes, int nAtrributeCount )
{
	AllocateMesh( nInputVertexCount, nInputIndexCount, nVertexStride, pAttributes, nAtrributeCount );
	V_memcpy( m_pVerts, pInputVerts, GetTotalVertexSizeInBytes() );
	V_memcpy( m_pIndices, pInputIndices, GetTotalIndexSizeInBytes() );
}


void CMesh::InitExternalMesh( float *pVerts, int nVertexCount, uint32 *pIndices, int nIndexCount, int nVertexStride, CMeshVertexAttribute *pAttributes, int nAtrributeCount )
{
	if ( !pAttributes )
	{
		pAttributes = &g_PositionAttribute;
		nAtrributeCount = 1;
	}
	FreeAllMemory();
	m_bAllocatedMeshData = false;
	m_nVertexCount = nVertexCount;
	m_nVertexStrideFloats = nVertexStride;
	m_nIndexCount = nIndexCount;
	m_nAttributeCount = nAtrributeCount;
	m_pVerts = pVerts;
	m_pIndices = pIndices;
	m_pAttributes = pAttributes;
}

void CMesh::AppendMesh( const CMesh &inputMesh )
{
	Assert( inputMesh.m_nAttributeCount == m_nAttributeCount );
	Assert( inputMesh.m_nVertexStrideFloats == m_nVertexStrideFloats );

	// Find total sizes
	int nTotalIndices = m_nIndexCount + inputMesh.m_nIndexCount;
	int nTotalVertices = m_nVertexCount + inputMesh.m_nVertexCount;

	float *pNewVB = new float[ nTotalVertices * m_nVertexStrideFloats ];
	uint32 *pNewIB = new uint32[ nTotalIndices ];

	Q_memcpy( pNewVB, m_pVerts, m_nVertexCount * m_nVertexStrideFloats * sizeof( float ) );
	Q_memcpy( pNewIB, m_pIndices, m_nIndexCount * sizeof( uint32 ) );

	int nCurrentIndex = m_nIndexCount;
	
	// copy vertices over
	Q_memcpy( pNewVB + m_nVertexCount * m_nVertexStrideFloats, 
			  inputMesh.m_pVerts, 
			  inputMesh.m_nVertexCount * m_nVertexStrideFloats * sizeof( float ) );

	for ( int i=0; i<inputMesh.m_nIndexCount; ++i )
	{
		pNewIB[ nCurrentIndex ] = inputMesh.m_pIndices[ i ] + m_nVertexCount;
		nCurrentIndex ++;
	}

	delete []m_pVerts;
	delete []m_pIndices;
	m_pVerts = pNewVB;
	m_pIndices = pNewIB;

	m_nVertexCount = nTotalVertices;
	m_nIndexCount = nTotalIndices;
}

bool CMesh::CalculateBounds( Vector *pMinOut, Vector *pMaxOut, int nStartVertex, int nVertexCount ) const
{
	Assert( pMinOut && pMaxOut );

	Vector vMin( FLT_MAX, FLT_MAX, FLT_MAX );
	Vector vMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	int nPosOffset = FindFirstAttributeOffset( VERTEX_ELEMENT_POSITION );
	if ( nPosOffset < 0 )
		return false;

	float *pPositions = ( m_pVerts + nStartVertex * m_nVertexStrideFloats + nPosOffset );

	if ( nVertexCount == 0 )
		nVertexCount = m_nVertexCount;

	for ( int v=0; v<nVertexCount; ++v )
	{
		vMin.x = MIN( pPositions[0], vMin.x );
		vMin.y = MIN( pPositions[1], vMin.y );
		vMin.z = MIN( pPositions[2], vMin.z );

		vMax.x = MAX( pPositions[0], vMax.x );
		vMax.y = MAX( pPositions[1], vMax.y );
		vMax.z = MAX( pPositions[2], vMax.z );

		pPositions += m_nVertexStrideFloats;
	}

	*pMinOut = vMin;
	*pMaxOut = vMax;
	return true;
}

int CMesh::FindFirstAttributeOffset( VertexElement_t nType ) const
{
	for ( int a=0; a<m_nAttributeCount; ++a )
	{
		if ( m_pAttributes[ a ].m_nType == nType )
		{
			return m_pAttributes[ a ].m_nOffsetFloats;
		}
	}

	return -1;
}

// Portal 2 port: this tree compiles only the CMesh storage, bounds and attribute functions
// above into the Portal 2 game modules (convexhull.cpp and the Portal 2 player
// use them). The rest of the CS:GO meshutils mesh.cpp (adjacency, tangents,
// render input layouts, welding, splitting, cloth data) needs the CS:GO render
// system and is not part of this build.
