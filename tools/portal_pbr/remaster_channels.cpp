//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Preserve Source texture channels while staging high-resolution PBR
//          sets. Color detail is produced by the pinned external upscaler;
//          this tool owns exact alpha and vector-aware normal resizing.
//
//=============================================================================//

#include <png.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace
{

struct Image
{
	int width = 0;
	int height = 0;
	std::vector<std::uint8_t> rgba;
};

bool ReadPng( const std::string &path, Image &out )
{
	png_image png{};
	png.version = PNG_IMAGE_VERSION;
	if ( !png_image_begin_read_from_file( &png, path.c_str() ) )
	{
		std::cerr << path << ": " << png.message << '\n';
		return false;
	}
	if ( png.width == 0 || png.height == 0 || png.width > 4096 || png.height > 4096 )
	{
		std::cerr << path << ": invalid dimensions\n";
		png_image_free( &png );
		return false;
	}
	png.format = PNG_FORMAT_RGBA;
	out.width = static_cast<int>( png.width );
	out.height = static_cast<int>( png.height );
	out.rgba.resize( PNG_IMAGE_SIZE( png ) );
	if ( !png_image_finish_read( &png, nullptr, out.rgba.data(), 0, nullptr ) )
	{
		std::cerr << path << ": " << png.message << '\n';
		png_image_free( &png );
		return false;
	}
	png_image_free( &png );
	return true;
}

bool WritePng( const std::string &path, const Image &image )
{
	png_image png{};
	png.version = PNG_IMAGE_VERSION;
	png.width = static_cast<png_uint_32>( image.width );
	png.height = static_cast<png_uint_32>( image.height );
	png.format = PNG_FORMAT_RGBA;
	if ( !png_image_write_to_file( &png, path.c_str(), 0, image.rgba.data(), 0, nullptr ) )
	{
		std::cerr << path << ": " << png.message << '\n';
		png_image_free( &png );
		return false;
	}
	png_image_free( &png );
	return true;
}

std::array<float, 4> Pixel( const Image &image, int x, int y )
{
	x = std::clamp( x, 0, image.width - 1 );
	y = std::clamp( y, 0, image.height - 1 );
	const std::size_t start = ( std::size_t( y ) * image.width + x ) * 4;
	return { float( image.rgba[start] ), float( image.rgba[start + 1] ),
	    float( image.rgba[start + 2] ), float( image.rgba[start + 3] ) };
}

std::array<float, 4> Sample(
    const Image &image, int x, int y, int targetWidth, int targetHeight, bool nearest )
{
	const float sourceX = ( float( x ) + 0.5f ) * image.width / targetWidth - 0.5f;
	const float sourceY = ( float( y ) + 0.5f ) * image.height / targetHeight - 0.5f;
	if ( nearest )
		return Pixel( image, int( std::round( sourceX ) ), int( std::round( sourceY ) ) );
	const int x0 = int( std::floor( sourceX ) );
	const int y0 = int( std::floor( sourceY ) );
	const float fx = sourceX - x0;
	const float fy = sourceY - y0;
	const auto a = Pixel( image, x0, y0 );
	const auto b = Pixel( image, x0 + 1, y0 );
	const auto c = Pixel( image, x0, y0 + 1 );
	const auto d = Pixel( image, x0 + 1, y0 + 1 );
	std::array<float, 4> result{};
	for ( int channel = 0; channel < 4; ++channel )
	{
		result[channel] = ( ( 1.0f - fx ) * a[channel] + fx * b[channel] ) * ( 1.0f - fy ) +
		                  ( ( 1.0f - fx ) * c[channel] + fx * d[channel] ) * fy;
	}
	return result;
}

std::uint8_t Byte( float value )
{
	return static_cast<std::uint8_t>( std::round( std::clamp( value, 0.0f, 255.0f ) ) );
}

bool ComposeBase( const Image &original, Image &upscaled, bool oneBitAlpha )
{
	if ( upscaled.width % original.width != 0 || upscaled.height % original.height != 0 ||
	     upscaled.width / original.width != upscaled.height / original.height )
		return false;
	for ( int y = 0; y < upscaled.height; ++y )
	{
		for ( int x = 0; x < upscaled.width; ++x )
		{
			const float alpha =
			    Sample( original, x, y, upscaled.width, upscaled.height, oneBitAlpha )[3];
			upscaled.rgba[( std::size_t( y ) * upscaled.width + x ) * 4 + 3] =
			    oneBitAlpha ? ( alpha >= 128.0f ? 255 : 0 ) : Byte( alpha );
		}
	}
	return true;
}

Image ResizeChannels( const Image &original, int width, int height, bool normalMap )
{
	Image output;
	output.width = width;
	output.height = height;
	output.rgba.resize( std::size_t( width ) * height * 4 );
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			const auto sample = Sample( original, x, y, width, height, false );
			const std::size_t start = ( std::size_t( y ) * width + x ) * 4;
			if ( normalMap )
			{
				float nx = sample[0] / 127.5f - 1.0f;
				float ny = sample[1] / 127.5f - 1.0f;
				float nz = sample[2] / 127.5f - 1.0f;
				const float length = std::sqrt( nx * nx + ny * ny + nz * nz );
				if ( length < 0.001f )
				{
					nx = ny = 0.0f;
					nz = 1.0f;
				}
				else
				{
					nx /= length;
					ny /= length;
					nz /= length;
				}
				output.rgba[start] = Byte( ( nx + 1.0f ) * 127.5f );
				output.rgba[start + 1] = Byte( ( ny + 1.0f ) * 127.5f );
				output.rgba[start + 2] = Byte( ( nz + 1.0f ) * 127.5f );
			}
			else
			{
				for ( int channel = 0; channel < 3; ++channel )
					output.rgba[start + channel] = Byte( sample[channel] );
			}
			output.rgba[start + 3] = Byte( sample[3] );
		}
	}
	return output;
}

bool ParseSize( const char *width, const char *height, int &outWidth, int &outHeight )
{
	try
	{
		outWidth = std::stoi( width );
		outHeight = std::stoi( height );
	}
	catch ( ... )
	{
		return false;
	}
	return outWidth > 0 && outWidth <= 4096 && outHeight > 0 && outHeight <= 4096;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc == 6 && std::string( argv[1] ) == "compose-base" )
	{
		Image original, upscaled;
		if ( !ReadPng( argv[2], original ) || !ReadPng( argv[3], upscaled ) )
			return 2;
		if ( !ComposeBase( original, upscaled, std::string( argv[5] ) == "onebit" ) )
		{
			std::cerr << "upscaled image does not have a uniform integer scale\n";
			return 2;
		}
		return WritePng( argv[4], upscaled ) ? 0 : 1;
	}
	if ( argc == 6 && ( std::string( argv[1] ) == "normal" || std::string( argv[1] ) == "mrao" ) )
	{
		Image original;
		int width = 0, height = 0;
		if ( !ParseSize( argv[4], argv[5], width, height ) || !ReadPng( argv[2], original ) )
			return 2;
		const Image output =
		    ResizeChannels( original, width, height, std::string( argv[1] ) == "normal" );
		return WritePng( argv[3], output ) ? 0 : 1;
	}
	std::cerr << "usage: remaster_channels compose-base original.png upscaled.png output.png "
	             "onebit|linear | normal|mrao original.png output.png width height\n";
	return 2;
}
