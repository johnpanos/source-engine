//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//
#define TIER1_INTERFACE_IMPLEMENTATION

#if defined( _WIN32 )
#include <windows.h>
#endif

#if !defined( DONT_PROTECT_FILEIO_FUNCTIONS )
#define DONT_PROTECT_FILEIO_FUNCTIONS // for protected_things.h
#endif

#if defined( PROTECTED_THINGS_ENABLE )
#undef PROTECTED_THINGS_ENABLE // from protected_things.h
#endif

#include <stdio.h>
#include <errno.h>
#include "interface.h"
#include "basetypes.h"
#include "tier0/dbg.h"
#include <string.h>
#include <stdlib.h>
#include "tier1/strtools.h"
#include "tier0/module_load_telemetry_internal.h"
#include "tier0/icommandline.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#ifdef _WIN32
#include <direct.h> // getcwd
#elif POSIX
#include <dlfcn.h>
#include <unistd.h>
#define _getcwd getcwd
#endif

#ifdef POSIX
#include <sys/stat.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;

InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) :
	m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}

// ------------------------------------------------------------------------------------ //
// CreateInterface.
// This is the primary exported function by a dll, referenced by name via dynamic binding
// that exposes an opqaue function pointer to the interface.
//
// We have the Internal variant so Sys_GetFactoryThis() returns the correct internal 
// symbol under GCC/Linux/Mac as CreateInterface is DLL_EXPORT so its global so the loaders
// on those OS's pick exactly 1 of the CreateInterface symbols to be the one that is process wide and 
// all Sys_GetFactoryThis() calls find that one, which doesn't work. Using the internal walkthrough here
// makes sure Sys_GetFactoryThis() has the dll specific symbol and GetProcAddress() returns the module specific
// function for CreateInterface again getting the dll specific symbol we need.
// ------------------------------------------------------------------------------------ //
void* CreateInterfaceInternal( const char *pName, int *pReturnCode )
{
	InterfaceReg *pCur;
	
	for (pCur=InterfaceReg::s_pInterfaceRegs; pCur; pCur=pCur->m_pNext)
	{
		if (strcmp(pCur->m_pName, pName) == 0)
		{
			if (pReturnCode)
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}
	
	if (pReturnCode)
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;	
}

void* CreateInterface( const char *pName, int *pReturnCode )
{
    return CreateInterfaceInternal( pName, pReturnCode );
}



#ifdef POSIX
// Linux doesn't have this function so this emulates its functionality
void *GetModuleHandle(const char *name)
{
	void *handle;

	if( name == NULL )
	{
		// hmm, how can this be handled under linux....
		// is it even needed?
		return NULL;
	}

    if( (handle=dlopen(name, RTLD_NOW))==NULL)
    {
            printf("DLOPEN Error:%s\n",dlerror());
            // couldn't open this file
            return NULL;
    }

	// read "man dlopen" for details
	// in short dlopen() inc a ref count
	// so dec the ref count by performing the close
	dlclose(handle);
	return handle;
}
#endif

#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : pModuleName - module name
//			*pName - proc name
//-----------------------------------------------------------------------------
static void *Sys_GetProcAddress( const char *pModuleName, const char *pName )
{
	HMODULE hModule = (HMODULE)GetModuleHandle( pModuleName );
#ifdef WIN32
	return (void *)GetProcAddress( hModule, pName );
#else
	return (void *)dlsym( (void *)hModule, pName );
#endif
}

#if !defined(LINUX)
static void *Sys_GetProcAddress( HMODULE hModule, const char *pName )
{
#ifdef WIN32
	return (void *)GetProcAddress( hModule, pName );
#else
	return (void *)dlsym( (void *)hModule, pName );
#endif
}
#endif

bool Sys_IsDebuggerPresent()
{
	return Plat_IsInDebugSession();
}

struct ThreadedLoadLibaryContext_t
{
	const char *m_pLibraryName;
	HMODULE m_hLibrary;
	int m_nProviderResult;
};

#ifdef _WIN32

// wraps LoadLibraryEx() since 360 doesn't support that
static HMODULE InternalLoadLibrary( const char *pName, Sys_Flags flags )
{
	if ( flags & SYS_NOLOAD )
		return GetModuleHandle( pName );
	else
		return LoadLibraryEx( pName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
}
uintp ThreadedLoadLibraryFunc( void *pParam )
{
	ThreadedLoadLibaryContext_t *pContext = (ThreadedLoadLibaryContext_t*)pParam;
	pContext->m_hLibrary = InternalLoadLibrary( pContext->m_pLibraryName, SYS_NOFLAGS );
	pContext->m_nProviderResult = pContext->m_hLibrary ? 0 : (int)GetLastError();
	return 0;
}

#endif // _WIN32

static HMODULE Sys_LoadLibraryWithError(
	const char *pLibraryName,
	Sys_Flags flags,
	int *pProviderResult,
	char *pProviderError,
	int nProviderErrorSize,
	char *pResolvedPath,
	int nResolvedPathSize )
{
	if ( pProviderResult )
		*pProviderResult = 0;
	if ( pProviderError && nProviderErrorSize > 0 )
		pProviderError[0] = '\0';

	char str[ 1024 ];
	// Note: DLL_EXT_STRING can be "_srv.so" or "_360.dll". So be careful
	//	when using the V_*Extension* routines...
	const char *pDllStringExtension = V_GetFileExtension( DLL_EXT_STRING );
	const char *pModuleExtension = pDllStringExtension ? ( pDllStringExtension - 1 ) : DLL_EXT_STRING;

	Q_strncpy( str, pLibraryName, sizeof(str) );

	{
		// always force the final extension to be .dll
		V_SetExtension( str, pModuleExtension, sizeof(str) );
	}

	Q_FixSlashes( str );
	if ( pResolvedPath && nResolvedPathSize > 0 )
		Q_strncpy( pResolvedPath, str, nResolvedPathSize );

#ifdef _WIN32
	ThreadedLoadLibraryFunc_t threadFunc = GetThreadedLoadLibraryFunc();
	if ( !threadFunc )
	{
		HMODULE hLibrary = InternalLoadLibrary( str, flags );
		if ( !hLibrary )
		{
			int nError = (int)GetLastError();
			if ( pProviderResult )
				*pProviderResult = nError;
			if ( pProviderError )
				Q_snprintf( pProviderError, nProviderErrorSize,
					"Win32 error %d", nError );
		}
		return hLibrary;
	}

	// We shouldn't be passing noload while threaded.
	Assert( !( flags & SYS_NOLOAD ) );

	ThreadedLoadLibaryContext_t context;
	context.m_pLibraryName = str;
	context.m_hLibrary = 0;
	context.m_nProviderResult = 0;

	ThreadHandle_t h = CreateSimpleThread( (ThreadFunc_t)ThreadedLoadLibraryFunc, &context );


	unsigned int nTimeout = 0;
	while( ThreadWaitForObject( h, true, nTimeout ) == TW_TIMEOUT )
	{
		nTimeout = threadFunc(0);
	}

	ReleaseThreadHandle( h );
	if ( !context.m_hLibrary )
	{
		if ( pProviderResult )
			*pProviderResult = context.m_nProviderResult;
		if ( pProviderError )
			Q_snprintf( pProviderError, nProviderErrorSize,
				"Win32 error %d", context.m_nProviderResult );
	}
	return context.m_hLibrary;

#elif POSIX
	int dlopen_mode = RTLD_NOW;

#ifndef ANDROID
	if ( flags & SYS_NOLOAD )
		dlopen_mode |= RTLD_NOLOAD;
#endif

	dlerror();
	HMODULE ret = ( HMODULE )dlopen( str, dlopen_mode );
	if ( !ret )
	{
		const char *pError = dlerror();
		if ( pProviderResult )
			*pProviderResult = errno;
		if ( pProviderError )
			Q_strncpy( pProviderError, pError ? pError : "dlopen failed",
				nProviderErrorSize );
		if ( !( flags & SYS_NOLOAD ) && pError &&
			( strstr( pError, "No such file" ) == 0 ) &&
			( strstr( pError, "image not found" ) == 0 ) )
		{
			Msg( "failed to dlopen %s error=%s\n", str, pError );
		}
	}
	
	return ret;
#endif
}

HMODULE Sys_LoadLibrary( const char *pLibraryName, Sys_Flags flags )
{
	return Sys_LoadLibraryWithError(
		pLibraryName, flags, NULL, NULL, 0, NULL, 0 );
}
static bool s_bRunningWithDebugModules = false;

#ifdef POSIX

#ifdef ANDROID
#define DEFAULT_LIB_PATH ""
#else
#define DEFAULT_LIB_PATH "bin/"
#endif

bool foundLibraryWithPrefix( char *pModuleAbsolutePath, size_t AbsolutePathSize, const char *pPath, const char *pModuleName )
{
	char str[1024];
	Q_strncpy( str, pModuleName, sizeof(str) );
	V_SetExtension( str, DLL_EXT_STRING, sizeof(str) );
	bool bFound = false;

	struct stat statBuf;
	Q_snprintf(pModuleAbsolutePath, AbsolutePathSize, "%s/" DEFAULT_LIB_PATH "lib%s", pPath, str);
	bFound |= stat(pModuleAbsolutePath, &statBuf) == 0;

	if( !bFound )
	{
		Q_snprintf(pModuleAbsolutePath, AbsolutePathSize, "%s/" DEFAULT_LIB_PATH "%s", pPath, str);
		bFound |= stat(pModuleAbsolutePath, &statBuf) == 0;
	}

	if( !bFound )
	{
		Q_snprintf(pModuleAbsolutePath, AbsolutePathSize, "%s/lib%s", pPath, str);
		bFound |= stat(pModuleAbsolutePath, &statBuf) == 0;
	}

	if( !bFound )
	{
		Q_snprintf(pModuleAbsolutePath, AbsolutePathSize, "%s/%s", pPath, str);
		bFound |= stat(pModuleAbsolutePath, &statBuf) == 0;
	}

	return bFound;
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Loads a DLL/component from disk and returns a handle to it
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
CSysModule *Sys_LoadModule( const char *pModuleName, Sys_Flags flags /* = SYS_NOFLAGS (0) */ )
{
	// If using the Steam filesystem, either the DLL must be a minimum footprint
	// file in the depot (MFP) or a filesystem GetLocalCopy() call must be made
	// prior to the call to this routine.
	char szCwd[1024];
#ifdef POSIX
	char szModuleName[1024] = { 0 };
#endif
	HMODULE hDLL = NULL;
	char szResolvedModuleName[2048];
	char szProviderError[512] = { 0 };
	int nProviderResult = 0;
	Q_strncpy( szResolvedModuleName, pModuleName,
		sizeof( szResolvedModuleName ) );

	if ( !Q_IsAbsolutePath( pModuleName ) )
	{
		// full path wasn't passed in, using the current working dir
		_getcwd( szCwd, sizeof( szCwd ) );
		

		if (szCwd[strlen(szCwd) - 1] == '/' || szCwd[strlen(szCwd) - 1] == '\\' )
		{
			szCwd[strlen(szCwd) - 1] = 0;
		}

		char szAbsoluteModuleName[2048];
#ifdef ANDROID
		char *libPath = getenv("APP_LIB_PATH");
		char *modLibPath = getenv("APP_MOD_LIB");
		bool bFound;

		if( modLibPath && *modLibPath ) // first load library from mod launcher
		{
			bFound = foundLibraryWithPrefix( szAbsoluteModuleName, sizeof(szAbsoluteModuleName), modLibPath, pModuleName );

			if( bFound )
			{
				Q_strncpy( szResolvedModuleName, szAbsoluteModuleName,
					sizeof( szResolvedModuleName ) );
				hDLL = Sys_LoadLibraryWithError( szAbsoluteModuleName, flags,
					&nProviderResult, szProviderError,
					sizeof( szProviderError ), szResolvedModuleName,
					sizeof( szResolvedModuleName ) );
			}

			if( !hDLL && bFound )
				Error("Can't find mod library %s\n", szAbsoluteModuleName);
		}

		if( !foundLibraryWithPrefix( szAbsoluteModuleName, sizeof(szAbsoluteModuleName), libPath, pModuleName ) )
		{
			Warning("Can't find module - %s\n", pModuleName);
			if ( !hDLL )
			{
				nProviderResult = ENOENT;
				Q_strncpy( szProviderError, "module path resolution failed",
					sizeof( szProviderError ) );
				Q_strncpy( szResolvedModuleName, szAbsoluteModuleName,
					sizeof( szResolvedModuleName ) );
			}
			CSysModule *pModule = reinterpret_cast<CSysModule *>( hDLL );
			Sys_RecordModuleLoad( pModule, pModuleName,
				szResolvedModuleName, nProviderResult, szProviderError );
			return pModule;
		}

#elif defined( POSIX )
		if( !foundLibraryWithPrefix(szAbsoluteModuleName, sizeof(szAbsoluteModuleName), szCwd, pModuleName) )
		{
			Warning("Can't find module - %s\n", pModuleName);
			nProviderResult = ENOENT;
			Q_strncpy( szProviderError, "module path resolution failed",
				sizeof( szProviderError ) );
			Q_strncpy( szResolvedModuleName, szAbsoluteModuleName,
				sizeof( szResolvedModuleName ) );
			Sys_RecordModuleLoad( NULL, pModuleName, szResolvedModuleName,
				nProviderResult, szProviderError );
			return NULL;
		}
#else
		Q_snprintf( szAbsoluteModuleName, sizeof(szAbsoluteModuleName), "%s/bin/%s", szCwd, pModuleName );
#endif
		Q_strncpy( szResolvedModuleName, szAbsoluteModuleName,
			sizeof( szResolvedModuleName ) );
		Msg("LoadLibrary: pModule: %s, path: %s\n", pModuleName, szAbsoluteModuleName);

		if( !hDLL )
			hDLL = Sys_LoadLibraryWithError( szAbsoluteModuleName, flags,
				&nProviderResult, szProviderError,
				sizeof( szProviderError ), szResolvedModuleName,
				sizeof( szResolvedModuleName ) );
	}
	else
	{
#ifdef POSIX
		Q_strncpy( szModuleName, pModuleName, sizeof(szModuleName) );
		V_SetExtension( szModuleName, DLL_EXT_STRING, sizeof(szModuleName) );

		struct stat statBuf;
		bool bFound = stat(szModuleName, &statBuf) == 0;

		if( !bFound )
		{
			Warning("Can't find module - %s\n", pModuleName);
			nProviderResult = ENOENT;
			Q_strncpy( szProviderError, "module path resolution failed",
				sizeof( szProviderError ) );
			Q_strncpy( szResolvedModuleName, szModuleName,
				sizeof( szResolvedModuleName ) );
			Sys_RecordModuleLoad( NULL, pModuleName, szResolvedModuleName,
				nProviderResult, szProviderError );
			return NULL;
		}

		Q_strncpy( szResolvedModuleName, szModuleName,
			sizeof( szResolvedModuleName ) );
		Msg("LoadLibrary: path: %s\n", szModuleName);
#else
		Q_strncpy( szResolvedModuleName, pModuleName,
			sizeof( szResolvedModuleName ) );
#endif
	}


	if ( !hDLL )
	{
		// full path failed, let LoadLibrary() try to search the PATH now
		Q_strncpy( szResolvedModuleName, pModuleName,
			sizeof( szResolvedModuleName ) );
		hDLL = Sys_LoadLibraryWithError( pModuleName, flags,
			&nProviderResult, szProviderError,
			sizeof( szProviderError ), szResolvedModuleName,
			sizeof( szResolvedModuleName ) );
#if defined( _DEBUG )
		if ( !hDLL )
		{
// So you can see what the error is in the debugger...
#if defined( _WIN32 )
			char *lpMsgBuf;
			
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			LocalFree( (HLOCAL)lpMsgBuf );
#else
			Msg( "Failed to load %s: %s\n", pModuleName,
				szProviderError[0] ? szProviderError : "unknown provider error" );
#endif // _WIN32
		}
#endif // DEBUG
	}

#if !defined(LINUX)
	// If running in the debugger, assume debug binaries are okay, otherwise they must run with -allowdebug
	if ( Sys_GetProcAddress( hDLL, "BuiltDebug" ) )
	{
		if ( hDLL && !CommandLine()->FindParm( "-allowdebug" ) && !Sys_IsDebuggerPresent() )
		{
			Error( "Module %s is a debug build\n", pModuleName );
		}

		DevWarning( "Module %s is a debug build\n", pModuleName );

		if ( !s_bRunningWithDebugModules )
		{
			s_bRunningWithDebugModules = true;
			
#if 0 //def IS_WINDOWS_PC
			char chMemoryName[ MAX_PATH ];
			DebugKernelMemoryObjectName( chMemoryName );
			
			(void) CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, chMemoryName );
			// Created a shared memory kernel object specific to process id
			// Existence of this object indicates that we have debug modules loaded
#endif
		}
	}
#endif

	CSysModule *pModule = reinterpret_cast<CSysModule *>( hDLL );
	Sys_RecordModuleLoad( pModule, pModuleName, szResolvedModuleName,
		nProviderResult, szProviderError );
	return pModule;
}

//-----------------------------------------------------------------------------
// Purpose: Determine if any debug modules were loaded
//-----------------------------------------------------------------------------
bool Sys_RunningWithDebugModules()
{
	if ( !s_bRunningWithDebugModules )
	{
#if 0 //def IS_WINDOWS_PC
		char chMemoryName[ MAX_PATH ];
		DebugKernelMemoryObjectName( chMemoryName );

		HANDLE hObject = OpenFileMapping( FILE_MAP_READ, FALSE, chMemoryName );
		if ( hObject && hObject != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hObject );
			s_bRunningWithDebugModules = true;
		}
#endif
	}
	return s_bRunningWithDebugModules;
}


//-----------------------------------------------------------------------------
// Purpose: Unloads a DLL/component from
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
void Sys_UnloadModule( CSysModule *pModule )
{
	if ( !pModule )
		return;

	HMODULE	hDLL = reinterpret_cast<HMODULE>(pModule);
	bool bSuccess = false;
	int nProviderResult = 0;
	char szProviderError[512] = { 0 };

#ifdef _WIN32
	bSuccess = FreeLibrary( hDLL ) != FALSE;
	if ( !bSuccess )
	{
		nProviderResult = (int)GetLastError();
		Q_snprintf( szProviderError, sizeof( szProviderError ),
			"Win32 error %d", nProviderResult );
	}
#elif defined(POSIX)
	dlerror();
	nProviderResult = dlclose((void *)hDLL);
	bSuccess = nProviderResult == 0;
	if ( !bSuccess )
	{
		const char *pError = dlerror();
		Q_strncpy( szProviderError, pError ? pError : "dlclose failed",
			sizeof( szProviderError ) );
	}
#endif

	Sys_RecordModuleUnload( pModule, bSuccess, nProviderResult,
		szProviderError );
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : module - windows HMODULE from Sys_LoadModule() 
//			*pName - proc name
// Output : factory for this module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( CSysModule *pModule )
{
	if ( !pModule )
	{
		Sys_RecordModuleEntryPoint( NULL, NULL, "CreateInterface", false,
			0, "module handle is null" );
		return NULL;
	}

	HMODULE	hDLL = reinterpret_cast<HMODULE>(pModule);
	CreateInterfaceFn pFactory = NULL;
	int nProviderResult = 0;
	char szProviderError[512] = { 0 };
#ifdef _WIN32
	pFactory = reinterpret_cast<CreateInterfaceFn>(
		GetProcAddress( hDLL, CREATEINTERFACE_PROCNAME ) );
	if ( !pFactory )
	{
		nProviderResult = (int)GetLastError();
		Q_snprintf( szProviderError, sizeof( szProviderError ),
			"Win32 error %d", nProviderResult );
	}
#elif defined(POSIX)
	// Linux gives this error:
	//../public/interface.cpp: In function `IBaseInterface *(*Sys_GetFactory
	//(CSysModule *)) (const char *, int *)':
	//../public/interface.cpp:154: ISO C++ forbids casting between
	//pointer-to-function and pointer-to-object
	//
	// so lets get around it :)
	dlerror();
	pFactory = (CreateInterfaceFn)(GetProcAddress( (void *)hDLL, CREATEINTERFACE_PROCNAME ));
	if ( !pFactory )
	{
		const char *pError = dlerror();
		Q_strncpy( szProviderError, pError ? pError : "dlsym failed",
			sizeof( szProviderError ) );
	}
#endif
	Sys_RecordModuleEntryPoint( pModule, NULL, "CreateInterface",
		pFactory != NULL, nProviderResult, szProviderError );
	return pFactory;
}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of this module
// Output : interface_instance_t
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactoryThis( void )
{
	return &CreateInterfaceInternal;
}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of the named module
// Input  : *pModuleName - name of the module
// Output : interface_instance_t - instance of that module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( const char *pModuleName )
{
	CreateInterfaceFn pFactory = NULL;
	int nProviderResult = 0;
	char szProviderError[512] = { 0 };
#ifdef _WIN32
	pFactory = static_cast<CreateInterfaceFn>(
		Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME ) );
	if ( !pFactory )
	{
		nProviderResult = (int)GetLastError();
		Q_snprintf( szProviderError, sizeof( szProviderError ),
			"Win32 error %d", nProviderResult );
	}
#elif defined(POSIX)
	// see Sys_GetFactory( CSysModule *pModule ) for an explanation
	dlerror();
	pFactory = (CreateInterfaceFn)(
		Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME ) );
	if ( !pFactory )
	{
		const char *pError = dlerror();
		Q_strncpy( szProviderError, pError ? pError : "dlsym failed",
			sizeof( szProviderError ) );
	}
#endif
	Sys_RecordModuleEntryPoint( NULL, pModuleName, "CreateInterface",
		pFactory != NULL, nProviderResult, szProviderError );
	return pFactory;
}

//-----------------------------------------------------------------------------
// Purpose: get the interface for the specified module and version
// Input  : 
// Output : 
//-----------------------------------------------------------------------------
bool Sys_LoadInterface(
	const char *pModuleName,
	const char *pInterfaceVersionName,
	CSysModule **pOutModule,
	void **pOutInterface )
{
	CSysModule *pMod = Sys_LoadModule( pModuleName );
	if ( !pMod )
		return false;

	CreateInterfaceFn fn = Sys_GetFactory( pMod );
	if ( !fn )
	{
		Sys_RecordModuleInterface( pMod, pInterfaceVersionName, false );
		Sys_UnloadModule( pMod );
		return false;
	}

	*pOutInterface = fn( pInterfaceVersionName, NULL );
	Sys_RecordModuleInterface( pMod, pInterfaceVersionName,
		*pOutInterface != NULL );
	if ( !( *pOutInterface ) )
	{
		Sys_UnloadModule( pMod );
		return false;
	}

	if ( pOutModule )
		*pOutModule = pMod;

	return true;
}

CSysModule *Sys_LoadModuleWithContext(
	const char *pRequestingSubsystem,
	int nSourceLine,
	const char *pModuleName,
	Sys_Flags flags )
{
	CScopedModuleLoadRequest request( pRequestingSubsystem, nSourceLine );
	return Sys_LoadModule( pModuleName, flags );
}

void Sys_UnloadModuleWithContext(
	const char *pRequestingSubsystem,
	int nSourceLine,
	CSysModule *pModule )
{
	CScopedModuleLoadRequest request( pRequestingSubsystem, nSourceLine );
	Sys_UnloadModule( pModule );
}

CreateInterfaceFn Sys_GetFactoryWithContext(
	const char *pRequestingSubsystem,
	int nSourceLine,
	CSysModule *pModule )
{
	CScopedModuleLoadRequest request( pRequestingSubsystem, nSourceLine );
	return Sys_GetFactory( pModule );
}

CreateInterfaceFn Sys_GetFactoryWithContext(
	const char *pRequestingSubsystem,
	int nSourceLine,
	const char *pModuleName )
{
	CScopedModuleLoadRequest request( pRequestingSubsystem, nSourceLine );
	return Sys_GetFactory( pModuleName );
}

bool Sys_LoadInterfaceWithContext(
	const char *pRequestingSubsystem,
	int nSourceLine,
	const char *pModuleName,
	const char *pInterfaceVersionName,
	CSysModule **pOutModule,
	void **pOutInterface )
{
	CScopedModuleLoadRequest request(
		pRequestingSubsystem, nSourceLine, pInterfaceVersionName );
	return Sys_LoadInterface( pModuleName, pInterfaceVersionName,
		pOutModule, pOutInterface );
}

//-----------------------------------------------------------------------------
// Purpose: Place this as a singleton at module scope (e.g.) and use it to get the factory from the specified module name.  
// 
// When the singleton goes out of scope (.dll unload if at module scope),
//  then it'll call Sys_UnloadModule on the module so that the refcount is decremented 
//  and the .dll actually can unload from memory.
//-----------------------------------------------------------------------------
CDllDemandLoader::CDllDemandLoader( char const *pchModuleName ) : 
	m_pchModuleName( pchModuleName ), 
	m_pRequestingSubsystem( "<legacy ABI caller>" ),
	m_nSourceLine( 0 ),
	m_hModule( 0 ),
	m_bLoadAttempted( false )
{
}

CDllDemandLoader::CDllDemandLoader(
	char const *pchModuleName,
	const char *pRequestingSubsystem,
	int nSourceLine ) :
	m_pchModuleName( pchModuleName ),
	m_pRequestingSubsystem( pRequestingSubsystem ),
	m_nSourceLine( nSourceLine ),
	m_hModule( 0 ),
	m_bLoadAttempted( false )
{
}

CDllDemandLoader::~CDllDemandLoader()
{
	Unload();
}

CreateInterfaceFn CDllDemandLoader::GetFactory()
{
	if ( !m_hModule && !m_bLoadAttempted )
	{
		m_bLoadAttempted = true;
		m_hModule = Sys_LoadModuleWithContext(
			m_pRequestingSubsystem, m_nSourceLine, m_pchModuleName );
	}

	if ( !m_hModule )
	{
		return NULL;
	}

	return Sys_GetFactoryWithContext(
		m_pRequestingSubsystem, m_nSourceLine, m_hModule );
}

void CDllDemandLoader::Unload()
{
	if ( m_hModule )
	{
		Sys_UnloadModuleWithContext(
			m_pRequestingSubsystem, m_nSourceLine, m_hModule );
		m_hModule = 0;
	}
}

#if defined( STAGING_ONLY ) && defined( _WIN32 )

typedef USHORT( WINAPI RtlCaptureStackBackTrace_FUNC )(
	ULONG frames_to_skip,
	ULONG frames_to_capture,
	PVOID *backtrace,
	PULONG backtrace_hash );

extern "C" int backtrace( void **buffer, int size )
{
	HMODULE hNTDll = GetModuleHandleA( "ntdll.dll" );
	static RtlCaptureStackBackTrace_FUNC * const pfnRtlCaptureStackBackTrace =
		( RtlCaptureStackBackTrace_FUNC * )GetProcAddress( hNTDll, "RtlCaptureStackBackTrace" );

	if ( !pfnRtlCaptureStackBackTrace )
		return 0;

	return (int)pfnRtlCaptureStackBackTrace( 2, size, buffer, 0 );
}

#endif // STAGING_ONLY && _WIN32
