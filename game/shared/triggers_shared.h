//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared trigger definitions.
//
//=============================================================================//

#ifndef TRIGGERS_SHARED_H
#define TRIGGERS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// Shared trigger definitions go here.

// Spawnflags for CTriggerPlayerMovement
const int SF_TRIGGER_MOVE_AUTODISABLE				= 0x0080;	// Disable auto movement
const int SF_TRIGGER_AUTO_DUCK						= 0x0800;	// Duck automatically
const int SF_TRIGGER_AUTO_WALK						= 0x1000;	// Walk automatically (Portal 2)

#endif // TRIGGERS_SHARED_H
