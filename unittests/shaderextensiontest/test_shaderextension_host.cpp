//========= Copyright Valve Corporation, All rights reserved. ============//
#include "../../materialsystem/shaderextension_compatibility_host.h"
#include "filesystem_passthru.h"
#include "../../materialsystem/IShaderSystem.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace
{
std::string g_Events;
int g_Failure;
int g_Checks;
void Check( bool condition )
{
	++g_Checks;
	if ( !condition )
	{
		std::fprintf(
		    stderr, "shader-extension check %d failed; events=%s\n", g_Checks, g_Events.c_str() );
		std::exit( 1 );
	}
}
void *HostFactory( const char *, int * )
{
	return NULL;
}
class Shaders : public IShaderDLLInternal
{
public:
	bool Connect( CreateInterfaceFn factory, bool materialSystem )
	{
		Check( factory == HostFactory && !materialSystem );
		g_Events += "connect;";
		return g_Failure != 4;
	}
	void Disconnect( bool materialSystem )
	{
		Check( !materialSystem );
		g_Events += "disconnect;";
	}
	int ShaderCount() const { return 7; }
	IShader *GetShader( int ) { return NULL; }
} g_Shaders;
void *ExtensionFactory( const char *version, int * )
{
	Check( std::strcmp( version, SHADER_DLL_INTERFACE_VERSION ) == 0 );
	g_Events += "version;";
	return g_Failure == 3 ? NULL : &g_Shaders;
}
class FileSystem : public CFileSystemPassThru
{
public:
	CSysModule *LoadModule( const char *path, const char *pathID, bool validated )
	{
		Check( std::strcmp( path, "game_shader_test" ) == 0 );
		Check( std::strcmp( pathID, "GAMEBIN" ) == 0 && !validated );
		g_Events += "load;";
		return g_Failure == 1 ? NULL : reinterpret_cast<CSysModule *>( this );
	}
	void UnloadModule( CSysModule *module )
	{
		Check( module == reinterpret_cast<CSysModule *>( this ) );
		g_Events += "unload;";
	}
};
}

// Substitute only native acquisition and host-factory lookup. The actual host,
// filesystem contract, and frozen shader ABI compile unchanged in this fixture.
CreateInterfaceFn Sys_GetFactoryWithContext( const char *, int, CSysModule * )
{
	g_Events += "factory;";
	return g_Failure == 2 ? NULL : ExtensionFactory;
}
CreateInterfaceFn Sys_GetFactoryThis()
{
	return HostFactory;
}
CScopedModuleLoadRequest::CScopedModuleLoadRequest( const char *, int, const char * )
{
}
CScopedModuleLoadRequest::~CScopedModuleLoadRequest()
{
}

int main()
{
	FileSystem fileSystem;
	Check( !CShaderExtensionCompatibilityHost::Load( fileSystem, NULL ) );
	Check( !CShaderExtensionCompatibilityHost::Load( fileSystem, "" ) );
	Check( g_Events.empty() );
	const char *expected[] = { "load;factory;version;connect;disconnect;unload;", "load;",
	    "load;factory;unload;", "load;factory;version;unload;",
	    "load;factory;version;connect;disconnect;unload;" };
	for ( int failure = 0; failure != 5; ++failure )
	{
		g_Failure = failure;
		g_Events.clear();
		CShaderExtensionCompatibilityHost *host =
		    CShaderExtensionCompatibilityHost::Load( fileSystem, "game_shader_test" );
		Check( ( host != NULL ) == ( failure == 0 ) );
		if ( host )
		{
			Check( host->Shaders().ShaderCount() == 7 );
			Check( g_Events == "load;factory;version;connect;" );
			delete host;
		}
		Check( g_Events == expected[failure] );
	}
	std::printf( "CHECKS %d\n", g_Checks );
	return 0;
}
