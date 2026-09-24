//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point push entity that also pushes through portals and moves paint
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_pointpush.h"
#include "portal_base2d.h"
#include "particle_system.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

struct PortalVortexEffect_t
{
	PortalVortexEffect_t( CParticleSystem *pParticleSystem, CPortal_Base2D *pPortalOwner ) : hParticleSystem( pParticleSystem ), hPortalOwner( pPortalOwner ) {}

	CHandle<CParticleSystem>	hParticleSystem;
	CHandle<CPortal_Base2D>		hPortalOwner;
};

// Tracks the vortex particles placed at portals a point push is blowing through
class CPortalVortexEffectManager : public CAutoGameSystem
{
public:
	CPortalVortexEffectManager( void ) : CAutoGameSystem( "CPortalVortexEffectManager" ) {}

	void AddVortexEffect( CPortal_Base2D *pPortal, bool bPushing )
	{
		// Only one effect per portal
		for ( int itr = 0; itr < m_Effects.Count(); itr++ )
		{
			if ( m_Effects[itr].hPortalOwner == pPortal )
				return;
		}

		CParticleSystem *pParticleSystem = static_cast< CParticleSystem * >( CreateEntityByName( "info_particle_system" ) );
		if ( pParticleSystem != NULL )
		{
			pParticleSystem->KeyValue( "start_active", "1" );
			pParticleSystem->KeyValue( "effect_name", "broken_tube_suck" );
			pParticleSystem->SetParent( pPortal );

			Vector vecForward;
			MatrixGetColumn( pPortal->EntityToWorldTransform(), 0, vecForward );

			Vector vecOrigin = pPortal->WorldSpaceCenter();
			QAngle vecAngles = pPortal->GetAbsAngles();

			if ( bPushing )
			{
				// Face back into the portal, out in front of it
				VectorAngles( -vecForward, vecAngles );
				vecOrigin += vecForward * 64.0f;
			}
			else
			{
				vecOrigin += vecForward * -32.0f;
			}

			pParticleSystem->SetAbsOrigin( vecOrigin );
			pParticleSystem->SetAbsAngles( vecAngles );
			pParticleSystem->DisableSaveRestore( true );

			DispatchSpawn( pParticleSystem );
			pParticleSystem->Activate();
		}

		m_Effects.AddToTail( PortalVortexEffect_t( pParticleSystem, pPortal ) );
	}

	void RemoveVortexEffect( CPortal_Base2D *pPortal )
	{
		for ( int i = 0; i < m_Effects.Count(); i++ )
		{
			if ( m_Effects[i].hPortalOwner == pPortal || m_Effects[i].hPortalOwner == pPortal->GetLinkedPortal() )
			{
				UTIL_Remove( m_Effects[i].hParticleSystem );
				m_Effects.Remove( i );
				break;
			}
		}
	}

	void Purge( void )
	{
		for ( int i = m_Effects.Count() - 1; i >= 0; i-- )
		{
			UTIL_Remove( m_Effects[i].hParticleSystem );
			m_Effects.FastRemove( i );
		}
	}

	virtual void LevelShutdownPostEntity( void )
	{
		Purge();
	}

private:
	CUtlVector<PortalVortexEffect_t>	m_Effects;
};

CPortalVortexEffectManager g_PortalVortexManager;


//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CPortal_PointPush )

	DEFINE_THINKFUNC( PushThink ),

	DEFINE_KEYFIELD( m_bEnabled,			FIELD_BOOLEAN,	"enabled" ),
	DEFINE_KEYFIELD( m_flMagnitude,			FIELD_FLOAT,	"magnitude" ),
	DEFINE_KEYFIELD( m_flRadius,			FIELD_FLOAT,	"radius" ),
	DEFINE_KEYFIELD( m_flInnerRadius,		FIELD_FLOAT,	"inner_radius" ),
	DEFINE_KEYFIELD( m_flConeOfInfluence,	FIELD_FLOAT,	"influence_cone" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPortal_PointPush, DT_PortalPointPush )
	SendPropBool( SENDINFO( m_bEnabled ) ),
	SendPropFloat( SENDINFO( m_flMagnitude ) ),
	SendPropFloat( SENDINFO( m_flRadius ) ),
	SendPropFloat( SENDINFO( m_flInnerRadius ) ),
	SendPropFloat( SENDINFO( m_flConeOfInfluence ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( point_push, CPortal_PointPush );

void CPortal_PointPush::DrawDebugGeometryOverlays( void )
{
	Vector vecExtents( 2.0f, 2.0f, 2.0f );
	NDebugOverlay::Box( GetAbsOrigin(), -vecExtents, vecExtents, 0, 255, 0, 16, 0.05f );

	// Show the radius of influence
	NDebugOverlay::Sphere( GetAbsOrigin(), vec3_angle, m_flRadius, 0, 255, 0, 0, false, 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortal_PointPush::Activate( void )
{
	if ( m_bEnabled )
	{
		SetThink( &CPortal_PointPush::PushThink );
		SetNextThink( gpGlobals->curtime + 0.05f );
	}

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: The client simulates paint pushing, so it always needs us
//-----------------------------------------------------------------------------
int CPortal_PointPush::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Place a vortex effect on a portal we're pushing through
//-----------------------------------------------------------------------------
void CPortal_PointPush::CreatePortalVortexEffect( const Vector &vecOrigin, CPortal_Base2D *pPortal )
{
	CPortal_Base2D *pPartnerPortal = pPortal->GetLinkedPortal();

	// Only show the effect when the portal is well inside our radius
	float flDistFromOriginSqr = ( pPortal->WorldSpaceCenter() - vecOrigin ).Length2DSqr();
	float flMinRadiusSqr = Square( m_flRadius * 0.5f );
	if ( flDistFromOriginSqr >= flMinRadiusSqr )
		return;

	g_PortalVortexManager.AddVortexEffect( pPortal, ( m_flMagnitude > 0.0f ) );

	// We need to know if either side of this portal pair moves or fizzles
	pPortal->AddPortalEventListener( this );
	pPartnerPortal->AddPortalEventListener( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
//-----------------------------------------------------------------------------
void CPortal_PointPush::PushEntity( CBaseEntity *pTarget, const Vector &vecPushPoint )
{
	Vector vecPushDir;
	
	if ( HasSpawnFlags( SF_PUSH_DIRECTIONAL ) )
	{
		GetVectors( &vecPushDir, NULL, NULL );
	}
	else
	{
		vecPushDir = ( pTarget->BodyTarget( vecPushPoint, false ) - vecPushPoint );
	}

	float dist = VectorNormalize( vecPushDir );
	
	float flFalloff = ( HasSpawnFlags( SF_PUSH_NO_FALLOFF ) ) ? 1.0f : RemapValClamped( dist, m_flRadius, m_flRadius*0.25f, 0.0f, 1.0f );
	
	switch( pTarget->GetMoveType() )
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
		break;

	case MOVETYPE_VPHYSICS:
		{
			IPhysicsObject *pPhys = pTarget->VPhysicsGetObject();
			if ( pPhys )
			{
				// UNDONE: Assume the velocity is for a 100kg object, scale with mass
				pPhys->ApplyForceCenter( m_flMagnitude * flFalloff * 100.0f * vecPushDir * pPhys->GetMass() * gpGlobals->frametime );
				return;
			}
		}
		break;

	case MOVETYPE_STEP:
		{
			// NPCs cannot be lifted up properly, they need to move in 2D
			vecPushDir.z = 0.0f;
			
			// NOTE: Falls through!
		}

	default:
		{
			Vector vecPush = (m_flMagnitude * vecPushDir * flFalloff);
			if ( pTarget->GetFlags() & FL_BASEVELOCITY )
			{
				vecPush = vecPush + pTarget->GetBaseVelocity();
			}
			if ( vecPush.z > 0 && (pTarget->GetFlags() & FL_ONGROUND) )
			{
				pTarget->SetGroundEntity( NULL );
				Vector origin = pTarget->GetAbsOrigin();
				origin.z += 1.0f;
				pTarget->SetAbsOrigin( origin );
			}

			pTarget->SetBaseVelocity( vecPush );
			pTarget->AddFlag( FL_BASEVELOCITY );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortal_PointPush::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;
	SetThink( &CPortal_PointPush::PushThink );
	SetNextThink( gpGlobals->curtime + 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortal_PointPush::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
	SetThink( NULL );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Remove the vortex effects when a portal we push through goes away
//-----------------------------------------------------------------------------
void CPortal_PointPush::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	if ( nEventType == PORTALEVENT_FIZZLE || nEventType == PORTALEVENT_MOVED )
	{
		g_PortalVortexManager.RemoveVortexEffect( pNotifier );
		pNotifier->RemovePortalEventListener( this );
		if ( pNotifier->GetLinkedPortal() )
		{
			pNotifier->GetLinkedPortal()->RemovePortalEventListener( this );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Push the entities within our radius of a (possibly portal-transformed) point
//-----------------------------------------------------------------------------
void CPortal_PointPush::PushEntitiesAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortal )
{
	// Get a collection of entities in a radius around us
	CBaseEntity *pEnts[256];
	int numEnts = UTIL_EntitiesInSphere( pEnts, 256, vecOrigin, m_flRadius, 0 );
	for ( int i = 0; i < numEnts; i++ )
	{
		// Must be solid
		if ( pEnts[i]->IsSolid() == false )
			continue;

		// Cannot be parented (only push parents)
		if ( pEnts[i]->GetMoveParent() != NULL )
			continue;

		// Must be moveable
		if ( pEnts[i]->GetMoveType() != MOVETYPE_VPHYSICS && 
			 pEnts[i]->GetMoveType() != MOVETYPE_WALK && 
			 pEnts[i]->GetMoveType() != MOVETYPE_STEP )
			continue; 

		// If we don't want to push players, don't
		if ( pEnts[i]->IsPlayer() && HasSpawnFlags( SF_PUSH_PLAYER ) == false )
			continue;

		// If we don't want to push physics, don't
		if ( pEnts[i]->GetMoveType() == MOVETYPE_VPHYSICS && HasSpawnFlags( SF_PUSH_PHYSICS ) == false )
			continue;

		// Test against our cone of influence
		if ( m_flConeOfInfluence != 0.0f )
		{
			float flMinAngle = cos( DEG2RAD( m_flConeOfInfluence ) );
			if ( TestWithinCone( vecOrigin, pEnts[i]->BodyTarget( vecOrigin, false ), vecAngles, flMinAngle ) == false )
				continue;
		}

		// When pushing through a portal, only push things in front of the exit portal
		if ( pPortal && TestWithinCone( pPortal->GetAbsOrigin(), pEnts[i]->BodyTarget( pPortal->GetAbsOrigin(), false ), pPortal->GetAbsAngles(), 0.0f ) == false )
			continue;

		// Test for LOS if asked to
		if ( HasSpawnFlags( SF_PUSH_TEST_LOS ) )
		{
			Vector vecStartPos = vecOrigin;
			Vector vecEndPos = pEnts[i]->BodyTarget( vecStartPos, false );

			if ( m_flInnerRadius != 0.0f )
			{
				// Find a point on our inner radius sphere to begin from
				Vector vecDirToTarget = ( vecEndPos - vecStartPos );
				VectorNormalize( vecDirToTarget );
				vecStartPos = vecOrigin + ( vecDirToTarget * m_flInnerRadius );
			}

			trace_t tr;
			UTIL_TraceLine( vecStartPos, 
							pEnts[i]->BodyTarget( vecStartPos, false ), 
							MASK_SOLID_BRUSHONLY, 
							this, 
							COLLISION_GROUP_NONE, 
							&tr );

			// Shielded
			if ( tr.fraction < 1.0f && tr.m_pEnt != pEnts[i] )
				continue;
		}

		// Push it along
		PushEntity( pEnts[i], vecOrigin );
	}
}
