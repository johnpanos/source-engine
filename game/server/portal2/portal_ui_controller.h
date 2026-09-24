//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server-side hook for Portal 2 UI state at level start.
//
//=============================================================================//

#ifndef PORTAL_UI_CONTROLLER_H
#define PORTAL_UI_CONTROLLER_H

class CPortal_UI_Controller
{
public:
	void OnLevelStart();
};

extern CPortal_UI_Controller g_portal_ui_controller;

#endif // PORTAL_UI_CONTROLLER_H
