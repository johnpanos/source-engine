//========= Copyright Valve Corporation, All rights reserved. ============//

#include "engine/audio/media_providers.h"
#include "engine/audio/ivoicecodec.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

static unsigned int s_Checks;
static void Check( bool condition, const char *description )
{
	++s_Checks;
	if ( !condition )
	{
		std::fprintf( stderr, "FAIL: %s\n", description );
		std::exit( 1 );
	}
}

int main()
{
	const audio::VoiceCodecProvider *provider = Audio_OpusVoiceProvider();
	Check( provider && std::strcmp( provider->id, "vaudio_opus" ) == 0, "linked Opus provider" );
	const int invalid[] = { -1, 4, INT_MAX };
	for ( int quality : invalid )
	{
		IVoiceCodec *codec = provider->create();
		Check( codec != nullptr, "independent codec instance" );
		Check( !codec->Init( quality ), "unsupported negotiated quality rejected" );
		codec->Release();
	}

	for ( int instance = 0; instance < 2; ++instance )
	{
		IVoiceCodec *encoder = provider->create();
		IVoiceCodec *decoderA = provider->create();
		IVoiceCodec *decoderB = provider->create();
		Check( encoder && decoderA && decoderB, "independent encode and channel instances" );
		Check( encoder->Init( 3 ) && decoderA->Init( 3 ) && decoderB->Init( 3 ),
		    "negotiated quality 3 initializes" );
		short input[512];
		for ( unsigned int i = 0; i < 512; ++i )
			input[i] = i % 64 < 32 ? 4000 : -4000;
		char packet[128];
		const int encoded = encoder->Compress(
		    reinterpret_cast<const char *>( input ), 512, packet, sizeof( packet ), true );
		Check( encoded == 64, "legacy quality 3 packet size" );
		short outputA[512] = {};
		short outputB[512] = {};
		Check( decoderA->Decompress(
		           packet, encoded, reinterpret_cast<char *>( outputA ), sizeof( outputA ) ) == 512,
		    "channel A decoded sample count" );
		Check( decoderB->Decompress(
		           packet, encoded, reinterpret_cast<char *>( outputB ), sizeof( outputB ) ) == 512,
		    "channel B decoded sample count" );
		Check( std::memcmp( outputA, outputB, sizeof( outputA ) ) == 0,
		    "fresh decoder channels preserve identical independent state" );
		long long energy = 0;
		for ( short sample : outputA )
			energy += static_cast<long long>( sample ) * sample;
		Check( energy > 1000000, "decoded PCM is not silent" );
		Check( decoderA->ResetState(), "codec state reset" );
		Check( decoderA->Decompress(
		           packet, encoded, reinterpret_cast<char *>( outputA ), sizeof( outputA ) ) == 512,
		    "decode after reset" );
		Check( std::memcmp( outputA, outputB, sizeof( outputA ) ) == 0,
		    "reset restores initial decoder output" );
		encoder->Release();
		decoderA->Release();
		decoderB->Release();
	}
	std::printf( "PASS linked Opus provider: %u checks\n", s_Checks );
	return 0;
}
