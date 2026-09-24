//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side chain of beams that follows a trace through portals.
//
// Reconstructed from the retail Linux client.so (Portal 2 build 2014), where
// the helper's functions follow C_PortalLaser's. Not original Valve source;
// the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_portal_beam_helper.h"
#include "beam_shared.h"
#include "portal_util_shared.h"
#include "c_portal_base2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_PortalBeamHelper::C_PortalBeamHelper()
{
	m_pBeamTemplate = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: The beams are client-only entities, so the helper removes them
//-----------------------------------------------------------------------------
C_PortalBeamHelper::~C_PortalBeamHelper()
{
	m_pBeamTemplate = NULL;

	for ( int i = 0; i < m_beams.Count(); ++i )
	{
		m_beams[i]->Remove();
		m_beams[i] = NULL;
	}

	m_beams.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: The template is the first beam; later beams copy its settings
//-----------------------------------------------------------------------------
void C_PortalBeamHelper::Init( C_Beam *pBeamTemplate )
{
	m_pBeamTemplate = pBeamTemplate;
	m_beams.AddToTail( pBeamTemplate );
}

//-----------------------------------------------------------------------------
// Purpose: CBeam::TurnOff clears EF_NODRAW
//-----------------------------------------------------------------------------
void C_PortalBeamHelper::TurnOn()
{
	for ( int i = 0; i < m_beams.Count(); ++i )
	{
		m_beams[i]->TurnOff();
	}
}

//-----------------------------------------------------------------------------
// Purpose: CBeam::TurnOn sets EF_NODRAW
//-----------------------------------------------------------------------------
void C_PortalBeamHelper::TurnOff()
{
	for ( int i = 0; i < m_beams.Count(); ++i )
	{
		m_beams[i]->TurnOn();
	}
}

//-----------------------------------------------------------------------------
// Purpose: New beams match the template, except for the halo index
//-----------------------------------------------------------------------------
C_Beam *C_PortalBeamHelper::CreateBeam()
{
	C_Beam *pBeam = C_Beam::BeamCreate(
	    STRING( m_pBeamTemplate->GetModelName() ), m_pBeamTemplate->GetWidth() );

	pBeam->SetType( m_pBeamTemplate->GetType() );
	pBeam->SetBeamFlags( m_pBeamTemplate->GetBeamFlags() );
	pBeam->SetHaloScale( m_pBeamTemplate->GetHaloScale() );
	pBeam->SetWidth( m_pBeamTemplate->GetWidth() );
	pBeam->SetEndWidth( m_pBeamTemplate->GetEndWidth() );
	pBeam->SetFadeLength( m_pBeamTemplate->GetFadeLength() );
	pBeam->SetNoise( m_pBeamTemplate->GetNoise() );

	const color32 clr = m_pBeamTemplate->GetRenderColor();
	pBeam->SetColor( clr.r, clr.g, clr.b );
	pBeam->SetBrightness( m_pBeamTemplate->GetBrightness() );
	pBeam->SetFrame( m_pBeamTemplate->GetFrame() );
	pBeam->SetScrollRate( (int)m_pBeamTemplate->GetScrollRate() );
	pBeam->SetHDRColorScale( m_pBeamTemplate->GetHDRColorScale() );

	return pBeam;
}

//-----------------------------------------------------------------------------
// Purpose: Places one beam per segment of the trace through portals
//-----------------------------------------------------------------------------
void C_PortalBeamHelper::UpdatePoints( const Vector &vStartPoint, const Vector &vEndPoint,
    unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace )
{
	Vector vStart = vStartPoint;
	Vector vDir = vEndPoint - vStartPoint;
	VectorNormalize( vDir );

	trace_t tr;
	int nBeams = 0;
	while ( nBeams < MAX_PORTAL_BEAMS )
	{
		if ( m_beams.Count() == nBeams )
		{
			m_beams.AddToTail( CreateBeam() );
		}

		C_Beam *pBeam = m_beams[nBeams];

		// Segments after the first come out of a portal (or off reflect paint)
		// at the width the previous segment ends with
		if ( nBeams != 0 )
		{
			pBeam->SetWidth( m_pBeamTemplate->GetEndWidth() );
			pBeam->SetEndWidth( m_pBeamTemplate->GetEndWidth() );
		}

		Ray_t ray;
		ray.Init( vStart, vStart + vDir * MAX_TRACE_LENGTH );

		UTIL_ClearTrace( tr );
		UTIL_TraceRay( ray, fMask, pTraceFilter, &tr );

		++nBeams;

		C_Portal_Base2D *pPortal = NULL;
		if ( UTIL_DidTraceTouchPortals( ray, tr, &pPortal ) && pPortal &&
		     pPortal->IsActivedAndLinked() )
		{
			// Nothing to draw at the end that enters the portal
			pBeam->SetHaloTexture( 0 );
			pBeam->PointsInit( vStart, tr.endpos );
			pBeam->SetAbsOrigin( vStart );
			pBeam->TurnOff();

			// Continue out of the linked portal
			Ray_t rayTransformed;
			UTIL_Portal_RayTransform( pPortal->m_matrixThisToLinked, ray, rayTransformed );

			vDir = rayTransformed.m_Delta;
			VectorNormalize( vDir );

			UTIL_Portal_PointTransform( pPortal->m_matrixThisToLinked, tr.endpos, vStart );
		}
		else
		{
			pBeam->PointsInit( vStart, tr.endpos );
			pBeam->SetAbsOrigin( vStart );
			pBeam->TurnOff();

			if ( !UTIL_Paint_Reflect( tr, vStart, vDir ) )
				break;
		}
	}

	// Remove the beams this trace no longer needs
	for ( int i = m_beams.Count() - 1; i >= nBeams; --i )
	{
		m_beams[i]->Remove();
		m_beams[i] = NULL;
	}
	m_beams.SetCountNonDestructively( nBeams );

	if ( pTrace )
	{
		*pTrace = tr;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_PortalBeamHelper::UpdatePointDirection( const Vector &vStartPoint, const Vector &vDirection,
    unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace )
{
	UpdatePoints(
	    vStartPoint, vStartPoint + vDirection * MAX_TRACE_LENGTH, fMask, pTraceFilter, pTrace );
}
