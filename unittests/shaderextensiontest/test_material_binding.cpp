//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Exercise the actual material system's typed provider boundary without
//          Connect factories, graphics initialization, or a native window.
//
//=============================================================================//

#include "appframework/linked_systems.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/idebugtextureinfo.h"
#include "shaderapi/ishaderapi.h"
#include "shaderapi/IShaderDevice.h"
#include "shaderapi/ishadershadow.h"
#include "render/legacy_shader_provider.h"
#include "render/builtin_shader_provider.h"

#if defined( USE_SDL3 )
#include <SDL3/SDL.h>
#endif

#include <cstdio>
#include <cstring>

namespace
{

int g_Checks = 0;
int g_Failures = 0;
int g_FactoryCalls = 0;
int g_MissingService = -1;
render::LegacyShaderServices g_ActualServices;

void Check( bool passed, const char *condition, int line )
{
	++g_Checks;
	if ( !passed )
	{
		++g_Failures;
		std::fprintf( stderr, "material binding line %d: %s\n", line, condition );
	}
}

#define CHECK( condition ) Check( ( condition ), #condition, __LINE__ )

bool CountingFactory( render::LegacyShaderServices *services )
{
	++g_FactoryCalls;
	*services = g_ActualServices;
	switch ( g_MissingService )
	{
	case 0:
		services->manager = NULL;
		break;
	case 1:
		services->api = NULL;
		break;
	case 2:
		services->device = NULL;
		break;
	case 3:
		services->shadow = NULL;
		break;
	case 4:
		services->hardware = NULL;
		break;
	case 5:
		services->debugTextures = NULL;
		break;
	}
	return true;
}

bool FailingFactory( render::LegacyShaderServices *services )
{
	++g_FactoryCalls;
	// Even a fully populated output cannot commit if the factory reports failure.
	*services = g_ActualServices;
	return false;
}

void CheckViews( IMaterialSystem &material, const render::LegacyShaderServices &services )
{
	CHECK( material.QueryInterface( SHADER_DEVICE_MGR_INTERFACE_VERSION ) == services.manager );
	CHECK( material.QueryInterface( SHADERAPI_INTERFACE_VERSION ) == services.api );
	CHECK( material.QueryInterface( SHADER_DEVICE_INTERFACE_VERSION ) == services.device );
	CHECK( material.QueryInterface( SHADERSHADOW_INTERFACE_VERSION ) == services.shadow );
	CHECK( material.QueryInterface( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION ) ==
	       services.hardware );
	CHECK( material.QueryInterface( DEBUG_TEXTURE_INFO_VERSION ) == services.debugTextures );
	CHECK( material.QueryInterface( "SourceUnregisteredShaderInterface999" ) == NULL );
}

bool CheckMalformed( IMaterialSystem *material, render::LegacyShaderProvider &provider )
{
	const int callsBefore = g_FactoryCalls;
	const bool accepted = MaterialSystem_BindShaderProvider( material, &provider );
	CHECK( !accepted );
	CHECK( g_FactoryCalls == callsBefore );
	CheckViews( *material, render::LegacyShaderServices() );
	return !accepted;
}

int g_BuiltinConnects;
IShaderDLLInternal *UnusedBuiltinConnect( const BuiltinShaderHostServices & )
{
	++g_BuiltinConnects;
	return NULL;
}
void UnusedBuiltinDisconnect()
{
	CHECK( false );
}
void *MissingMaterialServices( const char *, int * )
{
	return NULL;
}

void CheckBuiltinBinding( IMaterialSystem *material )
{
	BuiltinShaderProvider candidate = {
	    "binding-fixture", "fixture", UnusedBuiltinConnect, UnusedBuiltinDisconnect };
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( NULL, &candidate ) );
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, NULL ) );
	int foreign;
	CHECK( !MaterialSystem_BindBuiltinShaderProvider(
	    reinterpret_cast<IMaterialSystem *>( &foreign ), &candidate ) );
	BuiltinShaderProvider malformed = candidate;
	malformed.id = NULL;
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, &malformed ) );
	malformed = candidate;
	malformed.legacyModuleName = NULL;
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, &malformed ) );
	malformed = candidate;
	malformed.connect = NULL;
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, &malformed ) );
	malformed = candidate;
	malformed.disconnect = NULL;
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, &malformed ) );
	CHECK( MaterialSystem_BindBuiltinShaderProvider( material, &candidate ) );
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, &candidate ) );
	CHECK( !material->Connect( MissingMaterialServices ) );
	CHECK( !MaterialSystem_BindBuiltinShaderProvider( material, &candidate ) );
	CHECK( g_BuiltinConnects == 0 );
	material->Disconnect();
	CHECK( MaterialSystem_BindBuiltinShaderProvider( material, &candidate ) );
	CHECK( !material->Connect( NULL ) );
	material->Disconnect();
	CHECK( MaterialSystem_BindBuiltinShaderProvider( material, &candidate ) );
	material->Disconnect();
	CHECK( g_BuiltinConnects == 0 );
}

void Run()
{
#if defined( USE_SDL3 )
	CHECK( SDL_WasInit( SDL_INIT_VIDEO ) == 0 );
#endif
	IMaterialSystem *material = MaterialSystem_Create();
	const render::LegacyShaderProvider *actual = ShaderBackend_Describe();
	CHECK( material != NULL );
	CHECK( actual != NULL );
	if ( !material || !actual )
		return;
	CheckBuiltinBinding( material );
	CHECK( actual->id && actual->id[0] );
	CHECK( actual->legacyModuleName && actual->legacyModuleName[0] );
	CHECK( actual->create != NULL );
	if ( !actual->create )
		return;
	CHECK( actual->create( &g_ActualServices ) );
	CHECK( g_ActualServices.IsComplete() );
	CheckViews( *material, render::LegacyShaderServices() );

	render::LegacyShaderProvider candidate = { "binding-fixture", "fixture", CountingFactory };
	CHECK( !MaterialSystem_BindShaderProvider( NULL, &candidate ) );
	// The foreign pointer names an actual live provider object. It is never
	// dereferenced or treated as a material implementation by this entry point.
	IMaterialSystem *foreign = reinterpret_cast<IMaterialSystem *>( g_ActualServices.api );
	CHECK( foreign != material );
	CHECK( !MaterialSystem_BindShaderProvider( foreign, &candidate ) );
	CHECK( !MaterialSystem_BindShaderProvider( material, NULL ) );
	CHECK( g_FactoryCalls == 0 );
	CheckViews( *material, render::LegacyShaderServices() );

	candidate.id = NULL;
	if ( !CheckMalformed( material, candidate ) )
		return;
	candidate.id = "";
	if ( !CheckMalformed( material, candidate ) )
		return;
	candidate.id = "binding-fixture";
	candidate.create = NULL;
	if ( !CheckMalformed( material, candidate ) )
		return;
	candidate.create = FailingFactory;
	CHECK( !MaterialSystem_BindShaderProvider( material, &candidate ) );
	CHECK( g_FactoryCalls == 1 );
	CheckViews( *material, render::LegacyShaderServices() );

	candidate.create = CountingFactory;
	for ( int missing = 0; missing < 5; ++missing )
	{
		g_MissingService = missing;
		const int callsBefore = g_FactoryCalls;
		CHECK( !MaterialSystem_BindShaderProvider( material, &candidate ) );
		CHECK( g_FactoryCalls == callsBefore + 1 );
		CheckViews( *material, render::LegacyShaderServices() );
	}

	// Unsupported legacy names must not silently choose the linked default.
	material->SetShaderAPI( "source-unavailable-shader-provider" );
	CheckViews( *material, render::LegacyShaderServices() );

	// Debug textures are optional; the five required services remain available.
	g_MissingService = 5;
	CHECK( MaterialSystem_BindShaderProvider( material, &candidate ) );
	render::LegacyShaderServices withoutDebug = g_ActualServices;
	withoutDebug.debugTextures = NULL;
	CheckViews( *material, withoutDebug );
	CHECK( !MaterialSystem_BindShaderProvider( material, actual ) );
	CheckViews( *material, withoutDebug );

	// Failure before Connect must permit rollback and a clean second binding.
	CHECK( !material->Connect( NULL ) );
	material->Disconnect();
	CheckViews( *material, render::LegacyShaderServices() );
	CHECK( MaterialSystem_BindShaderProvider( material, actual ) );
	CheckViews( *material, g_ActualServices );

	// Once bound, invalid/replacement factories cannot run or mutate live views.
	candidate.create = FailingFactory;
	const int callsBefore = g_FactoryCalls;
	CHECK( !MaterialSystem_BindShaderProvider( material, &candidate ) );
	CHECK( g_FactoryCalls == callsBefore );
	CHECK( !MaterialSystem_BindShaderProvider( material, actual ) );
	CheckViews( *material, g_ActualServices );
	material->Disconnect();
	CheckViews( *material, render::LegacyShaderServices() );

	// The retained ABI entry selects the same linked, typed implementation.
	material->SetShaderAPI( actual->legacyModuleName );
	CheckViews( *material, g_ActualServices );
	material->Disconnect();
	CheckViews( *material, render::LegacyShaderServices() );

	// Tool roots can explicitly select the null provider even when the material
	// library's compatibility default is a graphics provider. Selection alone must
	// neither initialize video nor retain the previous provider's borrowed views.
	const render::LegacyShaderProvider *nullProvider = NullShaderBackend_Describe();
	CHECK( nullProvider != NULL );
	if ( nullProvider )
	{
		CHECK( std::strcmp( nullProvider->id, "null" ) == 0 );
		render::LegacyShaderServices nullServices;
		CHECK( nullProvider->create( &nullServices ) );
		CHECK( nullServices.IsComplete() );
		CHECK( MaterialSystem_BindShaderProvider( material, nullProvider ) );
		CheckViews( *material, nullServices );
		CHECK( !MaterialSystem_BindShaderProvider( material, actual ) );
		CHECK( !material->Connect( NULL ) );
		material->Disconnect();
		CheckViews( *material, render::LegacyShaderServices() );
		CHECK( MaterialSystem_BindShaderProvider( material, nullProvider ) );
		CheckViews( *material, nullServices );
		material->Disconnect();
		CheckViews( *material, render::LegacyShaderServices() );
	}
#if defined( USE_SDL3 )
	CHECK( SDL_WasInit( SDL_INIT_VIDEO ) == 0 );
#endif
}

} // namespace

int main()
{
	Run();
	std::printf( "CONFORMANCE %d %d\n", g_Checks, g_Failures );
	return g_Checks > 0 && g_Failures == 0 ? 0 : 1;
}
