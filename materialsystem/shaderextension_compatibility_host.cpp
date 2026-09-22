//========= Copyright Valve Corporation, All rights reserved. ============//

#include "shaderextension_compatibility_host.h"
#include "filesystem.h"
#include "IShaderSystem.h"
#include <memory>

// This compatibility host alone negotiates the retained mod-shader ABI. GAMEBIN
// resolution and its existing validation policy belong to the filesystem bridge
// until the separate resolver migration. This is an in-process compatibility
// boundary, not a sandbox for untrusted native code.
struct CShaderExtensionCompatibilityHost::State
{
	IFileSystem *m_pFileSystem;
	CSysModule *m_pModule;
	IShaderDLLInternal *m_pShaders;
};

CShaderExtensionCompatibilityHost *CShaderExtensionCompatibilityHost::Load(
    IFileSystem &fileSystem, const char *pFileName )
{
	if ( !pFileName || !pFileName[0] )
		return NULL;

	// Allocate ownership before loading native code so later acquisition failures
	// unwind through this scope without leaking module handles or host borrowers.
	std::unique_ptr<CShaderExtensionCompatibilityHost> host(
	    new CShaderExtensionCompatibilityHost( fileSystem ) );
	State &state = *host->m_pState;
	state.m_pModule = Sys_LoadModuleFromFileSystem( &fileSystem, pFileName, "GAMEBIN", false );
	if ( !state.m_pModule )
		return NULL;

	CreateInterfaceFn factory = Sys_GetFactory( state.m_pModule );
	state.m_pShaders =
	    factory ? static_cast<IShaderDLLInternal *>( factory( SHADER_DLL_INTERFACE_VERSION, NULL ) )
	            : NULL;
	if ( !state.m_pShaders )
		return NULL;

	// Connect can register CVars and borrow host services before reporting
	// failure. The scope rolls back those effects while the module is resident.
	if ( !state.m_pShaders->Connect( Sys_GetFactoryThis(), false ) )
		return NULL;

	return host.release();
}

CShaderExtensionCompatibilityHost::CShaderExtensionCompatibilityHost( IFileSystem &fileSystem )
    : m_pState( new State )
{
	m_pState->m_pFileSystem = &fileSystem;
	m_pState->m_pModule = NULL;
	m_pState->m_pShaders = NULL;
}

CShaderExtensionCompatibilityHost::~CShaderExtensionCompatibilityHost()
{
	if ( m_pState->m_pShaders )
		m_pState->m_pShaders->Disconnect( false );
	if ( m_pState->m_pModule )
		m_pState->m_pFileSystem->UnloadModule( m_pState->m_pModule );
	delete m_pState;
}

IShaderDLLInternal &CShaderExtensionCompatibilityHost::Shaders() const
{
	return *m_pState->m_pShaders;
}
