//========= Portal 2 reconstruction ============================================//
//
// Purpose: Optional multiplayer statistics hook for the server.
//
// Stats persistence and reporting are deferred to restore the gameplay build.
// Not original Valve source; the provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_mp_stats.h"

#include "tier0/memdbgon.h"

static CPortalMPStats s_PortalMPStats;

CPortalMPStats *GetPortalMPStats()
{
	return &s_PortalMPStats;
}
