//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "audio_pch.h"

#include "engine/audio/device_provider.h"
#include "device_selection.h"

#ifdef OSX
ConVar snd_audioqueue( "snd_audioqueue", "1" );
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool snd_firsttime = true;

/* 
 * Global variables. Must be visible to window-procedure function 
 *  so it can unlock and free the data block after it has been played. 
 */ 
IAudioDevice *g_AudioDevice = NULL;

/*
==================
S_BlockSound
==================
*/
void S_BlockSound( void )
{
	if ( !g_AudioDevice )
		return;

	g_AudioDevice->Pause();
}

/*
==================
S_UnblockSound
==================
*/
void S_UnblockSound( void )
{
	if ( !g_AudioDevice )
		return;

	g_AudioDevice->UnPause();
}

/*
==================
AutoDetectInit

Try to find a sound device to mix for.
Returns a CAudioNULLDevice if nothing is found.
==================
*/
IAudioDevice *IAudioDevice::AutoDetectInit( bool waveOnly )
{
	IAudioDevice *pDevice = Engine_CreateSelectedAudioDevice( snd_firsttime, waveOnly );
	snd_firsttime = false;
	return pDevice;
}

DLL_EXPORT const audio::DeviceProvider *Audio_NullProvider()
{
	static const audio::DeviceProvider provider = { "null", Audio_GetNullDevice };
	return &provider;
}

/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
void SNDDMA_Shutdown( void )
{
	if ( g_AudioDevice != Audio_GetNullDevice() )
	{
		if ( g_AudioDevice )
		{
			g_AudioDevice->Shutdown();
			delete g_AudioDevice;
		}

		// the NULL device is always valid
		g_AudioDevice = Audio_GetNullDevice();
	}
}

