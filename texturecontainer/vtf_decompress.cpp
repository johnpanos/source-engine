//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: The CPU decompressor for VTF 7.6 AXC mip runs (P2:CE / Strata).
//=============================================================================//
#include "texturecontainer/vtf_decompress.h"

#include <limits>

#include <zlib.h>
#ifdef VTF_DECOMPRESS_ZSTD
#include <zstd.h>
#endif

namespace texturecontainer::vtf
{
bool SupportsDecompression( CompressionMethod method ) noexcept
{
	switch ( method )
	{
	case CompressionMethod::Deflate:
		return true;
	case CompressionMethod::Zstandard:
#ifdef VTF_DECOMPRESS_ZSTD
		return true;
#else
		return false;
#endif
	}
	return false;
}

const char *DecompressInto( CompressionMethod method, const void *input, std::size_t inputSize,
    void *output, std::size_t outputSize ) noexcept
{
	switch ( method )
	{
	case CompressionMethod::Deflate:
	{
		if ( inputSize > std::numeric_limits<uLong>::max() ||
		     outputSize > std::numeric_limits<uLongf>::max() )
			return "vtf: Deflate run exceeds zlib size limits";
		uLongf size = uLongf( outputSize );
		if ( uncompress( static_cast<Bytef *>( output ), &size, static_cast<const Bytef *>( input ),
		         uLong( inputSize ) ) == Z_OK &&
		     size == outputSize )
			return nullptr;
		return "vtf: Deflate run failed or returned an incomplete image";
	}
	case CompressionMethod::Zstandard:
#ifdef VTF_DECOMPRESS_ZSTD
	{
		const std::size_t size = ZSTD_decompress( output, outputSize, input, inputSize );
		if ( !ZSTD_isError( size ) && size == outputSize )
			return nullptr;
		return "vtf: Zstandard run failed or returned an incomplete image";
	}
#else
		return "vtf: this build has no Zstandard decompressor (libzstd)";
#endif
	}
	return "vtf: unknown compression method";
}

bool Decompress( CompressionMethod method, std::string_view input, std::span<std::uint8_t> output,
    std::string &error )
{
	if ( const char *failure =
	         DecompressInto( method, input.data(), input.size(), output.data(), output.size() ) )
	{
		error = failure;
		return false;
	}
	return true;
}
} // namespace texturecontainer::vtf
