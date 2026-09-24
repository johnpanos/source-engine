//========= Portal 2 reconstruction ============================================//
//
// Purpose: Optional playtest research telemetry hook.
//
// CSV research reporting is deferred while restoring the game build. These
// entry points have no gameplay outputs in the retained callers. Not original
// Valve source; the repository's provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_research_data_tracker.h"

#include "tier0/memdbgon.h"

CPortal2ResearchDataTracker g_Portal2ResearchDataTracker;

void CPortal2ResearchDataTracker::SetPlayerName( CPortal_Player *pPlayer )
{
	(void)pPlayer;
}

void CPortal2ResearchDataTracker::Event_PlayerGaveUp()
{
}

void CPortal2ResearchDataTracker::IncrementDeath( CPortal_Player *pPlayer )
{
	(void)pPlayer;
}

void CPortal2ResearchDataTracker::IncrementPortalFired( CPortal_Player *pPlayer )
{
	(void)pPlayer;
}

void CPortal2ResearchDataTracker::IncrementStepsTaken( CPortal_Player *pPlayer )
{
	(void)pPlayer;
}
