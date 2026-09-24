//===== Copyright 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: Engine-owned registry for the process's matchmaking framework.
//          The CS:GO-era engine loaded the matchmaking module as an app system
//          so the client and server game modules shared one instance. This
//          engine does not; the game module that owns a framework publishes it
//          here and the other game modules borrow it.
//
//===========================================================================//

#ifndef IMATCHFRAMEWORKHOST_H
#define IMATCHFRAMEWORKHOST_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

class IMatchFramework;

abstract_class IMatchFrameworkHost
{
public:
	// Publishes an initialized framework. Fails if another one is published.
	// The owner must withdraw it before shutting it down.
	virtual bool PublishMatchFramework( IMatchFramework *pFramework ) = 0;

	// Withdraws the published framework; borrowers must have stopped using it.
	virtual void WithdrawMatchFramework( IMatchFramework *pFramework ) = 0;

	// The published framework, or NULL.
	virtual IMatchFramework *GetMatchFramework() = 0;
};

#define VENGINE_MATCHFRAMEWORKHOST_INTERFACE_VERSION "VEngineMatchFrameworkHost001"

#endif // IMATCHFRAMEWORKHOST_H
