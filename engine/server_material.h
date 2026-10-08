//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Material definitions for the dedicated product (RFC 0001 R12),
//			which composes no material system. The engine's server paths and the
//			preserved IVModelInfo material queries (func_breakablesurf, server
//			mods) receive these IMaterials: a scoped compatibility bridge over
//			the material's VMT definition, read with the rules and device
//			profile the dedicated server's material system used
//			(materialsystem/vmt_definition.h). A definition draws nothing; its
//			rendering queries answer as an unshaded material does.
//
//=============================================================================//

#ifndef SERVER_MATERIAL_H
#define SERVER_MATERIAL_H

class IMaterial;
struct studiohdr_t;

// The definition of material `pName` (lowercased, '/' separators, extension
// removed, under materials/), created on first use and kept until
// ServerMaterial_Shutdown. A missing or invalid material returns the one
// error definition, whose IsErrorMaterial() is true. Never null.
IMaterial *ServerMaterial_Find( const char *pName );

// CStudioRender::GetMaterialList's rule for a studio model's materials over
// the definitions: each texture name under the model's texture directories,
// the first that loads (else the error definition), unique, at most count.
int ServerMaterial_GetStudioMaterialList(
    studiohdr_t *pStudioHdr, int count, IMaterial **ppMaterials );

// Points the engine's empty material (g_materialEmpty) at the definition of
// debug/debugempty, as InitMaterialSystem does with a material system.
void ServerMaterial_Init();

// Releases every definition.
void ServerMaterial_Shutdown();

#endif // SERVER_MATERIAL_H
