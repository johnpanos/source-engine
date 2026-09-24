//========= Portal 2 reconstruction ============================================//
//
// Purpose: Optional playtest research telemetry hook.
//
//=============================================================================//

#ifndef PORTAL2_RESEARCH_DATA_TRACKER_H
#define PORTAL2_RESEARCH_DATA_TRACKER_H

class CPortal_Player;

class CPortal2ResearchDataTracker
{
public:
	void SetPlayerName( CPortal_Player *pPlayer );
	void Event_PlayerGaveUp();
	void IncrementDeath( CPortal_Player *pPlayer );
	void IncrementPortalFired( CPortal_Player *pPlayer );
	void IncrementStepsTaken( CPortal_Player *pPlayer );
};

extern CPortal2ResearchDataTracker g_Portal2ResearchDataTracker;

#endif // PORTAL2_RESEARCH_DATA_TRACKER_H
