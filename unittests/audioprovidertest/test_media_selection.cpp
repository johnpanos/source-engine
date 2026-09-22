//========= Copyright Valve Corporation, All rights reserved. ============//

#include "engine/audio/media_providers.h"
#include <cstdio>
#include <cstdlib>
#include <string>

class IVoiceRecord
{
};
static IVoiceRecord s_Recorder;
static std::string s_Attempts;
static bool s_RecordAvailable;
static unsigned int s_Checks;
static int s_SampleRate;

static void Check( bool condition, const char *description )
{
	++s_Checks;
	if ( !condition )
	{
		std::fprintf( stderr, "FAIL: %s\n", description );
		std::exit( 1 );
	}
}

static IVAudio *MP3()
{
	return nullptr;
}
static IVoiceCodec *Voice()
{
	return nullptr;
}
static IVoiceRecord *Record( int sampleRate )
{
	s_Attempts += 'R';
	s_SampleRate = sampleRate;
	return s_RecordAvailable ? &s_Recorder : nullptr;
}
static IVoiceRecord *Fallback( int sampleRate )
{
	s_Attempts += 'F';
	s_SampleRate = sampleRate;
	return &s_Recorder;
}

int main()
{
	const audio::MP3Provider mp3 = { "mp3", MP3 };
	const audio::VoiceCodecProvider opus = { "vaudio_opus", Voice };
	const audio::VoiceRecordProvider recording = { "record", Record };
	const audio::VoiceRecordProvider fallback = { "fallback", Fallback };
	audio::VoiceProtocolBinding bindings[] = {
	    { "vaudio_celt", &opus, 3 }, { "vaudio_speex", &opus, 4 }, { "vaudio_opus", &opus, 3 } };
	audio::MediaProviders providers;
	providers.mp3 = &mp3;
	providers.recording = &recording;
	providers.recordingFallback = &fallback;
	providers.voiceBindings = bindings;
	providers.voiceBindingCount = 3;
	Check( providers.IsValid(), "complete catalog" );
	Check(
	    providers.FindVoice( "VAUDIO_CELT" ) == &bindings[0], "legacy case-insensitive protocol" );
	Check( providers.FindVoice( "vaudio_speex" )->quality == 4,
	    "fallback preserves negotiated quality" );
	Check( providers.FindVoice( "vaudio_opus" )->provider == &opus, "exact provider binding" );
	Check( !providers.FindVoice( "vaudio_opus.dll" ), "filenames cannot select providers" );
	Check( !providers.FindVoice( "../vaudio_opus" ), "paths cannot select providers" );
	Check( !providers.FindVoice( nullptr ), "missing protocol rejected" );
	Check(
	    !providers.FindVoice( "steam" ), "Steam remains its explicit independent provider path" );

	s_RecordAvailable = true;
	Check( providers.CreateRecorder( 22050 ) == &s_Recorder, "primary recorder succeeds" );
	Check( s_Attempts == "R" && s_SampleRate == 22050, "rate preserved and no extra fallback" );
	s_RecordAvailable = false;
	s_Attempts.clear();
	Check( providers.CreateRecorder( 11025 ) == &s_Recorder, "capture fallback after failure" );
	Check( s_Attempts == "RF" && s_SampleRate == 11025, "capture fallback order and rate" );
	s_Attempts.clear();
	Check( !providers.CreateRecorder( 0 ), "invalid rate fails before provider acquisition" );
	Check( s_Attempts.empty(), "invalid rate makes no callback" );
	providers.recordingFallback = &recording;
	Check( !providers.CreateRecorder( 11025 ), "duplicate failing recorder is not repeated" );
	Check( s_Attempts == "R", "one attempt for duplicated descriptor" );

	bindings[1].protocolName = "VAUDIO_CELT";
	s_Attempts.clear();
	Check( !providers.IsValid(), "duplicate protocol aliases rejected" );
	Check(
	    !providers.CreateRecorder( 22050 ), "malformed unrelated codec blocks partial mutation" );
	Check( s_Attempts.empty(), "malformed catalog acquires no recorder" );
	bindings[1].protocolName = "vaudio_speex";
	bindings[1].quality = -1;
	Check( !providers.IsValid(), "invalid negotiated quality rejected" );
	bindings[1].quality = 4;
	providers.voiceBindings = nullptr;
	Check( !providers.IsValid(), "missing required binding array rejected" );
	providers.voiceBindingCount = 0;
	Check( providers.IsValid(), "voice-disabled product is explicit" );
	Check( !providers.FindVoice( "vaudio_celt" ), "unbuilt codec does not silently succeed" );
	providers.mp3 = nullptr;
	Check( !providers.IsValid(), "required MP3 provider absent" );

	std::printf( "PASS audio media selection: %u checks\n", s_Checks );
	return 0;
}
