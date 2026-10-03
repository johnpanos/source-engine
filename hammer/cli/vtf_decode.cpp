//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: Headless material import through Hammer's shared VTF decoder.
#include "hammer/formats/vtf_image.h"

#include <fstream>
#include <iostream>
#include <string>
#include <zlib.h>
#include <zstd.h>

namespace
{
bool Decompress( hammer::formats::VtfCompressionMethod method, std::string_view input,
    std::span<std::uint8_t> output, std::string &error )
{
	if ( method == hammer::formats::VtfCompressionMethod::Zstandard )
	{
		const auto size =
		    ZSTD_decompress( output.data(), output.size(), input.data(), input.size() );
		if ( !ZSTD_isError( size ) && size == output.size() )
			return true;
	}
	else if ( method == hammer::formats::VtfCompressionMethod::Deflate )
	{
		uLongf size = output.size();
		if ( uncompress( output.data(), &size, reinterpret_cast<const Bytef *>( input.data() ),
		         input.size() ) == Z_OK &&
		     size == output.size() )
			return true;
	}
	error = "VTF decompression failed or returned an incomplete image";
	return false;
}
} // namespace

int main( int argc, char **argv )
{
	if ( argc != 3 )
	{
		std::cerr << "usage: hammer_vtf_decode input.vtf output.pam\n";
		return 2;
	}
	std::ifstream input( argv[1], std::ios::binary | std::ios::ate );
	const auto size = input.tellg();
	if ( !input || size <= 0 || size > 1024 * 1024 * 1024 )
	{
		std::cerr << "VTF input missing or exceeds the 1 GiB host-tool bound\n";
		return 1;
	}
	std::string encoded( static_cast<std::size_t>( size ), '\0' );
	input.seekg( 0 );
	if ( !input.read( encoded.data(), size ) )
		return 1;
	std::string error;
	auto image = hammer::formats::DecodeVtf( encoded, error, Decompress );
	if ( !image )
	{
		std::cerr << error << '\n';
		return 1;
	}
	// Open the output only after the complete input was validated and decoded.
	std::ofstream output( argv[2], std::ios::binary );
	output << "P7\nWIDTH " << image->width << "\nHEIGHT " << image->height
	       << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
	output.write( reinterpret_cast<const char *>( image->rgba.data() ), image->rgba.size() );
	output.close();
	return output ? 0 : 1;
}
