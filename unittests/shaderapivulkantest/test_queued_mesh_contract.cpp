//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native Vulkan backend's queued-mesh contract (the queued
//          material system, mat_queue_mode 2). The main thread builds a
//          dynamic mesh into memory IShaderAPI::ComputeVertexDescription
//          describes; the render thread locks the backend's mesh and copies
//          those bytes verbatim from its lowest component pointer
//          (CMatQueuedMesh::ExecuteDefferredBuild, reproduced here). For every
//          vertex format this suite builds the same vertices directly into a
//          backend mesh and through that replay, and requires identical
//          records: same stride, same components, and the identity (white
//          color, weight 1 on bone 0, zero elsewhere) for the components a
//          format lacks, even when the replayed bytes there were garbage and
//          a builder wrote stray values.
//
//          Negative control: a D3D9-style packed description replayed into
//          the same mesh must be detected.
//
//          CPU only: no Vulkan device or display is needed.
//
//===========================================================================//

#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "render/legacy_shader_provider.h"
#include "shaderapi/IShaderDevice.h"
#include "shaderapi/ishaderapi.h"
#include "testing/conformance_result.h"
#include "vulkan_mesh_layout.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// Test seam exported by the backend (see shaderapivulkan.cpp).
extern "C" bool ShaderBackend_NativeMeshRecords(
    IMesh *pMesh, const unsigned char **ppRecords, int *pVertexCount, int *pStride );

namespace
{

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool ok, const std::string &what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", what.c_str() );
	}
}

const int kVertices = 5;

float Value( int v, int component, int k )
{
	return float( v * 100 + component * 10 + k ) + 0.25f;
}

template <typename T> T *At( void *base, int stride, int v )
{
	return reinterpret_cast<T *>( static_cast<unsigned char *>( base ) + size_t( v ) * stride );
}

// What a mesh builder writes: every component of `format`, through the
// description's pointers and strides. With `stray`, also a color and bone
// weights the format lacks, which a description must discard (D3D9 points
// them at scratch with size 0).
void WriteVertices( const VertexDesc_t &desc, VertexFormat_t format, int count, bool stray )
{
	for ( int v = 0; v < count; ++v )
	{
		if ( format & VERTEX_POSITION )
		{
			float *p = At<float>( desc.m_pPosition, desc.m_VertexSize_Position, v );
			for ( int k = 0; k < 3; ++k )
				p[k] = Value( v, 0, k );
		}
		if ( format & VERTEX_NORMAL )
		{
			float *n = At<float>( desc.m_pNormal, desc.m_VertexSize_Normal, v );
			for ( int k = 0; k < 3; ++k )
				n[k] = Value( v, 1, k );
		}
		if ( ( format & VERTEX_COLOR ) || stray )
		{
			unsigned char *c = At<unsigned char>( desc.m_pColor, desc.m_VertexSize_Color, v );
			for ( int k = 0; k < 4; ++k )
				c[k] = static_cast<unsigned char>( 16 * v + 3 * k + 1 );
		}
		if ( format & VERTEX_SPECULAR )
		{
			unsigned char *c = At<unsigned char>( desc.m_pSpecular, desc.m_VertexSize_Specular, v );
			for ( int k = 0; k < 4; ++k )
				c[k] = static_cast<unsigned char>( 16 * v + 5 * k + 2 );
		}
		const int weights = stray ? 2 : std::min( NumBoneWeights( format ), 2 );
		for ( int k = 0; k < weights; ++k )
			At<float>( desc.m_pBoneWeight, desc.m_VertexSize_BoneWeight, v )[k] =
			    0.125f * ( k + 1 );
		if ( format & VERTEX_BONE_INDEX )
		{
			unsigned char *b =
			    At<unsigned char>( desc.m_pBoneMatrixIndex, desc.m_VertexSize_BoneMatrixIndex, v );
			for ( int k = 0; k < 4; ++k )
				b[k] = static_cast<unsigned char>( v + k );
		}
		for ( int k = 0; k < UserDataSize( format ); ++k )
			At<float>( desc.m_pUserData, desc.m_VertexSize_UserData, v )[k] = Value( v, 2, k );
		if ( format & VERTEX_TANGENT_S )
			for ( int k = 0; k < 3; ++k )
				At<float>( desc.m_pTangentS, desc.m_VertexSize_TangentS, v )[k] = Value( v, 3, k );
		if ( format & VERTEX_TANGENT_T )
			for ( int k = 0; k < 3; ++k )
				At<float>( desc.m_pTangentT, desc.m_VertexSize_TangentT, v )[k] = Value( v, 4, k );
		for ( int set = 0; set < VERTEX_MAX_TEXTURE_COORDINATES; ++set )
		{
			const int size = TexCoordSize( set, format );
			for ( int k = 0; k < size; ++k )
				At<float>( desc.m_pTexCoord[set], desc.m_VertexSize_TexCoord[set], v )[k] =
				    Value( v, 5 + set, k );
		}
		if ( format & VERTEX_WRINKLE )
			*At<float>( desc.m_pWrinkle, desc.m_VertexSize_Wrinkle, v ) = Value( v, 13, 0 );
	}
}

// CMatQueuedMesh::ExecuteDefferredBuild: lock the backend's mesh and copy the
// main-thread bytes from its lowest component pointer.
void Replay( IMesh *mesh, const std::vector<unsigned char> &bytes, int count )
{
	MeshDesc_t desc;
	memset( &desc, 0, sizeof( desc ) );
	mesh->LockMesh( count, 0, desc );
	void *dest = desc.m_pPosition;
	if ( desc.m_VertexSize_Position == 0 )
	{
		dest = reinterpret_cast<void *>( ~uintptr_t( 0 ) );
		auto lowest = [&]( int size, void *p )
		{
			if ( size != 0 && p < dest )
				dest = p;
		};
		lowest( desc.m_VertexSize_BoneWeight, desc.m_pBoneWeight );
		lowest( desc.m_VertexSize_BoneMatrixIndex, desc.m_pBoneMatrixIndex );
		lowest( desc.m_VertexSize_Normal, desc.m_pNormal );
		lowest( desc.m_VertexSize_Color, desc.m_pColor );
		lowest( desc.m_VertexSize_Specular, desc.m_pSpecular );
		lowest( desc.m_VertexSize_TangentS, desc.m_pTangentS );
		lowest( desc.m_VertexSize_TangentT, desc.m_pTangentT );
		lowest( desc.m_VertexSize_Wrinkle, desc.m_pWrinkle );
	}
	memcpy( dest, bytes.data(), bytes.size() );
	mesh->UnlockMesh( count, 0, desc );
}

std::vector<unsigned char> Records( IMesh *mesh, int *stride )
{
	const unsigned char *records = nullptr;
	int count = 0;
	*stride = 0;
	if ( !ShaderBackend_NativeMeshRecords( mesh, &records, &count, stride ) || count != kVertices )
		return std::vector<unsigned char>();
	return std::vector<unsigned char>( records, records + size_t( count ) * *stride );
}

// A D3D9-style packed description (components tightly in order), the
// layout this backend must NOT be confused with.
void DescribePacked( unsigned char *buffer, VertexFormat_t format, MeshDesc_t &desc )
{
	static unsigned char scratch[render_vulkan::kMeshScratchBytes];
	int offset = 0;
	auto place = [&]( bool present, int bytes, int &size ) -> unsigned char *
	{
		size = 0;
		if ( !present )
			return scratch;
		unsigned char *p = buffer + offset;
		offset += bytes;
		return p;
	};
	desc.m_pPosition = reinterpret_cast<float *>(
	    place( ( format & VERTEX_POSITION ) != 0, 12, desc.m_VertexSize_Position ) );
	desc.m_pNormal = reinterpret_cast<float *>(
	    place( ( format & VERTEX_NORMAL ) != 0, 12, desc.m_VertexSize_Normal ) );
	desc.m_pColor = place( ( format & VERTEX_COLOR ) != 0, 4, desc.m_VertexSize_Color );
	desc.m_pSpecular = place( false, 0, desc.m_VertexSize_Specular );
	desc.m_pBoneWeight =
	    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_BoneWeight ) );
	desc.m_pBoneMatrixIndex = place( false, 0, desc.m_VertexSize_BoneMatrixIndex );
	desc.m_pUserData = reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_UserData ) );
	desc.m_pTangentS = reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_TangentS ) );
	desc.m_pTangentT = reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_TangentT ) );
	desc.m_pWrinkle = reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_Wrinkle ) );
	for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
		desc.m_pTexCoord[i] = reinterpret_cast<float *>( place( TexCoordSize( i, format ) > 0,
		    4 * TexCoordSize( i, format ), desc.m_VertexSize_TexCoord[i] ) );
	const int stride = offset;
	int *sizes[] = {
	    &desc.m_VertexSize_Position, &desc.m_VertexSize_Normal, &desc.m_VertexSize_Color };
	for ( int *size : sizes )
		*size = stride;
	for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
		desc.m_VertexSize_TexCoord[i] = TexCoordSize( i, format ) ? stride : 0;
	desc.m_ActualVertexSize = stride;
	desc.m_NumBoneWeights = 2;
}

// The format the queued material system gives a dynamic mesh for an explicit
// vertex format (CMatQueuedMesh::OnGetDynamicMesh): the bone weights of the
// current skinning, with bone indices when skinned.
VertexFormat_t QueuedDynamicFormat( VertexFormat_t format, int bones )
{
	bones = std::max( bones, NumBoneWeights( format ) );
	format &= ~VERTEX_BONE_WEIGHT_MASK;
	format |= VERTEX_BONEWEIGHT( bones );
	if ( bones > 0 )
		format |= VERTEX_BONE_INDEX;
	return format;
}

struct Case
{
	const char *name;
	VertexFormat_t format;
};

// Builds `format` directly into `direct` and through the queued replay into
// `replayed` (both meshes the backend returned for it), and compares.
void CheckFormat( IShaderAPI *api, const Case &c, IMesh *direct, IMesh *replayedTarget,
    const char *path, bool packedControl )
{
	const std::string name = std::string( path ) + " " + c.name;
	MeshDesc_t desc;
	memset( &desc, 0, sizeof( desc ) );
	direct->LockMesh( kVertices, 0, desc );
	const int lockStride = desc.m_ActualVertexSize;
	WriteVertices( desc, c.format, kVertices, true );
	direct->UnlockMesh( kVertices, 0, desc );
	int directStride = 0;
	const std::vector<unsigned char> expected = Records( direct, &directStride );
	Check( !expected.empty(), name + ": direct build is readable" );

	// The main thread's build, into memory that held garbage.
	MeshDesc_t queued;
	memset( &queued, 0, sizeof( queued ) );
	api->ComputeVertexDescription( nullptr, c.format, queued );
	const int queuedStride = queued.m_ActualVertexSize;
	Check( queuedStride == lockStride && queuedStride == directStride,
	    name + ": ComputeVertexDescription stride matches the mesh lock (" +
	        std::to_string( queuedStride ) + " vs " + std::to_string( lockStride ) + ")" );
	std::vector<unsigned char> bytes( size_t( kVertices ) * std::max( queuedStride, 1 ), 0xCD );
	memset( &queued, 0, sizeof( queued ) );
	if ( packedControl )
		DescribePacked( bytes.data(), c.format, queued );
	else
		api->ComputeVertexDescription( bytes.data(), c.format, queued );
	bytes.resize( size_t( kVertices ) * queued.m_ActualVertexSize );
	WriteVertices( queued, c.format, kVertices, true );
	if ( !packedControl && !( c.format & VERTEX_COLOR ) &&
	     !render_vulkan::MeshFormatIsColorStream( c.format ) )
	{
		// The stray color went to scratch: the replayed bytes there are the
		// allocation's garbage, so a white record below comes from the unlock.
		bool garbage = true;
		for ( int v = 0; v < kVertices; ++v )
			for ( int k = 0; k < 4; ++k )
				garbage =
				    garbage &&
				    bytes[size_t( v ) * queuedStride + render_vulkan::kMeshColorOffset + k] == 0xCD;
		Check( garbage, name + ": the replayed memory holds garbage at the absent color" );
	}
	Replay( replayedTarget, bytes, kVertices );
	int replayedStride = 0;
	const std::vector<unsigned char> replayed = Records( replayedTarget, &replayedStride );

	const bool identical = replayedStride == directStride && replayed == expected;
	if ( packedControl )
	{
		Check( !identical, name + ": a packed (D3D9) description replayed is detected" );
		return;
	}
	Check( identical, name + ": replayed records equal the direct build byte for byte" );

	if ( render_vulkan::MeshFormatIsColorStream( c.format ) || expected.empty() )
		return;
	// The identities of absent components hold in both builds, although the
	// builder wrote stray values and the replayed memory held garbage.
	for ( int v = 0; v < kVertices; ++v )
	{
		const unsigned char *record = expected.data() + size_t( v ) * directStride;
		if ( !( c.format & VERTEX_COLOR ) )
		{
			static const unsigned char white[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
			Check( !memcmp( record + render_vulkan::kMeshColorOffset, white, 4 ),
			    name + ": absent color is white" );
		}
		float weights[2];
		memcpy( weights, record + render_vulkan::kMeshBoneWeightOffset, sizeof( weights ) );
		if ( NumBoneWeights( c.format ) == 0 )
			Check( weights[0] == 1.0f && weights[1] == 0.0f,
			    name + ": unskinned vertex has weight 1 on bone 0" );
		else if ( NumBoneWeights( c.format ) == 1 )
			Check( weights[1] == 0.0f, name + ": one-weight vertex has no second weight" );
	}
}

} // namespace

int main()
{
	render::LegacyShaderServices services;
	if ( !NativeVulkanShaderBackend_Create( &services ) || !services.IsComplete() )
	{
		std::fprintf( stderr, "FAIL: NativeVulkanShaderBackend_Create did not return services\n" );
		return 1;
	}
	const render::LegacyShaderProvider *provider = NativeVulkanShaderBackend_Describe();
	Check( provider && provider->supportsQueuedRendering,
	    "the native provider declares the queued material-system contract" );

	const Case cases[] = {
	    { "lightmapped world", VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S |
	                               VERTEX_TANGENT_T | VERTEX_TEXCOORD_SIZE( 0, 2 ) |
	                               VERTEX_TEXCOORD_SIZE( 1, 2 ) | VERTEX_TEXCOORD_SIZE( 2, 2 ) },
	    { "skinned model", VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR |
	                           VERTEX_USERDATA_SIZE( 4 ) | VERTEX_BONEWEIGHT( 2 ) |
	                           VERTEX_BONE_INDEX | VERTEX_TEXCOORD_SIZE( 0, 2 ) },
	    { "one-bone model", VERTEX_POSITION | VERTEX_NORMAL | VERTEX_BONEWEIGHT( 1 ) |
	                            VERTEX_BONE_INDEX | VERTEX_TEXCOORD_SIZE( 0, 2 ) },
	    { "unlit ui", VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE( 0, 2 ) },
	    { "position only", VERTEX_POSITION },
	    { "spritecard (wide)", VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE( 0, 4 ) |
	                               VERTEX_TEXCOORD_SIZE( 1, 4 ) | VERTEX_TEXCOORD_SIZE( 2, 4 ) |
	                               VERTEX_TEXCOORD_SIZE( 3, 4 ) | VERTEX_TEXCOORD_SIZE( 4, 3 ) },
	    { "flex stream",
	        VERTEX_POSITION | VERTEX_NORMAL | VERTEX_WRINKLE | VERTEX_FORMAT_USE_EXACT_FORMAT },
	};

	for ( const Case &c : cases )
	{
		// Static meshes: one built directly, one replayed into.
		IMesh *direct = services.device->CreateStaticMesh( c.format, "test" );
		IMesh *replayed = services.device->CreateStaticMesh( c.format, "test" );
		CheckFormat( services.api, c, direct, replayed, "static", false );
		services.device->DestroyStaticMesh( direct );
		services.device->DestroyStaticMesh( replayed );

		// The dynamic mesh the render thread replays into, built directly
		// first (it is one mesh, so the direct records are copied out first).
		const VertexFormat_t dynamicFormat =
		    QueuedDynamicFormat( c.format, NumBoneWeights( c.format ) );
		const Case dynamic = { c.name, dynamicFormat };
		IMesh *mesh = services.api->GetDynamicMeshEx(
		    nullptr, c.format, NumBoneWeights( c.format ), false, nullptr, nullptr );
		CheckFormat( services.api, dynamic, mesh, mesh, "dynamic", false );
	}

	// The static-prop color stream (one D3DCOLOR per vertex).
	{
		const Case stream = { "color stream", VERTEX_SPECULAR };
		IMesh *direct = services.device->CreateStaticMesh( stream.format, "test" );
		IMesh *replayed = services.device->CreateStaticMesh( stream.format, "test" );
		CheckFormat( services.api, stream, direct, replayed, "static", false );
		services.device->DestroyStaticMesh( direct );
		services.device->DestroyStaticMesh( replayed );
	}

	// The queued flex mesh (CMatQueuedMesh's flex format) replays into the
	// backend's flex mesh.
	{
		const Case flex = { "flex mesh",
		    VERTEX_POSITION | VERTEX_NORMAL | VERTEX_WRINKLE | VERTEX_FORMAT_USE_EXACT_FORMAT };
		IMesh *mesh = services.api->GetFlexMesh();
		CheckFormat( services.api, flex, mesh, mesh, "flex", false );
		Check( mesh != services.api->GetDynamicMeshEx(
		                   nullptr, flex.format, 0, false, nullptr, nullptr ),
		    "the flex mesh is not the dynamic mesh (a flex build cannot overwrite it)" );
	}

	// A zero-vertex queued lock describes no memory the builder can overrun.
	{
		MeshDesc_t desc;
		memset( &desc, 0, sizeof( desc ) );
		services.api->ComputeVertexDescription( nullptr, 0, desc );
		Check( desc.m_pPosition && desc.m_ActualVertexSize > 0 &&
		           desc.m_ActualVertexSize <= render_vulkan::kMeshScratchBytes,
		    "a format-less description stays within one scratch record" );
	}

	// Negative control: the comparator detects a description that disagrees
	// with the mesh lock.
	{
		const Case &world = cases[0];
		IMesh *direct = services.device->CreateStaticMesh( world.format, "test" );
		IMesh *replayed = services.device->CreateStaticMesh( world.format, "test" );
		CheckFormat( services.api, world, direct, replayed, "control", true );
		services.device->DestroyStaticMesh( direct );
		services.device->DestroyStaticMesh( replayed );
	}

	std::printf( "queued mesh contract: %lu checks, %lu failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
