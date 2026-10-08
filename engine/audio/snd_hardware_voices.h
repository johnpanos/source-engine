//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Hardware voices: a playback device whose hardware resamples, pitches and
// pans sounds itself (the 3DS DSP) takes a channel off the software mixer.
// The mixer still owns the channel (position, looping, end, operators); for
// a claimed channel it advances the mixer with SkipSamples instead of
// painting, and the device plays the same samples. Engine-private; the
// preserved IAudioDevice vtable is unchanged.
//
// All calls run on the sound mixing sequence (S_Update).
//
//=============================================================================//

#ifndef SND_HARDWARE_VOICES_H
#define SND_HARDWARE_VOICES_H

struct channel_t;

class IAudioHardwareVoices
{
public:
	// Called once per paint for each audible channel. True: the device plays
	// it this paint (started or updated: rate, volume); the caller must not
	// paint it. False: the software mixer paints it.
	virtual bool Play( channel_t *pChannel ) = 0;

	// The channel is being freed. letFinish: it ended naturally (the mixer
	// reached the end ahead of the hardware by the mix-ahead), so its last
	// queued samples still play; otherwise it stops now.
	virtual void Release( channel_t *pChannel, bool letFinish ) = 0;

	// End of a paint: voices whose channel was not played this paint (game
	// pause of mouths, culled) pause; finished voices are recycled.
	virtual void EndPaint() = 0;

protected:
	~IAudioHardwareVoices() {}
};

// Set by the device that provides them between its Init and Shutdown; null
// for every other device.
extern IAudioHardwareVoices *g_pAudioHardwareVoices;

#endif // SND_HARDWARE_VOICES_H
