//========= Portal 2 reconstruction ============================================//
//
// Purpose: Receive the cooperative versus score entity's published state.
//
// The retail Linux client names DT_InfoPortalScore and its seven score/time
// fields plus the game description. Disassembly of the receive-table setup
// establishes the order and a 260-byte description buffer. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_InfoPortalScore : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_InfoPortalScore, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	int GetBlueScore() const { return m_nBlueScore; }
	int GetBlueRounds() const { return m_nBlueRounds; }
	int GetOrangeScore() const { return m_nOrangeScore; }
	int GetOrangeRounds() const { return m_nOrangeRounds; }
	int GetRoundsToWin() const { return m_nRoundsToWin; }
	float GetTotalTime() const { return m_flTotalTime; }
	const char *GetGameDescription() const { return m_strGameDescription; }

private:
	int m_nBlueScore = 0;
	int m_nBlueRounds = 0;
	int m_nOrangeScore = 0;
	int m_nOrangeRounds = 0;
	int m_nRoundsToWin = 0;
	float m_flTotalTime = 0.0f;
	char m_strGameDescription[260] = {};
};

// Source receive-table macros require their declaration layout.
// clang-format off
IMPLEMENT_CLIENTCLASS_DT( C_InfoPortalScore, DT_InfoPortalScore, CInfoPortalScore )
	RecvPropInt( RECVINFO( m_nBlueScore ) ),
	RecvPropInt( RECVINFO( m_nBlueRounds ) ),
	RecvPropInt( RECVINFO( m_nOrangeScore ) ),
	RecvPropInt( RECVINFO( m_nOrangeRounds ) ),
	RecvPropInt( RECVINFO( m_nRoundsToWin ) ),
	RecvPropTime( RECVINFO( m_flTotalTime ) ),
	RecvPropString( RECVINFO( m_strGameDescription ) ),
END_RECV_TABLE()
    // clang-format on
