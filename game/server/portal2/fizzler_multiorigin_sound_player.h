//========= Portal 2 reconstruction ============================================//
//
// Purpose: fizzler_multiorigin_sound_player: one looping fizzler sound shared
//          by every fizzler in the level. The client spatializes it from the
//          nearest fizzler instead of playing one loop per field.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Behavior follows the
// retail Portal 2 server's FizzlerMultiOriginSoundPlayer (the Portal 2
// trigger_portal_cleanser creates it with "VFX.FizzlerLp"). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
// Networked: DT_FizzlerMultiOriginSoundPlayer has no fields of its own
// (baseclass DT_BaseEntity). Server class name "FizzlerMultiOriginSoundPlayer";
// the retail client class is ClientFizzlerMultiOriginSoundPlayer. The entity
// is always transmitted.
//
//=============================================================================//

#ifndef FIZZLER_MULTIORIGIN_SOUND_PLAYER_H
#define FIZZLER_MULTIORIGIN_SOUND_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

class CSoundPatch;

class FizzlerMultiOriginSoundPlayer : public CBaseEntity
{
public:
	DECLARE_CLASS( FizzlerMultiOriginSoundPlayer, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	FizzlerMultiOriginSoundPlayer( void );
	virtual ~FizzlerMultiOriginSoundPlayer( void );

	// Creates, spawns and starts a player for the looping sound pSoundName
	static FizzlerMultiOriginSoundPlayer *Create( IRecipientFilter &filter, const char *pSoundName );

	virtual void	Spawn( void );
	virtual int		UpdateTransmitState( void );

	void			RemoveThink( void );

private:
	CSoundPatch		*m_pSound;
};

#endif // FIZZLER_MULTIORIGIN_SOUND_PLAYER_H
