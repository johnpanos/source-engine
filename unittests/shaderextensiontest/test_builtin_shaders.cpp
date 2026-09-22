//========= Copyright Valve Corporation, All rights reserved. ============//
#include "../../materialsystem/builtin_shader_provider.h"
#include "../../materialsystem/IShaderSystem.h"
#include "materialsystem/materialsystem_config.h"
#include "vstdlib/cvar.h"
#include "icvar.h"
#include "tier0/icommandline.h"
#include <cstdio>
#include <cstring>

namespace
{
int g_Checks;
int g_Failures;
void Check( bool condition, int line )
{
	++g_Checks;
	if ( !condition )
	{
		++g_Failures;
		std::fprintf( stderr, "builtin shader check failed at line %d\n", line );
	}
}
#define CHECK( condition ) Check( ( condition ), __LINE__ )

void *CvarFixtureFactory( const char *name, int * )
{
	return std::strcmp( name, CVAR_INTERFACE_VERSION ) == 0 ? VStdLib_GetICVar() : NULL;
}

void Run()
{
	const BuiltinShaderProvider *provider = StandardShaderLibrary_Describe();
	CHECK( provider != NULL );
	if ( !provider )
		return;
	CHECK( IsBuiltinShaderProviderSelected( provider, NULL ) );
	CHECK( IsBuiltinShaderProviderSelected( provider, "" ) );
	CHECK( IsBuiltinShaderProviderSelected( provider, "source-standard-materials" ) );
	CHECK( IsBuiltinShaderProviderSelected( provider, "stdshader_dx9" ) );
	CHECK( IsBuiltinShaderProviderSelected( provider, "STDSHADER_DX9" DLL_EXT_STRING ) );
	CHECK( !IsBuiltinShaderProviderSelected( provider, "stdshader_dx8" ) );
	CHECK( !IsBuiltinShaderProviderSelected( provider, "stdshader_dbg" ) );
	CHECK( !IsBuiltinShaderProviderSelected( provider, "shader_test" ) );
	CHECK( !IsBuiltinShaderProviderSelected( provider, "../stdshader_dx9" DLL_EXT_STRING ) );
	CHECK( !IsBuiltinShaderProviderSelected( provider, "s" ) );
	CHECK( !IsBuiltinShaderProviderSelected( NULL, NULL ) );
	BuiltinShaderProvider malformed = *provider;
	malformed.id = NULL;
	CHECK( !IsBuiltinShaderProviderSelected( &malformed, NULL ) );
	malformed = *provider;
	malformed.legacyModuleName = NULL;
	CHECK( !IsBuiltinShaderProviderSelected( &malformed, NULL ) );
	malformed = *provider;
	malformed.connect = NULL;
	CHECK( !IsBuiltinShaderProviderSelected( &malformed, NULL ) );
	malformed = *provider;
	malformed.disconnect = NULL;
	CHECK( !IsBuiltinShaderProviderSelected( &malformed, NULL ) );

	// Host rendering services are deliberately opaque here: enumeration and
	// lifecycle must not call rendering APIs or require a window/device. CVars
	// are the real linked registry so registration and rollback are observable.
	int token;
	MaterialSystem_Config_t config;
	CommandLine()->CreateCmdLine( "builtin_shader_conformance" );
	ICvar *cvars = VStdLib_GetICVar();
	CHECK( cvars->Connect( CvarFixtureFactory ) );
	BuiltinShaderHostServices host = { reinterpret_cast<IMaterialSystemHardwareConfig *>( &token ),
	    &config, reinterpret_cast<IShaderSystem *>( &token ), cvars };
	CHECK( cvars != NULL );
	provider->disconnect();
	for ( int missing = 0; missing != 4; ++missing )
	{
		BuiltinShaderHostServices incomplete = host;
		switch ( missing )
		{
		case 0:
			incomplete.hardware = NULL;
			break;
		case 1:
			incomplete.config = NULL;
			break;
		case 2:
			incomplete.shaders = NULL;
			break;
		case 3:
			incomplete.cvars = NULL;
			break;
		}
		CHECK( !provider->connect( incomplete ) );
		CHECK( !cvars->FindVar( "mat_disable_lightwarp" ) );
		provider->disconnect();
	}

	for ( int repeat = 0; repeat != 2; ++repeat )
	{
		IShaderDLLInternal *shaders = provider->connect( host );
		CHECK( shaders != NULL );
		if ( !shaders )
			return;
		CHECK( shaders->ShaderCount() > 0 );
		CHECK( !shaders->GetShader( -1 ) );
		CHECK( !shaders->GetShader( shaders->ShaderCount() ) );
		bool foundLightmapped = false;
		bool foundWireframe = false;
		for ( int i = 0; i < shaders->ShaderCount(); ++i )
		{
			IShader *shader = shaders->GetShader( i );
			CHECK( shader && shader->GetName() && shader->GetName()[0] );
			if ( !shader || !shader->GetName() )
				continue;
			foundLightmapped |= std::strcmp( shader->GetName(), "LightmappedGeneric" ) == 0;
			foundWireframe |= std::strcmp( shader->GetName(), "Wireframe_DX9" ) == 0;
		}
		CHECK( foundLightmapped && foundWireframe );
		ConVar *registered = cvars->FindVar( "mat_disable_lightwarp" );
		CHECK( registered != NULL );
		CHECK( !provider->connect( host ) );
		CHECK( cvars->FindVar( "mat_disable_lightwarp" ) == registered );
		provider->disconnect();
		CHECK( !cvars->FindVar( "mat_disable_lightwarp" ) );
		provider->disconnect();
	}
	cvars->Disconnect();
}
}

int main()
{
	Run();
	std::printf( "CONFORMANCE %d %d\n", g_Checks, g_Failures );
	return g_Checks > 0 && !g_Failures ? 0 : 1;
}
