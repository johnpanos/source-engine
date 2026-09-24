//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ====
//
// Purpose:
//
//
// Portal 2 port: from the CS:GO source drop; the repository's provenance and distribution warning applies.
// Retail Portal 2 has neither FetchEventData nor logic_eventlistener_itemequip
// (both CS:GO additions), so they are not ported.
//
//=============================================================================

#ifndef LOGIC_EVENTLISTENER_H
#define LOGIC_EVENTLISTENER_H
#pragma once

#include "GameEventListener.h"

//-----------------------------------------------------------------------------
// Purpose: Used to relay outputs/inputs from the events to the world and vice versa
//-----------------------------------------------------------------------------
class CLogicEventListener : public CLogicalEntity, public CGameEventListener
{
	DECLARE_CLASS( CLogicEventListener, CLogicalEntity );
	DECLARE_DATADESC();

public:
	virtual void Spawn( void );
	virtual void FireGameEvent( IGameEvent *event );
	
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

private:

	string_t	m_iszEventName;
	bool		m_bIsEnabled;
	int			m_nTeam;

	COutputEvent m_OnEventFired;

};
#endif	// LOGIC_EVENTLISTENER_H