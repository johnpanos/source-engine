//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//


#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef OSX
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "isys.h"
#include "console/conproc.h"
#include "dedicated.h"
#include "engine_hlds_api.h"
#include "checksum_md5.h"
#include "idedicatedexports.h"
#include "tier0/vcrmode.h"
#include "tier0/dbg.h"
#include "mathlib/mathlib.h"
#include "interface.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tier0/native_module_load_telemetry.h"
#include "materialsystem/imaterialsystem.h"
#include "appframework/linked_systems.h"
#include "render/legacy_shader_provider.h"
#include "istudiorender.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "vphysics_interface.h"
#include "icvar.h"
#include "filesystem/IQueuedLoader.h"
#include "console/TextConsoleUnix.h"

// platform.h maps the Win32 spelling to dlsym on POSIX. Keep this class method
// name literal now that dlsym itself is wrapped by loader telemetry.
#ifdef GetProcAddress
#undef GetProcAddress
#endif

bool InitInstance( );

char g_szEXEName[ MAX_PATH ];

extern CTextConsoleUnix console;

//-----------------------------------------------------------------------------
// Purpose: Implements OS Specific layer ( loosely )
//-----------------------------------------------------------------------------
class CSys : public ISys
{
public:
	virtual		~CSys();

	virtual bool LoadModules( CDedicatedAppSystemGroup *pAppSystemGroup );

	void		Sleep( int msec );
	bool		GetExecutableName( char *out );
	void		ErrorMessage( int level, const char *msg );

	void		WriteStatusText( char *szText );
	void		UpdateStatus( int force );

	long		LoadLibrary( char *lib );
	void		FreeLibrary( long library );
	void		*GetProcAddress( long library, const char *name );

	bool		CreateConsoleWindow( void );
	void		DestroyConsoleWindow( void );

	void		ConsoleOutput ( char *string );
	char		*ConsoleInput ( int index, char *buf, int buflen );
	void		Printf( const char *fmt, ...);
};

static CSys g_Sys;
ISys *sys = &g_Sys;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSys::~CSys()
{
	sys = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : msec
// Output : 
//-----------------------------------------------------------------------------
void CSys::Sleep( int msec )
{
	usleep(msec * 1000);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : handle, function name-
// Output : void *
//-----------------------------------------------------------------------------
void *CSys::GetProcAddress( long library, const char *name )
{
	return dlsym( library, name );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *lib -
// Output : long
//-----------------------------------------------------------------------------
long CSys::LoadLibrary( char *lib )
{
	void *hDll = NULL;

	char cwd[1024];
	char absolute_lib[1024];

	if (!getcwd(cwd, sizeof(cwd)))
		ErrorMessage(1, "Sys_LoadLibrary: Couldn't determine current directory.");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	Q_snprintf(absolute_lib, sizeof( absolute_lib ), "%s/%s", cwd, lib);

	hDll = dlopen( absolute_lib, RTLD_NOW );
	if ( !hDll )
	{
		ErrorMessage( 1, dlerror() );
	}
	return (long)hDll;
}

void CSys::FreeLibrary( long library )
{
	if ( !library )
		return;

	dlclose( (void *)library );
}

bool CSys::GetExecutableName( char *out )
{
	char *name = strrchr(g_szEXEName, '/' );
	if ( name )
	{
		strcpy( out, name + 1);
		return true;
	}
	else
	{
		return false;
	}
}

/*
==============
ErrorMessage

Engine is erroring out, display error in message box
==============
*/
void CSys::ErrorMessage( int level, const char *msg )
{
	Error( "%s\n", msg );
	exit( -1 );
}

void CSys::UpdateStatus( int force )
{
}

/*
================
ConsoleOutput

Print text to the dedicated console
================
*/
void CSys::ConsoleOutput (char *string)
{
	console.Print(string);
}

/*
==============
Printf

Engine is printing to console
==============
*/
void CSys::Printf( const char *fmt, ...)
{
	// Dump text to debugging console.
	va_list argptr;
	char szText[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (szText, sizeof( szText ), fmt, argptr);
	va_end (argptr);

	// Get Current text and append it.
	ConsoleOutput( szText );
}

/*
================
ConsoleInput

================
*/
char *CSys::ConsoleInput( int index, char *buf, int buflen )
{
	return console.GetLine( index, buf, buflen );
}

/*
==============
WriteStatusText

==============
*/
void CSys::WriteStatusText( char *szText )
{
}

/*
==============
CreateConsoleWindow

Create console window ( overridable? )
==============
*/
bool CSys::CreateConsoleWindow( void )
{
	return true;
}

/*
==============
DestroyConsoleWindow

==============
*/
void CSys::DestroyConsoleWindow( void )
{
}

/*
================
GameInit
================
*/
bool CSys::LoadModules( CDedicatedAppSystemGroup *pAppSystemGroup )
{
	// Linked instances preserve the established Connect/Init order. The embedded
	// filesystem owns the queued loader; modules outlive all borrowed services.
	IMaterialSystem *material = MaterialSystem_Create();
	IDedicatedServerAPI *server = Engine_CreateDedicatedAPI();
	const char *pPhysicsModule = CommandLine()->ParmValue( "-physics", "vphysics" );
	char physicsDLLName[MAX_PATH];
	Q_snprintf( physicsDLLName, sizeof( physicsDLLName ), "%s" DLL_EXT_STRING, pPhysicsModule );
	AppModule_t physicsAppModule = pAppSystemGroup->LoadPhysicsModule( physicsDLLName );
	if ( physicsAppModule == APP_MODULE_INVALID )
	{
		Warning( "Failed to load physics provider '%s'.\n", physicsDLLName );
		return false;
	}

	if ( !pAppSystemGroup->AddComposedSystem(
	         Engine_CreateCvarQuery(), CVAR_QUERY_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem(
	         SoundEmitterSystem_Create(), SOUNDEMITTERSYSTEM_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem( material, MATERIAL_SYSTEM_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem(
	         StudioRender_Create(), STUDIO_RENDER_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem( physicsAppModule, VPHYSICS_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem( DataCache_Create(), DATACACHE_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem( MDLCache_Create(), MDLCACHE_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem(
	         StudioDataCache_Create(), STUDIO_DATA_CACHE_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem(
	         Dedicated_CreateQueuedLoader(), QUEUEDLOADER_INTERFACE_VERSION ) ||
	     !pAppSystemGroup->AddComposedSystem( server, VENGINE_HLDS_API_VERSION ) )
	{
		return false;
	}
	if ( !MaterialSystem_BindShaderProvider( material, NullShaderBackend_Describe() ) )
		return false;
	// The dedicated server renders nothing and requires no render feature.
	const render::RenderProfileRequest renderRequest = render::PreferAvailableRenderFeatures();
	if ( !MaterialSystem_SetRenderProfileRequest( material, &renderRequest ) )
		return false;
	engine = server;
	return true;
}

bool NET_Init()
{
	return true;
}

void NET_Shutdown()
{
}

extern int main(int argc, char *argv[]);
DLL_EXPORT int DedicatedMain( int argc, char *argv[] );

int DedicatedMain( int argc, char *argv[] )
{
	return main(argc,argv);
}
