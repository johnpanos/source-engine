//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef ENGINE_AUDIO_DEVICE_SELECTION_H
#define ENGINE_AUDIO_DEVICE_SELECTION_H

class IAudioDevice;

// Private bridge from the existing sound owner to its containing engine's
// injected plan. No catalog, native SDK, or configuration lookup crosses it.
IAudioDevice *Engine_CreateSelectedAudioDevice( bool firstStart, bool waveOnly );

#endif // ENGINE_AUDIO_DEVICE_SELECTION_H
