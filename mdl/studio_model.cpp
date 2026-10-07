//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Studio model mesh reader (content.studio-model). See
//			public/mdl/studio_model.h. The byte layouts below are those of
//			public/studio.h, public/optimize.h and the VVD header as the
//			files store them (32-bit fields, pointers stored as 32-bit
//			integers), written out as offsets so the reader depends on no
//			engine header and no host struct layout.
//
//=============================================================================//

#include "mdl/studio_model.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <optional>

namespace mdl
{

namespace
{

// --- Layouts ---------------------------------------------------------------------------

constexpr std::int32_t kMinMdlVersion = 44;
constexpr std::int32_t kMaxMdlVersion = 49;
constexpr std::int32_t kVvdVersion = 4;
constexpr std::int32_t kVtxVersion = 7;

// studiohdr_t
constexpr std::uint32_t kMdlHeaderSize = 240;
constexpr std::uint32_t kMdlVersion = 4;
constexpr std::uint32_t kMdlChecksum = 8;
constexpr std::uint32_t kMdlName = 12;
constexpr std::uint32_t kMdlNameSize = 64;
constexpr std::uint32_t kMdlHullMin = 104;
constexpr std::uint32_t kMdlHullMax = 116;
constexpr std::uint32_t kMdlViewMin = 128;
constexpr std::uint32_t kMdlViewMax = 140;
constexpr std::uint32_t kMdlNumTextures = 204; // then textureindex, numcdtextures,
                                               // cdtextureindex, numskinref,
                                               // numskinfamilies, skinindex,
                                               // numbodyparts, bodypartindex
constexpr std::uint32_t kTextureStride = 64;   // mstudiotexture_t
constexpr std::uint32_t kBodyPartStride = 16;  // mstudiobodyparts_t
constexpr std::uint32_t kModelStride = 148;    // mstudiomodel_t
constexpr std::uint32_t kModelNumMeshes = 72;  // then meshindex, numvertices, vertexindex
constexpr std::uint32_t kMeshStride = 116;     // mstudiomesh_t
constexpr std::uint32_t kFlexStride = 60;     // mstudioflex_t
constexpr std::uint32_t kMdlNumFlexDesc = 260; // then flexdescindex
constexpr std::uint32_t kMdlFlexScale = 392;
constexpr std::uint32_t kFlexesConverted = 0x00004000;
constexpr std::uint32_t kFlexScaleFlag = 0x00200000;

// studiohdr_t: skeleton, animations, sequences, animation blocks
constexpr std::uint32_t kMdlFlags = 152;
constexpr std::uint32_t kMdlNumBones = 156;      // then boneindex
constexpr std::uint32_t kMdlNumLocalAnim = 180;  // then localanimindex, numlocalseq,
                                                 // localseqindex
constexpr std::uint32_t kMdlAnimBlockName = 348; // then numanimblocks, animblockindex
constexpr std::uint32_t kMdlNumIncludes = 336;   // then includemodelindex
constexpr std::uint32_t kIncludeStride = 8;      // mstudiomodelgroup_t: label, name
constexpr std::uint32_t kMaxBones = 256;         // a VVD bone reference is one byte

// mstudiobone_t
constexpr std::uint32_t kBoneStride = 216;
constexpr std::uint32_t kBoneParent = 4;
constexpr std::uint32_t kBonePosition = 32;
constexpr std::uint32_t kBoneQuaternion = 44;
constexpr std::uint32_t kBoneEuler = 60;
constexpr std::uint32_t kBonePosScale = 72;
constexpr std::uint32_t kBoneRotScale = 84;
constexpr std::uint32_t kBonePoseToBone = 96;
constexpr std::uint32_t kBoneFlags = 160;
constexpr std::uint32_t kBoneSaveFramePos = 0x00200000;   // BONE_HAS_SAVEFRAME_POS
constexpr std::uint32_t kBoneSaveFrameRot = 0x00400000;   // BONE_HAS_SAVEFRAME_ROT
constexpr std::uint32_t kBoneSaveFrameRot32 = 0x00800000; // Quaternion32: not read

// mstudioseqdesc_t
constexpr std::uint32_t kSeqStride = 212;
constexpr std::uint32_t kSeqLabel = 4;
constexpr std::uint32_t kSeqFlags = 12;
constexpr std::uint32_t kSeqBbMin = 32;
constexpr std::uint32_t kSeqBbMax = 44;
constexpr std::uint32_t kSeqNumBlends = 56;
constexpr std::uint32_t kSeqAnimIndex = 60;
constexpr std::uint32_t kSeqWeightList = 156;
constexpr std::uint32_t kSeqPost = 0x10; // STUDIO_POST

// mstudioanimdesc_t
constexpr std::uint32_t kAnimStride = 100;
constexpr std::uint32_t kAnimFlags = 12;
constexpr std::uint32_t kAnimBlock = 52; // then animindex
constexpr std::uint32_t kAnimSectionIndex = 80;
constexpr std::uint32_t kAnimSectionFrames = 84;
constexpr std::uint32_t kAnimZeroFrameCount = 90; // short
constexpr std::uint32_t kAnimZeroFrameIndex = 92;
constexpr std::uint32_t kAnimDelta = 0x04;     // STUDIO_DELTA
constexpr std::uint32_t kAnimFrameAnim = 0x40; // STUDIO_FRAMEANIM

// mstudioanim_t (RLE records): bone, flags, nextoffset, then data
constexpr std::uint8_t kAnimRawPos = 0x01;
constexpr std::uint8_t kAnimRawRot = 0x02;
constexpr std::uint8_t kAnimAnimPos = 0x04;
constexpr std::uint8_t kAnimAnimRot = 0x08;
constexpr std::uint8_t kAnimRecordDelta = 0x10;
constexpr std::uint8_t kAnimRawRot2 = 0x20;

// mstudio_frame_anim_t: constantsoffset, frameoffset, framelength, unused[3],
// then one flag byte per bone
constexpr std::uint32_t kFrameAnimFlags = 24;
constexpr std::uint8_t kFrameConstPos = 0x01;
constexpr std::uint8_t kFrameConstRot = 0x02;
constexpr std::uint8_t kFrameAnimPos = 0x04;
constexpr std::uint8_t kFrameAnimRot = 0x08;
constexpr std::uint8_t kFrameAnimPos2 = 0x10;
constexpr std::uint8_t kFrameConstPos2 = 0x20;
constexpr std::uint8_t kFrameConstRot2 = 0x40;
constexpr std::uint8_t kFrameAnimRot2 = 0x80;

// vertexFileHeader_t
constexpr std::uint32_t kVvdHeaderSize = 64;
constexpr std::uint32_t kVvdMaxLods = 8;
constexpr std::uint32_t kVvdFixupStride = 12;  // vertexFileFixup_t
constexpr std::uint32_t kVvdVertexStride = 48; // mstudiovertex_t
constexpr std::uint32_t kVvdPosition = 16;
constexpr std::uint32_t kVvdNormal = 28;
constexpr std::uint32_t kVvdUv = 40;

// OptimizedModel (pack 1)
constexpr std::uint32_t kVtxHeaderSize = 36;
constexpr std::uint32_t kVtxBodyPartStride = 8;
constexpr std::uint32_t kVtxModelStride = 8;
constexpr std::uint32_t kVtxLodStride = 12;
constexpr std::uint32_t kVtxMeshStride = 9;
constexpr std::uint32_t kVtxStripGroupStride = 25;
constexpr std::uint32_t kVtxStripGroupStride49 = 33;
constexpr std::uint32_t kVtxStripStride = 27;
constexpr std::uint32_t kVtxStripStride49 = 35;
constexpr std::uint32_t kVtxVertexStride = 9;
constexpr std::uint32_t kVtxVertexMeshId = 4;
constexpr std::uint8_t kStripIsTriStrip = 0x02;

// --- Bounded reading -------------------------------------------------------------------

// Reads little-endian fields from one file. The first failure is latched:
// later reads return zero and change nothing, so a caller checks Failed()
// once after a group of reads.
class Reader
{
public:
	Reader( std::string_view data, ModelFile file ) : m_data( data ), m_file( file ) {}

	std::uint64_t Size() const { return m_data.size(); }
	bool Failed() const { return m_error.has_value(); }
	const ModelError &Error() const { return *m_error; }

	void Fail( ModelStatus status, std::uint64_t offset )
	{
		if ( !m_error )
		{
			const std::uint64_t clamped =
			    std::min<std::uint64_t>( offset, std::numeric_limits<std::uint32_t>::max() );
			m_error = ModelError{ status, m_file, static_cast<std::uint32_t>( clamped ) };
		}
	}

	bool Has( std::uint64_t offset, std::uint64_t length )
	{
		if ( Failed() )
		{
			return false;
		}
		if ( offset > m_data.size() || length > m_data.size() - offset )
		{
			Fail( ModelStatus::Truncated, offset );
			return false;
		}
		return true;
	}

	std::int32_t I32( std::uint64_t offset ) { return static_cast<std::int32_t>( U32( offset ) ); }

	std::uint32_t U32( std::uint64_t offset )
	{
		if ( !Has( offset, 4 ) )
		{
			return 0;
		}
		const auto *p = reinterpret_cast<const unsigned char *>( m_data.data() + offset );
		return std::uint32_t( p[0] ) | ( std::uint32_t( p[1] ) << 8 ) |
		       ( std::uint32_t( p[2] ) << 16 ) | ( std::uint32_t( p[3] ) << 24 );
	}

	std::uint16_t U16( std::uint64_t offset )
	{
		if ( !Has( offset, 2 ) )
		{
			return 0;
		}
		const auto *p = reinterpret_cast<const unsigned char *>( m_data.data() + offset );
		return static_cast<std::uint16_t>( p[0] | ( p[1] << 8 ) );
	}

	std::int16_t I16( std::uint64_t offset ) { return static_cast<std::int16_t>( U16( offset ) ); }

	std::uint64_t U64( std::uint64_t offset )
	{
		const std::uint64_t low = U32( offset );
		return low | ( std::uint64_t( U32( offset + 4 ) ) << 32 );
	}

	std::uint8_t U8( std::uint64_t offset )
	{
		if ( !Has( offset, 1 ) )
		{
			return 0;
		}
		return static_cast<std::uint8_t>( m_data[offset] );
	}

	float F32( std::uint64_t offset )
	{
		const std::uint32_t bits = U32( offset );
		float value = 0.0f;
		static_assert( sizeof( value ) == sizeof( bits ) );
		std::memcpy( &value, &bits, sizeof( value ) );
		return value;
	}

	Float3 Vec( std::uint64_t offset )
	{
		return { F32( offset ), F32( offset + 4 ), F32( offset + 8 ) };
	}

	// A count read at 'offset': negative is BadCount.
	std::uint32_t Count( std::uint64_t offset )
	{
		const std::int32_t value = I32( offset );
		if ( value < 0 )
		{
			Fail( ModelStatus::BadCount, offset );
			return 0;
		}
		return static_cast<std::uint32_t>( value );
	}

	// 'base' plus the signed offset at 'field', as an absolute position: it
	// must name a position inside the file (BadOffset otherwise).
	std::uint64_t Relative( std::uint64_t base, std::uint64_t field )
	{
		const std::int64_t delta = I32( field );
		const std::int64_t at = static_cast<std::int64_t>( base ) + delta;
		if ( Failed() )
		{
			return 0;
		}
		if ( at < 0 || static_cast<std::uint64_t>( at ) > m_data.size() )
		{
			Fail( ModelStatus::BadOffset, field );
			return 0;
		}
		return static_cast<std::uint64_t>( at );
	}

	// 'count' records of 'stride' bytes from 'start' must fit in the file.
	bool Array( std::uint64_t start, std::uint64_t count, std::uint64_t stride )
	{
		return Has( start, count * stride );
	}

	// The NUL-terminated string at 'offset', lower-cased unless 'keepCase'.
	std::string String( std::uint64_t offset, bool keepCase = false )
	{
		if ( !Has( offset, 1 ) )
		{
			return {};
		}
		const std::size_t end = m_data.find( '\0', offset );
		if ( end == std::string_view::npos )
		{
			Fail( ModelStatus::Truncated, offset );
			return {};
		}
		std::string out( m_data.substr( offset, end - offset ) );
		if ( !keepCase )
		{
			for ( char &c : out )
			{
				c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
			}
		}
		return out;
	}

private:
	std::string_view m_data;
	ModelFile m_file;
	std::optional<ModelError> m_error;
};

std::string Slashes( std::string text )
{
	std::replace( text.begin(), text.end(), '\\', '/' );
	return text;
}

std::string MaterialDirectory( std::string text )
{
	text = Slashes( std::move( text ) );
	while ( !text.empty() && text.front() == '/' )
	{
		text.erase( text.begin() );
	}
	if ( !text.empty() && text.back() != '/' )
	{
		text.push_back( '/' );
	}
	return text;
}

// --- The three files -------------------------------------------------------------------

struct MdlHeader
{
	std::uint32_t numTextures = 0;
	std::uint64_t textureIndex = 0;
	std::uint32_t numCdTextures = 0;
	std::uint64_t cdTextureIndex = 0;
	std::uint32_t numSkinRef = 0;
	std::uint32_t numSkinFamilies = 0;
	std::uint64_t skinIndex = 0;
	std::uint32_t numBodyParts = 0;
	std::uint64_t bodyPartIndex = 0;
};

// The VVD's LOD 0 vertex list (after fixups), as positions of records.
struct VvdVertices
{
	std::vector<std::uint64_t> records;
	std::uint64_t dataStart = 0;
	std::uint64_t tangentStart = 0;
};

std::optional<ModelError> ReadVvd( Reader &vvd, std::int32_t checksum, VvdVertices &out )
{
	if ( !vvd.Has( 0, kVvdHeaderSize ) )
	{
		return vvd.Error();
	}
	if ( vvd.U32( 0 ) != 0x56534449u ) // "IDSV"
	{
		vvd.Fail( ModelStatus::BadMagic, 0 );
		return vvd.Error();
	}
	if ( vvd.I32( 4 ) != kVvdVersion )
	{
		vvd.Fail( ModelStatus::UnsupportedVersion, 4 );
		return vvd.Error();
	}
	if ( vvd.I32( 8 ) != checksum )
	{
		vvd.Fail( ModelStatus::ChecksumMismatch, 8 );
		return vvd.Error();
	}
	const std::uint32_t numLods = vvd.Count( 12 );
	if ( !vvd.Failed() && ( numLods < 1 || numLods > kVvdMaxLods ) )
	{
		vvd.Fail( ModelStatus::BadCount, 12 );
	}
	const std::uint32_t lod0 = vvd.Count( 16 );
	const std::uint32_t numFixups = vvd.Count( 48 );
	const std::uint64_t fixupStart = vvd.Count( 52 );
	const std::uint64_t dataStart = vvd.Count( 56 );
	const std::uint64_t tangentStart = vvd.Count( 60 );
	if ( vvd.Failed() )
	{
		return vvd.Error();
	}
	// The vertex records end where the tangents begin (or at the file's end).
	const std::uint64_t dataEnd = tangentStart > 0 ? tangentStart : vvd.Size();
	if ( dataStart < kVvdHeaderSize || dataStart > dataEnd || dataEnd > vvd.Size() )
	{
		vvd.Fail( ModelStatus::BadOffset, 56 );
		return vvd.Error();
	}
	const std::uint64_t available = ( dataEnd - dataStart ) / kVvdVertexStride;
	if ( tangentStart != 0 &&
	     ( tangentStart > vvd.Size() || available > ( vvd.Size() - tangentStart ) / 16 ) )
	{
		vvd.Fail( ModelStatus::Truncated, tangentStart );
		return vvd.Error();
	}
	out.dataStart = dataStart;
	out.tangentStart = tangentStart;
	const auto record = [&]( std::uint64_t index )
	{
		return dataStart + index * kVvdVertexStride;
	};

	if ( numFixups == 0 )
	{
		if ( lod0 > available )
		{
			vvd.Fail( ModelStatus::Truncated, record( available ) );
			return vvd.Error();
		}
		out.records.reserve( lod0 );
		for ( std::uint64_t i = 0; i < lod0; ++i )
		{
			out.records.push_back( record( i ) );
		}
		return std::nullopt;
	}
	if ( !vvd.Array( fixupStart, numFixups, kVvdFixupStride ) )
	{
		return vvd.Error();
	}
	for ( std::uint64_t f = 0; f < numFixups; ++f )
	{
		const std::uint64_t at = fixupStart + f * kVvdFixupStride;
		const std::int32_t lod = vvd.I32( at );
		const std::uint64_t source = vvd.Count( at + 4 );
		const std::uint64_t count = vvd.Count( at + 8 );
		if ( vvd.Failed() )
		{
			return vvd.Error();
		}
		if ( lod < 0 )
		{
			continue; // a fixup no level of detail uses
		}
		if ( source + count > available )
		{
			vvd.Fail( ModelStatus::BadIndex, at + 4 );
			return vvd.Error();
		}
		for ( std::uint64_t i = 0; i < count; ++i )
		{
			out.records.push_back( record( source + i ) );
		}
	}
	return std::nullopt;
}

Vertex ReadVertex( Reader &vvd, std::uint64_t record, const VvdVertices &vertices )
{
	Vertex v;
	v.position = vvd.Vec( record + kVvdPosition );
	v.normal = vvd.Vec( record + kVvdNormal );
	if ( vertices.tangentStart != 0 )
	{
		const std::uint64_t tangent =
		    vertices.tangentStart + ( record - vertices.dataStart ) / kVvdVertexStride * 16;
		v.tangent = vvd.Vec( tangent );
		v.tangentSign = vvd.F32( tangent + 12 );
	}
	v.u = vvd.F32( record + kVvdUv );
	v.v = vvd.F32( record + kVvdUv + 4 );
	return v;
}

// Appends the triangles of one VTX strip group, as mesh-local vertex indices
// (counter-clockwise), to 'indices'.
bool ReadStripGroup( Reader &vtx, std::uint64_t group, bool v49, std::uint32_t meshVertices,
    std::vector<std::uint32_t> &indices, std::vector<std::uint32_t> *groupVertices = nullptr )
{
	const std::uint32_t numVerts = vtx.Count( group );
	const std::uint64_t verts = vtx.Relative( group, group + 4 );
	const std::uint32_t numIndices = vtx.Count( group + 8 );
	const std::uint64_t indexStart = vtx.Relative( group, group + 12 );
	const std::uint32_t numStrips = vtx.Count( group + 16 );
	const std::uint64_t strips = vtx.Relative( group, group + 20 );
	if ( vtx.Failed() || !vtx.Array( verts, numVerts, kVtxVertexStride ) ||
	     !vtx.Array( indexStart, numIndices, 2 ) ||
	     !vtx.Array( strips, numStrips, v49 ? kVtxStripStride49 : kVtxStripStride ) )
	{
		return false;
	}
	// The group's vertices name mesh vertices.
	std::vector<std::uint32_t> meshVertex( numVerts );
	for ( std::uint32_t i = 0; i < numVerts; ++i )
	{
		const std::uint64_t at = verts + std::uint64_t( i ) * kVtxVertexStride + kVtxVertexMeshId;
		meshVertex[i] = vtx.U16( at );
		if ( meshVertex[i] >= meshVertices )
		{
			vtx.Fail( ModelStatus::BadIndex, at );
			return false;
		}
	}
	if ( groupVertices )
		*groupVertices = meshVertex;
	const auto groupIndex = [&]( std::uint64_t i, std::uint32_t &out )
	{
		const std::uint64_t at = indexStart + i * 2;
		const std::uint16_t index = vtx.U16( at );
		if ( vtx.Failed() || index >= numVerts )
		{
			vtx.Fail( ModelStatus::BadIndex, at );
			return false;
		}
		out = meshVertex[index];
		return true;
	};
	const auto emit = [&]( std::uint32_t a, std::uint32_t b, std::uint32_t c )
	{
		// Stored clockwise from outside; emitted counter-clockwise.
		indices.push_back( a );
		indices.push_back( c );
		indices.push_back( b );
	};
	for ( std::uint32_t s = 0; s < numStrips; ++s )
	{
		const std::uint64_t strip =
		    strips + std::uint64_t( s ) * ( v49 ? kVtxStripStride49 : kVtxStripStride );
		const std::uint64_t count = vtx.Count( strip );
		const std::uint64_t first = vtx.Count( strip + 4 );
		const std::uint8_t flags = vtx.U8( strip + 18 );
		if ( vtx.Failed() )
		{
			return false;
		}
		if ( first + count > numIndices )
		{
			vtx.Fail( ModelStatus::BadIndex, strip );
			return false;
		}
		if ( flags & kStripIsTriStrip )
		{
			for ( std::uint64_t i = 0; i + 2 < count; ++i )
			{
				std::uint32_t a = 0, b = 0, c = 0;
				if ( !groupIndex( first + i, a ) || !groupIndex( first + i + 1, b ) ||
				     !groupIndex( first + i + 2, c ) )
				{
					return false;
				}
				if ( a == b || b == c || a == c )
				{
					continue; // a strip's joining triangle
				}
				if ( i % 2 == 0 )
				{
					emit( a, b, c );
				}
				else
				{
					emit( b, a, c );
				}
			}
			continue;
		}
		if ( count % 3 != 0 )
		{
			vtx.Fail( ModelStatus::BadCount, strip );
			return false;
		}
		for ( std::uint64_t i = 0; i < count; i += 3 )
		{
			std::uint32_t a = 0, b = 0, c = 0;
			if ( !groupIndex( first + i, a ) || !groupIndex( first + i + 1, b ) ||
			     !groupIndex( first + i + 2, c ) )
			{
				return false;
			}
			emit( a, b, c );
		}
	}
	return true;
}

void Extend( Float3 &mins, Float3 &maxs, const Float3 &p, bool first )
{
	if ( first )
	{
		mins = maxs = p;
		return;
	}
	mins = { std::min( mins.x, p.x ), std::min( mins.y, p.y ), std::min( mins.z, p.z ) };
	maxs = { std::max( maxs.x, p.x ), std::max( maxs.y, p.y ), std::max( maxs.z, p.z ) };
}

// --- Compressed values (public/mathlib/compressed_vector.h) -------------------------------

// float16: IEEE half precision, except that infinities read as +-65504 and NaNs
// as 0 (float16::Convert16bitFloatTo32bits).
float Half( std::uint16_t bits )
{
	const bool negative = ( bits >> 15 ) != 0;
	const int exponent = ( bits >> 10 ) & 31;
	const int mantissa = bits & 1023;
	float value = 0.0f;
	if ( exponent == 31 )
	{
		value = mantissa == 0 ? 65504.0f : 0.0f;
	}
	else if ( exponent == 0 )
	{
		value = std::ldexp( float( mantissa ) / 1024.0f, -14 );
	}
	else
	{
		value = std::ldexp( 1.0f + float( mantissa ) / 1024.0f, exponent - 15 );
	}
	return negative ? -value : value;
}

Float3 Vector48( Reader &r, std::uint64_t at )
{
	return { Half( r.U16( at ) ), Half( r.U16( at + 2 ) ), Half( r.U16( at + 4 ) ) };
}

bool ReadFlexes( Reader &mdl, const Model &model, std::uint64_t mesh,
    std::uint32_t vertexCount, std::vector<Flex> &out )
{
	const std::uint32_t count = mdl.Count( mesh + 16 );
	const std::uint64_t flexes = mdl.Relative( mesh, mesh + 20 );
	if ( mdl.Failed() || !mdl.Array( flexes, count, kFlexStride ) )
		return false;
	if ( !count )
		return true;
	const float scale =
	    ( model.flags & kFlexScaleFlag ) ? mdl.F32( kMdlFlexScale ) : 1.0f / 4096.0f;
	if ( !std::isfinite( scale ) || scale <= 0.0f ||
	     scale > std::numeric_limits<float>::max() / 32768.0f )
	{
		mdl.Fail( ModelStatus::BadCount, kMdlFlexScale );
		return false;
	}
	const auto component = [&]( std::uint64_t at )
	{
		if ( model.flags & kFlexesConverted )
			return float( mdl.I16( at ) ) * scale;
		// CMDLCache converts the on-disk half to signed fixed point once.
		// Match that truncation so the same bytes give the same live shape.
		const float quantized = std::trunc( Half( mdl.U16( at ) ) / scale );
		if ( !std::isfinite( quantized ) || quantized < -32768.0f || quantized > 32767.0f )
		{
			mdl.Fail( ModelStatus::BadCount, at );
			return 0.0f;
		}
		return quantized * scale;
	};
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		const std::uint64_t at = flexes + std::uint64_t( i ) * kFlexStride;
		Flex flex;
		flex.descriptor = mdl.Count( at );
		flex.pair = mdl.Count( at + 28 );
		if ( flex.descriptor >= model.flexDescriptors.size() ||
		     ( flex.pair && flex.pair >= model.flexDescriptors.size() ) )
		{
			mdl.Fail( ModelStatus::BadIndex, at );
			return false;
		}
		for ( std::uint32_t j = 0; j < 4; ++j )
		{
			flex.targets[j] = mdl.F32( at + 4 + j * 4 );
			if ( !std::isfinite( flex.targets[j] ) ||
			     ( j && flex.targets[j] < flex.targets[j - 1] ) )
			{
				mdl.Fail( ModelStatus::BadCount, at + 4 + j * 4 );
				return false;
			}
		}
		const std::uint32_t vertices = mdl.Count( at + 20 );
		const std::uint64_t deltas = mdl.Relative( at, at + 24 );
		const std::uint8_t type = mdl.U8( at + 32 );
		if ( type > 1 )
		{
			mdl.Fail( ModelStatus::BadCount, at + 32 );
			return false;
		}
		const std::uint32_t stride = type == 1 ? 18u : 16u;
		if ( !mdl.Array( deltas, vertices, stride ) )
			return false;
		for ( std::uint32_t j = 0; j < vertices; ++j )
		{
			const std::uint64_t deltaAt = deltas + std::uint64_t( j ) * stride;
			FlexDelta delta;
			delta.vertex = mdl.U16( deltaAt );
			if ( delta.vertex >= vertexCount )
			{
				mdl.Fail( ModelStatus::BadIndex, deltaAt );
				return false;
			}
			delta.speed = mdl.U8( deltaAt + 2 );
			delta.side = mdl.U8( deltaAt + 3 );
			delta.position =
			    { component( deltaAt + 4 ), component( deltaAt + 6 ), component( deltaAt + 8 ) };
			delta.normal =
			    { component( deltaAt + 10 ), component( deltaAt + 12 ), component( deltaAt + 14 ) };
			if ( type == 1 )
				delta.wrinkle = float( mdl.I16( deltaAt + 16 ) ) * scale;
			flex.deltas.push_back( delta );
		}
		out.push_back( std::move( flex ) );
	}
	return !mdl.Failed();
}

float RebuildW( float x, float y, float z, bool negative )
{
	const float w = std::sqrt( std::max( 0.0f, 1.0f - x * x - y * y - z * z ) );
	return negative ? -w : w;
}

// Quaternion48: x and y in 16 bits, z in 15, the sign of w in the last bit.
Quaternion Quaternion48( Reader &r, std::uint64_t at )
{
	const std::uint16_t x = r.U16( at );
	const std::uint16_t y = r.U16( at + 2 );
	const std::uint16_t zw = r.U16( at + 4 );
	Quaternion q;
	q.x = ( int( x ) - 32768 ) * ( 1.0f / 32768.0f );
	q.y = ( int( y ) - 32768 ) * ( 1.0f / 32768.0f );
	q.z = ( int( zw & 0x7fff ) - 16384 ) * ( 1.0f / 16384.0f );
	q.w = RebuildW( q.x, q.y, q.z, ( zw >> 15 ) != 0 );
	return q;
}

// Quaternion64: x, y and z in 21 bits each, the sign of w in the last bit.
Quaternion Quaternion64( Reader &r, std::uint64_t at )
{
	const std::uint64_t bits = r.U64( at );
	const auto component = [&]( int shift )
	{
		return ( int( ( bits >> shift ) & 0x1fffff ) - 1048576 ) * ( 1.0f / 1048576.5f );
	};
	Quaternion q;
	q.x = component( 0 );
	q.y = component( 21 );
	q.z = component( 42 );
	q.w = RebuildW( q.x, q.y, q.z, ( bits >> 63 ) != 0 );
	return q;
}

// Quaternion48S: three 15-bit components and the index and sign of the
// largest, rebuilt from the unit length (bone_setup.cpp DecodeQuaternion48S).
Quaternion Quaternion48S( Reader &r, std::uint64_t at )
{
	const std::uint16_t packed[3] = { r.U16( at ), r.U16( at + 2 ), r.U16( at + 4 ) };
	constexpr float kScale = 1.0f / 23168.0f;
	constexpr int kShift = 16384;
	// The first stored component's index is the two sign bits of the first
	// two words; the other two follow it cyclically and the fourth is rebuilt.
	const int ia = ( packed[1] >> 15 ) + ( packed[0] >> 15 ) * 2;
	const int ib = ( ia + 1 ) % 4;
	const int ic = ( ia + 2 ) % 4;
	float q[4] = {};
	q[ia] = ( int( packed[0] & 0x7fff ) - kShift ) * kScale;
	q[ib] = ( int( packed[1] & 0x7fff ) - kShift ) * kScale;
	q[ic] = ( int( packed[2] & 0x7fff ) - kShift ) * kScale;
	q[( ia + 3 ) % 4] = RebuildW( q[ia], q[ib], q[ic], ( packed[2] >> 15 ) != 0 );
	return { q[0], q[1], q[2], q[3] };
}

// --- Quaternion arithmetic (mathlib's conventions) -----------------------------------------

// RadianEuler (x roll, y pitch, z yaw) to a quaternion: AngleQuaternion.
Quaternion EulerQuaternion( const Float3 &angles )
{
	const float sr = std::sin( angles.x * 0.5f ), cr = std::cos( angles.x * 0.5f );
	const float sp = std::sin( angles.y * 0.5f ), cp = std::cos( angles.y * 0.5f );
	const float sy = std::sin( angles.z * 0.5f ), cy = std::cos( angles.z * 0.5f );
	return { sr * cp * cy - cr * sp * sy, cr * sp * cy + sr * cp * sy, cr * cp * sy - sr * sp * cy,
	    cr * cp * cy + sr * sp * sy };
}

float QuaternionDot( const Quaternion &a, const Quaternion &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Quaternion Normalized( Quaternion q )
{
	const float length = std::sqrt( QuaternionDot( q, q ) );
	if ( length > 0.0f )
	{
		q = { q.x / length, q.y / length, q.z / length, q.w / length };
	}
	return q;
}

// p x q (Hamilton), q first aligned to p: QuaternionMult.
Quaternion Multiply( const Quaternion &p, Quaternion q )
{
	if ( QuaternionDot( p, q ) < 0.0f )
	{
		q = { -q.x, -q.y, -q.z, -q.w };
	}
	return { p.x * q.w + p.y * q.z - p.z * q.y + p.w * q.x,
	    -p.x * q.z + p.y * q.w + p.z * q.x + p.w * q.y,
	    p.x * q.y - p.y * q.x + p.z * q.w + p.w * q.z,
	    -p.x * q.x - p.y * q.y - p.z * q.z + p.w * q.w };
}

// The rotation 'p' scaled to the fraction 't' of its angle: QuaternionScale.
Quaternion Scaled( const Quaternion &p, float t )
{
	const float r = std::sqrt( p.x * p.x + p.y * p.y + p.z * p.z );
	const float sinsom = std::sin( std::asin( std::min( r, 1.0f ) ) * t );
	const float k = sinsom / ( r + std::numeric_limits<float>::epsilon() );
	Quaternion q{ p.x * k, p.y * k, p.z * k, 0.0f };
	const float rr = std::min( sinsom * sinsom, 1.0f );
	q.w = std::sqrt( 1.0f - rr );
	if ( p.w < 0.0f )
	{
		q.w = -q.w;
	}
	return q;
}

// Spherical interpolation from 'p' (t = 0) to 'q' (t = 1), q aligned to p.
Quaternion Slerp( const Quaternion &p, Quaternion q, float t )
{
	float cosom = QuaternionDot( p, q );
	if ( cosom < 0.0f )
	{
		q = { -q.x, -q.y, -q.z, -q.w };
		cosom = -cosom;
	}
	float sclp = 1.0f - t;
	float sclq = t;
	if ( cosom < 0.9999f )
	{
		const float omega = std::acos( cosom );
		const float sinom = std::sin( omega );
		sclp = std::sin( ( 1.0f - t ) * omega ) / sinom;
		sclq = std::sin( t * omega ) / sinom;
	}
	return Normalized( { sclp * p.x + sclq * q.x, sclp * p.y + sclq * q.y, sclp * p.z + sclq * q.z,
	    sclp * p.w + sclq * q.w } );
}

// --- Skeleton, animations and sequences ---------------------------------------------------

std::optional<ModelError> ReadBones( Reader &mdl, Model &model )
{
	const std::uint32_t numBones = mdl.Count( kMdlNumBones );
	const std::uint64_t boneIndex = mdl.Count( kMdlNumBones + 4 );
	if ( mdl.Failed() )
	{
		return mdl.Error();
	}
	if ( numBones < 1 || numBones > kMaxBones )
	{
		mdl.Fail( ModelStatus::BadCount, kMdlNumBones );
		return mdl.Error();
	}
	if ( !mdl.Array( boneIndex, numBones, kBoneStride ) )
	{
		return mdl.Error();
	}
	model.bones.reserve( numBones );
	for ( std::uint32_t i = 0; i < numBones; ++i )
	{
		const std::uint64_t at = boneIndex + std::uint64_t( i ) * kBoneStride;
		Bone bone;
		bone.name = mdl.String( mdl.Relative( at, at ), true );
		bone.parent = mdl.I32( at + kBoneParent );
		if ( !mdl.Failed() && ( bone.parent < -1 || bone.parent >= std::int32_t( i ) ) )
		{
			mdl.Fail( ModelStatus::BadIndex, at + kBoneParent );
		}
		bone.position = mdl.Vec( at + kBonePosition );
		bone.rotation = { mdl.F32( at + kBoneQuaternion ), mdl.F32( at + kBoneQuaternion + 4 ),
		    mdl.F32( at + kBoneQuaternion + 8 ), mdl.F32( at + kBoneQuaternion + 12 ) };
		bone.rotationEuler = mdl.Vec( at + kBoneEuler );
		bone.positionScale = mdl.Vec( at + kBonePosScale );
		bone.rotationScale = mdl.Vec( at + kBoneRotScale );
		for ( int r = 0; r < 3; ++r )
		{
			for ( int c = 0; c < 4; ++c )
			{
				bone.poseToBone.m[r][c] = mdl.F32( at + kBonePoseToBone + ( r * 4 + c ) * 4 );
			}
		}
		bone.flags = mdl.U32( at + kBoneFlags );
		if ( mdl.Failed() )
		{
			return mdl.Error();
		}
		model.bones.push_back( std::move( bone ) );
	}
	return std::nullopt;
}

// One value of a run-length animation stream at frame 0 (bone_setup.cpp
// ExtractAnimValue): runs of 'total' frames of which 'valid' carry a value;
// frames past the valid ones repeat the last.
float StreamValue( Reader &r, std::uint64_t at, float scale )
{
	std::uint8_t valid = r.U8( at );
	std::uint8_t total = r.U8( at + 1 );
	if ( total == 0 )
	{
		// A run of no frames: the engine steps over it to the next run.
		at += ( std::uint64_t( valid ) + 1 ) * 2;
		valid = r.U8( at );
		total = r.U8( at + 1 );
		if ( total == 0 )
		{
			return 0.0f;
		}
	}
	const std::uint64_t value = valid > 0 ? at + 2 : at + std::uint64_t( valid ) * 2;
	return float( r.I16( value ) ) * scale;
}

// The three streams of an mstudioanim_valueptr_t at 'at' (offsets relative
// to it; zero or negative: no stream, value 0).
Float3 StreamValues( Reader &r, std::uint64_t at, const Float3 &scale )
{
	float out[3] = {};
	const float scales[3] = { scale.x, scale.y, scale.z };
	for ( int k = 0; k < 3; ++k )
	{
		const std::int16_t offset = r.I16( at + std::uint64_t( k ) * 2 );
		if ( offset > 0 )
		{
			out[k] = StreamValue( r, at + std::uint64_t( offset ), scales[k] );
		}
	}
	return { out[0], out[1], out[2] };
}

Float3 Add( const Float3 &a, const Float3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// RLE bone x frame records from 'at' (CalcBoneQuaternion, CalcBonePosition).
bool DecodeRecords(
    Reader &r, std::uint64_t at, const std::vector<Bone> &bones, std::vector<BoneTransform> &pose )
{
	for ( ;; )
	{
		const std::uint8_t bone = r.U8( at );
		const std::uint8_t flags = r.U8( at + 1 );
		const std::int16_t next = r.I16( at + 2 );
		if ( r.Failed() )
		{
			return false;
		}
		if ( bone == 255 )
		{
			return true;
		}
		if ( bone >= bones.size() )
		{
			r.Fail( ModelStatus::BadIndex, at );
			return false;
		}
		const Bone &b = bones[bone];
		const bool delta = ( flags & kAnimRecordDelta ) != 0;
		const std::uint64_t data = at + 4;
		BoneTransform &out = pose[bone];
		if ( flags & kAnimRawRot )
		{
			out.rotation = Quaternion48( r, data );
		}
		else if ( flags & kAnimRawRot2 )
		{
			out.rotation = Quaternion64( r, data );
		}
		else if ( flags & kAnimAnimRot )
		{
			Float3 angles = StreamValues( r, data, b.rotationScale );
			if ( !delta )
			{
				angles = Add( angles, b.rotationEuler );
			}
			out.rotation = EulerQuaternion( angles );
		}
		else
		{
			out.rotation = delta ? Quaternion{} : b.rotation;
		}
		if ( flags & kAnimRawPos )
		{
			const std::uint64_t raw =
			    data + ( ( flags & kAnimRawRot ) ? 6 : 0 ) + ( ( flags & kAnimRawRot2 ) ? 8 : 0 );
			out.position = Vector48( r, raw );
		}
		else if ( flags & kAnimAnimPos )
		{
			const std::uint64_t streams = data + ( ( flags & kAnimAnimRot ) ? 6 : 0 );
			out.position = StreamValues( r, streams, b.positionScale );
			if ( !delta )
			{
				out.position = Add( out.position, b.position );
			}
		}
		else
		{
			out.position = delta ? Float3{} : b.position;
		}
		if ( r.Failed() )
		{
			return false;
		}
		if ( next == 0 )
		{
			return true;
		}
		if ( next < 0 )
		{
			r.Fail( ModelStatus::BadOffset, at + 2 );
			return false;
		}
		at += std::uint64_t( next );
	}
}

// Frame x bone data (STUDIO_FRAMEANIM) at 'at', frame 0.
bool DecodeFrameAnim(
    Reader &r, std::uint64_t at, const std::vector<Bone> &bones, std::vector<BoneTransform> &pose )
{
	std::uint64_t constants = r.Relative( at, at );
	std::uint64_t frame = r.Relative( at, at + 4 );
	if ( r.Failed() || !r.Has( at + kFrameAnimFlags, bones.size() ) )
	{
		return false;
	}
	const auto rotation = [&]( std::uint8_t flags, std::uint64_t where )
	{
		return ( flags & ( kFrameAnimRot2 | kFrameConstRot2 ) ) ? Quaternion48S( r, where )
		                                                        : Quaternion48( r, where );
	};
	const auto position = [&]( std::uint8_t flags, std::uint64_t where, std::uint64_t &size )
	{
		if ( flags & ( kFrameAnimPos2 | kFrameConstPos2 ) )
		{
			size = 12;
			return r.Vec( where );
		}
		size = 6;
		return Vector48( r, where );
	};
	for ( std::size_t i = 0; i < bones.size(); ++i )
	{
		const std::uint8_t flags = r.U8( at + kFrameAnimFlags + i );
		BoneTransform &out = pose[i];
		if ( flags & ( kFrameAnimRot | kFrameAnimRot2 ) )
		{
			out.rotation = rotation( flags, frame );
			frame += 6;
		}
		else if ( flags & ( kFrameConstRot | kFrameConstRot2 ) )
		{
			out.rotation = rotation( flags, constants );
			constants += 6;
		}
		std::uint64_t size = 0;
		if ( flags & ( kFrameAnimPos | kFrameAnimPos2 ) )
		{
			out.position = position( flags, frame, size );
			frame += size;
		}
		else if ( flags & ( kFrameConstPos | kFrameConstPos2 ) )
		{
			out.position = position( flags, constants, size );
			constants += size;
		}
		if ( r.Failed() )
		{
			return false;
		}
	}
	return true;
}

// The zero-frame data at 'at' (CalcZeroframeData), its first frame: per bone
// in order, a Vector48 run when it saves positions and a Quaternion64 run
// when it saves rotations, 'count' entries each.
bool DecodeZeroFrame( Reader &mdl, std::uint64_t at, std::uint32_t count,
    const std::vector<Bone> &bones, std::vector<BoneTransform> &pose )
{
	for ( std::size_t i = 0; i < bones.size(); ++i )
	{
		if ( bones[i].flags & kBoneSaveFrameRot32 )
		{
			// A later layout (Quaternion32 rotations) whose runs this reader
			// does not size: refused rather than misread.
			mdl.Fail( ModelStatus::UnsupportedVersion, at );
			return false;
		}
		if ( bones[i].flags & kBoneSaveFramePos )
		{
			pose[i].position = Vector48( mdl, at );
			at += 6 * std::uint64_t( count );
		}
		if ( bones[i].flags & kBoneSaveFrameRot )
		{
			pose[i].rotation = Quaternion64( mdl, at );
			at += 8 * std::uint64_t( count );
		}
		if ( mdl.Failed() )
		{
			return false;
		}
	}
	return true;
}

struct AnimBlocks
{
	std::uint32_t count = 0;
	std::uint64_t table = 0;
	std::string_view ani;
	ModelFile aniFile = ModelFile::Ani; // how errors in 'ani' are reported
};

// Frame 0 of the animation description at 'desc', every bone: its data, or the
// default (the reference; identity for a delta animation) where it has none.
std::optional<ModelError> DecodeFirstFrame( Reader &mdl, std::uint64_t desc,
    const AnimBlocks &blocks, const std::vector<Bone> &bones, std::vector<BoneTransform> &pose )
{
	const std::uint32_t flags = mdl.U32( desc + kAnimFlags );
	const bool delta = ( flags & kAnimDelta ) != 0;
	pose.clear();
	for ( const Bone &bone : bones )
	{
		pose.push_back( delta ? BoneTransform{} : BoneTransform{ bone.position, bone.rotation } );
	}
	std::int32_t block = mdl.I32( desc + kAnimBlock );
	std::uint64_t indexField = desc + kAnimBlock + 4;
	const std::int32_t sectionFrames = mdl.I32( desc + kAnimSectionFrames );
	if ( !mdl.Failed() && sectionFrames < 0 )
	{
		mdl.Fail( ModelStatus::BadCount, desc + kAnimSectionFrames );
	}
	if ( !mdl.Failed() && sectionFrames > 0 )
	{
		// Frame 0 is in section 0.
		const std::uint64_t section = mdl.Relative( desc, desc + kAnimSectionIndex );
		if ( mdl.Has( section, 8 ) )
		{
			block = mdl.I32( section );
			indexField = section + 4;
		}
	}
	if ( mdl.Failed() )
	{
		return mdl.Error();
	}
	const auto zeroFrame = [&]() -> std::optional<ModelError>
	{
		const std::int16_t count = mdl.I16( desc + kAnimZeroFrameCount );
		const std::int32_t index = mdl.I32( desc + kAnimZeroFrameIndex );
		if ( mdl.Failed() )
		{
			return mdl.Error();
		}
		if ( index == 0 )
		{
			return std::nullopt; // no zero-frame data: the defaults stand
		}
		if ( count < 1 )
		{
			mdl.Fail( ModelStatus::BadCount, desc + kAnimZeroFrameCount );
			return mdl.Error();
		}
		const std::uint64_t at = mdl.Relative( desc, desc + kAnimZeroFrameIndex );
		if ( mdl.Failed() || !DecodeZeroFrame( mdl, at, std::uint32_t( count ), bones, pose ) )
		{
			return mdl.Error();
		}
		return std::nullopt;
	};
	if ( block == -1 )
	{
		return zeroFrame(); // "model needs to be recompiled": the engine's fallback
	}
	if ( block == 0 )
	{
		const std::uint64_t at = mdl.Relative( desc, indexField );
		const bool ok =
		    !mdl.Failed() && ( ( flags & kAnimFrameAnim ) ? DecodeFrameAnim( mdl, at, bones, pose )
		                                                  : DecodeRecords( mdl, at, bones, pose ) );
		return ok ? std::nullopt : std::optional<ModelError>( mdl.Error() );
	}
	if ( block < 0 || std::uint32_t( block ) >= blocks.count )
	{
		mdl.Fail( ModelStatus::BadIndex, indexField - 4 );
		return mdl.Error();
	}
	const std::uint64_t entry = blocks.table + std::uint64_t( block ) * 8;
	const std::uint64_t start = mdl.Count( entry );
	const std::uint64_t end = mdl.Count( entry + 4 );
	const std::int32_t index = mdl.I32( indexField );
	if ( mdl.Failed() )
	{
		return mdl.Error();
	}
	if ( blocks.ani.empty() )
	{
		// The block is not loaded: the engine draws the zero-frame data.
		if ( mdl.I32( desc + kAnimZeroFrameIndex ) == 0 )
		{
			return ModelError{ ModelStatus::MissingFile, blocks.aniFile, 0 };
		}
		return zeroFrame();
	}
	if ( start > end || end > blocks.ani.size() )
	{
		mdl.Fail( ModelStatus::BadOffset, entry );
		return mdl.Error();
	}
	if ( index < 0 || std::uint64_t( index ) > end - start )
	{
		mdl.Fail( ModelStatus::BadOffset, indexField );
		return mdl.Error();
	}
	// The block's bytes end at its end: a read past it is Truncated in ani.
	Reader ani( blocks.ani.substr( 0, end ), blocks.aniFile );
	const std::uint64_t at = start + std::uint64_t( index );
	const bool ok = ( flags & kAnimFrameAnim ) ? DecodeFrameAnim( ani, at, bones, pose )
	                                           : DecodeRecords( ani, at, bones, pose );
	return ok ? std::nullopt : std::optional<ModelError>( ani.Error() );
}

// A sequence as its own model stores it: its record, its bone weights and
// the unweighted first frame of its animation, over that model's bones.
struct RawSequence
{
	Sequence sequence; // label, flags, box, blends, animation
	std::vector<float> weights;
	std::vector<BoneTransform> frame;
};

// Every local sequence of the model 'mdl' holds, over its bones 'bones'.
std::optional<ModelError> ReadRawSequences( Reader &mdl, std::string_view ani, ModelFile aniFile,
    const std::vector<Bone> &bones, std::vector<RawSequence> &out )
{
	const std::uint32_t numAnims = mdl.Count( kMdlNumLocalAnim );
	const std::uint64_t animIndex = mdl.Count( kMdlNumLocalAnim + 4 );
	const std::uint32_t numSeqs = mdl.Count( kMdlNumLocalAnim + 8 );
	const std::uint64_t seqIndex = mdl.Count( kMdlNumLocalAnim + 12 );
	AnimBlocks blocks;
	blocks.count = mdl.Count( kMdlAnimBlockName + 4 );
	blocks.table = mdl.Count( kMdlAnimBlockName + 8 );
	blocks.ani = ani;
	blocks.aniFile = aniFile;
	if ( mdl.Failed() || !mdl.Array( animIndex, numAnims, kAnimStride ) ||
	     !mdl.Array( seqIndex, numSeqs, kSeqStride ) ||
	     !mdl.Array( blocks.table, blocks.count, 8 ) )
	{
		return mdl.Error();
	}
	std::vector<std::optional<std::vector<BoneTransform>>> frames( numAnims );
	out.reserve( numSeqs );
	for ( std::uint32_t s = 0; s < numSeqs; ++s )
	{
		const std::uint64_t at = seqIndex + std::uint64_t( s ) * kSeqStride;
		RawSequence raw;
		Sequence &seq = raw.sequence;
		seq.label = mdl.String( mdl.Relative( at, at + kSeqLabel ), true );
		seq.flags = mdl.U32( at + kSeqFlags );
		seq.boundsMin = mdl.Vec( at + kSeqBbMin );
		seq.boundsMax = mdl.Vec( at + kSeqBbMax );
		seq.blendCount = mdl.I32( at + kSeqNumBlends );
		const std::uint64_t blends = mdl.Relative( at, at + kSeqAnimIndex );
		const std::uint64_t weights = mdl.Relative( at, at + kSeqWeightList );
		if ( mdl.Failed() )
		{
			return mdl.Error();
		}
		if ( seq.blendCount < 1 )
		{
			mdl.Fail( ModelStatus::BadCount, at + kSeqNumBlends );
			return mdl.Error();
		}
		seq.animation = mdl.I16( blends );
		if ( !mdl.Failed() && ( seq.animation < 0 || std::uint32_t( seq.animation ) >= numAnims ) )
		{
			mdl.Fail( ModelStatus::BadIndex, blends );
		}
		if ( mdl.Failed() || !mdl.Array( weights, bones.size(), 4 ) )
		{
			return mdl.Error();
		}
		raw.weights.reserve( bones.size() );
		for ( std::size_t i = 0; i < bones.size(); ++i )
		{
			raw.weights.push_back( mdl.F32( weights + i * 4 ) );
		}
		std::optional<std::vector<BoneTransform>> &frame = frames[std::size_t( seq.animation )];
		if ( !frame )
		{
			std::vector<BoneTransform> decoded;
			const std::uint64_t desc = animIndex + std::uint64_t( seq.animation ) * kAnimStride;
			if ( std::optional<ModelError> error =
			         DecodeFirstFrame( mdl, desc, blocks, bones, decoded ) )
			{
				return error;
			}
			frame = std::move( decoded );
		}
		raw.frame = *frame;
		out.push_back( std::move( raw ) );
	}
	return std::nullopt;
}

// The sequence over 'bones' (this model's): bone i takes the raw sequence's
// bone map[i] (-1: none, the reference), blended by that bone's weight
// (bone_setup.cpp: InitPose, then SlerpBones; a delta adds its weighted
// frame to the reference, a post delta after it).
Sequence Combine(
    const std::vector<Bone> &bones, const std::vector<std::int32_t> &map, const RawSequence &raw )
{
	Sequence seq = raw.sequence;
	const bool delta = ( seq.flags & kDeltaSequence ) != 0;
	const bool post = ( seq.flags & kSeqPost ) != 0;
	seq.firstFrame.reserve( bones.size() );
	seq.boneWeights.reserve( bones.size() );
	for ( std::size_t i = 0; i < bones.size(); ++i )
	{
		const BoneTransform reference{ bones[i].position, bones[i].rotation };
		const std::int32_t j = map[i];
		const float w = j >= 0 ? raw.weights[std::size_t( j )] : 0.0f;
		seq.boneWeights.push_back( w );
		BoneTransform out = reference;
		if ( !( w > 0.0f ) )
		{
			// no influence: the reference
		}
		else if ( delta )
		{
			const BoneTransform &d = raw.frame[std::size_t( j )];
			const Quaternion scaled = Scaled( d.rotation, std::min( w, 1.0f ) );
			out.rotation = Normalized( post ? Multiply( reference.rotation, scaled )
			                                : Multiply( scaled, reference.rotation ) );
			out.position = { reference.position.x + d.position.x * w,
			    reference.position.y + d.position.y * w, reference.position.z + d.position.z * w };
		}
		else if ( w >= 1.0f )
		{
			out = raw.frame[std::size_t( j )];
		}
		else
		{
			const BoneTransform &a = raw.frame[std::size_t( j )];
			out.rotation = Slerp( reference.rotation, a.rotation, w );
			out.position = { reference.position.x + ( a.position.x - reference.position.x ) * w,
			    reference.position.y + ( a.position.y - reference.position.y ) * w,
			    reference.position.z + ( a.position.z - reference.position.z ) * w };
		}
		seq.firstFrame.push_back( out );
	}
	return seq;
}

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() && std::equal( a.begin(), a.end(), b.begin(),
	                                   []( char x, char y )
	                                   {
		                                   return std::tolower( static_cast<unsigned char>( x ) ) ==
		                                          std::tolower( static_cast<unsigned char>( y ) );
	                                   } );
}

// Adds 'raw' (from group 'group', its bones mapped by 'map') unless a sequence
// of its label is already there and is not a forward declaration.
void AppendSequence(
    Model &model, const std::vector<std::int32_t> &map, const RawSequence &raw, std::int32_t group )
{
	Sequence seq = Combine( model.bones, map, raw );
	seq.group = group;
	for ( Sequence &existing : model.sequences )
	{
		if ( EqualsNoCase( existing.label, seq.label ) )
		{
			if ( existing.flags & kOverrideSequence )
			{
				existing = std::move( seq );
			}
			return;
		}
	}
	model.sequences.push_back( std::move( seq ) );
}

std::optional<ModelError> ReadSequences( Reader &mdl, const ModelBytes &bytes, Model &model )
{
	std::vector<RawSequence> own;
	if ( std::optional<ModelError> error =
	         ReadRawSequences( mdl, bytes.ani, ModelFile::Ani, model.bones, own ) )
	{
		return error;
	}
	std::vector<std::int32_t> identity( model.bones.size() );
	for ( std::size_t i = 0; i < identity.size(); ++i )
	{
		identity[i] = static_cast<std::int32_t>( i );
	}
	model.sequences.reserve( own.size() );
	for ( const RawSequence &raw : own )
	{
		AppendSequence( model, identity, raw, 0 );
	}

	// The $includemodel list, as stored.
	const std::uint32_t numIncludes = mdl.Count( kMdlNumIncludes );
	const std::uint64_t includeIndex = mdl.Count( kMdlNumIncludes + 4 );
	if ( mdl.Failed() || !mdl.Array( includeIndex, numIncludes, kIncludeStride ) )
	{
		return mdl.Error();
	}
	for ( std::uint32_t i = 0; i < numIncludes; ++i )
	{
		const std::uint64_t at = includeIndex + std::uint64_t( i ) * kIncludeStride;
		model.includeModels.push_back( Slashes( mdl.String( mdl.Relative( at, at + 4 ) ) ) );
		if ( mdl.Failed() )
		{
			return mdl.Error();
		}
	}

	// The included models' sequences, group by group.
	for ( std::size_t g = 0; g < bytes.includes.size(); ++g )
	{
		Reader include( bytes.includes[g].mdl, ModelFile::IncludeMdl );
		if ( !include.Has( 0, kMdlHeaderSize ) )
		{
			return include.Error();
		}
		if ( include.U32( 0 ) != 0x54534449u ) // "IDST"
		{
			include.Fail( ModelStatus::BadMagic, 0 );
			return include.Error();
		}
		const std::int32_t version = include.I32( kMdlVersion );
		if ( version < kMinMdlVersion || version > kMaxMdlVersion )
		{
			include.Fail( ModelStatus::UnsupportedVersion, kMdlVersion );
			return include.Error();
		}
		Model skeleton;
		if ( std::optional<ModelError> error = ReadBones( include, skeleton ) )
		{
			return error;
		}
		std::vector<RawSequence> theirs;
		if ( std::optional<ModelError> error = ReadRawSequences(
		         include, bytes.includes[g].ani, ModelFile::IncludeAni, skeleton.bones, theirs ) )
		{
			return error;
		}
		// This model's bone i is the included model's bone of the same name.
		std::vector<std::int32_t> map( model.bones.size(), -1 );
		for ( std::size_t b = 0; b < model.bones.size(); ++b )
		{
			for ( std::size_t k = 0; k < skeleton.bones.size(); ++k )
			{
				if ( EqualsNoCase( model.bones[b].name, skeleton.bones[k].name ) )
				{
					map[b] = static_cast<std::int32_t>( k );
					break;
				}
			}
		}
		for ( const RawSequence &raw : theirs )
		{
			AppendSequence( model, map, raw, static_cast<std::int32_t>( g + 1 ) );
		}
	}
	return std::nullopt;
}

// A VVD record's bone weights; each used bone must name one of 'numBones'.
BoneWeights ReadWeights( Reader &vvd, std::uint64_t record, std::size_t numBones )
{
	BoneWeights w;
	w.count = vvd.U8( record + 15 );
	if ( !vvd.Failed() && ( w.count < 1 || w.count > 3 ) )
	{
		vvd.Fail( ModelStatus::BadCount, record + 15 );
		return w;
	}
	for ( std::uint8_t k = 0; k < 3; ++k )
	{
		w.weights[k] = vvd.F32( record + 4u * k );
		w.bones[k] = vvd.U8( record + 12 + k );
		if ( k < w.count && !vvd.Failed() && w.bones[k] >= numBones )
		{
			vvd.Fail( ModelStatus::BadIndex, record + 12 + k );
		}
	}
	return w;
}

} // namespace

std::size_t Model::TriangleCount() const
{
	std::size_t count = 0;
	for ( const Mesh &mesh : meshes )
	{
		count += mesh.indices.size() / 3;
	}
	return count;
}

const char *StatusName( ModelStatus status )
{
	switch ( status )
	{
	case ModelStatus::MissingFile:
		return "MissingFile";
	case ModelStatus::BadMagic:
		return "BadMagic";
	case ModelStatus::UnsupportedVersion:
		return "UnsupportedVersion";
	case ModelStatus::Truncated:
		return "Truncated";
	case ModelStatus::BadCount:
		return "BadCount";
	case ModelStatus::BadOffset:
		return "BadOffset";
	case ModelStatus::BadIndex:
		return "BadIndex";
	case ModelStatus::ChecksumMismatch:
		return "ChecksumMismatch";
	}
	return "?";
}

const char *FileName( ModelFile file )
{
	switch ( file )
	{
	case ModelFile::Mdl:
		return "mdl";
	case ModelFile::Vvd:
		return "vvd";
	case ModelFile::Vtx:
		return "vtx";
	case ModelFile::Ani:
		return "ani";
	case ModelFile::IncludeMdl:
		return "included mdl";
	case ModelFile::IncludeAni:
		return "included ani";
	}
	return "?";
}

std::string Describe( const ModelError &error )
{
	return std::string( StatusName( error.status ) ) + " in " + FileName( error.file ) +
	       " at byte " + std::to_string( error.offset );
}

static foundation::Expected<Model, ModelError> ParseModelImpl( const ModelBytes &bytes,
    std::int32_t body, bool allBodies, std::optional<std::uint32_t> selectedLod )
{
	using foundation::MakeUnexpected;
	Reader mdl( bytes.mdl, ModelFile::Mdl );
	Reader vvd( bytes.vvd, ModelFile::Vvd );
	Reader vtx( bytes.vtx, ModelFile::Vtx );

	// --- MDL header ---
	if ( !mdl.Has( 0, kMdlHeaderSize ) )
	{
		return MakeUnexpected( mdl.Error() );
	}
	if ( mdl.U32( 0 ) != 0x54534449u ) // "IDST"
	{
		mdl.Fail( ModelStatus::BadMagic, 0 );
		return MakeUnexpected( mdl.Error() );
	}
	Model model;
	model.version = mdl.I32( kMdlVersion );
	if ( model.version < kMinMdlVersion || model.version > kMaxMdlVersion )
	{
		mdl.Fail( ModelStatus::UnsupportedVersion, kMdlVersion );
		return MakeUnexpected( mdl.Error() );
	}
	const bool v49 = model.version == 49;
	model.checksum = mdl.I32( kMdlChecksum );
	{
		const std::string_view raw = bytes.mdl.substr( kMdlName, kMdlNameSize );
		model.name = std::string( raw.substr( 0, raw.find( '\0' ) ) );
	}
	model.flags = mdl.U32( kMdlFlags );
	const std::uint32_t flexCount = mdl.Count( kMdlNumFlexDesc );
	const std::uint64_t flexDescriptors = mdl.Count( kMdlNumFlexDesc + 4 );
	if ( !mdl.Array( flexDescriptors, flexCount, 4 ) )
		return MakeUnexpected( mdl.Error() );
	for ( std::uint32_t i = 0; i < flexCount; ++i )
	{
		const std::uint64_t at = flexDescriptors + std::uint64_t( i ) * 4;
		model.flexDescriptors.push_back( mdl.String( mdl.Relative( at, at ) ) );
	}
	model.hullMins = mdl.Vec( kMdlHullMin );
	model.hullMaxs = mdl.Vec( kMdlHullMax );
	model.viewMins = mdl.Vec( kMdlViewMin );
	model.viewMaxs = mdl.Vec( kMdlViewMax );
	MdlHeader h;
	h.numTextures = mdl.Count( kMdlNumTextures );
	h.textureIndex = mdl.Count( kMdlNumTextures + 4 );
	h.numCdTextures = mdl.Count( kMdlNumTextures + 8 );
	h.cdTextureIndex = mdl.Count( kMdlNumTextures + 12 );
	h.numSkinRef = mdl.Count( kMdlNumTextures + 16 );
	h.numSkinFamilies = mdl.Count( kMdlNumTextures + 20 );
	h.skinIndex = mdl.Count( kMdlNumTextures + 24 );
	h.numBodyParts = mdl.Count( kMdlNumTextures + 28 );
	h.bodyPartIndex = mdl.Count( kMdlNumTextures + 32 );
	if ( mdl.Failed() )
	{
		return MakeUnexpected( mdl.Error() );
	}

	// --- Textures, material directories, skins ---
	if ( !mdl.Array( h.textureIndex, h.numTextures, kTextureStride ) )
	{
		return MakeUnexpected( mdl.Error() );
	}
	for ( std::uint64_t i = 0; i < h.numTextures; ++i )
	{
		const std::uint64_t texture = h.textureIndex + i * kTextureStride;
		const std::uint64_t name = mdl.Relative( texture, texture );
		model.textures.push_back( Slashes( mdl.String( name ) ) );
	}
	if ( !mdl.Array( h.cdTextureIndex, h.numCdTextures, 4 ) )
	{
		return MakeUnexpected( mdl.Error() );
	}
	for ( std::uint64_t i = 0; i < h.numCdTextures; ++i )
	{
		const std::uint64_t name = mdl.Relative( 0, h.cdTextureIndex + i * 4 );
		model.cdMaterials.push_back( MaterialDirectory( mdl.String( name ) ) );
	}
	if ( !mdl.Array( h.skinIndex, std::uint64_t( h.numSkinFamilies ) * h.numSkinRef, 2 ) )
	{
		return MakeUnexpected( mdl.Error() );
	}
	for ( std::uint64_t f = 0; f < h.numSkinFamilies; ++f )
	{
		std::vector<std::int16_t> family;
		for ( std::uint64_t r = 0; r < h.numSkinRef; ++r )
		{
			const std::uint64_t at = h.skinIndex + ( f * h.numSkinRef + r ) * 2;
			const auto texture = static_cast<std::int16_t>( mdl.U16( at ) );
			if ( texture < 0 || static_cast<std::uint32_t>( texture ) >= h.numTextures )
			{
				mdl.Fail( ModelStatus::BadIndex, at );
			}
			family.push_back( texture );
		}
		model.skinFamilies.push_back( std::move( family ) );
	}
	if ( mdl.Failed() )
	{
		return MakeUnexpected( mdl.Error() );
	}

	// --- Skeleton (before the vertices, whose weights name bones) ---
	if ( std::optional<ModelError> error = ReadBones( mdl, model ) )
	{
		return MakeUnexpected( *error );
	}

	// --- VVD and VTX headers ---
	VvdVertices vertices;
	if ( std::optional<ModelError> error = ReadVvd( vvd, model.checksum, vertices ) )
	{
		return MakeUnexpected( *error );
	}
	if ( !vtx.Has( 0, kVtxHeaderSize ) )
	{
		return MakeUnexpected( vtx.Error() );
	}
	if ( vtx.I32( 0 ) != kVtxVersion )
	{
		vtx.Fail( ModelStatus::UnsupportedVersion, 0 );
		return MakeUnexpected( vtx.Error() );
	}
	if ( vtx.I32( 16 ) != model.checksum )
	{
		vtx.Fail( ModelStatus::ChecksumMismatch, 16 );
		return MakeUnexpected( vtx.Error() );
	}
	const std::uint32_t lodCount = vtx.Count( 20 );
	if ( lodCount < 1 || lodCount > kVvdMaxLods )
	{
		vtx.Fail( ModelStatus::BadCount, 20 );
		return MakeUnexpected( vtx.Error() );
	}
	if ( selectedLod && *selectedLod >= lodCount )
	{
		vtx.Fail( ModelStatus::BadIndex, 20 );
		return MakeUnexpected( vtx.Error() );
	}
	model.lodTextures.assign( lodCount, model.textures );
	const std::uint64_t replacementLists = vtx.Relative( 0, 24 );
	if ( replacementLists )
	{
		if ( !vtx.Array( replacementLists, lodCount, 8 ) )
			return MakeUnexpected( vtx.Error() );
		for ( std::uint32_t lod = 0; lod < lodCount; ++lod )
		{
			const std::uint64_t list = replacementLists + std::uint64_t( lod ) * 8;
			const std::uint32_t count = vtx.Count( list );
			const std::uint64_t replacements = vtx.Relative( list, list + 4 );
			if ( vtx.Failed() || !vtx.Array( replacements, count, 6 ) )
				return MakeUnexpected( vtx.Error() );
			std::vector<bool> replaced( model.textures.size() );
			for ( std::uint32_t i = 0; i < count; ++i )
			{
				const std::uint64_t at = replacements + std::uint64_t( i ) * 6;
				const std::uint16_t material = vtx.U16( at );
				if ( material >= model.textures.size() || replaced[material] )
				{
					vtx.Fail( ModelStatus::BadIndex, at );
					return MakeUnexpected( vtx.Error() );
				}
				const std::uint64_t name = vtx.Relative( at, at + 2 );
				model.lodTextures[lod][material] = Slashes( vtx.String( name ) );
				replaced[material] = true;
			}
		}
	}
	if ( vtx.Failed() )
		return MakeUnexpected( vtx.Error() );
	const std::uint32_t vtxBodyParts = vtx.Count( 28 );
	const std::uint64_t vtxBodyPartIndex = vtx.Relative( 0, 32 );
	if ( vtx.Failed() )
	{
		return MakeUnexpected( vtx.Error() );
	}
	if ( vtxBodyParts != h.numBodyParts )
	{
		vtx.Fail( ModelStatus::BadCount, 28 );
		return MakeUnexpected( vtx.Error() );
	}
	if ( !mdl.Array( h.bodyPartIndex, h.numBodyParts, kBodyPartStride ) ||
	     !vtx.Array( vtxBodyPartIndex, vtxBodyParts, kVtxBodyPartStride ) )
	{
		return MakeUnexpected( mdl.Failed() ? mdl.Error() : vtx.Error() );
	}

	// --- Body parts: selected submodels/LODs, or every geometry variant ---
	bool anyVertex = false;
	for ( std::uint32_t p = 0; p < h.numBodyParts; ++p )
	{
		const std::uint64_t part = h.bodyPartIndex + std::uint64_t( p ) * kBodyPartStride;
		const std::uint32_t numModels = mdl.Count( part + 4 );
		const std::int32_t base = mdl.I32( part + 8 );
		const std::uint64_t models = mdl.Relative( part, part + 12 );
		const std::uint64_t vtxPart = vtxBodyPartIndex + std::uint64_t( p ) * kVtxBodyPartStride;
		const std::uint32_t vtxModels = vtx.Count( vtxPart );
		const std::uint64_t vtxModelIndex = vtx.Relative( vtxPart, vtxPart + 4 );
		if ( mdl.Failed() || vtx.Failed() )
		{
			return MakeUnexpected( mdl.Failed() ? mdl.Error() : vtx.Error() );
		}
		if ( vtxModels != numModels )
		{
			vtx.Fail( ModelStatus::BadCount, vtxPart );
			return MakeUnexpected( vtx.Error() );
		}
		model.bodyParts.push_back(
		    { base > 0 ? static_cast<std::uint32_t>( base ) : 1u, numModels } );
		if ( numModels == 0 )
		{
			continue;
		}
		if ( base <= 0 )
		{
			mdl.Fail( ModelStatus::BadCount, part + 8 );
			return MakeUnexpected( mdl.Error() );
		}
		const std::uint32_t selected = model.bodyParts.back().SelectedModel( body );
		const std::uint32_t firstModel = allBodies ? 0u : selected;
		const std::uint32_t endModel = allBodies ? numModels : selected + 1u;
		for ( std::uint32_t index = firstModel; index < endModel; ++index )
		{
			const std::uint64_t mdlModel = models + std::uint64_t( index ) * kModelStride;
			const std::uint64_t vtxModel = vtxModelIndex + std::uint64_t( index ) * kVtxModelStride;
			if ( !mdl.Has( mdlModel, kModelStride ) || !vtx.Has( vtxModel, kVtxModelStride ) )
			{
				return MakeUnexpected( mdl.Failed() ? mdl.Error() : vtx.Error() );
			}
			const std::uint32_t numMeshes = mdl.Count( mdlModel + kModelNumMeshes );
			const std::uint64_t meshes = mdl.Relative( mdlModel, mdlModel + kModelNumMeshes + 4 );
			const std::int32_t vertexIndex = mdl.I32( mdlModel + kModelNumMeshes + 12 );
			const std::uint32_t numLods = vtx.Count( vtxModel );
			const std::uint64_t lods = vtx.Relative( vtxModel, vtxModel + 4 );
			if ( mdl.Failed() || vtx.Failed() )
			{
				return MakeUnexpected( mdl.Failed() ? mdl.Error() : vtx.Error() );
			}
			if ( vertexIndex < 0 ||
			     vertexIndex % static_cast<std::int32_t>( kVvdVertexStride ) != 0 )
			{
				mdl.Fail( ModelStatus::BadOffset, mdlModel + kModelNumMeshes + 12 );
				return MakeUnexpected( mdl.Error() );
			}
			if ( numMeshes == 0 )
			{
				continue;
			}
			if ( numLods < 1 )
			{
				vtx.Fail( ModelStatus::BadCount, vtxModel );
				return MakeUnexpected( vtx.Error() );
			}
			if ( !vtx.Array( lods, numLods, kVtxLodStride ) )
			{
				return MakeUnexpected( vtx.Error() );
			}
			if ( numLods != model.lodTextures.size() )
			{
				vtx.Fail( ModelStatus::BadCount, vtxModel );
				return MakeUnexpected( vtx.Error() );
			}
			const std::uint32_t firstLod = selectedLod.value_or( 0u );
			const std::uint32_t endLod = selectedLod ? firstLod + 1u : numLods;
			for ( std::uint32_t lod = firstLod; lod < endLod; ++lod )
			{
				const std::uint64_t lodAt = lods + std::uint64_t( lod ) * kVtxLodStride;

				const std::uint32_t vtxMeshCount = vtx.Count( lodAt );
				const std::uint64_t vtxMeshes = vtx.Relative( lodAt, lodAt + 4 );
				if ( vtx.Failed() )
				{
					return MakeUnexpected( vtx.Error() );
				}
				if ( vtxMeshCount != numMeshes )
				{
					vtx.Fail( ModelStatus::BadCount, lodAt );
					return MakeUnexpected( vtx.Error() );
				}
				if ( !mdl.Array( meshes, numMeshes, kMeshStride ) ||
				     !vtx.Array( vtxMeshes, numMeshes, kVtxMeshStride ) )
				{
					return MakeUnexpected( mdl.Failed() ? mdl.Error() : vtx.Error() );
				}
				const std::uint64_t modelFirst =
				    static_cast<std::uint64_t>( vertexIndex ) / kVvdVertexStride;
				for ( std::uint32_t m = 0; m < numMeshes; ++m )
				{
					const std::uint64_t mesh = meshes + std::uint64_t( m ) * kMeshStride;
					Mesh out;
					out.bodyPart = static_cast<std::int32_t>( p );
					out.bodyModel = index;
					out.lod = lod;
					out.textureRef = mdl.I32( mesh );
					const std::uint32_t meshVertices = mdl.Count( mesh + 8 );
					const std::int32_t vertexOffset = mdl.I32( mesh + 12 );
					if ( !ReadFlexes( mdl, model, mesh, meshVertices, out.flexes ) )
						return MakeUnexpected( mdl.Error() );
					if ( mdl.Failed() )
					{
						return MakeUnexpected( mdl.Error() );
					}
					if ( out.textureRef < 0 ||
					     static_cast<std::uint32_t>( out.textureRef ) >= h.numSkinRef )
					{
						mdl.Fail( ModelStatus::BadIndex, mesh );
						return MakeUnexpected( mdl.Error() );
					}
					if ( vertexOffset < 0 ||
					     modelFirst + static_cast<std::uint64_t>( vertexOffset ) + meshVertices >
					         vertices.records.size() )
					{
						mdl.Fail( ModelStatus::BadIndex, mesh + 12 );
						return MakeUnexpected( mdl.Error() );
					}
					const std::uint64_t first =
					    modelFirst + static_cast<std::uint64_t>( vertexOffset );

					const std::uint64_t vtxMesh = vtxMeshes + std::uint64_t( m ) * kVtxMeshStride;
					const std::uint32_t numGroups = vtx.Count( vtxMesh );
					const std::uint64_t groups = vtx.Relative( vtxMesh, vtxMesh + 4 );
					const std::uint64_t groupStride =
					    v49 ? kVtxStripGroupStride49 : kVtxStripGroupStride;
					if ( vtx.Failed() || !vtx.Array( groups, numGroups, groupStride ) )
					{
						return MakeUnexpected( vtx.Error() );
					}
					for ( std::uint32_t g = 0; g < numGroups; ++g )
					{
						std::vector<std::uint32_t> groupVertices;
						if ( !ReadStripGroup( vtx, groups + std::uint64_t( g ) * groupStride, v49,
						         meshVertices, out.indices, allBodies ? &groupVertices : nullptr ) )
						{
							return MakeUnexpected( vtx.Error() );
						}
						if ( allBodies )
						{
							// Every strip group, in the order vrad and the engine's
							// static-prop colour meshes walk them.
							VertexColorGroup colorGroup;
							colorGroup.bodyPart = out.bodyPart;
							colorGroup.bodyModel = index;
							colorGroup.lod = lod;
							colorGroup.vertices.reserve( groupVertices.size() );
							for ( std::uint32_t id : groupVertices )
								colorGroup.vertices.push_back(
								    ReadVertex( vvd, vertices.records[first + id], vertices ) );
							model.vertexColorGroups.push_back( std::move( colorGroup ) );
						}
					}
					if ( out.indices.empty() )
					{
						continue;
					}
					out.vertices.reserve( meshVertices );
					out.weights.reserve( meshVertices );
					for ( std::uint64_t v = 0; v < meshVertices; ++v )
					{
						const std::uint64_t record = vertices.records[first + v];
						out.vertices.push_back( ReadVertex( vvd, record, vertices ) );
						out.weights.push_back( ReadWeights( vvd, record, model.bones.size() ) );
					}
					if ( vvd.Failed() )
					{
						return MakeUnexpected( vvd.Error() );
					}
					for ( std::uint32_t index : out.indices )
					{
						Extend( model.mins, model.maxs, out.vertices[index].position, !anyVertex );
						anyVertex = true;
					}
					model.meshes.push_back( std::move( out ) );
				}
			}
		}
	}

	// --- Sequences and their first frames ---
	if ( !allBodies )
	{
		if ( std::optional<ModelError> error = ReadSequences( mdl, bytes, model ) )
			return MakeUnexpected( *error );
	}
	return model;
}

foundation::Expected<Model, ModelError> ParseModel(
    const ModelBytes &bytes, std::int32_t body, std::uint32_t lod )
{
	return ParseModelImpl( bytes, body, false, lod );
}

foundation::Expected<Model, ModelError> ParseModelGeometryVariants( const ModelBytes &bytes )
{
	return ParseModelImpl( bytes, 0, true, std::nullopt );
}

std::string CanonicalModelPath( std::string_view path )
{
	std::string out = Slashes( std::string( path ) );
	for ( char &c : out )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	while ( !out.empty() && out.front() == '/' )
	{
		out.erase( out.begin() );
	}
	return out;
}

namespace
{

// Reads the .ani file 'mdl' names, when it names animation blocks.
void ReadAniFile( const IModelFiles &files, std::string_view mdl, std::string &ani )
{
	Reader header( mdl, ModelFile::Mdl );
	const std::int32_t blocks = header.I32( kMdlAnimBlockName + 4 );
	const std::int32_t nameAt = header.I32( kMdlAnimBlockName );
	if ( !header.Failed() && blocks > 0 && nameAt > 0 )
	{
		const std::string name = header.String( std::uint64_t( nameAt ) );
		if ( !header.Failed() && !name.empty() )
		{
			(void)files.Read( CanonicalModelPath( name ), ani );
		}
	}
}

// Appends the models 'mdl' includes, each followed by its own includes (the
// engine's group order), with their .ani files. A malformed list, an absent
// file, a model already visited or a nesting deeper than 8 adds nothing.
void CollectIncludes( const IModelFiles &files, std::string_view mdl, int depth,
    std::vector<std::string> &visited, std::vector<std::pair<std::string, std::string>> &out )
{
	constexpr int kMaxDepth = 8;
	Reader header( mdl, ModelFile::Mdl );
	const std::uint32_t count = header.Count( kMdlNumIncludes );
	const std::uint64_t index = header.Count( kMdlNumIncludes + 4 );
	if ( depth >= kMaxDepth || header.Failed() || !header.Array( index, count, kIncludeStride ) )
	{
		return;
	}
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		const std::uint64_t at = index + std::uint64_t( i ) * kIncludeStride;
		const std::string name =
		    CanonicalModelPath( header.String( header.Relative( at, at + 4 ) ) );
		if ( header.Failed() )
		{
			return;
		}
		if ( std::find( visited.begin(), visited.end(), name ) != visited.end() )
		{
			continue;
		}
		visited.push_back( name );
		std::string bytes;
		if ( !files.Read( name, bytes ) )
		{
			continue;
		}
		std::string ani;
		ReadAniFile( files, bytes, ani );
		out.emplace_back( std::move( bytes ), std::move( ani ) );
		const std::string nested = out.back().first;
		CollectIncludes( files, nested, depth + 1, visited, out );
	}
}

} // namespace

foundation::Expected<Model, ModelError> LoadModel(
    const IModelFiles &files, std::string_view mdlPath, std::int32_t body )
{
	using foundation::MakeUnexpected;
	const std::string path = CanonicalModelPath( mdlPath );
	constexpr std::string_view kExtension = ".mdl";
	if ( path.size() <= kExtension.size() ||
	     path.compare( path.size() - kExtension.size(), kExtension.size(), kExtension ) != 0 )
	{
		return MakeUnexpected( ModelError{ ModelStatus::MissingFile, ModelFile::Mdl, 0 } );
	}
	const std::string stem = path.substr( 0, path.size() - kExtension.size() );
	std::string mdl, vvd, vtx;
	if ( !files.Read( path, mdl ) )
	{
		return MakeUnexpected( ModelError{ ModelStatus::MissingFile, ModelFile::Mdl, 0 } );
	}
	if ( !files.Read( stem + ".vvd", vvd ) )
	{
		return MakeUnexpected( ModelError{ ModelStatus::MissingFile, ModelFile::Vvd, 0 } );
	}
	bool haveVtx = false;
	for ( const char *suffix : { ".dx90.vtx", ".vtx", ".dx80.vtx", ".sw.vtx" } )
	{
		if ( files.Read( stem + suffix, vtx ) )
		{
			haveVtx = true;
			break;
		}
	}
	if ( !haveVtx )
	{
		return MakeUnexpected( ModelError{ ModelStatus::MissingFile, ModelFile::Vtx, 0 } );
	}
	// The animation-block file the header names (its absence is decided where
	// a first frame needs it) and the included models, depth first.
	std::string ani;
	ReadAniFile( files, mdl, ani );
	std::vector<std::pair<std::string, std::string>> included;
	std::vector<std::string> visited{ path };
	CollectIncludes( files, mdl, 0, visited, included );
	ModelBytes bytes{ mdl, vvd, vtx, ani };
	for ( const auto &[includeMdl, includeAni] : included )
	{
		bytes.includes.push_back( { includeMdl, includeAni } );
	}
	return ParseModel( bytes, body );
}

std::vector<ResolvedMaterial> ResolveMaterials( const Model &model, const IModelFiles &files )
{
	std::vector<ResolvedMaterial> out;
	out.reserve( model.textures.size() );
	for ( const std::string &texture : model.textures )
	{
		ResolvedMaterial resolved;
		for ( const std::string &directory : model.cdMaterials )
		{
			const std::string candidate = directory + texture;
			if ( files.Exists( "materials/" + candidate + ".vmt" ) )
			{
				resolved = { candidate, true };
				break;
			}
		}
		if ( !resolved.found )
		{
			resolved.name =
			    ( model.cdMaterials.empty() ? std::string() : model.cdMaterials.front() ) + texture;
		}
		out.push_back( std::move( resolved ) );
	}
	return out;
}

std::int32_t TextureIndex( const Model &model, const Mesh &mesh, std::int32_t skin )
{
	if ( model.skinFamilies.empty() )
	{
		return -1;
	}
	const std::size_t family =
	    skin >= 0 && static_cast<std::size_t>( skin ) < model.skinFamilies.size()
	        ? static_cast<std::size_t>( skin )
	        : 0;
	const std::vector<std::int16_t> &row = model.skinFamilies[family];
	if ( mesh.textureRef < 0 || static_cast<std::size_t>( mesh.textureRef ) >= row.size() )
	{
		return -1;
	}
	return row[static_cast<std::size_t>( mesh.textureRef )];
}

std::int32_t FindSequence( const Model &model, std::string_view label )
{
	for ( std::size_t i = 0; i < model.sequences.size(); ++i )
	{
		if ( EqualsNoCase( model.sequences[i].label, label ) )
		{
			return static_cast<std::int32_t>( i );
		}
	}
	return -1;
}

std::vector<BoneTransform> ReferencePose( const Model &model )
{
	std::vector<BoneTransform> pose;
	pose.reserve( model.bones.size() );
	for ( const Bone &bone : model.bones )
	{
		pose.push_back( { bone.position, bone.rotation } );
	}
	return pose;
}

Matrix3x4 TransformMatrix( const Quaternion &q, const Float3 &p )
{
	// QuaternionMatrix.
	Matrix3x4 m;
	m.m[0] = { 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z, 2.0f * q.x * q.y - 2.0f * q.w * q.z,
	    2.0f * q.x * q.z + 2.0f * q.w * q.y, p.x };
	m.m[1] = { 2.0f * q.x * q.y + 2.0f * q.w * q.z, 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z,
	    2.0f * q.y * q.z - 2.0f * q.w * q.x, p.y };
	m.m[2] = { 2.0f * q.x * q.z - 2.0f * q.w * q.y, 2.0f * q.y * q.z + 2.0f * q.w * q.x,
	    1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y, p.z };
	return m;
}

Matrix3x4 Concat( const Matrix3x4 &a, const Matrix3x4 &b )
{
	Matrix3x4 out;
	for ( int r = 0; r < 3; ++r )
	{
		for ( int c = 0; c < 4; ++c )
		{
			float v = a.m[r][0] * b.m[0][c] + a.m[r][1] * b.m[1][c] + a.m[r][2] * b.m[2][c];
			if ( c == 3 )
			{
				v += a.m[r][3];
			}
			out.m[r][c] = v;
		}
	}
	return out;
}

Float3 TransformPoint( const Matrix3x4 &m, const Float3 &p )
{
	return { m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2] * p.z + m.m[0][3],
	    m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2] * p.z + m.m[1][3],
	    m.m[2][0] * p.x + m.m[2][1] * p.y + m.m[2][2] * p.z + m.m[2][3] };
}

std::vector<Matrix3x4> BoneToModel( const Model &model, std::span<const BoneTransform> pose )
{
	std::vector<Matrix3x4> out;
	if ( pose.size() != model.bones.size() )
	{
		return out;
	}
	out.reserve( pose.size() );
	for ( std::size_t i = 0; i < pose.size(); ++i )
	{
		const Matrix3x4 local = TransformMatrix( pose[i].rotation, pose[i].position );
		const std::int32_t parent = model.bones[i].parent;
		// ReadBones guarantees a parent precedes its child.
		out.push_back( parent >= 0 ? Concat( out[std::size_t( parent )], local ) : local );
	}
	return out;
}

Model PoseModel( const Model &model, std::span<const BoneTransform> pose )
{
	const std::vector<Matrix3x4> boneToModel = BoneToModel( model, pose );
	if ( boneToModel.empty() )
	{
		return model;
	}
	std::vector<Matrix3x4> skin;
	skin.reserve( boneToModel.size() );
	for ( std::size_t i = 0; i < boneToModel.size(); ++i )
	{
		skin.push_back( Concat( boneToModel[i], model.bones[i].poseToBone ) );
	}
	Model out = model;
	out.mins = {};
	out.maxs = {};
	bool any = false;
	for ( Mesh &mesh : out.meshes )
	{
		for ( std::size_t v = 0; v < mesh.vertices.size(); ++v )
		{
			const BoneWeights weights = v < mesh.weights.size() ? mesh.weights[v] : BoneWeights{};
			Vertex &vertex = mesh.vertices[v];
			Float3 position;
			Float3 normal;
			for ( std::uint8_t k = 0; k < weights.count && k < 3; ++k )
			{
				const std::size_t bone = weights.bones[k];
				if ( bone >= skin.size() )
				{
					continue;
				}
				const Matrix3x4 &m = skin[bone];
				const float w = weights.weights[k];
				const Float3 p = TransformPoint( m, vertex.position );
				const Float3 &n = vertex.normal;
				position = { position.x + w * p.x, position.y + w * p.y, position.z + w * p.z };
				normal = { normal.x + w * ( m.m[0][0] * n.x + m.m[0][1] * n.y + m.m[0][2] * n.z ),
				    normal.y + w * ( m.m[1][0] * n.x + m.m[1][1] * n.y + m.m[1][2] * n.z ),
				    normal.z + w * ( m.m[2][0] * n.x + m.m[2][1] * n.y + m.m[2][2] * n.z ) };
			}
			const float length =
			    std::sqrt( normal.x * normal.x + normal.y * normal.y + normal.z * normal.z );
			if ( length > 0.0f )
			{
				normal = { normal.x / length, normal.y / length, normal.z / length };
			}
			vertex.position = position;
			vertex.normal = normal;
		}
		for ( std::uint32_t index : mesh.indices )
		{
			Extend( out.mins, out.maxs, mesh.vertices[index].position, !any );
			any = true;
		}
	}
	return out;
}

Model PoseModel( const Model &model, std::int32_t sequence )
{
	if ( sequence < 0 || static_cast<std::size_t>( sequence ) >= model.sequences.size() ||
	     ( model.flags & kStaticPropFlag ) )
	{
		return model;
	}
	return PoseModel( model, model.sequences[static_cast<std::size_t>( sequence )].firstFrame );
}

} // namespace mdl
