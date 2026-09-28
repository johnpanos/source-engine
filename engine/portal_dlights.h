//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic lights through open portals (render.portal-lights.v1,
//			RFC 0016 K7 with RFC 0011 G10's portal set).
//
//			Once per frame, before the dlights are marked on the world,
//			PortalDLights_Update images every live dlight in front of an open
//			portal whose radius reaches the portal's rectangle through the
//			pair (render/portal_lights.h), into a free dlight slot: the same
//			light at its image behind the linked portal, lit only where the
//			path really passes through the portal. The consumers of dlight
//			slots clip image slots with PortalDLights_Reaches: world lightmaps
//			per luxel, models at their lighting origin. Paths that cannot clip
//			yet (displacements, the per-pixel light set) leave image slots out
//			(PortalDLights_ImageMask), so light never leaks behind a wall.
//
//			The previous frame's images are released first; images are never
//			imaged again (one hop). An image with no free slot is dropped and
//			counted (r_portal_dlights_report). r_portal_dlights 0 turns this
//			off.
//
//=============================================================================//

#ifndef ENGINE_PORTAL_DLIGHTS_H
#define ENGINE_PORTAL_DLIGHTS_H

class Vector;

void PortalDLights_Update();

// The dlight slots that hold images this frame.
unsigned int PortalDLights_ImageMask();

// Whether dlight slot `slot` lights a receiver at `worldPoint`: always for a
// slot that holds no image, and for an image only through its portal.
bool PortalDLights_Reaches( int slot, const Vector &worldPoint );

#endif // ENGINE_PORTAL_DLIGHTS_H
