//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent VTF-blob builder for the decoder suites (RFC 0002,
//			hammer.formats). Emits a real VTF 7.x byte layout (header fields at the
//			documented offsets, mips stored smallest-first) so the decoder is tested
//			against a fixture serialized by code OTHER than the decoder itself. Can
//			emit both the legacy 7.2 "image after header+thumbnail" layout and the
//			7.4 resource-dictionary layout, so both offset paths are covered.
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_VTF_H
#define HAMMERTEST_FAKE_VTF_H

#include <cstdint>
#include <string>
#include <vector>

namespace hammertest
{

// ImageFormat values used by the fixtures (subset of public/bitmap/imageformat.h).
enum
{
	VTF_FMT_BGR888 = 3,
	VTF_FMT_BGRA8888 = 12,
	VTF_FMT_DXT1 = 13,
	VTF_FMT_DXT5 = 15,
};

namespace detail
{

inline std::size_t VtfMipBytes( int format, int w, int h )
{
	if ( w < 1 )
		w = 1;
	if ( h < 1 )
		h = 1;
	switch ( format )
	{
	case VTF_FMT_BGRA8888:
		return std::size_t( w ) * h * 4;
	case VTF_FMT_BGR888:
		return std::size_t( w ) * h * 3;
	case VTF_FMT_DXT1:
		return std::size_t( ( w + 3 ) / 4 ) * ( ( h + 3 ) / 4 ) * 8;
	case VTF_FMT_DXT5:
		return std::size_t( ( w + 3 ) / 4 ) * ( ( h + 3 ) / 4 ) * 16;
	default:
		return 0;
	}
}

inline void PutU16At( std::string &b, std::size_t off, std::uint16_t v )
{
	b[off] = char( v & 0xff );
	b[off + 1] = char( ( v >> 8 ) & 0xff );
}

inline void PutU32At( std::string &b, std::size_t off, std::uint32_t v )
{
	b[off + 0] = char( v & 0xff );
	b[off + 1] = char( ( v >> 8 ) & 0xff );
	b[off + 2] = char( ( v >> 16 ) & 0xff );
	b[off + 3] = char( ( v >> 24 ) & 0xff );
}

} // namespace detail

// Builds a VTF. 'mipsLargestFirst[0]' is the mip-0 payload, [1] mip 1, ...; a
// missing/short mip is zero-filled to its computed size. When 'useResourceDict'
// is true a 7.4 header with a single image resource is emitted; otherwise a 7.2
// header with the image immediately after it (no thumbnail).
inline std::string BuildVtf(
    int width,
    int height,
    int format,
    int mipCount,
    const std::vector<std::string> &mipsLargestFirst,
    bool useResourceDict )
{
	using namespace detail;

	const std::size_t kHeaderRegion = useResourceDict ? 0x58 : 0x50;
	std::string blob( kHeaderRegion, '\0' );

	blob[0] = 'V';
	blob[1] = 'T';
	blob[2] = 'F';
	blob[3] = '\0';
	PutU32At( blob, 0x04, 7 );                            // major
	PutU32At( blob, 0x08, useResourceDict ? 4 : 2 );      // minor
	PutU32At( blob, 0x0c, std::uint32_t( kHeaderRegion ) ); // headerSize
	PutU16At( blob, 0x10, std::uint16_t( width ) );
	PutU16At( blob, 0x12, std::uint16_t( height ) );
	PutU32At( blob, 0x14, 0 );                            // flags (not envmap)
	PutU16At( blob, 0x18, 1 );                            // frameCount
	PutU16At( blob, 0x1a, 0 );                            // startFrame
	PutU32At( blob, 0x34, std::uint32_t( format ) );      // imageFormat
	blob[0x38] = char( mipCount );                        // mipCount
	PutU32At( blob, 0x39, 0xffffffffu );                  // lowResImageFormat = NONE
	blob[0x3d] = 0;                                       // lowResImageWidth
	blob[0x3e] = 0;                                       // lowResImageHeight
	PutU16At( blob, 0x3f, 1 );                            // depth

	if ( useResourceDict )
	{
		PutU32At( blob, 0x48, 1 );        // numResources
		blob[0x50] = char( 0x30 );        // VTF_LEGACY_RSRC_IMAGE tag byte 0
		blob[0x51] = 0;
		blob[0x52] = 0;
		blob[0x53] = 0;                   // flags
		PutU32At( blob, 0x54, std::uint32_t( kHeaderRegion ) ); // image data offset
	}

	// Append mips smallest-first.
	for ( int mip = mipCount - 1; mip >= 0; --mip )
	{
		int mw = width >> mip;
		int mh = height >> mip;
		if ( mw < 1 )
			mw = 1;
		if ( mh < 1 )
			mh = 1;
		const std::size_t need = VtfMipBytes( format, mw, mh );
		std::string data;
		if ( mip < int( mipsLargestFirst.size() ) )
			data = mipsLargestFirst[mip];
		data.resize( need, '\0' );
		blob += data;
	}
	return blob;
}

} // namespace hammertest

#endif // HAMMERTEST_FAKE_VTF_H
