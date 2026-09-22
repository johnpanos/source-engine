// Private implementation hooks shared by the legacy loader and its telemetry
// tracker. Do not include outside Tier 1.

#ifndef TIER1_MODULE_LOAD_TELEMETRY_INTERNAL_H
#define TIER1_MODULE_LOAD_TELEMETRY_INTERNAL_H

#include "tier1/module_load_telemetry.h"

class CSysModule;

void Sys_RecordModuleLoad(
	CSysModule *pModule,
	const char *pRequestedPath,
	const char *pResolvedPath,
	int nProviderResult,
	const char *pProviderError );

void Sys_RecordModuleEntryPoint(
	CSysModule *pModule,
	const char *pRequestedPath,
	const char *pEntryPoint,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError );

void Sys_RecordModuleInterface(
	CSysModule *pModule,
	const char *pInterfaceName,
	bool bSuccess );

void Sys_RecordModuleUnload(
	CSysModule *pModule,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError );

#endif // TIER1_MODULE_LOAD_TELEMETRY_INTERNAL_H
