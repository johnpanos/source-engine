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

// The definitions export these from their module (DLL_EXPORT); MSVC requires
// the declarations to agree (C2375).
#if defined( _MSC_VER )
#define DEVICE_SELECTION_EXPORT __declspec( dllexport )
#else
#define DEVICE_SELECTION_EXPORT
#endif

// Private bridge from the existing sound owner to its containing engine's
// injected plan. No catalog, native SDK, or configuration lookup crosses it.
extern "C" DEVICE_SELECTION_EXPORT IAudioDevice *Engine_CreateSelectedAudioDevice(
    bool firstStart, bool waveOnly );

extern "C" DEVICE_SELECTION_EXPORT IVAudio *Engine_CreateMP3Audio();
extern "C" DEVICE_SELECTION_EXPORT const audio::VoiceProtocolBinding *Engine_FindVoiceCodec(
    const char *protocolName );
extern "C" DEVICE_SELECTION_EXPORT const audio::VoiceProtocolBinding *Engine_DefaultVoiceCodec();
extern "C" DEVICE_SELECTION_EXPORT IVoiceRecord *Engine_CreateVoiceRecorder( int sampleRate );

#endif // ENGINE_AUDIO_DEVICE_SELECTION_H
