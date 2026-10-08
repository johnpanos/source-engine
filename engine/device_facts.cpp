//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device facts the engine's map loading reads (RFC 0001 R12); see
//			device_facts.h.
//
//=============================================================================//

#include "device_facts.h"

#ifndef SWDS
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "tier2/tier2.h"
#else
#include "../materialsystem/vmt_definition.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef SWDS

int Engine_DXSupportLevel()
{
	return g_pMaterialSystemHardwareConfig->GetDXSupportLevel();
}

bool Engine_HDREnabled()
{
	return g_pMaterialSystemHardwareConfig->GetHDREnabled();
}

void Engine_SetHDREnabled( bool bEnable )
{
	g_pMaterialSystemHardwareConfig->SetHDREnabled( bEnable );
}

bool Engine_SupportsVertexAndPixelShaders()
{
	return g_pMaterialSystemHardwareConfig &&
	       g_pMaterialSystemHardwareConfig->SupportsVertexAndPixelShaders();
}

#else

int Engine_DXSupportLevel()
{
	return VmtDedicatedServerProfile().dxSupportLevel;
}

bool Engine_HDREnabled()
{
	return true;
}

void Engine_SetHDREnabled( bool )
{
}

bool Engine_SupportsVertexAndPixelShaders()
{
	return true;
}

#endif
