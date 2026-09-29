//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Model loading / unloading interface
//
// $NoKeywords: $
//=============================================================================//

#ifndef OVERLAY_H
#define OVERLAY_H

#ifdef _WIN32
#pragma once
#endif


// This is a workaround for the fact that we get massive decal flicker
// when looking at a decal at a glancing angle while standing right next to it.
#define OVERLAY_AVOID_FLICKER_NORMAL_OFFSET	0.1f

class IMaterial;
class Vector;
class Vector2D;

//-----------------------------------------------------------------------------
// Overlay fragments
//-----------------------------------------------------------------------------
typedef unsigned short OverlayFragmentHandle_t;

enum
{
	OVERLAY_FRAGMENT_INVALID = (OverlayFragmentHandle_t)~0
};


//=============================================================================
//
// Overlay Manager Interface
//
class IOverlayMgr
{
public:
	// Memory allocation/de-allocation.
	virtual bool	LoadOverlays( ) = 0;
	virtual void	UnloadOverlays( ) = 0;

	virtual void	CreateFragments( void ) = 0;

	virtual void	ReSortMaterials( void ) = 0;

	// Drawing
	// clears all
	virtual void	ClearRenderLists() = 0;
	// clears a particular sort group
	virtual void	ClearRenderLists( int nSortGroup ) = 0;
	virtual void	AddFragmentListToRenderList( int nSortGroup, OverlayFragmentHandle_t iFragment, bool bDisp ) = 0;
	virtual void	RenderOverlays( int nSortGroup ) = 0;

	// Sets the client renderable for an overlay's material proxy to bind to
	virtual void	SetOverlayBindProxy( int iOverlayID, void *pBindProxy ) = 0;

	// RFC 0011 area lights (world_emitters.cpp): each overlay's fragments, as
	// polygons in world space with their mapped texture coordinates and the
	// normal of the surface they lie on.
	typedef void ( *FragmentVisitor_t )( void *pContext, int iOverlay, IMaterial *pMaterial,
	    const Vector &normal, const Vector *pPositions, const Vector2D *pTexCoords, int nCount );
	virtual void EnumerateFragments( FragmentVisitor_t visit, void *pContext ) = 0;
};


//-----------------------------------------------------------------------------
// Overlay manager singleton
//-----------------------------------------------------------------------------
IOverlayMgr *OverlayMgr();

#endif // OVERLAY_H
