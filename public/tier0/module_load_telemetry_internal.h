// Private implementation hooks shared by the legacy loader and its telemetry
// tracker. Do not include outside Tier 1.

#ifndef TIER0_MODULE_LOAD_TELEMETRY_INTERNAL_H
#define TIER0_MODULE_LOAD_TELEMETRY_INTERNAL_H

#include "tier1/module_load_telemetry.h"

class CSysModule;

PLATFORM_INTERFACE void Sys_RecordModuleLoad(
	CSysModule *pModule,
	const char *pRequestedPath,
	const char *pResolvedPath,
	int nProviderResult,
	const char *pProviderError );

PLATFORM_INTERFACE void Sys_RecordModuleEntryPoint(
	CSysModule *pModule,
	const char *pRequestedPath,
	const char *pEntryPoint,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError );

PLATFORM_INTERFACE void Sys_RecordModuleInterface(
	CSysModule *pModule,
	const char *pInterfaceName,
	bool bSuccess );

PLATFORM_INTERFACE void Sys_RecordModuleUnload(
	CSysModule *pModule,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError );

#endif // TIER0_MODULE_LOAD_TELEMETRY_INTERNAL_H
