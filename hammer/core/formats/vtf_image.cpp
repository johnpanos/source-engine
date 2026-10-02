//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the strict-core VTF decoder. See
//			public/hammer/formats/vtf_image.h for the contract and scope.
//
//=============================================================================//

#include "hammer/formats/vtf_image.h"
#include "texturecontainer/vtf_container.h"

#define BCDECDEF static inline
#define BCDEC_IMPLEMENTATION
#include "external/bcdec/bcdec.h"

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
	FMT_STRATA_BC7 = 70,
};

std::size_t FormatMipBytes( int format, int width, int height )
{
	return texturecontainer::vtf::ImageBytes( format, width, height );
}

void PutPixel(
    VtfImage &img, int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a )
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
	std::uint32_t bits = std::uint32_t( src[4] ) | ( std::uint32_t( src[5] ) << 8 ) |
	                     ( std::uint32_t( src[6] ) << 16 ) | ( std::uint32_t( src[7] ) << 24 );
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
	case FMT_STRATA_BC7:
	{
		const int blocksX = ( img.width + 3 ) / 4;
		const int blocksY = ( img.height + 3 ) / 4;
		for ( int byBlock = 0; byBlock < blocksY; ++byBlock )
		{
			for ( int bxBlock = 0; bxBlock < blocksX; ++bxBlock )
			{
				const unsigned char *block =
				    src + std::size_t( byBlock * blocksX + bxBlock ) * BCDEC_BC7_BLOCK_SIZE;
				unsigned char pixels[4 * 4 * 4];
				bcdec_bc7( block, pixels, 4 * 4 );
				for ( int y = 0; y < 4; ++y )
				{
					for ( int x = 0; x < 4; ++x )
					{
						const unsigned char *pixel = pixels + ( y * 4 + x ) * 4;
						PutPixel( img, bxBlock * 4 + x, byBlock * 4 + y, pixel[0], pixel[1],
						    pixel[2], pixel[3] );
					}
				}
			}
		}
		return true;
	}
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
				const unsigned char *blk =
				    src + std::size_t( byBlock * blocksX + bxBlock ) * blockBytes;
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

} // namespace

std::optional<VtfInfo> ReadVtfInfo( const std::string &bytes, std::string &error )
{
	auto header = texturecontainer::vtf::ReadHeader(
	    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
	if ( !header )
	{
		error = header.Error();
		return std::nullopt;
	}
	const auto &h = header.Value();
	return VtfInfo{
	    h.width, h.height, 7, int( h.minor ), h.format, h.mips, h.frames, h.depth, h.flags };
}

std::optional<VtfImage> DecodeVtf( const std::string &bytes, std::string &error )
{
	return DecodeVtf( bytes, error, nullptr );
}

std::optional<VtfImage> DecodeVtf(
    const std::string &bytes, std::string &error, VtfMipDecompressor decompressor )
{
	const auto encoded = std::as_bytes( std::span( bytes.data(), bytes.size() ) );
	auto header = texturecontainer::vtf::ReadHeader( encoded );
	if ( !header )
	{
		error = header.Error();
		return std::nullopt;
	}
	auto layout = texturecontainer::vtf::ReadLayout( encoded, header.Value() );
	if ( !layout )
	{
		error = layout.Error();
		return std::nullopt;
	}
	// This adapter is a preview: first frame, first cube face, first volume
	// slice. The container reader still validates every authored image run.
	const auto &h = header.Value();
	for ( const auto &run : layout.Value().images )
	{
		if ( run.mip || run.frame || run.face )
			continue;
		auto data = texturecontainer::vtf::ReadImage( encoded, layout.Value(), run, decompressor );
		if ( !data )
		{
			error = data.Error();
			return std::nullopt;
		}
		VtfImage img{ h.width, h.height, {} };
		const auto rgbaBytes = texturecontainer::vtf::ImageBytes( 0, h.width, h.height );
		if ( !rgbaBytes )
		{
			error = "vtf: preview exceeds image size limit";
			return std::nullopt;
		}
		img.rgba.resize( rgbaBytes );
		const std::string pixels(
		    reinterpret_cast<const char *>( data.Value().data() ), data.Value().size() );
		if ( !DecodeBlock( h.format, pixels, 0, img ) )
		{
			error = "vtf: unsupported preview pixel format";
			return std::nullopt;
		}
		return img;
	}
	error = "vtf: missing preview image";
	return std::nullopt;
}

} // namespace hammer::formats
