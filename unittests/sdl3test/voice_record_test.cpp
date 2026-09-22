//========= Copyright Valve Corporation, All rights reserved. ============//
// Native SDL3 capture and bounded-transport conformance for the actual provider.
// Compile with the engine's SDL3 flags; SDL_AUDIO_DRIVER=dummy avoids microphone access.

#include "../../engine/audio/voice_record_sdl.cpp"

#include <stdio.h>

namespace
{
int s_Checks = 0;

bool Check( bool result, const char *description )
{
	++s_Checks;
	if ( !result )
		fprintf( stderr, "FAIL: %s\n", description );
	return result;
}
}

int main()
{
	bool passed = true;
	char storage[8] = {};
	AudioBuf buffer;
	buffer.data = buffer.readPtr = buffer.writePtr = storage;
	buffer.size = sizeof( storage );
	buffer.used = 0;
	char source[] = "abcdefghijklmnop";
	char output[8] = {};
	buffer.Write( source, 6 );
	passed &= Check( buffer.Read( output, 4 ) == 4 && memcmp( output, "abcd", 4 ) == 0,
	    "capture preserves sample order before wrap" );
	buffer.Write( source + 6, 6 );
	passed &= Check( buffer.Read( output, 8 ) == 8 && memcmp( output, "efghijkl", 8 ) == 0,
	    "exactly full buffer is readable after wrap" );
	buffer.Write( source, 12 );
	passed &= Check( buffer.Read( output, 8 ) == 8 && memcmp( output, "efghijkl", 8 ) == 0,
	    "oversized capture retains newest samples without overflow" );
	buffer.Write( source, 6 );
	buffer.Write( source + 6, 6 );
	passed &= Check( buffer.Read( output, 8 ) == 8 && memcmp( output, "efghijkl", 8 ) == 0,
	    "slow consumer loses oldest samples with a bounded buffer" );
	passed &= Check( buffer.Read( output, 8 ) == 0, "drained buffer is empty" );
	passed &= Check( CreateVoiceRecord_SDL( 0 ) == NULL, "invalid sample rate is rejected" );
	passed &=
	    Check( CreateVoiceRecord_SDL( 200000 ) == NULL, "unsupported sample rate is rejected" );
	for ( int instance = 0; instance < 2; ++instance )
	{
		IVoiceRecord *voice = CreateVoiceRecord_SDL( 16000 );
		if ( !Check( voice != NULL, "construct a fresh recorder" ) )
			return 1;
		short samples[512];
		passed &=
		    Check( voice->GetRecordedData( samples, 512 ) == 0, "stopped recorder has no samples" );
		for ( int cycle = 0; cycle < 2; ++cycle )
		{
			passed &= Check( voice->RecordStart(), "start or restart recording" );
			int received = 0;
			for ( int attempt = 0; attempt < 100 && !received; ++attempt )
			{
				SDL_Delay( 10 );
				received = voice->GetRecordedData( samples, 512 );
			}
			passed &=
			    Check( received > 0 && received <= 512, "capture produces bounded PCM samples" );
			passed &=
			    Check( voice->GetRecordedData( NULL, 512 ) == 0, "null destination is rejected" );
			passed &= Check( voice->GetRecordedData( samples, -1 ) == 0,
			    "negative destination count is rejected" );
			voice->RecordStop();
			passed &=
			    Check( voice->GetRecordedData( samples, 512 ) == 0, "stop releases capture state" );
		}
		voice->Release();
		passed &= Check( !SDL_WasInit( SDL_INIT_AUDIO ), "release balances the audio subsystem" );
	}
	printf( "SDL3 voice recording: %d checks, %s\n", s_Checks, passed ? "pass" : "fail" );
	return passed ? 0 : 1;
}
