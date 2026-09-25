//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native Vulkan mesh vertex record (vulkan_mesh_layout.h).
//
//===========================================================================//

#include "vulkan_mesh_layout.h"

#include <cstring>

namespace render_vulkan
{

namespace
{

const float kUnitWeights[kMeshRecordBoneWeights] = { 1.0f, 0.0f };

void ZeroBytes( unsigned char *record, int offset, int bytes )
{
	memset( record + offset, 0, bytes );
}

} // namespace

bool MeshFormatIsColorStream( VertexFormat_t format )
{
	return format == VERTEX_SPECULAR;
}

bool MeshFormatIsWide( VertexFormat_t format )
{
	for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
	{
		const int size = TexCoordSize( i, format );
		if ( size > 2 || ( i >= 3 && size > 0 ) )
			return true;
	}
	return false;
}

int MeshRecordStride( VertexFormat_t format )
{
	if ( MeshFormatIsColorStream( format ) )
		return kMeshColorStreamStride;
	return MeshFormatIsWide( format ) ? kMeshWideStride : kMeshBaseStride;
}

void DescribeMeshRecords(
    unsigned char *records, VertexFormat_t format, unsigned char *scratch, VertexDesc_t &desc )
{
	const int stride = MeshRecordStride( format );
	// A component is either in the record with the record's stride, or in the
	// scratch with size 0 so the mesh builder's writes to it go nowhere.
	auto place = [&]( bool present, int offset, int &size ) -> unsigned char *
	{
		size = present ? stride : 0;
		return present ? records + offset : scratch;
	};

	desc.m_CompressionType = VERTEX_COMPRESSION_NONE;
	desc.m_NumBoneWeights = kMeshRecordBoneWeights;
	desc.m_ActualVertexSize = stride;
	desc.m_pWrinkle = reinterpret_cast<float *>( scratch );
	desc.m_VertexSize_Wrinkle = 0;

	if ( MeshFormatIsColorStream( format ) )
	{
		// Only the specular color; everything else goes to the scratch.
		desc.m_pPosition =
		    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_Position ) );
		desc.m_pBoneWeight =
		    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_BoneWeight ) );
		desc.m_pBoneMatrixIndex = place( false, 0, desc.m_VertexSize_BoneMatrixIndex );
		desc.m_pNormal = reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_Normal ) );
		desc.m_pColor = place( false, 0, desc.m_VertexSize_Color );
		desc.m_pSpecular = place( true, 0, desc.m_VertexSize_Specular );
		for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
			desc.m_pTexCoord[i] =
			    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_TexCoord[i] ) );
		desc.m_pTangentS =
		    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_TangentS ) );
		desc.m_pTangentT =
		    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_TangentT ) );
		desc.m_pUserData =
		    reinterpret_cast<float *>( place( false, 0, desc.m_VertexSize_UserData ) );
		return;
	}

	// Format 0 is a mesh created without a format: every base component is in
	// the record. The position is always there: the queued replay copies from
	// it, so both sides must agree the record starts with it.
	const bool known = format != 0;
	auto has = [&]( VertexFormat_t flag )
	{
		return !known || ( format & flag ) != 0;
	};

	desc.m_pPosition =
	    reinterpret_cast<float *>( place( true, kMeshPositionOffset, desc.m_VertexSize_Position ) );
	desc.m_pColor = place( has( VERTEX_COLOR ), kMeshColorOffset, desc.m_VertexSize_Color );
	// Model meshes read static lighting from a separate color mesh (SetColorMesh).
	desc.m_pSpecular = place( false, 0, desc.m_VertexSize_Specular );
	desc.m_pNormal = reinterpret_cast<float *>(
	    place( has( VERTEX_NORMAL ), kMeshNormalOffset, desc.m_VertexSize_Normal ) );
	desc.m_pBoneWeight = reinterpret_cast<float *>( place( !known || NumBoneWeights( format ) > 0,
	    kMeshBoneWeightOffset, desc.m_VertexSize_BoneWeight ) );
	desc.m_pBoneMatrixIndex =
	    place( has( VERTEX_BONE_INDEX ), kMeshBoneIndexOffset, desc.m_VertexSize_BoneMatrixIndex );
	desc.m_pUserData = reinterpret_cast<float *>( place(
	    !known || UserDataSize( format ) > 0, kMeshUserDataOffset, desc.m_VertexSize_UserData ) );
	desc.m_pTangentS = reinterpret_cast<float *>(
	    place( has( VERTEX_TANGENT_S ), kMeshTangentSOffset, desc.m_VertexSize_TangentS ) );
	desc.m_pTangentT = reinterpret_cast<float *>(
	    place( has( VERTEX_TANGENT_T ), kMeshTangentTOffset, desc.m_VertexSize_TangentT ) );

	const bool wide = known && MeshFormatIsWide( format );
	static const int kBaseTexCoordOffsets[3] = {
	    kMeshTexCoord0Offset, kMeshTexCoord1Offset, kMeshTexCoord2Offset };
	for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
	{
		const bool present = known ? TexCoordSize( i, format ) > 0 : i < 3;
		int offset = 0;
		if ( wide )
			offset = kMeshWideTexCoordOffset + i * 4 * static_cast<int>( sizeof( float ) );
		else if ( i < 3 )
			offset = kBaseTexCoordOffsets[i];
		desc.m_pTexCoord[i] = reinterpret_cast<float *>(
		    place( present && ( wide || i < 3 ), offset, desc.m_VertexSize_TexCoord[i] ) );
	}
}

void SeedMeshRecordDefaults( unsigned char *records, int count )
{
	for ( int v = 0; v < count; ++v )
	{
		unsigned char *record = records + static_cast<size_t>( v ) * kMeshBaseStride;
		memset( record + kMeshColorOffset, 0xFF, 4 );
		memcpy( record + kMeshBoneWeightOffset, kUnitWeights, sizeof( kUnitWeights ) );
		ZeroBytes( record, kMeshBoneIndexOffset, 4 );
		ZeroBytes( record, kMeshNormalOffset, kMeshTexCoord2Offset - kMeshNormalOffset + 8 );
	}
}

void FillAbsentMeshComponents( unsigned char *records, int count, VertexFormat_t format )
{
	if ( !format || MeshFormatIsColorStream( format ) )
		return;
	const int stride = MeshRecordStride( format );
	const bool wide = stride == kMeshWideStride;
	const int boneWeights = NumBoneWeights( format );
	for ( int v = 0; v < count; ++v )
	{
		unsigned char *record = records + static_cast<size_t>( v ) * stride;
		if ( !( format & VERTEX_COLOR ) )
			memset( record + kMeshColorOffset, 0xFF, 4 );
		if ( boneWeights == 0 )
			memcpy( record + kMeshBoneWeightOffset, kUnitWeights, sizeof( kUnitWeights ) );
		else if ( boneWeights == 1 )
			ZeroBytes( record, kMeshBoneWeightOffset + 4, 4 );
		if ( !( format & VERTEX_BONE_INDEX ) )
			ZeroBytes( record, kMeshBoneIndexOffset, 4 );
		if ( !( format & VERTEX_NORMAL ) )
			ZeroBytes( record, kMeshNormalOffset, 12 );
		if ( UserDataSize( format ) == 0 )
			ZeroBytes( record, kMeshUserDataOffset, 16 );
		if ( !( format & VERTEX_TANGENT_S ) )
			ZeroBytes( record, kMeshTangentSOffset, 12 );
		if ( !( format & VERTEX_TANGENT_T ) )
			ZeroBytes( record, kMeshTangentTOffset, 12 );

		if ( wide )
		{
			float *sets = reinterpret_cast<float *>( record + kMeshWideTexCoordOffset );
			for ( int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i )
			{
				for ( int f = TexCoordSize( i, format ); f < 4; ++f )
					sets[i * 4 + f] = 0.0f;
			}
			memcpy( record + kMeshTexCoord0Offset, sets, 2 * sizeof( float ) );
			memcpy( record + kMeshTexCoord1Offset, sets + 4, 2 * sizeof( float ) );
			memcpy( record + kMeshTexCoord2Offset, sets + 8, 2 * sizeof( float ) );
		}
		else
		{
			static const int kOffsets[3] = {
			    kMeshTexCoord0Offset, kMeshTexCoord1Offset, kMeshTexCoord2Offset };
			for ( int i = 0; i < 3; ++i )
			{
				float *set = reinterpret_cast<float *>( record + kOffsets[i] );
				for ( int f = TexCoordSize( i, format ); f < 2; ++f )
					set[f] = 0.0f;
			}
		}
	}
}

} // namespace render_vulkan
