//========= Copyright Valve Corporation, All rights reserved. ============//

#include "engine/audio/media_providers.h"
#include "vaudio/ivaudio.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

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

class Input : public IAudioStreamEvent
{
public:
	std::vector<unsigned char> bytes;
	int StreamRequestData( void *destination, int size, int offset ) override
	{
		if ( !destination || size < 0 || offset < 0 ||
		     static_cast<size_t>( offset ) >= bytes.size() )
			return 0;
		const size_t available = bytes.size() - offset;
		if ( static_cast<size_t>( size ) > available )
			size = static_cast<int>( available );
		std::memcpy( destination, bytes.data() + offset, size );
		return size;
	}
};

// Independent signal oracle: generated 440 Hz sine, mono 44.1 kHz. Ignore the
// encoder-delay region; lossy samples use energy/frequency ranges, not a golden.
static bool IsTone( const std::vector<short> &pcm )
{
	if ( pcm.size() < 11025 || pcm.size() > 14000 )
		return false;
	double energy = 0;
	unsigned int crossings = 0;
	for ( unsigned int i = 2048; i < 10868; ++i )
	{
		energy += static_cast<double>( pcm[i] ) * pcm[i];
		if ( ( pcm[i] < 0 ) != ( pcm[i - 1] < 0 ) )
			++crossings;
	}
	const double meanSquare = energy / 8820;
	return meanSquare > 2000000 && meanSquare < 16000000 && crossings >= 160 && crossings <= 192;
}

int main( int argc, char **argv )
{
	Check( argc == 2, "fixture path is required" );
	FILE *file = std::fopen( argv[1], "rb" );
	Check( file != nullptr, "required generated MP3 fixture exists" );
	Input input;
	unsigned char buffer[4096];
	size_t bytes;
	while ( ( bytes = std::fread( buffer, 1, sizeof( buffer ), file ) ) > 0 )
		input.bytes.insert( input.bytes.end(), buffer, buffer + bytes );
	Check( !std::ferror( file ) && !input.bytes.empty(), "fixture read complete and nonempty" );
	std::fclose( file );

	const audio::MP3Provider *provider = Audio_MP3Provider();
	Check( provider && std::strcmp( provider->id, "minimp3" ) == 0, "linked decoder identified" );
	for ( int instance = 0; instance < 2; ++instance )
	{
		IVAudio *audio = provider->create();
		Check( audio != nullptr, "fresh MP3 service" );
		Check( !audio->CreateMP3StreamDecoder( nullptr ), "missing input handler rejected" );
		IAudioStream *stream = audio->CreateMP3StreamDecoder( &input );
		Check( stream != nullptr, "actual MP3 decoder accepts generated input" );
		Check( stream->GetOutputRate() == 44100 && stream->GetOutputBits() == 16 &&
		           stream->GetOutputChannels() == 1,
		    "decoded signal format" );
		std::vector<short> pcm;
		short samples[1024];
		for ( unsigned int block = 0; block < 64; ++block )
		{
			const int decoded = stream->Decode( samples, sizeof( samples ) );
			Check( decoded >= 0 && decoded <= static_cast<int>( sizeof( samples ) ) &&
			           decoded % 2 == 0,
			    "decoder output remains within caller buffer" );
			if ( !decoded )
				break;
			pcm.insert( pcm.end(), samples, samples + decoded / 2 );
		}
		Check( IsTone( pcm ), "independent tone energy and frequency oracle" );
		Check( !IsTone( std::vector<short>( pcm.size(), 0 ) ),
		    "signal oracle rejects seeded silence" );
		audio->DestroyMP3StreamDecoder( stream );
		delete audio;
	}
	std::printf( "PASS linked MP3 provider: %u checks\n", s_Checks );
	return 0;
}
