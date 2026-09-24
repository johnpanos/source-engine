//========= Portal 2 reconstruction ============================================//
//
// Purpose: Spatialize the shared fizzler loop at the nearest cleanser field.
//
// The retained retail client names ClientFizzlerMultiOriginSoundPlayer and
// DT_FizzlerMultiOriginSoundPlayer. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_FIZZLER_MULTIORIGIN_SOUND_PLAYER_H
#define C_FIZZLER_MULTIORIGIN_SOUND_PLAYER_H

#include "c_baseentity.h"

class ClientFizzlerMultiOriginSoundPlayer : public C_BaseEntity
{
public:
	DECLARE_CLASS( ClientFizzlerMultiOriginSoundPlayer, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	virtual bool GetSoundSpatialization( SpatializationInfo_t &info );
};

#endif // C_FIZZLER_MULTIORIGIN_SOUND_PLAYER_H
