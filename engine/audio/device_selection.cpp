//========= Copyright Valve Corporation, All rights reserved. ============//

#include "engine/audio/device_provider.h"

namespace audio
{

static bool ValidProvider( const DeviceProvider *provider )
{
	return provider && provider->id && provider->id[0] && provider->create;
}

bool DeviceSelection::IsValid() const
{
	return ValidProvider( primary ) && ValidProvider( nullProvider ) &&
	       ( !fallback || ValidProvider( fallback ) ) &&
	       ( !waveOnly || ValidProvider( waveOnly ) ) && ( primaryOnRestart || fallback );
}

const DeviceProvider *DeviceSelection::Create(
    bool firstStart, bool waveOnlyRequested, IAudioDevice **device ) const
{
	if ( !device )
		return nullptr;
	*device = nullptr;
	if ( !IsValid() )
		return nullptr;

	const DeviceProvider *candidates[3] = {
	    nullptr, nullptr, allowNullFallback ? nullProvider : nullptr };
	if ( waveOnlyRequested && waveOnly )
	{
		candidates[0] = waveOnly;
	}
	else
	{
		if ( firstStart || primaryOnRestart )
			candidates[0] = primary;
		candidates[1] = fallback;
	}

	for ( unsigned int i = 0; i < 3; ++i )
	{
		const DeviceProvider *provider = candidates[i];
		if ( !provider )
			continue;
		bool attempted = false;
		for ( unsigned int j = 0; j < i; ++j )
			attempted = attempted || candidates[j] == provider;
		if ( attempted )
			continue;
		*device = provider->create();
		if ( *device )
			return provider;
	}
	return nullptr;
}

} // namespace audio
