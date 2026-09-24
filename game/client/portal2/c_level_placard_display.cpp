//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the level placard sign entity.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_LevelPlacardDisplay : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_LevelPlacardDisplay, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_LevelPlacardDisplay();
	virtual ~C_LevelPlacardDisplay();

	bool IsEnabled( void ) const { return m_bEnabled; }

private:
	bool	m_bEnabled;
};

IMPLEMENT_CLIENTCLASS_DT( C_LevelPlacardDisplay, DT_LevelPlacardDisplay, CLevelPlacardDisplay )
	RecvPropBool( RECVINFO( m_bEnabled ) ),
END_RECV_TABLE()

C_LevelPlacardDisplay::C_LevelPlacardDisplay()
{
}

C_LevelPlacardDisplay::~C_LevelPlacardDisplay()
{
}
