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

Vertex ReadVertex( Reader &vvd, std::uint64_t record )
{
	Vertex v;
	v.position = vvd.Vec( record + kVvdPosition );
	v.normal = vvd.Vec( record + kVvdNormal );
	v.u = vvd.F32( record + kVvdUv );
	v.v = vvd.F32( record + kVvdUv + 4 );
	return v;
}

// Appends the triangles of one VTX strip group, as mesh-local vertex indices
// (counter-clockwise), to 'indices'.
bool ReadStripGroup( Reader &vtx, std::uint64_t group, bool v49, std::uint32_t meshVertices,
    std::vector<std::uint32_t> &indices )
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
	}
	return "?";
}

std::string Describe( const ModelError &error )
{
	return std::string( StatusName( error.status ) ) + " in " + FileName( error.file ) +
	       " at byte " + std::to_string( error.offset );
}

foundation::Expected<Model, ModelError> ParseModel( const ModelBytes &bytes, std::int32_t body )
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

	// --- Body parts: the chosen model of each, its LOD 0 meshes ---
	const std::uint32_t chosenBody = body < 0 ? 0u : static_cast<std::uint32_t>( body );
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
		if ( numModels == 0 )
		{
			continue;
		}
		if ( base <= 0 )
		{
			mdl.Fail( ModelStatus::BadCount, part + 8 );
			return MakeUnexpected( mdl.Error() );
		}
		const std::uint32_t index = ( chosenBody / static_cast<std::uint32_t>( base ) ) % numModels;
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
		if ( vertexIndex < 0 || vertexIndex % static_cast<std::int32_t>( kVvdVertexStride ) != 0 )
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
		if ( !vtx.Has( lods, kVtxLodStride ) )
		{
			return MakeUnexpected( vtx.Error() );
		}
		const std::uint32_t vtxMeshCount = vtx.Count( lods );
		const std::uint64_t vtxMeshes = vtx.Relative( lods, lods + 4 );
		if ( vtx.Failed() )
		{
			return MakeUnexpected( vtx.Error() );
		}
		if ( vtxMeshCount != numMeshes )
		{
			vtx.Fail( ModelStatus::BadCount, lods );
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
			out.textureRef = mdl.I32( mesh );
			const std::uint32_t meshVertices = mdl.Count( mesh + 8 );
			const std::int32_t vertexOffset = mdl.I32( mesh + 12 );
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
			const std::uint64_t first = modelFirst + static_cast<std::uint64_t>( vertexOffset );

			const std::uint64_t vtxMesh = vtxMeshes + std::uint64_t( m ) * kVtxMeshStride;
			const std::uint32_t numGroups = vtx.Count( vtxMesh );
			const std::uint64_t groups = vtx.Relative( vtxMesh, vtxMesh + 4 );
			const std::uint64_t groupStride = v49 ? kVtxStripGroupStride49 : kVtxStripGroupStride;
			if ( vtx.Failed() || !vtx.Array( groups, numGroups, groupStride ) )
			{
				return MakeUnexpected( vtx.Error() );
			}
			for ( std::uint32_t g = 0; g < numGroups; ++g )
			{
				if ( !ReadStripGroup( vtx, groups + std::uint64_t( g ) * groupStride, v49,
				         meshVertices, out.indices ) )
				{
					return MakeUnexpected( vtx.Error() );
				}
			}
			if ( out.indices.empty() )
			{
				continue;
			}
			out.vertices.reserve( meshVertices );
			for ( std::uint64_t v = 0; v < meshVertices; ++v )
			{
				out.vertices.push_back( ReadVertex( vvd, vertices.records[first + v] ) );
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
	return model;
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
	return ParseModel( ModelBytes{ mdl, vvd, vtx }, body );
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

} // namespace mdl
