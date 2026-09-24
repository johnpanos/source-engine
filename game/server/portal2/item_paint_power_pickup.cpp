//========= Portal 2 reconstruction ============================================//
//
// Purpose: Pickup that grants a paint power to the player's paint gun.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "items.h"
#include "player.h"
#include "weapon_paintgun.h"
#include "paint_color_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: Tell every client that pPlayer's active weapon changed paint color.
//-----------------------------------------------------------------------------
void PaintPowerPickup( int colorIndex, CBasePlayer *pPlayer )
{
	if ( pPlayer && pPlayer->GetActiveWeapon() )
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

		CBroadcastRecipientFilter filter;
		filter.MakeReliable();
		UserMessageBegin( filter, "ChangePaintColor" );
			WRITE_EHANDLE( pWeapon );
			WRITE_BYTE( colorIndex );
		MessageEnd();
	}
}


class CPaintPowerPickup : public CItem
{
public:
	DECLARE_CLASS( CPaintPowerPickup, CItem );
	DECLARE_DATADESC();

	CPaintPowerPickup() : m_PaintPowerType( NO_POWER ) {}

	virtual void Spawn();
	virtual void Precache();
	virtual void Touch( CBaseEntity *pOther );

private:
	int m_PaintPowerType;
};


BEGIN_DATADESC( CPaintPowerPickup )
	DEFINE_KEYFIELD( m_PaintPowerType, FIELD_INTEGER, "PaintType" ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( item_paint_power_pickup, CPaintPowerPickup );


void CPaintPowerPickup::Spawn()
{
	Precache();
	SetModel( "models/items/healthkit.mdl" );

	Color renderColor = MapPowerToVisualColor( m_PaintPowerType );
	SetRenderColor( renderColor.r(), renderColor.g(), renderColor.b() );

	BaseClass::Spawn();

	// Reconstruction note: the 2010 binary sets bit 31 of m_fFlags. That build's
	// CPropButton::Spawn sets the same bit where the imported prop_button.cpp uses
	// FL_UNPAINTABLE, and CPaintDatabase::AddPaint skips entities with it.
	AddFlag( FL_UNPAINTABLE );

	// The pickup is a trigger only; it has no physics object.
	VPhysicsDestroyObject();
}


void CPaintPowerPickup::Precache()
{
	PrecacheModel( "models/items/healthkit.mdl" );
}


void CPaintPowerPickup::Touch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( pOther );
	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( pWeapon && FClassnameIs( pWeapon, "weapon_paintgun" ) )
	{
		CWeaponPaintGun *pPaintGun = dynamic_cast< CWeaponPaintGun* >( pWeapon );
		if ( pPaintGun && !pPaintGun->HasPaintPower( (PaintPowerType)m_PaintPowerType ) )
		{
			pPaintGun->ActivatePaint( (PaintPowerType)m_PaintPowerType );
			pPaintGun->EmitSound( "BaseCombatWeapon.WeaponMaterialize" );
			PaintPowerPickup( m_PaintPowerType, pPlayer );
		}
	}
}
