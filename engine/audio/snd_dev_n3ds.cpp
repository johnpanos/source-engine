//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The Nintendo 3DS playback device, on the DSP through libctru's NDSP.
//
// The DSP has 24 voices that each resample (any rate, with interpolation),
// pitch and pan their own samples and mix them in hardware. This device uses
// them as follows; it is not a bit-exact port of the software mixer (user
// direction, 2026-10-08: "it doesn't need to be 1:1").
//
//  * Voice 0 plays the software mixer's output: one looping 16-bit stereo
//    buffer in linear (DSP-visible) memory, written ahead of the DSP's read
//    position as the desktop devices' DMA ring is. Only sounds the hardware
//    cannot play take this path: compressed (ADPCM, MP3), streamed, sentence
//    and voice-chat sources, doppler-encoded wavs, and sounds that do not fit
//    the sample cache or a free voice.
//  * Voices 1-23 are hardware voices (IAudioHardwareVoices). A channel whose
//    source is fully loaded 16-bit PCM plays its samples from a copy in
//    linear memory, at the channel's pitch and its dry stereo volume (the
//    software mixer's facing-buffer volume, times the master volume). Loops
//    are a one-shot buffer to the end followed by a looping buffer from the
//    loop start. The mixer keeps the channel's position (SkipSamples), so
//    ends, loops and operator stacks are unchanged.
//
// Not reproduced for hardware voices: room DSP (off on the 3DS anyway,
// engine/n3ds_platform_defaults.cpp), directional and distance-variant wav
// encodings (both stereo halves play), mouth-sync pausing beyond pausing
// the voice, and sample-exact start (a voice starts when the mixer first
// reaches it, up to the mix-ahead earlier than the software path would be
// heard).
//
// Every call runs on the sound update sequence; NDSP locks internally.
//
//=============================================================================//

#include "audio_pch.h"
#include "engine/audio/device_provider.h"

#if !DEDICATED && defined( PLATFORM_3DS )

#include "snd_hardware_voices.h"
#include "snd_wave_source.h"
#include "soundchars.h"

#include <3ds.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool snd_firsttime;
extern bool MIX_ScaleChannelVolume( paintbuffer_t *ppaint, channel_t *pChannel, int volume[CCHANVOLUMES], int mixchans );
extern paintbuffer_t *MIX_GetPPaintFromIPaint( int ipaintbuffer );

namespace
{

// The software stream: 16384 stereo frames, ~0.37 s at 44.1 kHz.
const int kStreamBytes = 0x10000;
const int kStreamVoice = 0;
const int kFirstHardwareVoice = 1;
const int kVoiceCount = 24;

// Linear memory for hardware voices' samples. intro4 leaves ~7.5 MB of the
// 26 MB linear heap free (launcher_main/n3ds_main.cpp); sounds that do not
// fit play through the software stream.
ConVar snd_n3ds_sample_cache_kb( "snd_n3ds_sample_cache_kb", "3072", 0,
	"3DS: linear memory for hardware-voice samples, in KB" );
ConVar snd_n3ds_hardware_voices( "snd_n3ds_hardware_voices", "1", 0,
	"3DS: play fully loaded PCM sounds on the DSP's voices (0: everything through the software mixer)" );

// One sound's samples in linear memory, shared by every voice playing it.
struct Sample
{
	const void *source = nullptr;	// the engine's copy (identity)
	int bytes = 0;
	unsigned int check = 0;			// guards against reuse of the address
	void *linear = nullptr;
	int users = 0;
	unsigned int lastUse = 0;
};

const int kMaxSamples = 128;

unsigned int Fingerprint( const void *data, int bytes )
{
	const unsigned char *p = static_cast<const unsigned char *>( data );
	unsigned int h = 2166136261u ^ unsigned( bytes );
	const int n = bytes < 64 ? bytes : 64;
	for ( int i = 0; i < n; ++i )
		h = ( h ^ p[i] ) * 16777619u;
	for ( int i = bytes - n; i < bytes; ++i )
		h = ( h ^ p[i] ) * 16777619u;
	return h;
}

struct Voice
{
	channel_t *channel = nullptr;
	int guid = 0;
	Sample *sample = nullptr;
	ndspWaveBuf buffers[2];
	bool played = false;	// played this paint
	bool paused = false;
	bool finishing = false;	// released, playing out its queued samples
};

} // namespace

class CAudioDeviceN3ds : public CAudioDeviceBase, public IAudioHardwareVoices
{
public:
	CAudioDeviceN3ds() = default;
	~CAudioDeviceN3ds() override;

	bool		IsActive( void ) override { return m_pauseCount == 0; }
	bool		Init( void ) override;
	void		Shutdown( void ) override;
	void		PaintEnd( void ) override {}
	int			GetOutputPosition( void ) override;
	void		ChannelReset( int, int, float ) override {}
	void		Pause( void ) override;
	void		UnPause( void ) override;
	float		MixDryVolume( void ) override { return 0; }
	bool		Should3DMix( void ) override { return false; }
	void		StopAllSounds( void ) override;

	int			PaintBegin( float mixAheadTime, int soundtime, int paintedtime ) override;
	void		ClearBuffer( void ) override;
	void		MixBegin( int sampleCount ) override { MIX_ClearAllPaintBuffers( sampleCount, false ); }
	void		MixUpsample( int sampleCount, int filtertype ) override;
	void		Mix8Mono( channel_t *pChannel, char *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int timecompress ) override;
	void		Mix8Stereo( channel_t *pChannel, char *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int timecompress ) override;
	void		Mix16Mono( channel_t *pChannel, short *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int timecompress ) override;
	void		Mix16Stereo( channel_t *pChannel, short *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int timecompress ) override;

	void		TransferSamples( int end ) override;
	void		ApplyDSPEffects( int idsp, portable_samplepair_t *pbuffront, portable_samplepair_t *pbufrear, portable_samplepair_t *pbufcenter, int samplecount ) override;

	const char *DeviceName( void ) override		{ return "3DS DSP"; }
	int			DeviceChannels( void ) override		{ return 2; }
	int			DeviceSampleBits( void ) override	{ return 16; }
	int			DeviceSampleBytes( void ) override	{ return 2; }
	int			DeviceDmaSpeed( void ) override		{ return SOUND_DMA_SPEED; }
	int			DeviceSampleCount( void ) override	{ return kStreamBytes / 2; }

	// IAudioHardwareVoices
	bool		Play( channel_t *pChannel ) override;
	void		Release( channel_t *pChannel, bool letFinish ) override;
	void		EndPaint() override;

private:
	Voice		*FindVoice( channel_t *pChannel );
	Voice		*StartVoice( channel_t *pChannel );
	Sample		*AcquireSample( const void *data, int bytes );
	void		ReleaseSample( Sample *sample );
	void		StopVoice( int index );
	void		SetVolume( int index, channel_t *pChannel, bool stereo );

	bool		m_ndsp = false;
	short		*m_stream = nullptr;
	ndspWaveBuf	m_streamBuffer;
	int			m_pauseCount = 0;

	Voice		m_voices[kVoiceCount];
	Sample		m_samples[kMaxSamples];
	int			m_sampleBytes = 0;
	unsigned int m_clock = 0;
};

static CAudioDeviceN3ds *g_device = nullptr;

CAudioDeviceN3ds::~CAudioDeviceN3ds()
{
	g_device = nullptr;
}

static IAudioDevice *Audio_CreateN3dsDevice()
{
	if ( !g_device )
		g_device = new CAudioDeviceN3ds;
	if ( !g_device->Init() )
	{
		delete g_device;
		g_device = nullptr;
	}
	return g_device;
}

DLL_EXPORT const audio::DeviceProvider *Audio_N3dsProvider()
{
	static const audio::DeviceProvider provider = { "ndsp", Audio_CreateN3dsDevice };
	return &provider;
}

bool CAudioDeviceN3ds::Init( void )
{
	if ( m_ndsp )
		return true;
	m_bSurround = false;
	m_bSurroundCenter = false;
	m_bHeadphone = false;

	// Needs the DSP firmware dump (sdmc:/3ds/dspfirm.cdc); without it the
	// selection falls back to the null device.
	const Result result = ndspInit();
	if ( R_FAILED( result ) )
	{
		Warning( "3DS audio: ndspInit failed (0x%08lx); is sdmc:/3ds/dspfirm.cdc present?\n", (unsigned long)result );
		return false;
	}
	m_ndsp = true;

	m_stream = static_cast<short *>( linearAlloc( kStreamBytes ) );
	if ( !m_stream )
	{
		Warning( "3DS audio: no linear memory for the %d-byte stream\n", kStreamBytes );
		Shutdown();
		return false;
	}
	memset( m_stream, 0, kStreamBytes );
	DSP_FlushDataCache( m_stream, kStreamBytes );

	ndspSetOutputMode( NDSP_OUTPUT_STEREO );
	ndspChnReset( kStreamVoice );
	ndspChnSetInterp( kStreamVoice, NDSP_INTERP_LINEAR );
	ndspChnSetRate( kStreamVoice, float( SOUND_DMA_SPEED ) );
	ndspChnSetFormat( kStreamVoice, NDSP_FORMAT_STEREO_PCM16 );
	float mix[12] = { 1.0f, 1.0f };
	ndspChnSetMix( kStreamVoice, mix );
	memset( &m_streamBuffer, 0, sizeof( m_streamBuffer ) );
	m_streamBuffer.data_vaddr = m_stream;
	m_streamBuffer.nsamples = kStreamBytes / 4;
	m_streamBuffer.looping = true;
	ndspChnWaveBufAdd( kStreamVoice, &m_streamBuffer );

	g_pAudioHardwareVoices = this;
	if ( snd_firsttime )
		DevMsg( "3DS DSP sound initialized\n" );
	Msg( "RFC0001 provider: audio=ndsp voices=%d\n", kVoiceCount - kFirstHardwareVoice );
	return true;
}

void CAudioDeviceN3ds::Shutdown( void )
{
	if ( g_pAudioHardwareVoices == this )
		g_pAudioHardwareVoices = nullptr;
	if ( m_ndsp )
	{
		for ( int i = 0; i < kVoiceCount; ++i )
			ndspChnReset( i );
		ndspExit();
		m_ndsp = false;
	}
	for ( int i = 0; i < kVoiceCount; ++i )
		m_voices[i] = Voice();
	for ( int i = 0; i < kMaxSamples; ++i )
	{
		if ( m_samples[i].linear )
			linearFree( m_samples[i].linear );
		m_samples[i] = Sample();
	}
	m_sampleBytes = 0;
	if ( m_stream )
	{
		linearFree( m_stream );
		m_stream = nullptr;
	}
}

//-----------------------------------------------------------------------------
// The software stream
//-----------------------------------------------------------------------------
int CAudioDeviceN3ds::PaintBegin( float mixAheadTime, int soundtime, int paintedtime )
{
	unsigned int endtime = soundtime + mixAheadTime * DeviceDmaSpeed();
	const int frames = DeviceSampleCount() >> 1;
	if ( (int)( endtime - soundtime ) > frames )
		endtime = soundtime + frames;
	// Multiples of 4 samples, for the 11 kHz -> 44 kHz upsampling.
	if ( ( endtime - paintedtime ) & 0x3 )
		endtime -= ( endtime - paintedtime ) & 0x3;
	return endtime;
}

int CAudioDeviceN3ds::GetOutputPosition( void )
{
	// In stereo frames within the ring, as the other devices report it.
	return int( ndspChnGetSamplePos( kStreamVoice ) ) & ( ( DeviceSampleCount() >> 1 ) - 1 );
}

void CAudioDeviceN3ds::TransferSamples( int end )
{
	const int start = g_paintedtime;
	S_TransferStereo16( m_stream, PAINTBUFFER, start, end );

	// The DSP reads memory, not the CPU's cache: flush what was written.
	const int frames = DeviceSampleCount() >> 1;
	int count = end - start;
	if ( count <= 0 )
		return;
	if ( count > frames )
		count = frames;
	const int first = start & ( frames - 1 );
	const int head = count < frames - first ? count : frames - first;
	DSP_FlushDataCache( m_stream + first * 2, head * 4 );
	if ( count > head )
		DSP_FlushDataCache( m_stream, ( count - head ) * 4 );
}

void CAudioDeviceN3ds::ClearBuffer( void )
{
	if ( !m_stream )
		return;
	memset( m_stream, 0, kStreamBytes );
	DSP_FlushDataCache( m_stream, kStreamBytes );
}

void CAudioDeviceN3ds::Pause( void )
{
	if ( ++m_pauseCount != 1 )
		return;
	for ( int i = 0; i < kVoiceCount; ++i )
		ndspChnSetPaused( i, true );
}

void CAudioDeviceN3ds::UnPause( void )
{
	if ( m_pauseCount == 0 || --m_pauseCount != 0 )
		return;
	ndspChnSetPaused( kStreamVoice, false );
	for ( int i = kFirstHardwareVoice; i < kVoiceCount; ++i )
		ndspChnSetPaused( i, m_voices[i].paused );
}

void CAudioDeviceN3ds::StopAllSounds( void )
{
	for ( int i = kFirstHardwareVoice; i < kVoiceCount; ++i )
		StopVoice( i );
}

void CAudioDeviceN3ds::MixUpsample( int sampleCount, int filtertype )
{
	paintbuffer_t *ppaint = MIX_GetCurrentPaintbufferPtr();
	const int ifilter = ppaint->ifilter;
	Assert( ifilter < CPAINTFILTERS );
	S_MixBufferUpsample2x( sampleCount, ppaint->pbuf, &( ppaint->fltmem[ifilter][0] ), CPAINTFILTERMEM, filtertype );
	ppaint->ifilter++;
}

void CAudioDeviceN3ds::Mix8Mono( channel_t *pChannel, char *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int )
{
	int volume[CCHANVOLUMES];
	paintbuffer_t *ppaint = MIX_GetCurrentPaintbufferPtr();
	if ( MIX_ScaleChannelVolume( ppaint, pChannel, volume, 1 ) )
		Mix8MonoWavtype( pChannel, ppaint->pbuf + outputOffset, volume, (byte *)pData, inputOffset, rateScaleFix, outCount );
}

void CAudioDeviceN3ds::Mix8Stereo( channel_t *pChannel, char *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int )
{
	int volume[CCHANVOLUMES];
	paintbuffer_t *ppaint = MIX_GetCurrentPaintbufferPtr();
	if ( MIX_ScaleChannelVolume( ppaint, pChannel, volume, 2 ) )
		Mix8StereoWavtype( pChannel, ppaint->pbuf + outputOffset, volume, (byte *)pData, inputOffset, rateScaleFix, outCount );
}

void CAudioDeviceN3ds::Mix16Mono( channel_t *pChannel, short *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int )
{
	int volume[CCHANVOLUMES];
	paintbuffer_t *ppaint = MIX_GetCurrentPaintbufferPtr();
	if ( MIX_ScaleChannelVolume( ppaint, pChannel, volume, 1 ) )
		Mix16MonoWavtype( pChannel, ppaint->pbuf + outputOffset, volume, pData, inputOffset, rateScaleFix, outCount );
}

void CAudioDeviceN3ds::Mix16Stereo( channel_t *pChannel, short *pData, int outputOffset, int inputOffset, fixedint rateScaleFix, int outCount, int )
{
	int volume[CCHANVOLUMES];
	paintbuffer_t *ppaint = MIX_GetCurrentPaintbufferPtr();
	if ( MIX_ScaleChannelVolume( ppaint, pChannel, volume, 2 ) )
		Mix16StereoWavtype( pChannel, ppaint->pbuf + outputOffset, volume, pData, inputOffset, rateScaleFix, outCount );
}

void CAudioDeviceN3ds::ApplyDSPEffects( int idsp, portable_samplepair_t *pbuffront, portable_samplepair_t *pbufrear, portable_samplepair_t *pbufcenter, int samplecount )
{
	DSP_Process( idsp, pbuffront, pbufrear, pbufcenter, samplecount );
}

//-----------------------------------------------------------------------------
// Hardware voices
//-----------------------------------------------------------------------------
Voice *CAudioDeviceN3ds::FindVoice( channel_t *pChannel )
{
	for ( int i = kFirstHardwareVoice; i < kVoiceCount; ++i )
	{
		Voice &voice = m_voices[i];
		if ( voice.channel == pChannel && voice.guid == pChannel->guid && !voice.finishing )
			return &voice;
	}
	return nullptr;
}

Sample *CAudioDeviceN3ds::AcquireSample( const void *data, int bytes )
{
	const unsigned int check = Fingerprint( data, bytes );
	Sample *empty = nullptr;
	for ( int i = 0; i < kMaxSamples; ++i )
	{
		Sample &sample = m_samples[i];
		if ( sample.linear && sample.source == data && sample.bytes == bytes && sample.check == check )
		{
			sample.users++;
			sample.lastUse = m_clock;
			return &sample;
		}
		if ( !sample.linear && !empty )
			empty = &sample;
	}

	const int budget = snd_n3ds_sample_cache_kb.GetInt() * 1024;
	if ( bytes > budget )
		return nullptr;
	// Evict unused samples, least recently used first, until it fits.
	while ( m_sampleBytes + bytes > budget || !empty )
	{
		Sample *oldest = nullptr;
		for ( int i = 0; i < kMaxSamples; ++i )
		{
			Sample &sample = m_samples[i];
			if ( sample.linear && sample.users == 0 && ( !oldest || sample.lastUse < oldest->lastUse ) )
				oldest = &sample;
		}
		if ( !oldest )
			return nullptr;
		linearFree( oldest->linear );
		m_sampleBytes -= oldest->bytes;
		*oldest = Sample();
		if ( !empty )
			empty = oldest;
	}

	void *linear = linearAlloc( bytes );
	if ( !linear )
		return nullptr;
	memcpy( linear, data, bytes );
	DSP_FlushDataCache( linear, bytes );
	empty->source = data;
	empty->bytes = bytes;
	empty->check = check;
	empty->linear = linear;
	empty->users = 1;
	empty->lastUse = m_clock;
	m_sampleBytes += bytes;
	return empty;
}

void CAudioDeviceN3ds::ReleaseSample( Sample *sample )
{
	if ( sample && sample->users > 0 )
		sample->users--;
}

void CAudioDeviceN3ds::StopVoice( int index )
{
	Voice &voice = m_voices[index];
	if ( !voice.channel && !voice.finishing )
		return;
	ndspChnWaveBufClear( index );
	ReleaseSample( voice.sample );
	voice = Voice();
}

Voice *CAudioDeviceN3ds::StartVoice( channel_t *pChannel )
{
	if ( !pChannel->pMixer || pChannel->flags.isSentence || pChannel->wavtype == CHAR_DOPPLER )
		return nullptr;
	CAudioSource *source = pChannel->pMixer->GetSource();
	if ( !source || source->GetType() != CAudioSource::AUDIO_SOURCE_WAV || source->IsStreaming() ||
		source->IsVoiceSource() || source->Format() != WAVE_FORMAT_PCM || !source->IsCached() )
		return nullptr;
	const bool stereo = source->IsStereoWav();
	const int frameBytes = source->SampleSize();
	if ( frameBytes != ( stereo ? 4 : 2 ) )
		return nullptr;
	const int frames = source->SampleCount();
	if ( frames <= 0 )
		return nullptr;

	// Only a source whose whole data is one resident block.
	static char copyBuf[AUDIOSOURCE_COPYBUF_SIZE];
	void *data = nullptr;
	if ( source->GetOutputData( &data, 0, frames, copyBuf ) != frames || !data || data == copyBuf )
		return nullptr;

	int index = -1;
	for ( int i = kFirstHardwareVoice; i < kVoiceCount && index < 0; ++i )
	{
		if ( !m_voices[i].channel && !m_voices[i].finishing )
			index = i;
	}
	if ( index < 0 )
		return nullptr;

	Sample *sample = AcquireSample( data, frames * frameBytes );
	if ( !sample )
		return nullptr;

	int start = pChannel->pMixer->GetSamplePosition();
	if ( start < 0 || start >= frames )
		start = 0;
	int loopStart = -1;
	if ( source->IsLooped() )
	{
		int loopBlock = 0, leading = 0, trailing = 0;
		loopStart = static_cast<CAudioSourceWave *>( source )->GetLoopingInfo( &loopBlock, &leading, &trailing );
		if ( loopStart < 0 || loopStart >= frames )
			loopStart = 0;
	}

	Voice &voice = m_voices[index];
	voice.channel = pChannel;
	voice.guid = pChannel->guid;
	voice.sample = sample;

	ndspChnReset( index );
	ndspChnSetInterp( index, NDSP_INTERP_LINEAR );
	ndspChnSetFormat( index, stereo ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16 );
	ndspChnSetRate( index, pChannel->pitch * source->SampleRate() );
	SetVolume( index, pChannel, stereo );

	char *base = static_cast<char *>( sample->linear );
	memset( voice.buffers, 0, sizeof( voice.buffers ) );
	voice.buffers[0].data_vaddr = base + start * frameBytes;
	voice.buffers[0].nsamples = frames - start;
	ndspChnWaveBufAdd( index, &voice.buffers[0] );
	if ( loopStart >= 0 )
	{
		voice.buffers[1].data_vaddr = base + loopStart * frameBytes;
		voice.buffers[1].nsamples = frames - loopStart;
		voice.buffers[1].looping = true;
		ndspChnWaveBufAdd( index, &voice.buffers[1] );
	}
	return &voice;
}

void CAudioDeviceN3ds::SetVolume( int index, channel_t *pChannel, bool stereo )
{
	// The software mixer's dry (facing) volume for this channel, and the
	// master volume S_TransferStereo16 applies.
	int volume[CCHANVOLUMES];
	float mix[12] = {};
	if ( MIX_ScaleChannelVolume( MIX_GetPPaintFromIPaint( SOUND_BUFFER_FACING ), pChannel, volume, stereo ? 2 : 1 ) )
	{
		const float scale = S_GetMasterVolume() * ( 1.0f / 256.0f );
		mix[0] = volume[IFRONT_LEFT] * scale;
		mix[1] = volume[IFRONT_RIGHT] * scale;
	}
	ndspChnSetMix( index, mix );
}

bool CAudioDeviceN3ds::Play( channel_t *pChannel )
{
	if ( !snd_n3ds_hardware_voices.GetBool() )
		return false;
	Voice *voice = FindVoice( pChannel );
	if ( !voice )
	{
		voice = StartVoice( pChannel );
		if ( !voice )
			return false;
	}
	else
	{
		const int index = int( voice - m_voices );
		CAudioSource *source = pChannel->pMixer->GetSource();
		ndspChnSetRate( index, pChannel->pitch * source->SampleRate() );
		SetVolume( index, pChannel, source->IsStereoWav() );
	}
	voice->played = true;
	voice->sample->lastUse = m_clock;
	return true;
}

void CAudioDeviceN3ds::Release( channel_t *pChannel, bool letFinish )
{
	Voice *voice = FindVoice( pChannel );
	if ( !voice )
		return;
	const int index = int( voice - m_voices );
	if ( letFinish && !voice->paused && !voice->buffers[1].looping )
	{
		// The mixer reached the end a mix-ahead before the DSP: let the
		// queued samples play out; EndPaint recycles the voice.
		voice->channel = nullptr;
		voice->finishing = true;
		return;
	}
	StopVoice( index );
}

void CAudioDeviceN3ds::EndPaint()
{
	++m_clock;
	for ( int i = kFirstHardwareVoice; i < kVoiceCount; ++i )
	{
		Voice &voice = m_voices[i];
		if ( voice.finishing )
		{
			if ( !ndspChnIsPlaying( i ) || voice.buffers[0].status == NDSP_WBUF_DONE )
				StopVoice( i );
			continue;
		}
		if ( !voice.channel )
			continue;
		if ( voice.played == voice.paused )
		{
			voice.paused = !voice.played;
			ndspChnSetPaused( i, voice.paused );
		}
		voice.played = false;
	}
}

#endif // !DEDICATED && PLATFORM_3DS
