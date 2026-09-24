//========= Portal 2 reconstruction ============================================//
//
// Purpose: fizzler_multiorigin_sound_player: one looping fizzler sound shared
//          by every fizzler in the level.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Behavior follows the
// retail Portal 2 server's FizzlerMultiOriginSoundPlayer. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "fizzler_multiorigin_sound_player.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( FizzlerMultiOriginSoundPlayer )

	DEFINE_SOUNDPATCH( m_pSound ),

	DEFINE_THINKFUNC( RemoveThink ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( FizzlerMultiOriginSoundPlayer, DT_FizzlerMultiOriginSoundPlayer )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( fizzler_multiorigin_sound_player, FizzlerMultiOriginSoundPlayer );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
FizzlerMultiOriginSoundPlayer::FizzlerMultiOriginSoundPlayer( void )
{
	m_pSound = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
FizzlerMultiOriginSoundPlayer::~FizzlerMultiOriginSoundPlayer( void )
{
	if ( m_pSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.Shutdown( m_pSound );
		controller.SoundDestroy( m_pSound );
		m_pSound = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
FizzlerMultiOriginSoundPlayer *FizzlerMultiOriginSoundPlayer::Create( IRecipientFilter &filter, const char *pSoundName )
{
	FizzlerMultiOriginSoundPlayer *pPlayer = static_cast< FizzlerMultiOriginSoundPlayer * >( CreateEntityByName( "fizzler_multiorigin_sound_player" ) );
	if ( pPlayer )
	{
		DispatchSpawn( pPlayer );

		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		pPlayer->m_pSound = controller.SoundCreate( filter, pPlayer->entindex(), pSoundName );
		controller.Play( pPlayer->m_pSound, 1.0f, 100.0f );
	}

	return pPlayer;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void FizzlerMultiOriginSoundPlayer::Spawn( void )
{
	BaseClass::Spawn();

	SetThink( &FizzlerMultiOriginSoundPlayer::RemoveThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Every client plays the loop, wherever the fizzlers are
//-----------------------------------------------------------------------------
int FizzlerMultiOriginSoundPlayer::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Removes the shared loop once the last fizzler is gone
//-----------------------------------------------------------------------------
void FizzlerMultiOriginSoundPlayer::RemoveThink( void )
{
	// Portal 2 port: retail tests the Portal 2 cleanser's auto list count. This
	// tree builds the Portal 1 trigger_portal_cleanser, which has no auto list,
	// so look the fizzlers up by class name instead.
	if ( gEntList.FindEntityByClassname( NULL, "trigger_portal_cleanser" ) == NULL )
	{
		UTIL_Remove( this );
		return;
	}

	SetNextThink( gpGlobals->curtime );
}
