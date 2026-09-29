//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: env_projectedtexture as a light of the light model
//          (projected_lights.h).
//
//=============================================================================//
#include "cbase.h"
#include "projected_lights.h"

#include "cdll_client_int.h"
#include "igamesystem.h"

#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

struct Submission_t
{
	int m_nFrame;
	int m_nKey;
	projected_light::Light m_Light;
};

std::vector<Submission_t> s_Submissions;

class CProjectedLights : public CAutoGameSystemPerFrame
{
public:
	CProjectedLights()
	    : CAutoGameSystemPerFrame( "CProjectedLights" ), m_nFrame( -1 ), m_bPublished( false )
	{
	}

	virtual void LevelShutdownPostEntity()
	{
		s_Submissions.clear();
		m_bPublished = false;
	}

	// Once per frame, after the entities have simulated (and submitted).
	virtual void PreRender()
	{
		if ( m_nFrame == gpGlobals->framecount )
			return;
		m_nFrame = gpGlobals->framecount;
		if ( !projectedlights )
			return;
		projected_light::Light lights[projected_light::kMaxProjectedLights];
		int keys[projected_light::kMaxProjectedLights];
		int nCount = 0;
		for ( const Submission_t &submission : s_Submissions )
		{
			if ( submission.m_nFrame != gpGlobals->framecount ||
			     nCount == projected_light::kMaxProjectedLights )
				continue;
			lights[nCount] = submission.m_Light;
			keys[nCount] = submission.m_nKey;
			++nCount;
		}
		s_Submissions.clear();
		if ( nCount == 0 && !m_bPublished )
			return;
		projectedlights->SetProjectedLights( lights, keys, nCount );
		m_bPublished = nCount > 0;
	}

private:
	int m_nFrame;
	bool m_bPublished;
};

CProjectedLights s_ProjectedLights;

} // namespace

bool ProjectedLights_Active()
{
	static ConVarRef r_projected_lights( "r_projected_lights" );
	return projectedlights != NULL && r_projected_lights.IsValid() && r_projected_lights.GetBool();
}

void ProjectedLights_Submit( int nKey, const projected_light::Light &light )
{
	for ( Submission_t &submission : s_Submissions )
	{
		if ( submission.m_nKey == nKey )
		{
			submission.m_nFrame = gpGlobals->framecount;
			submission.m_Light = light;
			return;
		}
	}
	s_Submissions.push_back( Submission_t{ gpGlobals->framecount, nKey, light } );
}
