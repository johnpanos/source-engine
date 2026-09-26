//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device's thermal state, logged by the Apple app root; see
//          apple_thermal.mm.
//
//===========================================================================//

#ifndef APPLE_THERMAL_H
#define APPLE_THERMAL_H

// Logs the thermal state now and at every change ("Source: thermal state
// <nominal|fair|serious|critical> t=<steady-clock microseconds>").
void AppleThermal_StartLogging();

#endif // APPLE_THERMAL_H
