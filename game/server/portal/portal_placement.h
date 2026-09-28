//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PORTAL_PLACEMENT_H
#define PORTAL_PLACEMENT_H
#ifdef _WIN32
#pragma once
#endif


#include "portal_shareddefs.h"

struct CPortalCornerFitData;

bool FitPortalOnSurface( const CProp_Portal *pIgnorePortal, Vector &vOrigin, const Vector &vForward, const Vector &vRight, 
						 const Vector &vTopEdge, const Vector &vBottomEdge, const Vector &vRightEdge, const Vector &vLeftEdge, 
						 int iPlacedBy, ITraceFilter *pTraceFilterPortalShot, 
						 int iRecursions = 0, const CPortalCornerFitData *pPortalCornerFitData = 0, const int *p_piIntersectionIndex = 0, const int *piIntersectionCount = 0,
						 float fHalfWidth = PORTAL_HALF_WIDTH, float fHalfHeight = PORTAL_HALF_HEIGHT );
// The portal half size defaults to the standard portal; F-Stop portals are resizable.
bool IsPortalIntersectingNoPortalVolume( const Vector &vOrigin, const QAngle &qAngles, const Vector &vForward, float fHalfWidth = PORTAL_HALF_WIDTH, float fHalfHeight = PORTAL_HALF_HEIGHT );
bool IsPortalOverlappingOtherPortals( const CProp_Portal *pIgnorePortal, const Vector &vOrigin, const QAngle &qAngles, bool bFizzle = false, float fHalfWidth = PORTAL_HALF_WIDTH, float fHalfHeight = PORTAL_HALF_HEIGHT );
float VerifyPortalPlacement( const CProp_Portal *pIgnorePortal, Vector &vOrigin, QAngle &qAngles, int iPlacedBy, bool bTest = false, float fHalfWidth = PORTAL_HALF_WIDTH, float fHalfHeight = PORTAL_HALF_HEIGHT );


#endif // PORTAL_PLACEMENT_H
