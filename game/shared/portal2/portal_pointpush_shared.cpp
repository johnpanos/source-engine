//========= Portal 2 reconstruction ============================================//
//
// Purpose: Shared point push logic (entities on the server, paint on both sides)
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_util_shared.h"
#include "paint_stream_manager.h"
#include "paint_stream_shared.h"

#ifdef CLIENT_DLL
#include "c_portal_pointpush.h"
#include "c_portal_base2d.h"
#else
#include "portal_pointpush.h"
#include "portal_base2d.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar portal_pointpush_think_rate( "portal_pointpush_think_rate", "0.05f", FCVAR_REPLICATED | FCVAR_CHEAT, "The amount of time between thinks for the portal_pointpush." );
ConVar portal_pointpush_debug( "portal_pointpush_debug", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Debug the portal_pointpush." );


void CPortal_PointPush::PushThink( void )
{
	// Find every place our radius reaches through portals
	PortalRadiusExtensionVector portalRadiusExtensions;
	ExtendRadiusThroughPortals( GetAbsOrigin(), GetAbsAngles(), m_flRadius, portalRadiusExtensions );

	for ( int i = 0; i < portalRadiusExtensions.Count(); ++i )
	{
		PushAtPoint( portalRadiusExtensions[i].vecOrigin, portalRadiusExtensions[i].vecAngles, portalRadiusExtensions[i].pPortalFrom, portalRadiusExtensions[i].pPortalTo );
	}

#ifdef CLIENT_DLL
	SetNextClientThink( gpGlobals->curtime + portal_pointpush_think_rate.GetFloat() );
#else
	SetNextThink( gpGlobals->curtime + portal_pointpush_think_rate.GetFloat() );
#endif
}


void CPortal_PointPush::PushAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortalFrom, CPortal_Base2D *pPortalTo )
{
#ifdef GAME_DLL
	// Points reached through a portal draw in magenta, our own point in cyan
	Color debugColor = ( pPortalFrom ) ? Color( 255, 0, 255 ) : Color( 0, 255, 255 );

	if ( portal_pointpush_debug.GetBool() )
	{
		NDebugOverlay::Sphere( vecOrigin, m_flRadius, debugColor.r(), debugColor.g(), debugColor.b(), false, 0.1f );
		NDebugOverlay::Sphere( vecOrigin, 2.0f, debugColor.r(), debugColor.g(), debugColor.b(), false, 0.1f );
	}

	if ( pPortalFrom )
	{
		CreatePortalVortexEffect( vecOrigin, pPortalTo );
	}

	PushEntitiesAtPoint( vecOrigin, vecAngles, pPortalTo );
#endif

	PushPaintBlobsAtPoint( vecOrigin, vecAngles, pPortalTo );
}

//-----------------------------------------------------------------------------
// Purpose: Is vecEndPos within the cone around vecAngles' forward from vecOrigin
//-----------------------------------------------------------------------------
bool CPortal_PointPush::TestWithinCone( const Vector &vecOrigin, const Vector &vecEndPos, const QAngle &vecAngles, float flMinAngle )
{
	Vector vecDirToTarget = ( vecEndPos - vecOrigin );
	VectorNormalize( vecDirToTarget );

	Vector vecDirection;
	AngleVectors( vecAngles, &vecDirection );

	float flDot = DotProduct( vecDirToTarget, vecDirection );

	return ( flDot >= flMinAngle );
}

//-----------------------------------------------------------------------------
// Purpose: Push the paint blobs within our radius of a point
//-----------------------------------------------------------------------------
void CPortal_PointPush::PushPaintBlobsAtPoint( const Vector &vecOrigin, const QAngle &vecAngles, CPortal_Base2D *pPortal )
{
	float flRadiusSqr = m_flRadius * m_flRadius;
	Vector vecPushPoint = vecOrigin;

	// Reconstruction note: the 2010 builds walked the stream manager's
	// per-power streams ( GetPaintStream( 0 .. PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER - 1 ) ).
	// The imported stream manager keeps its streams in IPaintStreamAutoList instead.
	for ( int i = 0; i < IPaintStreamAutoList::AutoList().Count(); ++i )
	{
		CPaintStream *pStream = static_cast< CPaintStream* >( IPaintStreamAutoList::AutoList()[i] );
		if ( pStream == NULL )
			continue;

		int numBlobs = pStream->GetBlobsCount();
		for ( int j = 0; j < numBlobs; ++j )
		{
			CBasePaintBlob *pBlob = pStream->GetBlob( j );
			if ( pBlob == NULL )
				continue;

			// Streaking blobs are owned by their tractor beam
			if ( pBlob->IsStreaking() )
				continue;

			Vector vecBlobPos = pBlob->GetPosition();
			float flDistSqr = vecBlobPos.DistToSqr( vecPushPoint );
			if ( flDistSqr > flRadiusSqr )
				continue;

			// Test against our cone of influence
			if ( m_flConeOfInfluence != 0.0f )
			{
				float flMinAngle = cos( DEG2RAD( m_flConeOfInfluence ) );
				if ( TestWithinCone( vecOrigin, vecBlobPos, vecAngles, flMinAngle ) == false )
					continue;
			}

			// When pushing through a portal, only push blobs in front of the exit portal
			if ( pPortal && TestWithinCone( pPortal->GetAbsOrigin(), vecBlobPos, pPortal->GetAbsAngles(), 0.0f ) == false )
				continue;

			Vector vecOldBlobVelocity = pBlob->GetVelocity();
			Vector vecPushDir = vecBlobPos - vecPushPoint;
			float flDist = VectorNormalize( vecPushDir );

			// Reconstruction note: both builds scale the push by the magnitude only;
			// the distance falloff is folded to a constant.
			float flFalloff = 1.0f;
			NOTE_UNUSED( flDist );

			Vector vecPushVelocity = flFalloff * ( vecPushDir * m_flMagnitude );
			pBlob->SetVelocity( vecOldBlobVelocity + vecPushVelocity );
		}
	}
}
