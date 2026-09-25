//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native Vulkan backend's mesh vertex record: the one owner of
//          where each vertex component lives, for a mesh the backend locks
//          and for a buffer the queued material system fills on the main
//          thread (IShaderAPI::ComputeVertexDescription).
//
//          The queued material system builds a dynamic mesh into its own
//          memory, described by ComputeVertexDescription, and on the render
//          thread copies those bytes verbatim into the mesh the backend
//          returns from GetDynamicMesh (CMatQueuedMesh::ExecuteDefferredBuild):
//          it locks that mesh and copies from its lowest component pointer.
//          Both sides therefore describe a format through this file, so the
//          same bytes mean the same components.
//
//          A record has a fixed base layout. Formats with texture coordinates
//          wider than two floats, or more than three sets (SpriteCard), carry
//          all eight sets inline after the base, four floats each. Components
//          a format lacks point at caller scratch with size 0, as D3D9's do,
//          and receive their identity when the mesh is unlocked
//          (FillAbsentMeshComponents): white color, weight 1 on bone 0, zero
//          elsewhere. The shaders multiply by color and weights, so these
//          defaults must be the identity.
//
//===========================================================================//

#ifndef VULKAN_MESH_LAYOUT_H
#define VULKAN_MESH_LAYOUT_H

#include "materialsystem/imesh.h"

namespace render_vulkan
{

enum MeshRecordOffset
{
	kMeshPositionOffset = 0,    // float3
	kMeshColorOffset = 12,      // 4 bytes, D3DCOLOR (BGRA)
	kMeshTexCoord0Offset = 16,  // float2, base UV
	kMeshTexCoord1Offset = 24,  // float2, lightmap UV
	kMeshBoneWeightOffset = 32, // float2
	kMeshBoneIndexOffset = 40,  // 4 bytes
	kMeshNormalOffset = 44,     // float3
	kMeshUserDataOffset = 56,   // float4, the TANGENT stream (binormal sign in w)
	kMeshTangentSOffset = 72,   // float3
	kMeshTangentTOffset = 84,   // float3
	kMeshTexCoord2Offset = 96,  // float2, a bumped brush's lightmap page offset
	kMeshBaseStride = 104,
	// Wide formats: eight texture coordinate sets of four floats after the base.
	kMeshWideTexCoordOffset = kMeshBaseStride,
	kMeshWideTexCoordFloats = VERTEX_MAX_TEXTURE_COORDINATES * 4,
	kMeshWideStride = kMeshBaseStride + kMeshWideTexCoordFloats * 4,
	// A static-prop color stream: one tightly packed D3DCOLOR per vertex.
	kMeshColorStreamStride = 4,
	// Bone weights a record holds; the mesh builder writes N-1 of N weights.
	kMeshRecordBoneWeights = 2,
	// Bytes of scratch a caller supplies for components a format lacks. A whole
	// record: a zero-vertex lock still describes vertex 0, possibly into it.
	kMeshScratchBytes = kMeshWideStride,
};

// A static-prop color mesh (CColorMeshData): exactly VERTEX_SPECULAR.
bool MeshFormatIsColorStream( VertexFormat_t format );

// Texture coordinates wider than two floats, or any set beyond the three the
// base record carries.
bool MeshFormatIsWide( VertexFormat_t format );

// Bytes per vertex a format's records occupy.
int MeshRecordStride( VertexFormat_t format );

// Points desc's components at the record array starting at `records`.
// Components the format lacks point at `scratch` (kMeshScratchBytes, writable)
// with size 0. Format 0 means unknown: every base component is present, as
// for a mesh created without a format. m_ActualVertexSize is the stride.
void DescribeMeshRecords(
    unsigned char *records, VertexFormat_t format, unsigned char *scratch, VertexDesc_t &desc );

// Writes the identity of every base component into `count` records, for a
// mesh whose format is unknown (the builder may write any subset).
void SeedMeshRecordDefaults( unsigned char *records, int count );

// Writes the identity of the components `format` lacks into `count` records,
// and completes a wide format's records (unused floats of each set zero, the
// first three sets' xy copied into the base slots). A format of 0 or a color
// stream is left as written.
void FillAbsentMeshComponents( unsigned char *records, int count, VertexFormat_t format );

} // namespace render_vulkan

#endif // VULKAN_MESH_LAYOUT_H
