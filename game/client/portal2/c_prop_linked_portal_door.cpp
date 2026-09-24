//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the animated linked portal door
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_PropLinkedPortalDoor : public C_BaseAnimating, public CSignifierTarget
{
public:
	DECLARE_CLASS( C_PropLinkedPortalDoor, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

	// The coop ping signifier sits at the center of the door, facing out of it
	virtual bool OverrideSignifierPosition( void ) { return true; }
	virtual bool GetSignifierPosition( const Vector &vSource, Vector &vPositionOut, Vector &vNormalOut )
	{
		vPositionOut = WorldSpaceCenter();
		GetVectors( &vNormalOut, NULL, NULL );
		return true;
	}
};

IMPLEMENT_CLIENTCLASS_DT( C_PropLinkedPortalDoor, DT_PropLinkedPortalDoor, CPropLinkedPortalDoor )
END_RECV_TABLE()
