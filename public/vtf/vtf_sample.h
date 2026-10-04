//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A texture decoded on the CPU for sampling its color (RFC 0011):
//          emissive surfaces' emission (game/client/emissive_area_lights.cpp)
//          and projected lights' cookies (engine/projected_lights.cpp). The
//          mip no larger than a limit, as linear RGB (sRGB decoded) and alpha.
//
//===========================================================================//

#ifndef VTF_SAMPLE_H
#define VTF_SAMPLE_H

#include "bitmap/imageformat.h"
#include "mathlib/mathlib.h"
#include "tier1/utlbuffer.h"
#include "vtf/vtf.h"

#include <cmath>
#include <vector>

namespace vtf_sample
{

struct Texture
{
	bool valid = false;
	int width = 0;
	int height = 0;
	std::vector<float> rgb; // linear
	std::vector<float> alpha;

	// Nearest texel, wrapping.
	void Fetch( float u, float v, float out[3], float *pAlpha ) const
	{
		u -= floorf( u );
		v -= floorf( v );
		const int x = clamp( int( u * width ), 0, width - 1 );
		const int y = clamp( int( v * height ), 0, height - 1 );
		const int i = y * width + x;
		for ( int k = 0; k < 3; ++k )
			out[k] = rgb[3 * i + k];
		*pAlpha = alpha[i];
	}
};

// Decodes a VTF file's first mip no larger than `maxDimension`.
inline bool Decode( CUtlBuffer &buf, int maxDimension, Texture &out, int frame = 0 )
{
	out = Texture();
	IVTFTexture *pVTF = CreateVTFTexture();
	if ( pVTF->Unserialize( buf, true ) )
	{
		int nSkip = 0;
		int w = pVTF->Width(), h = pVTF->Height();
		while ( MAX( w, h ) > maxDimension && nSkip < pVTF->MipCount() - 1 )
		{
			w = MAX( w >> 1, 1 );
			h = MAX( h >> 1, 1 );
			++nSkip;
		}
		buf.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );
		if ( pVTF->Unserialize( buf, false, nSkip ) && MAX( frame, 0 ) < pVTF->FrameCount() )
		{
			pVTF->ConvertImageFormat( IMAGE_FORMAT_RGBA8888, false );
			out.width = pVTF->Width();
			out.height = pVTF->Height();
			const int selected = MAX( frame, 0 );
			const unsigned char *pData = pVTF->ImageData( selected, 0, 0 );
			const int nTexels = out.width * out.height;
			out.rgb.resize( size_t( nTexels ) * 3 );
			out.alpha.resize( size_t( nTexels ) );
			for ( int i = 0; i < nTexels; ++i )
			{
				for ( int k = 0; k < 3; ++k )
					out.rgb[3 * i + k] = SrgbGammaToLinear( pData[4 * i + k] / 255.0f );
				out.alpha[i] = pData[4 * i + 3] / 255.0f;
			}
			out.valid = nTexels > 0;
		}
	}
	DestroyVTFTexture( pVTF );
	return out.valid;
}

} // namespace vtf_sample

#endif // VTF_SAMPLE_H
