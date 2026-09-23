//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0007 PBRMetalRough compatibility registration. Until the
//          native material path implements this family, a material with
//          $fallbackmaterial follows CMaterial's fallback VMT loading path.
//          An absent fallback reaches Wireframe_DX9; content validation must
//          reject that case before a material reaches the runtime.
//
//===========================================================================//

#include "BaseVSShader.h"

DEFINE_FALLBACK_SHADER( PBRMetalRough, Wireframe_DX9 )
