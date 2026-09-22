//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Runtime observation for the legacy dynamic-module boundary.
//
// This API exists to support RFC 0001 retirement Phase A. It does not grant
// consumers new loading authority and must not be used as a service locator.
//
//=============================================================================//

#ifndef TIER1_MODULE_LOAD_TELEMETRY_H
#define TIER1_MODULE_LOAD_TELEMETRY_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

enum ModuleLoadTelemetryOperation_t
{
	MODULE_LOAD_TELEMETRY_LOAD = 0,
	MODULE_LOAD_TELEMETRY_ENTRY_POINT,
	MODULE_LOAD_TELEMETRY_INTERFACE,
	MODULE_LOAD_TELEMETRY_UNLOAD
};

// Strings are copied into the event and remain valid for the duration of the
// sink call. A sink that retains an event must copy the complete structure.
struct ModuleLoadTelemetryEvent_t
{
	ModuleLoadTelemetryOperation_t m_Operation;
	uint64 m_nLoadId;
	uint64 m_nLifetimeMicroseconds;
	int m_nSourceLine;
	int m_nProviderResult;
	bool m_bSuccess;
	char m_szRequestingSubsystem[260];
	char m_szRequestedPath[2048];
	char m_szResolvedPath[2048];
	char m_szRequestedEntryPoint[256];
	char m_szProviderError[512];
};

typedef void (*ModuleLoadTelemetrySinkFn)(
	const ModuleLoadTelemetryEvent_t &event,
	void *pUserData );

// Installs one process-local observer for the Tier 1 loader instance. Passing
// NULL disables observation. The loader owns no sink or user-data lifetime.
void Sys_SetModuleLoadTelemetrySink(
	ModuleLoadTelemetrySinkFn pSink,
	void *pUserData );

// Carries an original request site through filesystem path resolution and
// other compatibility layers before the native loader is reached.
class CScopedModuleLoadRequest
{
public:
	CScopedModuleLoadRequest(
		const char *pRequestingSubsystem,
		int nSourceLine,
		const char *pRequestedInterface = NULL );
	~CScopedModuleLoadRequest();

	const char *GetRequestingSubsystem() const { return m_pRequestingSubsystem; }
	int GetSourceLine() const { return m_nSourceLine; }
	const char *GetRequestedInterface() const { return m_pRequestedInterface; }

private:
	CScopedModuleLoadRequest *m_pPrevious;
	const char *m_pRequestingSubsystem;
	const char *m_pRequestedInterface;
	int m_nSourceLine;

	CScopedModuleLoadRequest( const CScopedModuleLoadRequest & );
	CScopedModuleLoadRequest &operator=( const CScopedModuleLoadRequest & );
};

#endif // TIER1_MODULE_LOAD_TELEMETRY_H
