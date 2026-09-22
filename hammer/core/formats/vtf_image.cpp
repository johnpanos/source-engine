//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the strict-core VTF decoder. See
//			public/hammer/formats/vtf_image.h for the contract and scope.
//
//			Header field offsets are the well-known VTF 7.x layout (matching
//			public/vtf/vtf.h's VTFFileHeaderV7_x_t), read by absolute byte offset
//			and little-endian so the decode is packing- and endian-independent:
//
//			  0x00 char  signature[4]   "VTF\0"
//			  0x04 u32   version[0]      (major)
//			  0x08 u32   version[1]      (minor)
//			  0x0C u32   headerSize
//			  0x10 u16   width
//			  0x12 u16   height
//			  0x14 u32   flags
//			  0x18 u16   frameCount
//			  0x1A u16   startFrame
//			  0x20 f32   reflectivity[3]
//			  0x30 f32   bumpScale
//			  0x34 s32   imageFormat
//			  0x38 u8    mipCount
//			  0x39 s32   lowResImageFormat
//			  0x3D u8    lowResImageWidth
//			  0x3E u8    lowResImageHeight
//			  0x3F u16   depth              (>= 7.2)
//			  0x48 u32   numResources       (>= 7.3; resource dict at 0x50, 8 bytes each)
//
//=============================================================================//

#include "hammer/formats/vtf_image.h"

namespace hammer::formats
{

namespace
{

// ImageFormat enum values (subset), matching public/bitmap/imageformat.h.
enum : int
{
	FMT_RGBA8888 = 0,
	FMT_ABGR8888 = 1,
	FMT_RGB888 = 2,
	FMT_BGR888 = 3,
	FMT_I8 = 5,
	FMT_IA88 = 6,
	FMT_A8 = 8,
	FMT_ARGB8888 = 11,
	FMT_BGRA8888 = 12,
	FMT_DXT1 = 13,
	FMT_DXT3 = 14,
	FMT_DXT5 = 15,
	FMT_BGRX8888 = 16,
};

constexpr std::uint32_t kFlagEnvmap = 0x00004000u;

std::uint16_t GetU16( const std::string &b, std::size_t off )
{
	const unsigned char *p = reinterpret_cast<const unsigned char *>( b.data() ) + off;
	return std::uint16_t( std::uint16_t( p[0] ) | ( std::uint16_t( p[1] ) << 8 ) );
}

std::uint32_t GetU32( const std::string &b, std::size_t off )
{
	const unsigned char *p = reinterpret_cast<const unsigned char *>( b.data() ) + off;
	return std::uint32_t( p[0] ) | ( std::uint32_t( p[1] ) << 8 ) | ( std::uint32_t( p[2] ) << 16 )
	    | ( std::uint32_t( p[3] ) << 24 );
}

std::int32_t GetS32( const std::string &b, std::size_t off )
{
	return static_cast<std::int32_t>( GetU32( b, off ) );
}

// The number of bytes one mip level occupies for a given format and dimensions.
// Block-compressed formats round up to whole 4x4 blocks. Returns 0 for an
// unsupported format (callers reject the file before relying on this).
std::size_t FormatMipBytes( int format, int w, int h )
{
	if ( w < 1 )
		w = 1;
	if ( h < 1 )
		h = 1;
	switch ( format )
	{
	case FMT_RGBA8888:
	case FMT_ABGR8888:
	case FMT_ARGB8888:
	case FMT_BGRA8888:
	case FMT_BGRX8888:
		return std::size_t( w ) * h * 4;
	case FMT_RGB888:
	case FMT_BGR888:
		return std::size_t( w ) * h * 3;
	case FMT_IA88:
		return std::size_t( w ) * h * 2;
	case FMT_I8:
	case FMT_A8:
		return std::size_t( w ) * h;
	case FMT_DXT1:
		return std::size_t( ( w + 3 ) / 4 ) * ( ( h + 3 ) / 4 ) * 8;
	case FMT_DXT3:
	case FMT_DXT5:
		return std::size_t( ( w + 3 ) / 4 ) * ( ( h + 3 ) / 4 ) * 16;
	default:
		return 0;
	}
}

bool FormatSupported( int format )
{
	return FormatMipBytes( format, 1, 1 ) != 0;
}

void PutPixel( VtfImage &img, int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a )
{
	if ( x < 0 || y < 0 || x >= img.width || y >= img.height )
		return;
	std::size_t i = ( std::size_t( y ) * img.width + x ) * 4;
	img.rgba[i + 0] = r;
	img.rgba[i + 1] = g;
	img.rgba[i + 2] = b;
	img.rgba[i + 3] = a;
}

// Expands a 16-bit RGB565 value to 8-bit components.
void Rgb565( std::uint16_t c, std::uint8_t &r, std::uint8_t &g, std::uint8_t &b )
{
	std::uint8_t r5 = std::uint8_t( ( c >> 11 ) & 0x1f );
	std::uint8_t g6 = std::uint8_t( ( c >> 5 ) & 0x3f );
	std::uint8_t b5 = std::uint8_t( c & 0x1f );
	r = std::uint8_t( ( r5 << 3 ) | ( r5 >> 2 ) );
	g = std::uint8_t( ( g6 << 2 ) | ( g6 >> 4 ) );
	b = std::uint8_t( ( b5 << 3 ) | ( b5 >> 2 ) );
}

// Decodes the DXT1 4x4 color block at 'src' into 'img' at pixel origin (bx,by).
// When 'alphaFromBlock' is true (DXT1 with 1-bit alpha), index 3 in the c0<=c1
// mode is treated as transparent; otherwise alpha is left untouched (opaque).
void DecodeDxtColorBlock(
    const unsigned char *src, VtfImage &img, int bx, int by, bool honorPunchthroughAlpha )
{
	std::uint16_t c0 = std::uint16_t( src[0] | ( src[1] << 8 ) );
	std::uint16_t c1 = std::uint16_t( src[2] | ( src[3] << 8 ) );
	std::uint8_t r[4], g[4], b[4], a[4];
	Rgb565( c0, r[0], g[0], b[0] );
	Rgb565( c1, r[1], g[1], b[1] );
	a[0] = a[1] = a[2] = a[3] = 255;
	if ( c0 > c1 )
	{
		r[2] = std::uint8_t( ( 2 * r[0] + r[1] ) / 3 );
		g[2] = std::uint8_t( ( 2 * g[0] + g[1] ) / 3 );
		b[2] = std::uint8_t( ( 2 * b[0] + b[1] ) / 3 );
		r[3] = std::uint8_t( ( r[0] + 2 * r[1] ) / 3 );
		g[3] = std::uint8_t( ( g[0] + 2 * g[1] ) / 3 );
		b[3] = std::uint8_t( ( b[0] + 2 * b[1] ) / 3 );
	}
	else
	{
		r[2] = std::uint8_t( ( r[0] + r[1] ) / 2 );
		g[2] = std::uint8_t( ( g[0] + g[1] ) / 2 );
		b[2] = std::uint8_t( ( b[0] + b[1] ) / 2 );
		r[3] = g[3] = b[3] = 0;
		if ( honorPunchthroughAlpha )
			a[3] = 0;
	}
	std::uint32_t bits = std::uint32_t( src[4] ) | ( std::uint32_t( src[5] ) << 8 )
	    | ( std::uint32_t( src[6] ) << 16 ) | ( std::uint32_t( src[7] ) << 24 );
	for ( int py = 0; py < 4; ++py )
	{
		for ( int px = 0; px < 4; ++px )
		{
			int idx = int( bits & 0x3 );
			bits >>= 2;
			PutPixel( img, bx + px, by + py, r[idx], g[idx], b[idx], a[idx] );
		}
	}
}

// Overwrites just the alpha channel of a 4x4 region from a DXT3 explicit-alpha
// block (4 bits per texel).
void ApplyDxt3Alpha( const unsigned char *src, VtfImage &img, int bx, int by )
{
	for ( int py = 0; py < 4; ++py )
	{
		std::uint16_t row = std::uint16_t( src[py * 2] | ( src[py * 2 + 1] << 8 ) );
		for ( int px = 0; px < 4; ++px )
		{
			std::uint8_t a4 = std::uint8_t( ( row >> ( px * 4 ) ) & 0xf );
			std::uint8_t a = std::uint8_t( ( a4 << 4 ) | a4 );
			int x = bx + px, y = by + py;
			if ( x < img.width && y < img.height )
				img.rgba[( std::size_t( y ) * img.width + x ) * 4 + 3] = a;
		}
	}
}

// Overwrites the alpha channel of a 4x4 region from a DXT5 interpolated-alpha block.
void ApplyDxt5Alpha( const unsigned char *src, VtfImage &img, int bx, int by )
{
	int a0 = src[0], a1 = src[1];
	std::uint8_t alpha[8];
	alpha[0] = std::uint8_t( a0 );
	alpha[1] = std::uint8_t( a1 );
	if ( a0 > a1 )
	{
		for ( int i = 1; i <= 6; ++i )
			alpha[i + 1] = std::uint8_t( ( ( 7 - i ) * a0 + i * a1 ) / 7 );
	}
	else
	{
		for ( int i = 1; i <= 4; ++i )
			alpha[i + 1] = std::uint8_t( ( ( 5 - i ) * a0 + i * a1 ) / 5 );
		alpha[6] = 0;
		alpha[7] = 255;
	}
	// 16 texels * 3 bits, packed into 6 bytes following the two endpoints.
	std::uint64_t bits = 0;
	for ( int i = 0; i < 6; ++i )
		bits |= std::uint64_t( src[2 + i] ) << ( 8 * i );
	for ( int t = 0; t < 16; ++t )
	{
		int idx = int( ( bits >> ( 3 * t ) ) & 0x7 );
		int px = t % 4, py = t / 4;
		int x = bx + px, y = by + py;
		if ( x < img.width && y < img.height )
			img.rgba[( std::size_t( y ) * img.width + x ) * 4 + 3] = alpha[idx];
	}
}

bool DecodeBlock( int format, const std::string &data, std::size_t dataOff, VtfImage &img )
{
	const std::size_t need = FormatMipBytes( format, img.width, img.height );
	if ( need == 0 || dataOff + need > data.size() )
		return false;
	const unsigned char *src = reinterpret_cast<const unsigned char *>( data.data() ) + dataOff;

	switch ( format )
	{
	case FMT_DXT1:
	case FMT_DXT3:
	case FMT_DXT5:
	{
		const int blocksX = ( img.width + 3 ) / 4;
		const int blocksY = ( img.height + 3 ) / 4;
		const int blockBytes = ( format == FMT_DXT1 ) ? 8 : 16;
		for ( int byBlock = 0; byBlock < blocksY; ++byBlock )
		{
			for ( int bxBlock = 0; bxBlock < blocksX; ++bxBlock )
			{
				const unsigned char *blk = src + std::size_t( byBlock * blocksX + bxBlock ) * blockBytes;
				const unsigned char *colorBlk = ( format == FMT_DXT1 ) ? blk : blk + 8;
				DecodeDxtColorBlock( colorBlk, img, bxBlock * 4, byBlock * 4, format == FMT_DXT1 );
				if ( format == FMT_DXT3 )
					ApplyDxt3Alpha( blk, img, bxBlock * 4, byBlock * 4 );
				else if ( format == FMT_DXT5 )
					ApplyDxt5Alpha( blk, img, bxBlock * 4, byBlock * 4 );
			}
		}
		return true;
	}
	default:
		break;
	}

	// Uncompressed formats: walk texels in row order.
	std::size_t p = dataOff;
	for ( int y = 0; y < img.height; ++y )
	{
		for ( int x = 0; x < img.width; ++x )
		{
			std::uint8_t r = 0, g = 0, b = 0, a = 255;
			switch ( format )
			{
			case FMT_RGBA8888:
				r = src[p - dataOff + 0];
				g = src[p - dataOff + 1];
				b = src[p - dataOff + 2];
				a = src[p - dataOff + 3];
				p += 4;
				break;
			case FMT_ABGR8888:
				a = src[p - dataOff + 0];
				b = src[p - dataOff + 1];
				g = src[p - dataOff + 2];
				r = src[p - dataOff + 3];
				p += 4;
				break;
			case FMT_ARGB8888:
				a = src[p - dataOff + 0];
				r = src[p - dataOff + 1];
				g = src[p - dataOff + 2];
				b = src[p - dataOff + 3];
				p += 4;
				break;
			case FMT_BGRA8888:
				b = src[p - dataOff + 0];
				g = src[p - dataOff + 1];
				r = src[p - dataOff + 2];
				a = src[p - dataOff + 3];
				p += 4;
				break;
			case FMT_BGRX8888:
				b = src[p - dataOff + 0];
				g = src[p - dataOff + 1];
				r = src[p - dataOff + 2];
				a = 255;
				p += 4;
				break;
			case FMT_RGB888:
				r = src[p - dataOff + 0];
				g = src[p - dataOff + 1];
				b = src[p - dataOff + 2];
				p += 3;
				break;
			case FMT_BGR888:
				b = src[p - dataOff + 0];
				g = src[p - dataOff + 1];
				r = src[p - dataOff + 2];
				p += 3;
				break;
			case FMT_IA88:
			{
				std::uint8_t iv = src[p - dataOff + 0];
				a = src[p - dataOff + 1];
				r = g = b = iv;
				p += 2;
				break;
			}
			case FMT_I8:
			{
				std::uint8_t iv = src[p - dataOff + 0];
				r = g = b = iv;
				p += 1;
				break;
			}
			case FMT_A8:
				a = src[p - dataOff + 0];
				r = g = b = 255;
				p += 1;
				break;
			default:
				return false;
			}
			PutPixel( img, x, y, r, g, b, a );
		}
	}
	return true;
}

// Parses the header common to 7.1-7.5 and derives the byte offset of the mip-0
// high-res image and the image format. Returns false with 'error' set on failure.
bool ParseHeader(
    const std::string &b,
    std::string &error,
    VtfInfo &info,
    std::size_t &mip0Offset )
{
	if ( b.size() < 0x40 )
	{
		error = "vtf: file smaller than a v7.1 header";
		return false;
	}
	if ( b[0] != 'V' || b[1] != 'T' || b[2] != 'F' || b[3] != '\0' )
	{
		error = "vtf: bad signature";
		return false;
	}
	info.majorVersion = int( GetU32( b, 0x04 ) );
	info.minorVersion = int( GetU32( b, 0x08 ) );
	if ( info.majorVersion != 7 )
	{
		error = "vtf: unsupported major version " + std::to_string( info.majorVersion );
		return false;
	}
	const std::uint32_t headerSize = GetU32( b, 0x0c );
	info.width = GetU16( b, 0x10 );
	info.height = GetU16( b, 0x12 );
	info.flags = GetU32( b, 0x14 );
	info.frameCount = GetU16( b, 0x18 );
	info.imageFormat = GetS32( b, 0x34 );
	info.mipCount = b[0x38] & 0xff;
	const int lowResFormat = GetS32( b, 0x39 );
	const int lowResW = b[0x3d] & 0xff;
	const int lowResH = b[0x3e] & 0xff;

	if ( info.width <= 0 || info.height <= 0 )
	{
		error = "vtf: zero dimension";
		return false;
	}
	if ( info.mipCount < 1 )
		info.mipCount = 1;
	if ( info.frameCount < 1 )
		info.frameCount = 1;
	if ( !FormatSupported( info.imageFormat ) )
	{
		error = "vtf: unsupported image format " + std::to_string( info.imageFormat );
		return false;
	}

	const int faces = ( info.flags & kFlagEnvmap ) ? 6 : 1;

	// Locate the high-res image data. On 7.3+ the resource dictionary carries an
	// explicit offset; earlier versions place it right after the header + low-res
	// thumbnail.
	std::size_t imageDataStart = 0;
	bool located = false;
	if ( info.minorVersion >= 3 && b.size() >= 0x4c )
	{
		const std::uint32_t numResources = GetU32( b, 0x48 );
		std::size_t rp = 0x50;
		for ( std::uint32_t i = 0; i < numResources && i < 32; ++i, rp += 8 )
		{
			if ( rp + 8 > b.size() )
				break;
			const unsigned char t0 = b[rp], t1 = b[rp + 1], t2 = b[rp + 2];
			// VTF_LEGACY_RSRC_IMAGE = 0x30,0,0.
			if ( t0 == 0x30 && t1 == 0 && t2 == 0 )
			{
				imageDataStart = GetU32( b, rp + 4 );
				located = true;
				break;
			}
		}
	}
	if ( !located )
	{
		std::size_t lowResBytes = 0;
		if ( lowResFormat >= 0 && lowResW > 0 && lowResH > 0 )
			lowResBytes = FormatMipBytes( lowResFormat, lowResW, lowResH );
		imageDataStart = std::size_t( headerSize ) + lowResBytes;
	}

	// Mips are stored smallest-first. Skip every mip below level 0, across all
	// frames/faces, to reach mip 0 (frame 0, face 0 is then the first block).
	std::size_t skip = 0;
	for ( int mip = info.mipCount - 1; mip >= 1; --mip )
	{
		int mw = info.width >> mip;
		int mh = info.height >> mip;
		if ( mw < 1 )
			mw = 1;
		if ( mh < 1 )
			mh = 1;
		skip += FormatMipBytes( info.imageFormat, mw, mh ) * std::size_t( info.frameCount ) * faces;
	}
	mip0Offset = imageDataStart + skip;
	return true;
}

} // namespace

std::optional<VtfInfo> ReadVtfInfo( const std::string &bytes, std::string &error )
{
	VtfInfo info;
	std::size_t mip0 = 0;
	if ( !ParseHeader( bytes, error, info, mip0 ) )
		return std::nullopt;
	return info;
}

std::optional<VtfImage> DecodeVtf( const std::string &bytes, std::string &error )
{
	VtfInfo info;
	std::size_t mip0 = 0;
	if ( !ParseHeader( bytes, error, info, mip0 ) )
		return std::nullopt;

	VtfImage img;
	img.width = info.width;
	img.height = info.height;
	img.rgba.assign( std::size_t( img.width ) * img.height * 4, 0 );

	if ( !DecodeBlock( info.imageFormat, bytes, mip0, img ) )
	{
		error = "vtf: image data truncated or unsupported for mip 0";
		return std::nullopt;
	}
	return img;
}

} // namespace hammer::formats
