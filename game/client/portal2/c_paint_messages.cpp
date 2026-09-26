//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client handlers for the paint user messages sent by the server
//			paint database (game/server/portal2/paint_database.cpp).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "hud_macros.h"
#include "usermessages.h"
#include "igamesystem.h"
#include "paint_color_manager.h"
#include "paintable_entity.h"
#include "portal_util_shared.h"
#include "c_weapon_paintgun.h"
#include "portal2_engine_compat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Reconstruction note: the 2010 UTIL_PaintBrushEntity( pBrushEntity, contactPoint, power )
// read its sphere radius and coat alpha itself; the imported helper takes both as
// arguments. These replicate the server paint database's settings of the same names.
static ConVar sv_paint_surface_sphere_radius( "sv_paint_surface_sphere_radius", "60.f", FCVAR_REPLICATED, "Radius of query sphere to paint all intersecting surfaces." );
static ConVar sv_erase_surface_sphere_radius( "sv_erase_surface_sphere_radius", "80.f", FCVAR_REPLICATED, "Radius of query sphere to erase all intersecting surfaces." );
static ConVar sv_paint_alpha_coat( "sv_paint_alpha_coat", "0.4f", FCVAR_REPLICATED | FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: Paint a group of world locations on one brush entity with one power.
//			The positions are sent as short offsets from their center.
//-----------------------------------------------------------------------------
void __MsgFunc_PaintWorld( bf_read &msg )
{
	// the listen server already painted the shared paintmaps
	if ( Portal2Engine::IsClientLocalToActiveServer() )
		return;

	{
		PaintPowerType power = static_cast< PaintPowerType >( msg.ReadByte() );
		C_BaseEntity *pBrushEntity = Portal2Engine::EntityFromUserMessageEHandle( msg.ReadLong() );

		int nPaintCount = msg.ReadByte();

		Vector vCenter;
		vCenter.x = msg.ReadFloat();
		vCenter.y = msg.ReadFloat();
		vCenter.z = msg.ReadFloat();

		const float flPaintRadius = ( power == NO_POWER ) ? sv_erase_surface_sphere_radius.GetFloat() : sv_paint_surface_sphere_radius.GetFloat();

		Vector vContactPoint;
		for ( int i = 0; i < nPaintCount; ++i )
		{
			vContactPoint.x = vCenter.x + msg.ReadShort();
			vContactPoint.y = vCenter.y + msg.ReadShort();
			vContactPoint.z = vCenter.z + msg.ReadShort();

			UTIL_PaintBrushEntity( pBrushEntity, vContactPoint, power, flPaintRadius, sv_paint_alpha_coat.GetFloat() );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Paint a (non-brush) paintable entity
//-----------------------------------------------------------------------------
void __MsgFunc_PaintEntity( bf_read &msg )
{
	C_BaseEntity *pEntity = Portal2Engine::EntityFromUserMessageEHandle( msg.ReadLong() );
	IPaintableEntity *pPaintableEnt = pEntity ? dynamic_cast< IPaintableEntity* >( pEntity ) : NULL;
	const PaintPowerType power = static_cast< PaintPowerType >( msg.ReadByte() );

	Vector pos;
	pos.x = msg.ReadFloat();
	pos.y = msg.ReadFloat();
	pos.z = msg.ReadFloat();

	// Reconstruction note: the binary calls Paint() without checking the cast.
	if ( pPaintableEnt )
	{
		pPaintableEnt->Paint( power, pos );
	}
}


//-----------------------------------------------------------------------------
// Purpose: The server changed the power of a paint gun
//-----------------------------------------------------------------------------
void __MsgFunc_ChangePaintColor( bf_read &msg )
{
	C_BaseEntity *pEntity = Portal2Engine::EntityFromUserMessageEHandle( msg.ReadLong() );
	if ( !pEntity )
	{
		DevMsg( "Failed to change paint color" );
		return;
	}

	C_WeaponPaintGun *pPaintGun = dynamic_cast< C_WeaponPaintGun* >( pEntity );
	if ( pPaintGun )
	{
		int power = msg.ReadByte();
		pPaintGun->SetSubType( power );
		pPaintGun->ChangeRenderColor( true );
	}
}


void __MsgFunc_RemoveAllPaint( bf_read &msg )
{
	// the listen server already cleared the shared paintmaps
	if ( Portal2Engine::IsClientLocalToActiveServer() )
		return;

	Portal2Engine::RemoveAllPaint();
}


void __MsgFunc_RemovePaint( bf_read &msg )
{
	C_BaseEntity *pEntity = Portal2Engine::EntityFromUserMessageEHandle( msg.ReadLong() );

	// the listen server already cleared the shared paintmaps
	if ( Portal2Engine::IsClientLocalToActiveServer() || !pEntity || !pEntity->IsBSPModel() )
		return;

	Portal2Engine::RemovePaint( pEntity->GetModel() );
}


void __MsgFunc_PaintAllSurfaces( bf_read &msg )
{
	// the listen server already painted the shared paintmaps
	if ( Portal2Engine::IsClientLocalToActiveServer() )
		return;

	{
		uint8 power = msg.ReadByte();
		Portal2Engine::PaintAllSurfaces( power );
	}
}


//-----------------------------------------------------------------------------
// Purpose: A chunk of the engine's paint records sent to a joining client
//			(total dwords, offset, count, dwords); loaded after the last one.
//-----------------------------------------------------------------------------
static CUtlVector< uint32 > s_PaintmapRecords;

void __MsgFunc_LoadPaintmapData( bf_read &msg )
{
	int nTotal = msg.ReadLong();
	int nOffset = msg.ReadLong();
	int nCount = msg.ReadByte();

	if ( nOffset == 0 )
	{
		s_PaintmapRecords.RemoveAll();
	}
	if ( nTotal < 0 || nOffset != s_PaintmapRecords.Count() || nOffset + nCount > nTotal )
	{
		DevWarning( "Paint: out of order paint records ignored\n" );
		s_PaintmapRecords.RemoveAll();
		return;
	}
	for ( int i = 0; i < nCount; ++i )
	{
		s_PaintmapRecords.AddToTail( ( uint32 )msg.ReadLong() );
	}

	// The listen server shares its paint maps with its own client.
	if ( s_PaintmapRecords.Count() == nTotal && !Portal2Engine::IsClientLocalToActiveServer() )
	{
		Portal2Engine::LoadPaintmapDataRLE( s_PaintmapRecords );
	}
	if ( s_PaintmapRecords.Count() == nTotal )
	{
		s_PaintmapRecords.Purge();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Hooks the paint messages for every split screen player
//-----------------------------------------------------------------------------
class C_PaintInitHelper : public CAutoGameSystem
{
	virtual bool Init()
	{
		for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
		{
			ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );

			HOOK_MESSAGE( PaintWorld );
			HOOK_MESSAGE( PaintEntity );
			HOOK_MESSAGE( ChangePaintColor );
			HOOK_MESSAGE( RemoveAllPaint );
			HOOK_MESSAGE( RemovePaint );
			HOOK_MESSAGE( PaintAllSurfaces );
			HOOK_MESSAGE( LoadPaintmapData );
		}

		return true;
	}
};

static C_PaintInitHelper s_PaintInitHelper;
