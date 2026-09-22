//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Typed linked audio providers for product composition. Native SDKs stay in
// the provider. This does not change the preserved IAudioDevice vtable.
//
//=============================================================================//

#ifndef ENGINE_AUDIO_DEVICE_PROVIDER_H
#define ENGINE_AUDIO_DEVICE_PROVIDER_H

class IAudioDevice;
class IEngineAPI;

namespace audio
{

// create returns an initialized device, or null after releasing partial native
// resources. Existing engine ownership applies: Shutdown then delete, except
// for the immortal null device. Descriptors and their linked code outlive the
// engine and every audio callback. Creation/restart runs on the engine sequence.
struct DeviceProvider
{
	const char *id;
	IAudioDevice *( *create )();
};

// The composition root chooses these borrowed descriptors from its Waf-built
// catalog. Fallback is explicit product policy. waveOnly preserves the legacy
// Windows caller override; a null override leaves the ordinary plan unchanged.
struct DeviceSelection
{
	const DeviceProvider *primary = nullptr;
	const DeviceProvider *fallback = nullptr;
	const DeviceProvider *waveOnly = nullptr;
	const DeviceProvider *nullProvider = nullptr;
	bool primaryOnRestart = true;
	bool allowNullFallback = true;

	bool IsValid() const;
	const DeviceProvider *Create(
	    bool firstStart, bool waveOnlyRequested, IAudioDevice **device ) const;
};

} // namespace audio

extern "C" const audio::DeviceProvider *Audio_SDLProvider();
extern "C" const audio::DeviceProvider *Audio_AudioQueueProvider();
extern "C" const audio::DeviceProvider *Audio_OpenALProvider();
extern "C" const audio::DeviceProvider *Audio_DirectSoundProvider();
extern "C" const audio::DeviceProvider *Audio_WaveProvider();
extern "C" const audio::DeviceProvider *Audio_NullProvider();

// Copies the plan before Connect. Invalid plans and replacement while connected
// fail without changing the current selection. Providers remain root-owned.
extern "C" bool Engine_BindAudioProviders(
    IEngineAPI *engine, const audio::DeviceSelection *selection );

#endif // ENGINE_AUDIO_DEVICE_PROVIDER_H
