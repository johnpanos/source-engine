//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef ENGINE_AUDIO_DEVICE_SELECTION_H
#define ENGINE_AUDIO_DEVICE_SELECTION_H

class IAudioDevice;
class IVAudio;
class IVoiceRecord;
namespace audio
{
struct VoiceProtocolBinding;
}

// Private bridge from the existing sound owner to its containing engine's
// injected plan. No catalog, native SDK, or configuration lookup crosses it.
extern "C" IAudioDevice *Engine_CreateSelectedAudioDevice( bool firstStart, bool waveOnly );

extern "C" IVAudio *Engine_CreateMP3Audio();
extern "C" const audio::VoiceProtocolBinding *Engine_FindVoiceCodec( const char *protocolName );
extern "C" const audio::VoiceProtocolBinding *Engine_DefaultVoiceCodec();
extern "C" IVoiceRecord *Engine_CreateVoiceRecorder( int sampleRate );

#endif // ENGINE_AUDIO_DEVICE_SELECTION_H
