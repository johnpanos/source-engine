//========= Portal 2 port ======================================================//
//
// Purpose: Frustum helper from the later (CS:GO-era) base game util_shared.h,
//          used by the Portal 2 client portal renderer and the server portal
//          visibility code.
//
//=============================================================================//

#ifndef PORTAL2_FRUSTUM_UTIL_H
#define PORTAL2_FRUSTUM_UTIL_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "mathlib/vplane.h"

// Builds the planes of the frustum from vFrustumOrigin through a convex
// polygon, first clipping the polygon by the iInputFrustumPlanes input planes.
// When the clipped polygon has more sides than the output can hold, its
// shortest sides are merged away. The last iPreserveCount input planes are
// appended to the output. Returns the number of output planes (0 when the
// polygon is clipped away).
int UTIL_CalcFrustumThroughConvexPolygon( const Vector *pPolyVertices, int iPolyVertCount, const Vector &vFrustumOrigin,
										  const VPlane *pInputFrustumPlanes, int iInputFrustumPlanes,
										  VPlane *pOutputFrustumPlanes, int iMaxOutputPlanes, int iPreserveCount );

#endif // PORTAL2_FRUSTUM_UTIL_H
