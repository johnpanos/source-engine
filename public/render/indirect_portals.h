//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The client's open portal pairs, published to the engine every
//          frame for the indirect-light producers (RFC 0011 G10: light
//          through open portals). Plain data across the client/engine
//          boundary: no engine, game or standard-library types.
//
//          A portal is listed only while it is open: activated and linked
//          to an activated portal. Both portals of a pair are listed, each
//          with the transform to its partner.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_PORTALS_H
#define RENDER_INDIRECT_PORTALS_H

namespace indirect_portals
{

static const char *const kIndirectLightPortalsVersion = "VIndirectLightPortals001";
static const int kMaxOpenPortals = 8;

// One open portal, in Source units: its centre, the unit axes of its frame
// (forward out of the wall it is on), its half extents along right and up,
// and the row-major 3x4 transform that maps a point in front of it to the
// point the linked portal shows it at (behind the linked portal): a ray
// entering this portal continues from the linked one along the mapped
// direction.
struct PortalInput
{
	float origin[3];
	float forward[3];
	float right[3];
	float up[3];
	float halfWidth;
	float halfHeight;
	float toLinked[12];
};

class IIndirectLightPortals
{
public:
	// The frame's open portals (at most kMaxOpenPortals; more are dropped),
	// copied; `count` 0 clears them.
	virtual void SetOpenPortals( const PortalInput *portals, int count ) = 0;

protected:
	~IIndirectLightPortals() {}
};

} // namespace indirect_portals

#endif // RENDER_INDIRECT_PORTALS_H
