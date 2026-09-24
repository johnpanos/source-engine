//========= Portal 2 reconstruction ============================================//
//
// Purpose: Hard light bridge emitter (prop_wall_projector).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "propwallprojector.h"
#include "projectedwallentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( prop_wall_projector, CPropWallProjector );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWallProjector::Spawn( void )
{
	BaseClass::Spawn();

	Precache();

	SetModel( "models/props/wall_emitter.mdl" );
	SetSolid( SOLID_VPHYSICS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropWallProjector::Precache( void )
{
	PrecacheModel( "models/props/wall_emitter.mdl" );
}

//-----------------------------------------------------------------------------
// Purpose: Wall projectors project light bridges
//-----------------------------------------------------------------------------
CBaseProjectedEntity *CPropWallProjector::CreateNewProjectedEntity( void )
{
	return CProjectedWallEntity::CreateNewInstance();
}
