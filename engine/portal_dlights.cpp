//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic lights through open portals (RFC 0016 K7); see
//			portal_dlights.h.
//
//=============================================================================//

#include "render_pch.h"
#include "portal_dlights.h"
#include "cl_main.h"
#include "client.h"
#include "gl_lightmap.h"
#include "indirect_light_host.h"
#include "r_efxextern.h"
#include "r_local.h"
#include "render/portal_lights.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar r_portal_dlights( "r_portal_dlights", "1", 0,
    "Dynamic lights shine through open portals (RFC 0016 K7): 1 on, 0 off." );
// Sensitivity fixture for tools/quality/portal_dlight_lab.py: images light
// without the portal clip (a leak the oracle must catch). Never set in play.
static ConVar r_portal_dlights_seed_noclip( "r_portal_dlights_seed_noclip", "0", FCVAR_CHEAT,
    "Test only: lights imaged through portals skip the portal clip (the leak the lab oracle must "
    "detect)." );
static ConVar r_portal_dlights_report( "r_portal_dlights_report", "0", 0,
    "Report the dynamic lights imaged through portals, and those dropped for want of a free "
    "dlight slot, whenever the counts change." );

namespace
{

// Keys of image slots; a slot whose key no longer matches was taken by
// another light and is no longer an image.
const int kImageKeyBase = 0x5043A000;

struct ImageSlot
{
	indirect_portals::PortalInput entry;
	float light[3];
};

ImageSlot s_Images[MAX_DLIGHTS];
unsigned int s_ImageMask = 0;
int s_LastImaged = -1;
int s_LastDropped = -1;

bool IsImage( int slot )
{
	return ( s_ImageMask & ( 1u << slot ) ) != 0 && cl_dlights[slot].key == kImageKeyBase + slot;
}

void Release()
{
	for ( int slot = 0; slot < MAX_DLIGHTS; ++slot )
	{
		if ( !IsImage( slot ) )
			continue;
		dlight_t &dl = cl_dlights[slot];
		dl.radius = 0.0f;
		dl.die = 0.0f;
		r_dlightchanged |= 1 << slot;
		r_dlightactive &= ~( 1 << slot );
		R_MarkDLightNotVisible( slot );
	}
	s_ImageMask = 0;
}

} // namespace

void PortalDLights_Update()
{
	Release();
	if ( !r_portal_dlights.GetBool() )
		return;
	indirect_portals::PortalInput portals[indirect_portals::kMaxOpenPortals];
	const int count = IndirectLight_OpenPortals( portals, indirect_portals::kMaxOpenPortals );
	if ( count == 0 )
		return;

	const float now = cl.GetTime();
	// Sources: the live dlights before any image is written.
	int sources[MAX_DLIGHTS];
	int sourceCount = 0;
	for ( int i = 0; i < MAX_DLIGHTS; ++i )
	{
		const dlight_t &dl = cl_dlights[i];
		if ( dl.die >= now && dl.IsRadiusGreaterThanZero() )
			sources[sourceCount++] = i;
	}
	int imaged = 0;
	int dropped = 0;
	int nextFree = 0;
	for ( int s = 0; s < sourceCount; ++s )
	{
		const dlight_t source = cl_dlights[sources[s]];
		const float origin[3] = { source.origin.x, source.origin.y, source.origin.z };
		for ( int p = 0; p < count; ++p )
		{
			if ( !portal_lights::EntersPortal( portals[p], origin, source.GetRadius() ) )
				continue;
			while ( nextFree < MAX_DLIGHTS && ( cl_dlights[nextFree].IsRadiusGreaterThanZero() ||
			                                      cl_dlights[nextFree].die >= now ) )
				++nextFree;
			if ( nextFree >= MAX_DLIGHTS )
			{
				++dropped;
				continue;
			}
			const int slot = nextFree++;
			dlight_t &image = cl_dlights[slot];
			image = source;
			float position[3], direction[3];
			const float sourceDirection[3] = {
			    source.m_Direction.x, source.m_Direction.y, source.m_Direction.z };
			portal_lights::ImagePoint( portals[p], origin, position );
			portal_lights::ImageVector( portals[p], sourceDirection, direction );
			image.origin.Init( position[0], position[1], position[2] );
			image.m_Direction.Init( direction[0], direction[1], direction[2] );
			image.key = kImageKeyBase + slot;
			image.decay = 0.0f;
			// Released at the next update; the margin keeps it alive until then.
			image.die = now + 1.0f;
			s_Images[slot].entry = portals[p];
			s_Images[slot].light[0] = origin[0];
			s_Images[slot].light[1] = origin[1];
			s_Images[slot].light[2] = origin[2];
			s_ImageMask |= 1u << slot;
			r_dlightchanged |= 1 << slot;
			r_dlightactive |= 1 << slot;
			g_bActiveDlights = true;
			++imaged;
		}
	}
	if ( r_portal_dlights_report.GetBool() &&
	     ( imaged != s_LastImaged || dropped != s_LastDropped ) )
		Msg( "portal dlights: %d imaged through %d open portal(s), %d dropped (no free slot)\n",
		    imaged, count, dropped );
	s_LastImaged = imaged;
	s_LastDropped = dropped;
}

unsigned int PortalDLights_ImageMask()
{
	unsigned int mask = 0;
	for ( int slot = 0; slot < MAX_DLIGHTS; ++slot )
		mask |= IsImage( slot ) ? 1u << slot : 0u;
	return mask;
}

bool PortalDLights_Reaches( int slot, const Vector &worldPoint )
{
	if ( slot < 0 || slot >= MAX_DLIGHTS || !IsImage( slot ) ||
	     r_portal_dlights_seed_noclip.GetBool() )
		return true;
	const float receiver[3] = { worldPoint.x, worldPoint.y, worldPoint.z };
	return portal_lights::ReachesThroughPortal(
	    s_Images[slot].entry, s_Images[slot].light, receiver );
}

//-----------------------------------------------------------------------------
// Test tooling (cheat): a persistent point dlight at a fixed place, so the
// portal light oracle (tools/quality/portal_dlights_check.py) has a
// deterministic source. `r_portal_dlight_test x y z radius r g b [exponent]`
// places it; `r_portal_dlight_test off` removes it.
//-----------------------------------------------------------------------------
CON_COMMAND_F( r_portal_dlight_test,
    "Place a test point dlight: x y z radius r g b [exponent], or off", FCVAR_CHEAT )
{
	const int kTestKey = 0x5043B000;
	for ( int i = 0; i < MAX_DLIGHTS; ++i )
	{
		if ( cl_dlights[i].key == kTestKey && cl_dlights[i].IsRadiusGreaterThanZero() )
		{
			cl_dlights[i].radius = 0.0f;
			cl_dlights[i].die = 0.0f;
		}
	}
	if ( args.ArgC() < 8 )
	{
		Msg( "portal dlights: test light off\n" );
		return;
	}
	dlight_t *dl = CL_AllocDlight( kTestKey );
	dl->origin.Init( atof( args[1] ), atof( args[2] ), atof( args[3] ) );
	dl->radius = atof( args[4] );
	dl->color.r = (byte)clamp( atoi( args[5] ), 0, 255 );
	dl->color.g = (byte)clamp( atoi( args[6] ), 0, 255 );
	dl->color.b = (byte)clamp( atoi( args[7] ), 0, 255 );
	dl->color.exponent = args.ArgC() > 8 ? (signed char)atoi( args[8] ) : 0;
	dl->die = FLT_MAX;
	Msg( "portal dlights: test light at %.1f %.1f %.1f radius %.1f\n", dl->origin.x, dl->origin.y,
	    dl->origin.z, dl->radius );
}
