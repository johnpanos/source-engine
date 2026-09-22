//========= Copyright Valve Corporation, All rights reserved. ============//
// Exercise the actual engine binding and legacy consumer without native audio.

#include "tier0/platform.h"
#include "appframework/linked_systems.h"
#include "engine_launcher_api.h"
#include "engine/audio/device_provider.h"
#include "engine/audio/media_providers.h"
#include "vaudio/ivaudio.h"
#include "../../engine/audio/device_selection.h"
#include <cstdio>
#include <cstdlib>

static unsigned int s_Checks;
static unsigned int s_Calls;

static void Check( bool condition, const char *description )
{
	++s_Checks;
	if ( !condition )
	{
		std::fprintf( stderr, "FAIL: %s\n", description );
		std::exit( 1 );
	}
}

static IAudioDevice *CountingFactory()
{
	++s_Calls;
	return Audio_NullProvider()->create();
}

static IVAudio *MP3Factory()
{
	++s_Calls;
	return Audio_MP3Provider()->create();
}

static IVoiceRecord *RecordFactory( int )
{
	++s_Calls;
	return nullptr;
}

static IVoiceCodec *VoiceFactory()
{
	return nullptr;
}

int main()
{
	IEngineAPI *engine = Engine_CreateClientAPI();
	Check( engine != nullptr, "linked engine exists" );
	// Missing required injection fails before a legacy factory can be consulted.
	Check( !engine->Connect( nullptr ), "unbound engine rejects Connect" );
	const audio::DeviceProvider primary = { "binding-fixture", CountingFactory };
	audio::DeviceSelection selection;
	selection.primary = &primary;
	selection.nullProvider = Audio_NullProvider();
	selection.allowNullFallback = false;
	Check( !Engine_BindAudioProviders( nullptr, &selection ), "null engine rejected" );
	IEngineAPI *foreign = reinterpret_cast<IEngineAPI *>( Audio_NullProvider()->create() );
	Check( !Engine_BindAudioProviders( foreign, &selection ), "foreign engine rejected" );
	Check( !Engine_BindAudioProviders( engine, nullptr ), "missing selection rejected" );
	Check( Engine_BindAudioProviders( engine, &selection ), "complete selection accepted" );
	Check( s_Calls == 0, "binding does not initialize audio early" );

	// The engine owns a copy: stack-plan changes cannot change its selection.
	selection.primary = nullptr;
	Check( !Engine_BindAudioProviders( engine, &selection ), "malformed replacement rejected" );
	Check( Engine_CreateSelectedAudioDevice( true, false ) == Audio_NullProvider()->create(),
	    "legacy startup consumes retained valid selection" );
	Check( s_Calls == 1, "rejected replacement preserved primary factory" );
	Check( Engine_CreateSelectedAudioDevice( false, true ) == Audio_NullProvider()->create(),
	    "legacy restart and wave-only compatibility use injected selection" );
	Check( s_Calls == 2, "restart invokes selected factory once" );

	const audio::MP3Provider mp3 = { "mp3-fixture", MP3Factory };
	const audio::VoiceRecordProvider recording = { "record-fixture", RecordFactory };
	const audio::VoiceCodecProvider voice = { "codec-fixture", VoiceFactory };
	const audio::VoiceProtocolBinding binding = { "vaudio_celt", &voice, 3 };
	audio::MediaProviders media;
	media.mp3 = &mp3;
	media.recording = &recording;
	media.voiceBindings = &binding;
	media.voiceBindingCount = 1;
	Check( !Engine_BindAudioMediaProviders( nullptr, &media ), "null media consumer rejected" );
	Check( !Engine_BindAudioMediaProviders( foreign, &media ), "foreign media consumer rejected" );
	Check( !Engine_BindAudioMediaProviders( engine, nullptr ), "null media plan rejected" );
	Check( Engine_BindAudioMediaProviders( engine, &media ), "complete media plan copied" );
	Check( s_Calls == 2, "media binding does not acquire decoder or recorder" );
	media.mp3 = nullptr;
	Check(
	    !Engine_BindAudioMediaProviders( engine, &media ), "malformed media replacement rejected" );
	IVAudio *decoder = Engine_CreateMP3Audio();
	Check( decoder != nullptr, "retained MP3 provider creates actual decoder service" );
	delete decoder;
	Check( s_Calls == 3, "copied MP3 factory survived invalid plan" );
	Check(
	    Engine_FindVoiceCodec( "VAUDIO_CELT" ) == &binding, "retained protocol catalog consumed" );
	Check( !Engine_FindVoiceCodec( "vaudio_celt.so" ), "consumer rejects codec filenames" );
	Check( !Engine_CreateVoiceRecorder( 22050 ), "optional recorder failure is explicit" );
	Check( s_Calls == 4, "recorder callback invoked once" );

	std::printf( "PASS engine audio binding: %u checks\n", s_Checks );
	return 0;
}
