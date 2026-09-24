//========= Portal 2 reconstruction ============================================//
//
// Purpose: Multiplayer point nugget pickup and the director that randomizes
//			which nuggets are available.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "recipientfilter.h"
#include "team.h"
#include "UtlSortVector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define NUGGET_MODEL	"models/effects/cappoint_hologram.mdl"

// Model skins for each nugget value
enum Nugget_Values_t
{
	NUGGET_VALUE_1 = 1,
	NUGGET_VALUE_5,
	NUGGET_VALUE_10,
	NUGGET_VALUE_25,
};


class CItem_Nugget : public CBaseAnimating
{
public:
	DECLARE_CLASS( CItem_Nugget, CBaseAnimating );
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void StartTouch( CBaseEntity *pOther );
	virtual CBaseEntity *Respawn( void );
	virtual void UpdateOnRemove( void );
	void MaterializeThink( void );

	float m_fRespawnTime;
	int m_nValue;
	string_t m_iGroupName;
};

BEGIN_DATADESC( CItem_Nugget )

	DEFINE_KEYFIELD( m_iGroupName, FIELD_STRING, "GroupName" ),
	DEFINE_KEYFIELD( m_fRespawnTime, FIELD_FLOAT, "RespawnTime" ),
	DEFINE_KEYFIELD( m_nValue, FIELD_INTEGER, "PointValue" ),

	DEFINE_THINKFUNC( MaterializeThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( item_nugget, CItem_Nugget );


//-----------------------------------------------------------------------------
// Purpose: Tracks every nugget so availability can be randomized per group
//-----------------------------------------------------------------------------
class CNuggetDirector : public CAutoGameSystem
{
public:
	void RegisterNugget( CItem_Nugget *pNugget );
	void UnregisterNugget( CItem_Nugget *pNugget );
	void RandomizeAvailableNuggets( float fUngroupedAvailabilityChance, float fGroupAvailabilityChance );
	virtual void LevelInitPostEntity( void );

	// Keeps nuggets of the same group adjacent
	class CNuggetGroupSort
	{
	public:
		bool Less( const CItem_Nugget *pLeft, const CItem_Nugget *pRight, void *pCtx )
		{
			return pLeft->m_iGroupName < pRight->m_iGroupName;
		}
	};

	CUtlVector<CItem_Nugget*> m_UngroupedNuggets;
	CUtlSortVector<CItem_Nugget*, CNuggetGroupSort> m_GroupedNuggets;
};

static CNuggetDirector g_NuggetDirector;

ConVar sv_randomize_nugget_availability( "sv_randomize_nugget_availability", "0", 0, "[PORTALMP] Randomize which nuggets are available on map start" );
ConVar sv_randomize_nugget_availability_ungroupedavailability( "sv_randomize_nugget_availability_ungroupedavailability", "0.5", 0, "[PORTALMP] 0.0 to 1.0 chances that an individual ungrouped nugget exists after randomization" );
ConVar sv_randomize_nugget_availability_groupavailability( "sv_randomize_nugget_availability_groupavailability", "0.5", 0, "[PORTALMP] 0.0 to 1.0 chances that a group of nuggets exists after randomization" );


void CItem_Nugget::Spawn( void )
{
	Precache();
	SetModel( NUGGET_MODEL );

	// Snap to a supported value and size the nugget to match
	float fScale;
	if ( m_nValue < 5 )
	{
		m_nValue = 1;
		m_nSkin = NUGGET_VALUE_1;
		fScale = 0.5f;
	}
	else if ( m_nValue < 10 )
	{
		m_nValue = 5;
		m_nSkin = NUGGET_VALUE_5;
		fScale = 0.6f;
	}
	else if ( m_nValue < 25 )
	{
		m_nValue = 10;
		m_nSkin = NUGGET_VALUE_10;
		fScale = 0.8f;
	}
	else
	{
		m_nValue = 25;
		m_nSkin = NUGGET_VALUE_25;
		fScale = 1.0f;
	}

	SetMoveType( MOVETYPE_NOCLIP );
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetCollisionGroup( COLLISION_GROUP_PLAYER );
	SetModelScale( fScale, 0.0f );

	// Spin in place
	ApplyLocalAngularVelocityImpulse( AngularImpulse( 0, 0, 180 ) );

	// The model is centered on its origin; move it so it rests on the ground
	float fScaledExtent = 24.0f * fScale;
	const Vector vModelOffset( 0, 0, -fScaledExtent );
	SetAbsOrigin( GetAbsOrigin() + vModelOffset );
	SetSize( Vector( -fScaledExtent, -fScaledExtent, -fScaledExtent ) - vModelOffset,
			 Vector( fScaledExtent, fScaledExtent, fScaledExtent ) - vModelOffset );

	BaseClass::Spawn();

	g_NuggetDirector.RegisterNugget( this );
}


void CItem_Nugget::Precache( void )
{
	PrecacheModel( NUGGET_MODEL );
	PrecacheScriptSound( "Nugget.Touch" );
	PrecacheScriptSound( "Nugget.Spawn" );
}


void CItem_Nugget::StartTouch( CBaseEntity *pOther )
{
	if ( IsEffectActive( EF_NODRAW ) )
		return;

	if ( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = static_cast<CBasePlayer*>( pOther );

	CSingleUserRecipientFilter user( pPlayer );
	user.MakeReliable();

	UserMessageBegin( user, "ItemPickup" );
		WRITE_STRING( GetClassname() );
	MessageEnd();

	CPASAttenuationFilter filter( pOther, "Nugget.Touch" );
	EmitSound( filter, pOther->entindex(), "Nugget.Touch" );

	pPlayer->IncrementFragCount( m_nValue );
	if ( pOther->GetTeam() )
	{
		pOther->GetTeam()->AddScore( m_nValue );
	}

	if ( m_fRespawnTime <= 0.0f )
	{
		UTIL_Remove( this );
	}
	else
	{
		Respawn();
	}
}


CBaseEntity *CItem_Nugget::Respawn( void )
{
	AddEffects( EF_NODRAW );
	SetThink( &CItem_Nugget::MaterializeThink );
	SetNextThink( gpGlobals->curtime + m_fRespawnTime );
	return this;
}


void CItem_Nugget::UpdateOnRemove( void )
{
	g_NuggetDirector.UnregisterNugget( this );
	BaseClass::UpdateOnRemove();
}


void CItem_Nugget::MaterializeThink( void )
{
	if ( !IsEffectActive( EF_NODRAW ) )
		return;

	EmitSound( "Nugget.Spawn" );
	RemoveEffects( EF_NODRAW );
}


//-----------------------------------------------------------------------------
// Nugget director
//-----------------------------------------------------------------------------
void CNuggetDirector::RegisterNugget( CItem_Nugget *pNugget )
{
	if ( pNugget->m_iGroupName == NULL_STRING || FStrEq( STRING( pNugget->m_iGroupName ), "" ) )
	{
		m_UngroupedNuggets.AddToTail( pNugget );
	}
	else
	{
		m_GroupedNuggets.Insert( pNugget );
	}
}


void CNuggetDirector::UnregisterNugget( CItem_Nugget *pNugget )
{
	if ( pNugget->m_iGroupName == NULL_STRING || FStrEq( STRING( pNugget->m_iGroupName ), "" ) )
	{
		m_UngroupedNuggets.FindAndFastRemove( pNugget );
	}
	else
	{
		m_GroupedNuggets.Remove( pNugget );
	}
}


void CNuggetDirector::RandomizeAvailableNuggets( float fUngroupedAvailabilityChance, float fGroupAvailabilityChance )
{
	// Each ungrouped nugget rolls on its own
	int iCount = m_UngroupedNuggets.Count();
	for ( int i = 0; i < iCount; ++i )
	{
		if ( RandomFloat( 0.0f, 1.0f ) <= fUngroupedAvailabilityChance )
		{
			m_UngroupedNuggets[i]->RemoveEffects( EF_NODRAW );
		}
		else
		{
			m_UngroupedNuggets[i]->AddEffects( EF_NODRAW );
		}
	}

	// Grouped nuggets are sorted by group, so roll once per group
	iCount = m_GroupedNuggets.Count();
	string_t iLastGroup = NULL_STRING;
	bool bExists = false;
	for ( int i = 0; i < iCount; ++i )
	{
		if ( iLastGroup != m_GroupedNuggets[i]->m_iGroupName )
		{
			iLastGroup = m_GroupedNuggets[i]->m_iGroupName;
			bExists = ( RandomFloat( 0.0f, 1.0f ) <= fGroupAvailabilityChance );
		}

		if ( bExists )
		{
			m_GroupedNuggets[i]->RemoveEffects( EF_NODRAW );
		}
		else
		{
			m_GroupedNuggets[i]->AddEffects( EF_NODRAW );
		}
	}
}


void CNuggetDirector::LevelInitPostEntity( void )
{
	if ( sv_randomize_nugget_availability.GetBool() )
	{
		RandomizeAvailableNuggets( sv_randomize_nugget_availability_ungroupedavailability.GetFloat(),
								   sv_randomize_nugget_availability_groupavailability.GetFloat() );
	}
}
