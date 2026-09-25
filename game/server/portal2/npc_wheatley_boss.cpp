//========= Portal 2 reconstruction ============================================//
//
// Purpose: The scripted Wheatley boss actor (npc_wheatley_boss).
//
// Written from the retail Linux server.so CNPC_Wheatley_Boss (parent
// CAI_BaseActor, network table DT_NPC_Wheatley_Boss): its Precache, Spawn,
// UpdateOnRemove and bone-follower update. Spawn reads the model's
// "bone_followers" key list inline, as CNPC_Furniture::CreateVPhysics does.
// Map scripts and choreography drive the boss; the entity itself only
// animates, talks and collides. Not original Valve source; the repository's
// provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "ai_baseactor.h"
#include "physics_bone_follower.h"
#include "gamerules.h"

#include "tier0/memdbgon.h"

class CNPC_Wheatley_Boss : public CAI_BaseActor
{
public:
	DECLARE_CLASS( CNPC_Wheatley_Boss, CAI_BaseActor );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	void	Precache( void ) OVERRIDE;
	void	Spawn( void ) OVERRIDE;
	void	UpdateOnRemove( void ) OVERRIDE;
	void	NPCThink( void ) OVERRIDE;
	Class_T	Classify( void ) OVERRIDE { return CLASS_NONE; }

private:
	void	CreateBoneFollowers( void );

	CBoneFollowerManager m_BoneFollowerManager;
};

LINK_ENTITY_TO_CLASS( npc_wheatley_boss, CNPC_Wheatley_Boss );

BEGIN_DATADESC( CNPC_Wheatley_Boss )
	DEFINE_EMBEDDED( m_BoneFollowerManager ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_Wheatley_Boss, DT_NPC_Wheatley_Boss )
END_SEND_TABLE()

void CNPC_Wheatley_Boss::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );
	BaseClass::Precache();
}

void CNPC_Wheatley_Boss::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	UTIL_SetSize( this, g_pGameRules->GetViewVectors()->m_vHullMin, g_pGameRules->GetViewVectors()->m_vHullMax );
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
	SetBloodColor( DONT_BLEED );
	m_iHealth = 8000;
	m_flFieldOfView = 0.5f;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );

	NPCInit();

	CreateBoneFollowers();
}

// The model's "bone_followers" key list gives the bones that collide.
void CNPC_Wheatley_Boss::CreateBoneFollowers( void )
{
	if ( m_BoneFollowerManager.GetNumBoneFollowers() )
		return;

	KeyValues *modelKeyValues = new KeyValues( "" );
	if ( modelKeyValues->LoadFromBuffer( modelinfo->GetModelName( GetModel() ), modelinfo->GetModelKeyValueText( GetModel() ) ) )
	{
		KeyValues *pkvBoneFollowers = modelKeyValues->FindKey( "bone_followers" );
		if ( pkvBoneFollowers )
		{
			for ( KeyValues *pBone = pkvBoneFollowers->GetFirstSubKey(); pBone; pBone = pBone->GetNextKey() )
			{
				m_BoneFollowerManager.AddBoneFollower( this, pBone->GetString() );
			}
		}
	}
	modelKeyValues->deleteThis();
}

void CNPC_Wheatley_Boss::NPCThink( void )
{
	BaseClass::NPCThink();
	m_BoneFollowerManager.UpdateBoneFollowers( this );
}

void CNPC_Wheatley_Boss::UpdateOnRemove( void )
{
	m_BoneFollowerManager.DestroyBoneFollowers();
	BaseClass::UpdateOnRemove();
}
