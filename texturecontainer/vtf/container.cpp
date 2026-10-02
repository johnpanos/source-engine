//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: One checked reader for VTF headers, resources and image runs.
//=============================================================================//
#include "texturecontainer/vtf_container.h"

#include <algorithm>
#include <bit>
#include <cstring>

namespace texturecontainer::vtf
{
namespace
{
using Bytes = std::span<const std::byte>;
auto Error( const char *text )
{
	return foundation::MakeUnexpected( text );
}
bool Fits( Bytes b, std::size_t offset, std::size_t size )
{
	return offset <= b.size() && size <= b.size() - offset;
}
std::uint32_t U32( Bytes b, std::size_t p )
{
	return std::to_integer<std::uint32_t>( b[p] ) |
	       ( std::to_integer<std::uint32_t>( b[p + 1] ) << 8 ) |
	       ( std::to_integer<std::uint32_t>( b[p + 2] ) << 16 ) |
	       ( std::to_integer<std::uint32_t>( b[p + 3] ) << 24 );
}
std::uint16_t U16( Bytes b, std::size_t p )
{
	return std::uint16_t(
	    std::to_integer<unsigned>( b[p] ) | ( std::to_integer<unsigned>( b[p + 1] ) << 8 ) );
}
const Resource *Find( const Header &h, std::uint32_t type )
{
	for ( const auto &r : h.resources )
		if ( ( r.type & 0xffffff ) == type )
			return &r;
	return nullptr;
}
std::size_t End( Bytes b, const Header &h, std::size_t start )
{
	std::size_t end = b.size();
	for ( const auto &r : h.resources )
		if ( !( r.type & kInline ) && r.value > start )
			end = std::min( end, std::size_t( r.value ) );
	return end;
}
std::uint32_t Dim( std::uint32_t value, unsigned mip )
{
	return std::max( 1U, value >> mip );
}
} // namespace

std::size_t ImageBytes( int format, std::uint32_t w, std::uint32_t h, std::uint32_t d )
{
	if ( !w || !h || !d )
		return 0;
	// VTF ImageFormat wire values. Pixel conversion and GPU representation are
	// consumer concerns; container offsets always use this storage description.
	constexpr unsigned char sizes[] = { 4, 4, 3, 3, 2, 1, 2, 1, 1, 3, 3, 4, 4, 0, 0, 0, 4, 2, 2, 2,
	    0, 2, 2, 4, 8, 8, 4, 4, 12, 16, 2, 4, 4, 4, 2, 4, 4 };
	std::uint64_t bytes = 0;
	// Legacy VTF volume block storage pads 2- and 3-slice mips to four.
	if ( ( format == 13 || format == 14 || format == 15 || format == 20 || format == 37 ||
	         format == 38 || format == 39 || format == 40 ) &&
	     d > 1 && d < 4 )
		d = 4;
	if ( format == 13 || format == 20 || format == 38 || format == 39 )
		bytes = ( ( std::uint64_t( w ) + 3 ) / 4 ) * ( ( std::uint64_t( h ) + 3 ) / 4 ) * 8;
	else if ( format == 14 || format == 15 || format == 37 || format == 40 || format == 70 )
		bytes = ( ( std::uint64_t( w ) + 3 ) / 4 ) * ( ( std::uint64_t( h ) + 3 ) / 4 ) * 16;
	else if ( format >= 0 && std::size_t( format ) < std::size( sizes ) )
		bytes = std::uint64_t( w ) * h * sizes[format];
	if ( !bytes || bytes > kMaxImageBytes / d )
		return 0;
	return std::size_t( bytes * d );
}

foundation::Expected<Header, const char *> ReadHeader( Bytes b )
{
	if ( b.size() < 64 || std::memcmp( b.data(), "VTF\0", 4 ) || U32( b, 4 ) != 7 )
		return Error( "vtf: invalid or truncated header" );
	Header h;
	h.minor = U32( b, 8 );
	h.headerBytes = U32( b, 12 );
	const std::size_t minimum = h.minor < 2 ? 64 : 80;
	if ( h.minor > 6 || h.headerBytes < minimum || h.headerBytes > b.size() )
		return Error( "vtf: unsupported version or truncated header region" );
	h.width = U16( b, 16 );
	h.height = U16( b, 18 );
	h.flags = U32( b, 20 );
	if ( h.minor <= 3 )
		h.flags &= ~0xD1780400U; // historical 7.3 flag meaning, before later reuse
	h.frames = U16( b, 24 );
	h.startFrame = U16( b, 26 );
	for ( unsigned i = 0; i < 3; ++i )
		h.reflectivity[i] = std::bit_cast<float>( U32( b, 32 + i * 4 ) );
	h.bumpScale = std::bit_cast<float>( U32( b, 48 ) );
	h.format = std::bit_cast<std::int32_t>( U32( b, 52 ) );
	h.mips = std::to_integer<std::uint8_t>( b[56] );
	h.thumbnailFormat = std::bit_cast<std::int32_t>( U32( b, 57 ) );
	h.thumbnailWidth = std::to_integer<std::uint8_t>( b[61] );
	h.thumbnailHeight = std::to_integer<std::uint8_t>( b[62] );
	h.depth = h.minor >= 2 ? U16( b, 63 ) : 1;
	if ( !h.width || !h.height || !h.depth || !h.frames || !h.mips ||
	     h.mips > std::bit_width( std::max( { h.width, h.height, h.depth } ) ) ||
	     !ImageBytes( h.format, h.width, h.height, h.depth ) )
		return Error( "vtf: invalid dimensions, mip/frame count or image format" );
	if ( ( h.flags & kEnvmap ) && ( h.width != h.height || h.depth != 1 ) )
		return Error( "vtf: cubemap must be square and non-volume" );
	if ( h.thumbnailFormat != -1 && !ImageBytes( h.thumbnailFormat, 1, 1 ) )
		return Error( "vtf: unsupported thumbnail format" );
	const std::uint32_t count = h.minor >= 3 ? U32( b, 68 ) : 0;
	if ( count > 32 || ( count && 80 + count * 8 > h.headerBytes ) )
		return Error( "vtf: invalid resource dictionary size" );
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		Resource r{ U32( b, 80 + i * 8 ), U32( b, 84 + i * 8 ) };
		const auto type = r.type & 0xffffff;
		if ( Find( h, type ) || ( r.type & 0xfd000000 ) ||
		     ( !( r.type & kInline ) && r.value < h.headerBytes ) ||
		     ( ( type == kImage || type == kThumbnail || type == kAuxCompression ) &&
		         ( r.type & kInline ) ) )
			return Error( "vtf: invalid or duplicate resource" );
		for ( const auto &other : h.resources )
			if ( !( r.type & kInline ) && !( other.type & kInline ) && r.value == other.value )
				return Error( "vtf: overlapping resource offsets" );
		h.resources.push_back( r );
	}
	if ( !count )
	{
		const auto thumbnail = ImageBytes( h.thumbnailFormat, h.thumbnailWidth, h.thumbnailHeight );
		if ( thumbnail )
			h.resources.push_back( { kThumbnail, h.headerBytes } );
		h.resources.push_back( { kImage, std::uint32_t( h.headerBytes + thumbnail ) } );
	}
	if ( !Find( h, kImage ) )
		return Error( "vtf: no image resource" );
	return h;
}

foundation::Expected<Range, const char *> ResourceData(
    Bytes b, const Header &h, const Resource &r )
{
	if ( r.type & kInline )
		return Error( "vtf: inline resource has no payload" );
	Range range{ r.value, 0 };
	const auto type = r.type & 0xffffff;
	if ( type == kImage )
	{
		if ( range.offset > b.size() )
			return Error( "vtf: image resource outside input" );
		range.size = End( b, h, range.offset ) - range.offset;
	}
	else if ( type == kThumbnail )
		range.size = ImageBytes( h.thumbnailFormat, h.thumbnailWidth, h.thumbnailHeight );
	else
	{
		if ( !Fits( b, range.offset, 4 ) )
			return Error( "vtf: truncated resource length" );
		range.size = U32( b, range.offset );
		range.offset += 4;
	}
	if ( range.size > kMaxImageBytes || !Fits( b, range.offset, range.size ) ||
	     range.offset + range.size > End( b, h, r.value ) )
		return Error( "vtf: truncated or overlapping resource payload" );
	return range;
}

foundation::Expected<Layout, const char *> ReadLayout(
    Bytes b, const Header &h, std::uint32_t finest )
{
	if ( finest >= h.mips )
		return Error( "vtf: requested mip outside authored chain" );
	const auto *image = Find( h, kImage );
	if ( !image )
		return Error( "vtf: no image resource" );
	auto imageRange = ResourceData( b, h, *image );
	if ( !imageRange )
		return foundation::MakeUnexpected( imageRange.Error() );
	for ( const auto &resource : h.resources )
	{
		if ( resource.type & kInline )
			continue;
		auto range = ResourceData( b, h, resource );
		if ( !range )
			return foundation::MakeUnexpected( range.Error() );
	}
	Layout layout;
	std::size_t perFace = 0;
	for ( unsigned mip = finest; mip < h.mips; ++mip )
	{
		const auto size =
		    ImageBytes( h.format, Dim( h.width, mip ), Dim( h.height, mip ), Dim( h.depth, mip ) );
		if ( !size || size > kMaxImageBytes - perFace )
			return Error( "vtf: image chain too large" );
		perFace += size;
	}
	if ( h.flags & kEnvmap )
	{
		layout.faces = 6;
		if ( h.minor < 6 && perFace * h.frames * 7 <= imageRange.Value().size )
			layout.faces = 7;
		// Historical SDK 7.1-7.4 files may contain an additional obsolete
		// sphere slice. It is represented here and ignored by the ABI adapter.
		if ( h.minor >= 1 && h.minor <= 4 && perFace * h.frames * 8 <= imageRange.Value().size )
			layout.faces = 8;
	}
	if ( perFace > kMaxImageBytes / h.frames / layout.faces ||
	     std::size_t( h.mips ) * h.frames * layout.faces > 65536 )
		return Error( "vtf: image topology too large" );
	std::span<const std::byte> lengths;
	if ( const auto *aux = Find( h, kAuxCompression ) )
	{
		auto data = ResourceData( b, h, *aux );
		if ( !data || data.Value().size < 4 || h.minor != 6 )
			return Error( "vtf: invalid AXC resource" );
		const auto payload = b.subspan( data.Value().offset, data.Value().size );
		if ( U16( payload, 0 ) )
		{
			const auto method = U16( payload, 2 );
			if ( method != 8 && method != 93 )
				return Error( "vtf: unsupported AXC method" );
			layout.compression = CompressionMethod( method );
			if ( payload.size() != 4 + std::size_t( h.mips ) * h.frames * layout.faces * 4 )
				return Error( "vtf: incorrect AXC length table" );
			lengths = payload.subspan( 4 );
		}
	}
	std::size_t offset = imageRange.Value().offset, index = 0;
	for ( unsigned mip = h.mips; mip-- > finest; )
	{
		for ( unsigned frame = 0; frame < h.frames; ++frame )
		{
			for ( unsigned face = 0; face < layout.faces; ++face, ++index )
			{
				Subresource s;
				s.mip = mip;
				s.frame = frame;
				s.face = face;
				s.width = Dim( h.width, mip );
				s.height = Dim( h.height, mip );
				s.depth = Dim( h.depth, mip );
				s.decodedBytes = ImageBytes( h.format, s.width, s.height, s.depth );
				s.stored = {
				    offset, layout.compression ? U32( lengths, index * 4 ) : s.decodedBytes };
				if ( !s.stored.size || !Fits( b, offset, s.stored.size ) ||
				     s.stored.size > imageRange.Value().offset + imageRange.Value().size - offset )
					return Error( "vtf: truncated image run" );
				layout.images.push_back( s );
				offset += s.stored.size;
			}
		}
	}
	return layout;
}

foundation::Expected<std::vector<std::byte>, const char *> ReadImage(
    Bytes b, const Layout &layout, const Subresource &s, Decompressor decode )
{
	if ( !Fits( b, s.stored.offset, s.stored.size ) || !s.decodedBytes ||
	     s.decodedBytes > kMaxImageBytes )
		return Error( "vtf: invalid image range" );
	const auto source = b.subspan( s.stored.offset, s.stored.size );
	if ( !layout.compression )
	{
		if ( source.size() != s.decodedBytes )
			return Error( "vtf: image size mismatch" );
		return std::vector<std::byte>( source.begin(), source.end() );
	}
	if ( !decode )
		return Error( "vtf: compressed image requires a decompressor" );
	std::vector<std::byte> result( s.decodedBytes );
	std::string error;
	if ( !decode( *layout.compression,
	         { reinterpret_cast<const char *>( source.data() ), source.size() },
	         { reinterpret_cast<std::uint8_t *>( result.data() ), result.size() }, error ) )
		return Error( "vtf: decompression failed" );
	return result;
}
} // namespace texturecontainer::vtf
