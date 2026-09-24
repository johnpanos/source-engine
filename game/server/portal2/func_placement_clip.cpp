//========= Portal 2 reconstruction ============================================//
//
// Purpose: Brush volume that is solid only to portal placement traces
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CFuncPlacementClip : public CBaseEntity
{
	DECLARE_CLASS( CFuncPlacementClip, CBaseEntity );
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual bool CreateVPhysics( void );

	void InputEnable( inputdata_t &data );
	void InputDisable( inputdata_t &data );
};

BEGIN_DATADESC( CFuncPlacementClip )
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_placement_clip, CFuncPlacementClip );


void CFuncPlacementClip::Spawn( void )
{
	SetLocalAngles( vec3_angle );
	SetMoveType( MOVETYPE_PUSH );
	SetModel( STRING( GetModelName() ) );

	AddFlag( FL_WORLDBRUSH );

	CreateVPhysics();

	AddEffects( EF_NODRAW );

	// Only the portal placement trace collides with this group
	SetCollisionGroup( COLLISION_GROUP_PLACEMENT_SOLID );
}

bool CFuncPlacementClip::CreateVPhysics( void )
{
	SetSolid( SOLID_BSP );
	VPhysicsInitStatic();
	return true;
}

void CFuncPlacementClip::InputEnable( inputdata_t &data )
{
	IPhysicsObject *pPhys = VPhysicsGetObject();
	if ( pPhys )
	{
		pPhys->EnableCollisions( true );
	}

	RemoveSolidFlags( FSOLID_NOT_SOLID );
}

void CFuncPlacementClip::InputDisable( inputdata_t &data )
{
	IPhysicsObject *pPhys = VPhysicsGetObject();
	if ( pPhys )
	{
		pPhys->EnableCollisions( false );
	}

	AddSolidFlags( FSOLID_NOT_SOLID );
}
