//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal mod render targets are specified by and accessable through this singleton
//
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#ifndef PORTALRENDERTARGETS_H_
#define PORTALRENDERTARGETS_H_
#ifdef _WIN32
#pragma once
#endif

#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render targets

#ifndef PORTAL 
#pragma message ( "This file should only be built with portal builds" )
#endif

// externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CPortalRenderTargets : public CBaseClientRenderTargets
{
	// no networked vars
	DECLARE_CLASS_GAMEROOT( CPortalRenderTargets, CBaseClientRenderTargets );
public:
	virtual void InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig );
	virtual void ShutdownClientRenderTargets();

	ITexture* GetPortal1Texture( void );
	ITexture* GetPortal2Texture( void );
	ITexture* GetDepthDoublerTexture( void );

	// Texture portals (devices without a stencil buffer): each
	// portal's view rendered with the portal as its image plane, two
	// textures per portal (the one shown and the one drawn next, so a portal
	// seen through a portal shows the previous view). Null when not made.
	enum { PORTAL_PLANE_WIDTH = 128, PORTAL_PLANE_HEIGHT = 256 };
	ITexture* GetPortalPlaneTexture( int nPortal, int nSlot );

	//recursive views require different water textures

private:
	CTextureReference m_Portal1Texture;
	CTextureReference m_Portal2Texture;
	CTextureReference m_DepthDoublerTexture;
	CTextureReference m_PortalPlaneTextures[2][2];


	ITexture* InitPortal1Texture ( IMaterialSystem* pMaterialSystem );
	ITexture* InitPortal2Texture ( IMaterialSystem* pMaterialSystem );
	ITexture* InitDepthDoublerTexture ( IMaterialSystem* pMaterialSystem );

	void InitPortalWaterTextures ( IMaterialSystem* pMaterialSystem );

};

extern CPortalRenderTargets* portalrendertargets;

const char *GetSubTargetNameForPortalRecursionLevel( int iRecursionLevel );

#endif //PORTALRENDERTARGETS_H_