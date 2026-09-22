//========= Copyright Valve Corporation, All rights reserved. ============//

#include "engine/audio/device_provider.h"
#include <cstdio>
#include <cstdlib>
#include <string>

// The selection seam borrows an opaque existing audio device. Native provider
// conformance is exercised by the engine; this fixture tests composition policy.
class IAudioDevice
{
};

static IAudioDevice s_Device;
static std::string s_Attempts;
static bool s_PrimaryAvailable;
static bool s_FallbackAvailable;
static bool s_WaveAvailable;
static bool s_NullAvailable = true;
static unsigned int s_Checks;

static void Check( bool condition, const char *description )
{
	++s_Checks;
	if ( !condition )
	{
		std::fprintf( stderr, "FAIL: %s (attempts: %s)\n", description, s_Attempts.c_str() );
		std::exit( 1 );
	}
}

static IAudioDevice *Primary()
{
	s_Attempts += 'P';
	return s_PrimaryAvailable ? &s_Device : nullptr;
}

static IAudioDevice *Fallback()
{
	s_Attempts += 'F';
	return s_FallbackAvailable ? &s_Device : nullptr;
}

static IAudioDevice *Wave()
{
	s_Attempts += 'W';
	return s_WaveAvailable ? &s_Device : nullptr;
}

static IAudioDevice *Null()
{
	s_Attempts += 'N';
	return s_NullAvailable ? &s_Device : nullptr;
}

static void Expect( const audio::DeviceSelection &plan, bool firstStart, bool waveOnly,
    const audio::DeviceProvider *expected, const char *attempts )
{
	s_Attempts.clear();
	IAudioDevice *device = &s_Device;
	Check( plan.Create( firstStart, waveOnly, &device ) == expected, "selected provider" );
	Check( device == ( expected ? &s_Device : nullptr ), "initialized device or cleared output" );
	Check( s_Attempts == attempts, "ordered attempts stop at first success" );
}

int main()
{
	const audio::DeviceProvider primary = { "primary", Primary };
	const audio::DeviceProvider fallback = { "fallback", Fallback };
	const audio::DeviceProvider wave = { "wave", Wave };
	const audio::DeviceProvider nullProvider = { "null", Null };
	audio::DeviceSelection plan;
	plan.primary = &primary;
	plan.fallback = &fallback;
	plan.waveOnly = &wave;
	plan.nullProvider = &nullProvider;
	Check( plan.IsValid(), "complete plan" );

	// SDL / AudioQueue: first startup and restart both try the primary.
	s_PrimaryAvailable = true;
	s_FallbackAvailable = true;
	Expect( plan, true, false, &primary, "P" );
	Expect( plan, false, false, &primary, "P" );

	// AudioQueue -> OpenAL, or DirectSound -> wave, only after failure.
	s_PrimaryAvailable = false;
	Expect( plan, true, false, &fallback, "PF" );
	s_FallbackAvailable = false;
	Expect( plan, true, false, &nullProvider, "PFN" );
	plan.allowNullFallback = false;
	Expect( plan, true, false, nullptr, "PF" );
	plan.allowNullFallback = true;

	// An explicit Windows wave-only request never attempts DirectSound.
	s_PrimaryAvailable = true;
	s_WaveAvailable = true;
	Expect( plan, true, true, &wave, "W" );
	s_WaveAvailable = false;
	Expect( plan, true, true, &nullProvider, "WN" );

	// Preserve DirectSound's first-start-only behavior across sound restarts.
	plan.primaryOnRestart = false;
	s_FallbackAvailable = true;
	Expect( plan, false, false, &fallback, "F" );
	Expect( plan, true, false, &primary, "P" );

	// Non-Windows profiles have no wave-only override.
	plan.waveOnly = nullptr;
	plan.primaryOnRestart = true;
	Expect( plan, true, true, &primary, "P" );

	// Explicit null selection is attempted once, including a broken provider.
	plan.primary = &nullProvider;
	plan.fallback = &nullProvider;
	Expect( plan, true, false, &nullProvider, "N" );
	s_NullAvailable = false;
	Expect( plan, true, false, nullptr, "N" );
	s_NullAvailable = true;

	// Full validation precedes any resource-acquiring callback, even if the
	// invalid secondary would otherwise be hidden by primary success.
	const audio::DeviceProvider broken = { "broken", nullptr };
	plan.primary = &primary;
	plan.fallback = &broken;
	Expect( plan, true, false, nullptr, "" );
	const audio::DeviceProvider unnamed = { "", Primary };
	plan.fallback = nullptr;
	plan.primary = &unnamed;
	Expect( plan, true, false, nullptr, "" );
	plan.primary = &primary;
	plan.nullProvider = nullptr;
	Expect( plan, true, false, nullptr, "" );
	plan.nullProvider = &nullProvider;
	plan.primaryOnRestart = false;
	Expect( plan, false, false, nullptr, "" );
	plan.primaryOnRestart = true;
	s_Attempts.clear();
	Check( !plan.Create( true, false, nullptr ), "missing output fails" );
	Check( s_Attempts.empty(), "missing output invokes no provider" );

	std::printf( "PASS audio provider selection: %u checks\n", s_Checks );
	return 0;
}
