//========= Copyright Valve Corporation, All rights reserved. ============//

#include "engine/audio/media_providers.h"

namespace audio
{

static bool SameProtocol( const char *left, const char *right )
{
	if ( !left || !right )
		return false;
	while ( *left && *right )
	{
		const char a = *left >= 'A' && *left <= 'Z' ? *left + 'a' - 'A' : *left;
		const char b = *right >= 'A' && *right <= 'Z' ? *right + 'a' - 'A' : *right;
		if ( a != b )
			return false;
		++left;
		++right;
	}
	return *left == *right;
}

static bool ValidRecording( const VoiceRecordProvider *provider )
{
	return provider && provider->id && provider->id[0] && provider->create;
}

bool MediaProviders::IsValid() const
{
	if ( !mp3 || !mp3->id || !mp3->id[0] || !mp3->create || !ValidRecording( recording ) ||
	     ( recordingFallback && !ValidRecording( recordingFallback ) ) ||
	     ( voiceBindingCount && !voiceBindings ) )
		return false;
	bool localProtocolPresent = !localVoiceProtocol;
	for ( unsigned int i = 0; i < voiceBindingCount; ++i )
	{
		const VoiceProtocolBinding &binding = voiceBindings[i];
		if ( !binding.protocolName || !binding.protocolName[0] || !binding.provider ||
		     !binding.provider->id || !binding.provider->id[0] || !binding.provider->create ||
		     binding.quality < 0 )
			return false;
		localProtocolPresent =
		    localProtocolPresent || SameProtocol( localVoiceProtocol, binding.protocolName );
		for ( unsigned int j = 0; j < i; ++j )
			if ( SameProtocol( binding.protocolName, voiceBindings[j].protocolName ) )
				return false;
	}
	return localProtocolPresent;
}

const VoiceProtocolBinding *MediaProviders::FindVoice( const char *protocolName ) const
{
	if ( !IsValid() )
		return nullptr;
	for ( unsigned int i = 0; i < voiceBindingCount; ++i )
		if ( SameProtocol( protocolName, voiceBindings[i].protocolName ) )
			return &voiceBindings[i];
	return nullptr;
}

IVoiceRecord *MediaProviders::CreateRecorder( int sampleRate ) const
{
	if ( !IsValid() || sampleRate <= 0 )
		return nullptr;
	IVoiceRecord *recorder = recording->create( sampleRate );
	if ( !recorder && recordingFallback && recordingFallback != recording )
		recorder = recordingFallback->create( sampleRate );
	return recorder;
}

} // namespace audio
