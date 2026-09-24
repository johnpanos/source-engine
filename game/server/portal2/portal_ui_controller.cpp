//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server-side hook for Portal 2 UI state at level start.
//
// The UI transition behavior is deferred while the gameplay build is being
// restored. Not original Valve source; the provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_ui_controller.h"

#include "tier0/memdbgon.h"

CPortal_UI_Controller g_portal_ui_controller;

void CPortal_UI_Controller::OnLevelStart()
{
}
