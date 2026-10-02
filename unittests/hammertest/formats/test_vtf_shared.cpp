//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: Independent wire fixtures shared by the VTF container and consumers.
//=============================================================================//
#include "texturecontainer/vtf_container.h"
#include "texturecontainer/vtf_image_reader.h"
#include "hammer/formats/vtf_image.h"
#include "testing/conformance_result.h"
#include "fake_vtf.h"
#ifdef VTF_SHARED_LEGACY
#include "tier1/utlbuffer.h"
#include "vtf/vtf.h"
#endif

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <span>
#include <string>

namespace
{
unsigned checks = 0, failures = 0;
void Check( bool good, const char *what )
{
	++checks;
	if ( !good )
	{
		++failures;
		std::printf( "FAIL: %s\n", what );
	}
}
auto Bytes( const std::string &s )
{
	return std::as_bytes( std::span( s.data(), s.size() ) );
}
bool InflateFixture( texturecontainer::vtf::CompressionMethod method, std::string_view bytes,
    std::span<std::uint8_t> out, std::string & )
{
	if ( method != texturecontainer::vtf::CompressionMethod::Zstandard || bytes.size() != 1 )
		return false;
	std::fill( out.begin(), out.end(), std::uint8_t( bytes[0] ) );
	return true;
}
void Reject( const std::string &bytes )
{
	std::string error;
	auto h = texturecontainer::vtf::ReadHeader( Bytes( bytes ) );
	Check( !h || !texturecontainer::vtf::ReadLayout( Bytes( bytes ), h.Value() ),
	    "container rejects malformed fixture" );
	Check( !hammer::formats::DecodeVtf( bytes, error ) && !error.empty(),
	    "preview rejects malformed fixture with diagnostic" );
	Check( !texturecontainer::ReadVtfImage( Bytes( bytes ) ), "lab rejects malformed fixture" );
#ifdef VTF_SHARED_LEGACY
	CUtlBuffer buffer( bytes.data(), int( bytes.size() ), CUtlBuffer::READ_ONLY );
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	Check( !texture->Unserialize( buffer ), "game rejects malformed fixture" );
#endif
}
#ifdef VTF_SHARED_LEGACY
void GameImages( const std::string &bytes, int frames, int faces, int depth, int skip )
{
	CUtlBuffer buffer( bytes.data(), int( bytes.size() ), CUtlBuffer::READ_ONLY );
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	const bool loaded = texture->Unserialize( buffer, false, skip );
	Check( loaded, "game reads independent topology fixture" );
	if ( !loaded )
		return;
	Check( texture->FrameCount() == frames && texture->FaceCount() == ( faces > 1 ? 7 : 1 ) &&
	           texture->Depth() == std::max( 1, depth >> skip ),
	    "game preserves topology" );
	for ( int mip = skip; mip < 3; ++mip )
		for ( int frame = 0; frame < frames; ++frame )
			for ( int face = 0; face < std::min( faces, 7 ); ++face )
			{
				const auto *data = texture->ImageData( frame, face, mip - skip );
				const auto size = texture->ComputeMipSize( mip - skip );
				Check( std::all_of( data, data + size,
				           [=]( unsigned char b )
				           {
					           return b == 1 + mip * 20 + frame * 8 + face;
				           } ),
				    "game subresource bytes match independent pattern" );
			}
	if ( faces == 6 )
		Check(
		    texture->ImageData( 0, 6, 0 )[0] == 0, "missing legacy sphere fallback initialized" );
}
#endif
// RGBA8 4x4 fixtures, three mips, unique byte for each mip/frame/face.
std::string Topology( int frames, int faces, int depth, int minor = 5 )
{
	using namespace hammertest::detail;
	auto s = hammertest::BuildVtf( 4, 4, 0, 1, {}, true );
	s.resize( 88 );
	PutU32At( s, 8, minor );
	PutU16At( s, 24, frames );
	PutU16At( s, 63, depth );
	PutU32At( s, 20, faces > 1 ? 0x4000 : 0 );
	s[56] = 3;
	for ( int mip = 2; mip >= 0; --mip )
		for ( int frame = 0; frame < frames; ++frame )
			for ( int face = 0; face < faces; ++face )
				s.append( std::size_t( 4 >> mip ) * ( 4 >> mip ) * std::max( 1, depth >> mip ) * 4,
				    char( 1 + mip * 20 + frame * 8 + face ) );
	return s;
}

// Read-only corpus input: little-endian host fixture stream of name length,
// payload length, name, VTF. The Python driver reads external VPK bytes into
// this pipe; no installed asset is copied to the repository.
int Corpus()
{
	unsigned files = 0;
	for ( ;; )
	{
		std::uint32_t sizes[2]{};
		const auto read = std::fread( sizes, 1, sizeof( sizes ), stdin );
		if ( !read ) break;
		if ( read != sizeof( sizes ) || sizes[0] > 4096 || sizes[1] > texturecontainer::vtf::kMaxImageBytes )
		{
			Check( false, "invalid corpus record" ); break;
		}
		std::string name( sizes[0], '\0' ), bytes( sizes[1], '\0' );
		if ( std::fread( name.data(), 1, name.size(), stdin ) != name.size() ||
		     std::fread( bytes.data(), 1, bytes.size(), stdin ) != bytes.size() )
		{
			Check( false, "truncated corpus record" ); break;
		}
		++files;
		const auto before = failures;
		auto header = texturecontainer::vtf::ReadHeader( Bytes( bytes ) );
		Check( bool( header ), "corpus header" );
		if ( !header )
		{
			std::printf( "FILE %s: %s\n", name.c_str(), header.Error() ); continue;
		}
		auto layout = texturecontainer::vtf::ReadLayout( Bytes( bytes ), header.Value() );
		Check( bool( layout ), "corpus layout" );
		if ( !layout )
		{
			std::printf( "FILE %s: %s\n", name.c_str(), layout.Error() ); continue;
		}
#ifdef VTF_SHARED_LEGACY
		if ( !layout.Value().compression && header.Value().format < NUM_IMAGE_FORMATS )
		{
			CUtlBuffer buffer( bytes.data(), int( bytes.size() ), CUtlBuffer::READ_ONLY );
			std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture( CreateVTFTexture(), &DestroyVTFTexture );
			const bool loaded = texture->Unserialize( buffer );
			Check( loaded, "corpus legacy adapter" );
			if ( loaded )
				for ( const auto &run : layout.Value().images )
				{
					if ( run.face >= unsigned( texture->FaceCount() ) ) continue;
					Check( run.decodedBytes == std::size_t( texture->ComputeMipSize( run.mip ) ) &&
					           !std::memcmp( bytes.data() + run.stored.offset,
					               texture->ImageData( run.frame, run.face, run.mip ), run.decodedBytes ),
					    "corpus game bytes match authored subresource" );
				}
		}
#endif
		if ( before != failures ) std::printf( "FILE %s\n", name.c_str() );
	}
	Check( files > 0, "corpus must contain files" );
	std::printf( "VTF_CORPUS %u\n", files );
	return testing::ReportConformance( checks, failures );
}
} // namespace

int main( int argc, char **argv )
{
	if ( argc == 2 && std::string_view( argv[1] ) == "--corpus" ) return Corpus();
	using namespace hammertest::detail;
	using namespace texturecontainer;
	for ( bool dictionary : { false, true } )
	{
		auto s = hammertest::BuildVtf( 2, 2, 3, 1, { std::string( 12, '\x35' ) }, dictionary );
		if ( dictionary )
		{
			// Move image away from header. A parser falling back to headerSize
			// instead of reading numResources at 0x44 returns the wrong pixels.
			s.insert( 88, 16, '\0' );
			PutU32At( s, 84, 104 );
		}
		auto image = ReadVtfImage( Bytes( s ) );
		std::string error;
		auto preview = hammer::formats::DecodeVtf( s, error );
		Check( image && preview && image.Value().levels[0].bytes[0] == std::byte{ 0x35 } &&
		           preview->rgba[0] == 0x35,
		    "lab and preview honor dictionary and legacy offsets" );
#ifdef VTF_SHARED_LEGACY
		CUtlBuffer buffer( s.data(), int( s.size() ), CUtlBuffer::READ_ONLY );
		std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
		    CreateVTFTexture(), &DestroyVTFTexture );
		Check( texture->Unserialize( buffer ) && texture->ImageData()[0] == 0x35,
		    "game honors same offset" );
#endif
	}
	for ( auto dims :
	    { std::array{ 2, 1, 1, 5 }, std::array{ 1, 1, 4, 5 }, std::array{ 1, 6, 1, 5 },
	        std::array{ 1, 7, 1, 5 }, std::array{ 2, 7, 1, 4 }, std::array{ 1, 8, 1, 4 } } )
	{
		auto s = Topology( dims[0], dims[1], dims[2], dims[3] );
		auto h = vtf::ReadHeader( Bytes( s ) );
		Check( bool( h ), "topology header valid" );
		if ( !h )
			continue;
		auto layout = vtf::ReadLayout( Bytes( s ), h.Value() );
		Check( layout && layout.Value().faces == unsigned( dims[1] ) &&
		           layout.Value().images.size() == std::size_t( 3 * dims[0] * dims[1] ),
		    "topology layout complete" );
		if ( layout )
			for ( const auto &r : layout.Value().images )
				Check( std::all_of( s.begin() + r.stored.offset,
				           s.begin() + r.stored.offset + r.stored.size,
				           [&]( unsigned char b )
				           {
					           return b == 1 + r.mip * 20 + r.frame * 8 + r.face;
				           } ),
				    "container offsets match independent pattern" );
#ifdef VTF_SHARED_LEGACY
		GameImages( s, dims[0], dims[1], dims[2], 0 );
		// Runtime streams only smaller mips; largest mip bytes are absent.
		s.resize( s.size() - std::size_t( 4 * 4 * dims[2] * 4 * dims[0] * dims[1] ) );
		GameImages( s, dims[0], dims[1], dims[2], 1 );
#endif
	}
	auto compressed = hammertest::BuildCompressedVtf( 4, 4, 3, 3, { "a", "b", "c" } );
	auto decoded = ReadVtfImage( Bytes( compressed ), InflateFixture );
	Check( decoded && decoded.Value().levels.size() == 3 &&
	           decoded.Value().levels[0].bytes[0] == std::byte{ 'a' } &&
	           decoded.Value().levels[2].bytes[0] == std::byte{ 'c' },
	    "lab uses shared AXC run ordering" );
	Check(
	    !ReadVtfImage( Bytes( compressed ) ), "missing decompression capability fails explicitly" );
	const auto good = Topology( 1, 1, 1 );
	for ( auto [offset, value] :
	    { std::pair{ 8U, 7U }, { 16U, 0U }, { 24U, 0U }, { 56U, 32U }, { 68U, 33U }, { 84U, 4U } } )
	{
		auto bad = good;
		if ( offset == 56 )
			bad[offset] = char( value );
		else
			PutU32At( bad, offset, value );
		Reject( bad );
	}
	Reject( good.substr( 0, 63 ) );
	Reject( good.substr( 0, good.size() - 1 ) );
#ifdef VTF_SHARED_LEGACY
	// Header-only reads succeed with no payload. A later full read must fail.
	auto head = good.substr( 0, 88 );
	CUtlBuffer buffer( head.data(), int( head.size() ), CUtlBuffer::READ_ONLY );
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	Check( texture->Unserialize( buffer, true ) && texture->Width() == 4,
	    "game header-only read requires no image payload" );
#endif
	return testing::ReportConformance( checks, failures );
}
