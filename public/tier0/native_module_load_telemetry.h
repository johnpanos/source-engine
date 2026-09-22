//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Native dynamic-loader adapters for RFC 0001 Phase A telemetry.
// Include this after platform loader declarations in implementation files that
// call the OS loader directly. The adapters preserve the native signatures and
// attach the original source location to the shared process telemetry stream.
//
//=============================================================================//

#ifndef TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H
#define TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H

#include "tier0/platform.h"

#if defined( _WIN32 )

#include <windows.h>

PLATFORM_INTERFACE HMODULE ModuleLoadTelemetry_LoadLibraryA(
	LPCSTR pPath, const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE HMODULE ModuleLoadTelemetry_LoadLibraryW(
	LPCWSTR pPath, const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE HMODULE ModuleLoadTelemetry_LoadLibraryExA(
	LPCSTR pPath, HANDLE hFile, DWORD nFlags,
	const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE HMODULE ModuleLoadTelemetry_LoadLibraryExW(
	LPCWSTR pPath, HANDLE hFile, DWORD nFlags,
	const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE FARPROC ModuleLoadTelemetry_GetProcAddress(
	HMODULE hModule, LPCSTR pEntryPoint,
	const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE BOOL ModuleLoadTelemetry_FreeLibrary(
	HMODULE hModule, const char *pRequester, int nSourceLine );

#if !defined( NATIVE_MODULE_LOAD_TELEMETRY_IMPLEMENTATION )
#undef LoadLibraryA
#undef LoadLibraryW
#undef LoadLibraryExA
#undef LoadLibraryExW
#undef GetProcAddress
#undef FreeLibrary
#define LoadLibraryA( path ) \
	ModuleLoadTelemetry_LoadLibraryA( path, __FILE__, __LINE__ )
#define LoadLibraryW( path ) \
	ModuleLoadTelemetry_LoadLibraryW( path, __FILE__, __LINE__ )
#define LoadLibraryExA( path, file, flags ) \
	ModuleLoadTelemetry_LoadLibraryExA( \
		path, file, flags, __FILE__, __LINE__ )
#define LoadLibraryExW( path, file, flags ) \
	ModuleLoadTelemetry_LoadLibraryExW( \
		path, file, flags, __FILE__, __LINE__ )
#define GetProcAddress( module, entry ) \
	ModuleLoadTelemetry_GetProcAddress( \
		module, entry, __FILE__, __LINE__ )
#define FreeLibrary( module ) \
	ModuleLoadTelemetry_FreeLibrary( module, __FILE__, __LINE__ )
#endif

#elif defined( POSIX )

#include <dlfcn.h>

PLATFORM_INTERFACE void *ModuleLoadTelemetry_dlopen(
	const char *pPath, int nMode,
	const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE void *ModuleLoadTelemetry_dlsym(
	void *pModule, const char *pEntryPoint,
	const char *pRequester, int nSourceLine );
PLATFORM_INTERFACE int ModuleLoadTelemetry_dlclose(
	void *pModule, const char *pRequester, int nSourceLine );

#if !defined( NATIVE_MODULE_LOAD_TELEMETRY_IMPLEMENTATION )
#undef dlopen
#undef dlsym
#undef dlclose
#define dlopen( path, mode ) \
	ModuleLoadTelemetry_dlopen( path, mode, __FILE__, __LINE__ )
#define dlsym( module, entry ) \
	ModuleLoadTelemetry_dlsym( module, entry, __FILE__, __LINE__ )
#define dlclose( module ) \
	ModuleLoadTelemetry_dlclose( module, __FILE__, __LINE__ )
#endif

#endif

#endif // TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H
