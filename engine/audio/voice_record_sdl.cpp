//========= Copyright 1996-2009, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// This module implements the voice record and compression functions 

//#include "audio_pch.h"
//#include "voice.h"
#include "tier0/platform.h"
#include "ivoicerecord.h"
#include "engine/audio/media_providers.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"

#include <assert.h>
#include <SDL3/SDL.h>

#define RECORDING_BUFFER_SECONDS 3
#define SAMPLE_COUNT 2048

// ------------------------------------------------------------------------------
// VoiceRecord_SDL
// ------------------------------------------------------------------------------

struct AudioBuf
{
	int Read(char *out, int len)
	{
		int nAvalible = used;

		if( nAvalible == 0 )
			return 0;

		if( len > nAvalible ) len = nAvalible;

		int diff = (data + size) - readPtr;


		if( len > diff )
		{
			memcpy(out, readPtr, diff );
			memcpy(out+diff, data, len-diff );
		} else memcpy(out, readPtr, len);

		readPtr += len;

		if( readPtr >= data + size )
			readPtr -= size;

		used -= len;
		return len;
	}

	void Write(char *in, int len)
	{
		// Voice capture retains the newest three seconds; old microphone input
		// is discarded explicitly when the consumer cannot keep up.
		if ( len >= size )
		{
			in += len - size;
			len = size;
			readPtr = writePtr = data;
			used = 0;
		}
		const int discard = MAX( 0, used + len - size );
		readPtr = data + ( readPtr - data + discard ) % size;
		used -= discard;
		used += len;
		int diff = (data + size) - writePtr;

		if( len > diff )
		{
			memcpy(writePtr, in, diff );
			memcpy(data, in+diff, len-diff );
		} else memcpy(writePtr, in, len);

		writePtr += len;

		if (writePtr >= (data + size))
			writePtr -= size;
	}

	int size;
	int used;
	char *data;
	char *readPtr;
	char *writePtr;
};

class VoiceRecord_SDL : public IVoiceRecord
{
protected:
	virtual				~VoiceRecord_SDL();
public:
	VoiceRecord_SDL();
	virtual void		Release();
	virtual bool		RecordStart();
	virtual void		RecordStop();

	// Initialize. The format of the data we expect from the provider is
	// 8-bit signed mono at the specified sample rate.
	virtual bool		Init(int sampleRate);
	virtual void		Idle() {}; // Stub
	void RenderBuffer( char *pszBuf, int size );

	// Get the most recent N samples.
	virtual int			GetRecordedData(short *pOut, int nSamplesWanted );

	SDL_AudioSpec m_ReceivedRecordingSpec;
	int m_BytesPerSample; // Да кому нужна эта ваша инкапсуляция?
	int m_nSampleRate;
private:
	bool				InitalizeInterfaces();	// Initialize the openal capture buffers and other interfaces
	void				ReleaseInterfaces();	// Release openal buffers and other interfaces
	void				ClearInterfaces();				// Clear members.
private:
	SDL_AudioDeviceID m_Device;
	SDL_AudioStream *m_pRecordingStream;
	bool m_bAudioInitialized;
	static void SDLCALL RecordingCallback(
	    void *userdata, SDL_AudioStream *stream, int additionalAmount, int totalAmount );
	AudioBuf m_AudioBuffer;
};

void audioRecordingCallback( void *userdata, uint8 *stream, int len )
{
	VoiceRecord_SDL *voice = (VoiceRecord_SDL*)userdata;
	voice->RenderBuffer( (char*)stream, len );
}

VoiceRecord_SDL::VoiceRecord_SDL() :
m_nSampleRate( 0 ) ,m_Device( 0 )
{
	m_pRecordingStream = NULL;
	m_bAudioInitialized = false;
	m_AudioBuffer.data = NULL;
	m_AudioBuffer.readPtr = NULL;
	m_AudioBuffer.writePtr = NULL;

	ClearInterfaces();
}

VoiceRecord_SDL::~VoiceRecord_SDL()
{
	ReleaseInterfaces();
	ClearInterfaces();
}

void VoiceRecord_SDL::Release()
{
	ReleaseInterfaces();
	ClearInterfaces();
	delete this;
}

bool VoiceRecord_SDL::RecordStart()
{
	if ( !m_Device )
		InitalizeInterfaces();

	if ( !m_Device )
		return false;

	return SDL_ResumeAudioStreamDevice( m_pRecordingStream );

	return true;
}


void VoiceRecord_SDL::RecordStop()
{
	// Stop capturing.
	if ( m_Device )
		SDL_PauseAudioDevice( m_Device );

	// Release the capture buffer interface and any other resources that are no
	// longer needed
	ReleaseInterfaces();
}

bool VoiceRecord_SDL::InitalizeInterfaces()
{
	if ( !SDL_InitSubSystem( SDL_INIT_AUDIO ) )
		return false;
	m_bAudioInitialized = true;
	const SDL_AudioSpec spec = { SDL_AUDIO_S16, 1, m_nSampleRate };
	m_BytesPerSample = sizeof( short );
	m_AudioBuffer.size = RECORDING_BUFFER_SECONDS * m_nSampleRate * m_BytesPerSample;
	free( m_AudioBuffer.data );
	m_AudioBuffer.data = static_cast<char *>( malloc( m_AudioBuffer.size ) );
	m_AudioBuffer.readPtr = m_AudioBuffer.writePtr = m_AudioBuffer.data;
	m_AudioBuffer.used = 0;
	if ( !m_AudioBuffer.data )
	{
		ReleaseInterfaces();
		return false;
	}
	m_pRecordingStream = SDL_OpenAudioDeviceStream(
	    SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &spec, &VoiceRecord_SDL::RecordingCallback, this );
	if ( !m_pRecordingStream )
	{
		Warning( "SDL recording initialization failed: %s\n", SDL_GetError() );
		ReleaseInterfaces();
		return false;
	}
	m_Device = SDL_GetAudioStreamDevice( m_pRecordingStream );
	m_BytesPerSample = sizeof( short );
	m_ReceivedRecordingSpec = spec;
	return true;
}

bool VoiceRecord_SDL::Init(int sampleRate)
{
	if ( sampleRate <= 0 || sampleRate > 192000 )
		return false;
	m_nSampleRate = sampleRate;
	ReleaseInterfaces();

	return true;
}

void VoiceRecord_SDL::ReleaseInterfaces()
{
	if ( m_pRecordingStream )
	{
		SDL_DestroyAudioStream( m_pRecordingStream );
		m_pRecordingStream = NULL;
	}
	if ( m_bAudioInitialized )
	{
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
		m_bAudioInitialized = false;
	}

	m_Device = 0;
}

void VoiceRecord_SDL::ClearInterfaces()
{
	if( m_AudioBuffer.data )
	{
		free( m_AudioBuffer.data );
		m_AudioBuffer.data = NULL;
		m_AudioBuffer.readPtr = NULL;
		m_AudioBuffer.writePtr = NULL;
	}
	m_Device = 0;
}

void VoiceRecord_SDL::RenderBuffer( char *pszBuf, int size )
{
	m_AudioBuffer.Write( pszBuf, size );
}

int VoiceRecord_SDL::GetRecordedData(short *pOut, int nSamples )
{
	if ( !m_pRecordingStream || !pOut || nSamples <= 0 || nSamples > INT_MAX / m_BytesPerSample )
		return 0;
	// SDL holds this same stream lock during the recording callback. No ring
	// bytes or cursor updates race with the producer, including full-buffer wrap.
	if ( !SDL_LockAudioStream( m_pRecordingStream ) )
		return 0;
	const int requestedBytes = nSamples * m_BytesPerSample;
	if ( m_AudioBuffer.used > requestedBytes )
	{
		const int discard = m_AudioBuffer.used - requestedBytes;
		m_AudioBuffer.readPtr =
		    m_AudioBuffer.data +
		    ( m_AudioBuffer.readPtr - m_AudioBuffer.data + discard ) % m_AudioBuffer.size;
		m_AudioBuffer.used = requestedBytes;
	}
	const int bytes = m_AudioBuffer.Read( reinterpret_cast<char *>( pOut ), requestedBytes );
	SDL_UnlockAudioStream( m_pRecordingStream );
	return bytes / m_BytesPerSample;
}

void SDLCALL VoiceRecord_SDL::RecordingCallback(
    void *userdata, SDL_AudioStream *stream, int additionalAmount, int totalAmount )
{
	VoiceRecord_SDL *voice = static_cast<VoiceRecord_SDL *>( userdata );
	char buffer[4096];
	while ( SDL_GetAudioStreamAvailable( stream ) > 0 )
	{
		const int bytes = SDL_GetAudioStreamData( stream, buffer, sizeof( buffer ) );
		if ( bytes <= 0 )
			break;
		voice->RenderBuffer( buffer, bytes );
	}
}

IVoiceRecord* CreateVoiceRecord_SDL(int sampleRate)
{
	VoiceRecord_SDL *pRecord = new VoiceRecord_SDL;
	if ( pRecord && pRecord->Init(sampleRate) )
		return pRecord;
	else if( pRecord )
		pRecord->Release();

	return NULL;
}

DLL_EXPORT const audio::VoiceRecordProvider *VoiceRecord_SDLProvider()
{
	static const audio::VoiceRecordProvider provider = { "sdl3", CreateVoiceRecord_SDL };
	return &provider;
}
