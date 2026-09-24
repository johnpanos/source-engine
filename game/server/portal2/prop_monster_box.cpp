//========= Portal 2 reconstruction ============================================//
//
// Purpose: Foldable monster turret physics prop.
//
// The retail RTTI names CPhysicsProp as the parent and the binary identifies
// both models, keyfields, inputs and fizzled output. This implements the
// physics/model transitions; monster animation and autonomous movement still
// need a native gameplay comparison. Not original Valve source; the
// repository's provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "prop_monster_box.h"

#include "tier0/memdbgon.h"

static const char *const s_pMonsterModel = "models/npcs/monsters/monster_a.mdl";
static const char *const s_pBoxModel = "models/npcs/monsters/monster_A_box.mdl";

LINK_ENTITY_TO_CLASS( prop_monster_box, CPropMonsterBox );

// clang-format off
BEGIN_DATADESC( CPropMonsterBox )
	DEFINE_KEYFIELD( m_bIsABox, FIELD_BOOLEAN, "StartAsBox" ),
	DEFINE_KEYFIELD( m_flBoxSwitchSpeed, FIELD_FLOAT, "BoxSwitchSpeed" ),
	DEFINE_FIELD( m_bForcedAsBox, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsShortcircuit, FIELD_BOOLEAN ),
	DEFINE_INPUTFUNC( FIELD_STRING, "BecomeBox", InputBecomeBox ),
	DEFINE_INPUTFUNC( FIELD_VOID, "BecomeMonster", InputBecomeMonster ),
	DEFINE_INPUTFUNC( FIELD_VOID, "BecomeShortcircuit", InputBecomeShortcircuit ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Dissolve", InputDissolve ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SilentDissolve", InputSilentDissolve ),
	DEFINE_THINKFUNC( MonsterThink ),
	DEFINE_OUTPUT( m_OnFizzled, "OnFizzled" ),
END_DATADESC()
    // clang-format on

    CPropMonsterBox::CPropMonsterBox()
{
	m_bIsABox = false;
	m_bForcedAsBox = false;
	m_bIsShortcircuit = false;
	m_flBoxSwitchSpeed = 400.0f;
}

void CPropMonsterBox::Precache()
{
	PrecacheModel( s_pMonsterModel );
	PrecacheModel( s_pBoxModel );
	BaseClass::Precache();
}

void CPropMonsterBox::Spawn()
{
	Precache();
	SetModelName( AllocPooledString( m_bIsABox ? s_pBoxModel : s_pMonsterModel ) );
	BaseClass::Spawn();
	SetThink( &CPropMonsterBox::MonsterThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CPropMonsterBox::SetBoxState( bool bBox )
{
	if ( m_bIsABox == bBox )
	{
		return;
	}
	m_bIsABox = bBox;
	VPhysicsDestroyObject();
	SetModel( bBox ? s_pBoxModel : s_pMonsterModel );
	CreateVPhysics();
}

void CPropMonsterBox::MonsterThink()
{
	if ( !m_bForcedAsBox && !m_bIsShortcircuit )
	{
		IPhysicsObject *pPhysics = VPhysicsGetObject();
		if ( pPhysics )
		{
			Vector velocity;
			AngularImpulse angularVelocity;
			pPhysics->GetVelocity( &velocity, &angularVelocity );
			SetBoxState( velocity.LengthSqr() >= m_flBoxSwitchSpeed * m_flBoxSwitchSpeed );
		}
	}
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CPropMonsterBox::BecomeBox( bool bPlayEffects )
{
	(void)bPlayEffects;
	m_bForcedAsBox = true;
	SetBoxState( true );
}

void CPropMonsterBox::BecomeMonster( bool bPlayEffects )
{
	(void)bPlayEffects;
	m_bForcedAsBox = false;
	if ( !m_bIsShortcircuit )
	{
		SetBoxState( false );
	}
}

void CPropMonsterBox::InputBecomeBox( inputdata_t &inputData )
{
	(void)inputData;
	BecomeBox( true );
}

void CPropMonsterBox::InputBecomeMonster( inputdata_t &inputData )
{
	(void)inputData;
	BecomeMonster( true );
}

void CPropMonsterBox::InputBecomeShortcircuit( inputdata_t &inputData )
{
	(void)inputData;
	m_bIsShortcircuit = true;
	m_bForcedAsBox = true;
	SetBoxState( true );
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if ( pPhysics )
	{
		pPhysics->EnableMotion( false );
	}
}

void CPropMonsterBox::InputDissolve( inputdata_t &inputData )
{
	m_OnFizzled.FireOutput( inputData.pActivator, this );
	Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
}

void CPropMonsterBox::InputSilentDissolve( inputdata_t &inputData )
{
	m_OnFizzled.FireOutput( inputData.pActivator, this );
	UTIL_Remove( this );
}
