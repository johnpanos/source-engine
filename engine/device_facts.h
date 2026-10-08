//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The device facts the engine's map loading reads (RFC 0001 R12): the
//			DX support level, whether HDR is enabled and whether the device has
//			vertex and pixel shaders. A client answers from the material
//			system's hardware config. The dedicated product (SWDS) composes no
//			material system and answers with the values the empty shader API
//			gave it: DX level 90, HDR always enabled (SetHDREnabled ignored),
//			shaders supported. So a dedicated server loads the same lumps.
//
//=============================================================================//

#ifndef DEVICE_FACTS_H
#define DEVICE_FACTS_H

int Engine_DXSupportLevel();
bool Engine_HDREnabled();
void Engine_SetHDREnabled( bool bEnable );
bool Engine_SupportsVertexAndPixelShaders();

#endif // DEVICE_FACTS_H
