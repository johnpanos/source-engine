//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef SHADEREXTENSION_COMPATIBILITY_HOST_H
#define SHADEREXTENSION_COMPATIBILITY_HOST_H

class IFileSystem;
class IShaderDLLInternal;

// Owns one retained ShaderDLL004 mod extension. All calls and shader uses run on
// the material-system sequence. The filesystem outlives this scope; every shader
// borrower must be released before destruction. Native handles and ABI factories
// remain private to the implementation.
class CShaderExtensionCompatibilityHost
{
public:
	static CShaderExtensionCompatibilityHost *Load(
	    IFileSystem &fileSystem, const char *pFileName );
	~CShaderExtensionCompatibilityHost();

	IShaderDLLInternal &Shaders() const;

private:
	struct State;
	explicit CShaderExtensionCompatibilityHost( IFileSystem &fileSystem );
	CShaderExtensionCompatibilityHost( const CShaderExtensionCompatibilityHost & );
	CShaderExtensionCompatibilityHost &operator=( const CShaderExtensionCompatibilityHost & );
	State *m_pState;
};

#endif
