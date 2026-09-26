//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device's thermal state, logged by the Apple app root.
//
// iOS and tvOS report how hot the device is as NSProcessInfo.thermalState
// (nominal, fair, serious, critical) and slow the CPU and GPU as it rises.
// The app root logs the state at startup and at every change, with the
// steady-clock time the -vkframestats stream uses for its frames (t,
// microseconds), so a long session's frame times can be read against the
// device's thermal state (tools/quality/ios_frame_pacing.py --soak).
//
//===========================================================================//

#import <Foundation/Foundation.h>

#include <SDL3/SDL.h>

#include <chrono>

#include "apple_thermal.h"

namespace
{
const char *ThermalStateName( NSProcessInfoThermalState state )
{
	switch ( state )
	{
	case NSProcessInfoThermalStateNominal:
		return "nominal";
	case NSProcessInfoThermalStateFair:
		return "fair";
	case NSProcessInfoThermalStateSerious:
		return "serious";
	case NSProcessInfoThermalStateCritical:
		return "critical";
	}
	return "unknown";
}

void LogThermalState()
{
	const long long us = std::chrono::duration_cast<std::chrono::microseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	                         .count();
	SDL_Log( "Source: thermal state %s t=%lld",
	    ThermalStateName( [NSProcessInfo processInfo].thermalState ), us );
}
} // namespace

void AppleThermal_StartLogging()
{
	LogThermalState();
	// The observer lives for the process, as the app root does.
	[[NSNotificationCenter defaultCenter]
	    addObserverForName:NSProcessInfoThermalStateDidChangeNotification
	                object:nil
	                 queue:nil
	            usingBlock:^( NSNotification * ) {
		            LogThermalState();
	            }];
}
