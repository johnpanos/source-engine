//========= Copyright Valve Corporation, All rights reserved. ============//
// Linked media factories used only by composition and the legacy audio adapter.

#ifndef ENGINE_AUDIO_MEDIA_PROVIDERS_H
#define ENGINE_AUDIO_MEDIA_PROVIDERS_H

class IVAudio;
class IVoiceCodec;
class IVoiceRecord;
class IEngineAPI;

namespace audio
{

// MP3 ownership: destroy every stream before deleting its IVAudio owner.
struct MP3Provider
{
	const char *id;
	IVAudio *( *create )();
};

// Voice instances are independent encoder/channel state. Call Init with the
// binding's negotiated quality, and Release on both failed Init and shutdown.
struct VoiceCodecProvider
{
	const char *id;
	IVoiceCodec *( *create )();
};

struct VoiceProtocolBinding
{
	const char *protocolName;
	const VoiceCodecProvider *provider;
	int quality;
};

// A successful factory owns an initialized recorder. Release acknowledges
// callback/thread shutdown before its storage is reclaimed. A failed factory
// cleans up before returning null, permitting the declared fallback.
struct VoiceRecordProvider
{
	const char *id;
	IVoiceRecord *( *create )( int sampleRate );
};

// The product root owns the immutable binding array and descriptors through
// engine Disconnect. Linked provider code outlives all decoders and recorders.
// Empty voice bindings explicitly disable non-Steam voice in this profile.
struct MediaProviders
{
	const MP3Provider *mp3 = nullptr;
	const VoiceProtocolBinding *voiceBindings = nullptr;
	unsigned int voiceBindingCount = 0;
	const char *localVoiceProtocol = nullptr;
	const VoiceRecordProvider *recording = nullptr;
	const VoiceRecordProvider *recordingFallback = nullptr;

	bool IsValid() const;
	const VoiceProtocolBinding *FindVoice( const char *protocolName ) const;
	IVoiceRecord *CreateRecorder( int sampleRate ) const;
};

} // namespace audio

extern "C" const audio::MP3Provider *Audio_MP3Provider();
extern "C" const audio::VoiceCodecProvider *Audio_OpusVoiceProvider();
extern "C" const audio::VoiceRecordProvider *VoiceRecord_SDLProvider();
extern "C" const audio::VoiceRecordProvider *VoiceRecord_AudioQueueProvider();
extern "C" const audio::VoiceRecordProvider *VoiceRecord_OpenALProvider();
extern "C" const audio::VoiceRecordProvider *VoiceRecord_DirectSoundProvider();
extern "C" bool Engine_BindAudioMediaProviders(
    IEngineAPI *engine, const audio::MediaProviders *providers );

#endif // ENGINE_AUDIO_MEDIA_PROVIDERS_H
