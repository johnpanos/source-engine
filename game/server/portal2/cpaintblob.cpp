//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server paint blob: paints surfaces and pushes physics objects.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "cpaintblob.h"
#include "paint_database.h"
#include "physics.h"
#include "physics_prop_ragdoll.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_paintblob_damage( "sv_paintblob_damage", "0.1", FCVAR_CHEAT );

ConVar paintblob_applies_impulse( "paintblob_applies_impulse", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Should paint blobs apply an impulse when they impact physics objects?" );

CPaintBlob::CPaintBlob() : m_pPaintingEntity( NULL ), m_ID( 0 )
{
}


CPaintBlob::~CPaintBlob()
{
}


void CPaintBlob::AddToPaintDatabase()
{
	if ( m_pPaintingEntity )
	{
		trace_t tr;
		tr.endpos = m_vecPosition;
		tr.plane.normal = m_vContactNormal;
		tr.m_pEnt = m_pPaintingEntity;
		tr.surface.flags = m_nSurfaceFlags;

		PaintDatabase.AddPaint( tr, m_paintType );

		m_pPaintingEntity = NULL;
	}
}


void CPaintBlob::SetupPaintInfo( const trace_t &tr )
{
	m_pPaintingEntity = tr.m_pEnt;

	SetPosition( tr.endpos );
	m_vContactNormal = tr.plane.normal;
	m_nSurfaceFlags = tr.surface.flags;
}


Vector GetPaintBlobDamage( const Vector &vecDir )
{
	float flForceScale = sv_paintblob_damage.GetFloat() * ImpulseScale( 75, 4 );
	Vector vecForce = vecDir;
	VectorNormalize( vecForce );
	vecForce *= flForceScale;
	vecForce *= phys_pushscale.GetFloat();

	return vecForce;
}


void CPaintBlob::PaintBlobPaint( const trace_t &tr )
{
	Vector vecTouchPos = tr.endpos;
	Vector vecNormal = tr.plane.normal;

	//Play the paint effect
	PlayEffect( vecTouchPos, vecNormal );

	//Remember the surface so the paint can be added to the database later
	SetupPaintInfo( tr );

	if ( paintblob_applies_impulse.GetBool() )
	{
		CBaseEntity *pOther = tr.m_pEnt;
		if ( !pOther->IsPlayer() )
		{
			//Apply an impulse to physics objects
			IPhysicsObject *pPhysics = pOther->VPhysicsGetObject();
			if ( pPhysics )
			{
				Vector vVel = GetPaintBlobDamage( m_vecVelocity.Normalized() ) / 10.0f;
				Vector aVel = vVel;

				//Ragdolls need the impulse applied to every element
				CRagdollProp *pRagdoll = dynamic_cast< CRagdollProp* >( pOther );
				if ( pRagdoll == NULL )
				{
					pPhysics->AddVelocity( &vVel, &aVel );
				}
				else
				{
					ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();
					for ( int j = 0; j < pRagdollPhys->listCount; ++j )
					{
						pRagdollPhys->list[j].pObject->AddVelocity( &vVel, &aVel );
					}
				}
			}
		}
	}
}


void CPaintBlob::AddBlobTeleportationHistory( const BlobTeleportationHistory_t& history )
{
	m_teleportationHistory.AddToTail( history );
}


void CPaintBlob::ClearTeleportationHistory()
{
	m_teleportationHistory.RemoveAll();
}


void CPaintBlob::GetTeleportationHistory( BlobTeleportationHistoryVector_t& historyList )
{
	int nHistoryCount = m_teleportationHistory.Count();
	if ( nHistoryCount != 0 )
	{
		historyList.AddMultipleToTail( nHistoryCount, m_teleportationHistory.Base() );
	}
}
