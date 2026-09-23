//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0008 WMSH version 1 payload layout.
//
// This header describes bytes; readers must decode little-endian fields and
// must not cast untrusted file storage to C++ structs. Version 1 is the first
// World Stage mesh slice. It carries triangle geometry, material batches,
// bounded meshlets, BSP face identity and leaf-to-meshlet references. A BSP2
// reader does not imply that its renderer consumes this payload.
//
// Header, 128 bytes:
//   0 magic "WMSH"; 4 version 1; 8 header size 128; 12 flags 0
//  16 vertex count; 20 index count; 24 triangle count; 28 batch count
//  32 meshlet count; 36 leaf count; 40 leaf reference count
//  44 material count; 48 material byte count; 52 reserved 0
//  56 vertex offset; 64 index offset; 72 triangle-face offset
//  80 batch offset; 88 meshlet offset; 96 leaf-range offset
// 104 leaf-reference offset; 112 material offset; 120 total size
// All counts are uint32; offsets and total size are uint64. Sections are
// 16-byte aligned and appear in the order above without overlap.
//
// Vertex, 40 bytes: position float3; octahedral normal snorm16x2;
// octahedral tangent snorm16x2; tangent handedness int8, three zero pad bytes;
// material UV float2; lightmap atlas UV float2.
// Index and triangle-to-BSP-face records are uint32. Each triangle has three
// indices and one face record. A meshlet's indices refer to its contiguous
// vertex span; version 1 uses deindexed corners to keep direct uploads simple.
//
// Batch, 24 bytes: material index, first index, index count, first meshlet,
// meshlet count, reserved 0 (all uint32). Batches are sorted by material path.
// Meshlet, 48 bytes: first index, index count, first vertex, vertex count
// (uint32); bounding sphere center float3 and radius float; normal cone axis
// float3 and minimum dot cutoff float. Limits: 64 vertices, 126 triangles.
// Leaf range, 8 bytes: first reference and reference count (uint32).
// Leaf references are uint32 meshlet indices. Materials are consecutive
// uint32 byte length + UTF-8 bytes, padded to four bytes each. Paths are
// sorted, unique, relative and slash-separated; they omit the .vmt suffix,
// NUL, control bytes and dot/dot-dot segments.
//
//=============================================================================//

#ifndef MAPCONTAINER_WORLD_MESH_FORMAT_H
#define MAPCONTAINER_WORLD_MESH_FORMAT_H

#include <cstdint>

namespace mapcontainer
{

static const uint32_t kLumpWorldMesh = 0x48534D57u; // "WMSH"
static const uint32_t kWorldMeshVersion = 1;
static const uint32_t kWorldMeshHeaderSize = 128;
static const uint32_t kWorldMeshVertexSize = 40;
static const uint32_t kWorldMeshBatchSize = 24;
static const uint32_t kWorldMeshMeshletSize = 48;
static const uint32_t kWorldMeshLeafRangeSize = 8;
static const uint32_t kWorldMeshMaxVerticesPerMeshlet = 64;
static const uint32_t kWorldMeshMaxTrianglesPerMeshlet = 126;

} // namespace mapcontainer

#endif // MAPCONTAINER_WORLD_MESH_FORMAT_H
