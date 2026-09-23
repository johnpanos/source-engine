//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

// C callable material system interface for the utils.

#include "materialsystem/imaterialsystem.h"
#include "render/legacy_shader_provider.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include <cmdlib.h>
#include "utilmatlib.h"
#include "tier0/dbg.h"
#include "filesystem.h"
#include "materialsystem/materialsystem_config.h"
#include "mathlib/mathlib.h"
#include "vstdlib/cvar.h"

#include <cstring>

namespace
{
CreateInterfaceFn g_ToolFileSystemFactory;
bool g_ToolCvarConnected;

void *ToolMaterialFactory( const char *name, int *returnCode )
{
	if ( !std::strcmp( name, CVAR_INTERFACE_VERSION ) )
		return VStdLib_GetICVar();
	return g_ToolFileSystemFactory( name, returnCode );
}
} // namespace

void LoadMaterialSystemInterface( CreateInterfaceFn fileSystemFactory )
{
	if( g_pMaterialSystem )
		return;
	if ( !fileSystemFactory )
		Error( "The material tool requires a filesystem factory" );
	g_ToolFileSystemFactory = fileSystemFactory;
	if ( !VStdLib_GetICVar()->Connect( ToolMaterialFactory ) )
		Error( "The material tool could not connect the CVar registry" );
	g_ToolCvarConnected = true;

	// The material provider is installed beside the host compiler tool.
#ifdef _WIN32
	const char *pDllName = "materialsystem" DLL_EXT_STRING;
#else
	const char *pDllName = "libmaterialsystem" DLL_EXT_STRING;
#endif
	CSysModule *materialSystemDLLHInst;
	materialSystemDLLHInst = Sys_LoadModuleFromFileSystem(
		g_pFullFileSystem, pDllName );
	if( !materialSystemDLLHInst )
	{
		Error( "Can't load %s\n", pDllName );
	}

	CreateInterfaceFn clientFactory = Sys_GetFactory( materialSystemDLLHInst );
	if ( clientFactory )
	{
		g_pMaterialSystem = (IMaterialSystem *)clientFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, NULL );
		if ( !g_pMaterialSystem )
		{
			Error( "Could not get the material system interface from %s (" __FILE__ ")", pDllName );
		}
	}
	else
	{
		Error( "Could not find factory interface in library %s", pDllName );
	}

	if ( !MaterialSystem_BindShaderProvider( g_pMaterialSystem, NullShaderBackend_Describe() ) ||
	     !g_pMaterialSystem->Connect( ToolMaterialFactory ) ||
	     g_pMaterialSystem->Init() != INIT_OK )
	{
		Error( "Could not start the null material provider!" );
	}
	g_pMaterialSystem->SetMaterialProxyFactory( NULL );
}

void InitMaterialSystem( const char *materialBaseDirPath, CreateInterfaceFn fileSystemFactory )
{
	LoadMaterialSystemInterface( fileSystemFactory );
	MaterialSystem_Config_t config;
	g_pMaterialSystem->OverrideConfig( config, false );
}

void ShutdownMaterialSystem( )
{
	if ( g_pMaterialSystem )
	{
		g_pMaterialSystem->Shutdown();
		g_pMaterialSystem->Disconnect();
		g_pMaterialSystem = NULL;
		g_ToolFileSystemFactory = NULL;
	}
	if ( g_ToolCvarConnected )
	{
		VStdLib_GetICVar()->Disconnect();
		g_ToolCvarConnected = false;
	}
}

MaterialSystemMaterial_t FindMaterial( const char *materialName, bool *pFound, bool bComplain )
{
	IMaterial *pMat = g_pMaterialSystem->FindMaterial( materialName, TEXTURE_GROUP_OTHER, bComplain );
	MaterialSystemMaterial_t matHandle = pMat;
	
	if ( pFound )
	{
		*pFound = true;
		if ( IsErrorMaterial( pMat ) )
			*pFound = false;
	}

	return matHandle;
}

void GetMaterialDimensions( MaterialSystemMaterial_t materialHandle, int *width, int *height )
{
	PreviewImageRetVal_t retVal;
	ImageFormat dummyImageFormat;
	IMaterial *material = ( IMaterial * )materialHandle;
	bool translucent;
	retVal = material->GetPreviewImageProperties( width, height, &dummyImageFormat, &translucent );
	if (retVal != MATERIAL_PREVIEW_IMAGE_OK ) 
	{
#if 0
		if (retVal == MATERIAL_PREVIEW_IMAGE_BAD ) 
		{
			Error( "problem getting preview image for %s", 
				g_pMaterialSystem->GetMaterialName( materialInfo[matID].materialHandle ) );
		}
#else
		*width = 128;
		*height = 128;
#endif
	}
}

void GetMaterialReflectivity( MaterialSystemMaterial_t materialHandle, float *reflectivityVect )
{
	IMaterial *material = ( IMaterial * )materialHandle;
	const IMaterialVar *reflectivityVar;

	bool found;
	reflectivityVar = material->FindVar( "$reflectivity", &found, false );
	if( !found )
	{
		Vector tmp;
		material->GetReflectivity( tmp );
		VectorCopy( tmp.Base(), reflectivityVect );
	}
	else
	{
		reflectivityVar->GetVecValue( reflectivityVect, 3 );
	}
}

int GetMaterialShaderPropertyBool( MaterialSystemMaterial_t materialHandle, int propID )
{
	IMaterial *material = ( IMaterial * )materialHandle;
	switch( propID )
	{
	case UTILMATLIB_NEEDS_BUMPED_LIGHTMAPS:
		return material->GetPropertyFlag( MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS );

	case UTILMATLIB_NEEDS_LIGHTMAP:
		return material->GetPropertyFlag( MATERIAL_PROPERTY_NEEDS_LIGHTMAP );

	default:
		Assert( 0 );
		return 0;
	}
}

int GetMaterialShaderPropertyInt( MaterialSystemMaterial_t materialHandle, int propID )
{
	IMaterial *material = ( IMaterial * )materialHandle;
	switch( propID )
	{
	case UTILMATLIB_OPACITY:
		if (material->IsTranslucent())
			return UTILMATLIB_TRANSLUCENT;
		if (material->IsAlphaTested())
			return UTILMATLIB_ALPHATEST;
		return UTILMATLIB_OPAQUE;

	default:
		Assert( 0 );
		return 0;
	}
}

const char *GetMaterialVar( MaterialSystemMaterial_t materialHandle, const char *propertyName )
{
	IMaterial *material = ( IMaterial * )materialHandle;
	IMaterialVar *var;
	bool found;
	var = material->FindVar( propertyName, &found, false );
	if( found )
	{
		return var->GetStringValue();
	}
	else
	{
		return NULL;
	}
}

const char *GetMaterialShaderName( MaterialSystemMaterial_t materialHandle )
{
	IMaterial *material = ( IMaterial * )materialHandle;
	return material->GetShaderName();
}
