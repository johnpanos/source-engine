//========= Copyright 1996-2009, Valve Corporation, All rights reserved. ============//
//
// Purpose: Matchmaking framework registry (see public/engine/imatchframeworkhost.h).
//          The engine only holds the pointer; the publishing game module owns
//          the framework and its lifetime.
//
//=============================================================================//

#include "engine/imatchframeworkhost.h"
#include "tier1/interface.h"
#include "tier0/dbg.h"

#include "tier0/memdbgon.h"

class CMatchFrameworkHost : public IMatchFrameworkHost
{
public:
	CMatchFrameworkHost() : m_pFramework( NULL )
	{
	}

	virtual bool PublishMatchFramework( IMatchFramework *pFramework )
	{
		Assert( pFramework );
		if ( !pFramework || ( m_pFramework && m_pFramework != pFramework ) )
			return false;

		m_pFramework = pFramework;
		return true;
	}

	virtual void WithdrawMatchFramework( IMatchFramework *pFramework )
	{
		Assert( pFramework == m_pFramework );
		if ( pFramework == m_pFramework )
			m_pFramework = NULL;
	}

	virtual IMatchFramework *GetMatchFramework()
	{
		return m_pFramework;
	}

private:
	IMatchFramework *m_pFramework;
};

EXPOSE_SINGLE_INTERFACE( CMatchFrameworkHost, IMatchFrameworkHost, VENGINE_MATCHFRAMEWORKHOST_INTERFACE_VERSION );
