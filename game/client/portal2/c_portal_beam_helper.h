//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side chain of beams that follows a trace through portals.
//			The laser (C_PortalLaser) draws one beam per portal hop with it.
//
// Reconstructed from the retail Linux client.so (Portal 2 build 2014): the
// helper is inlined into C_PortalLaser's Spawn, ClientThink, OnDataChanged and
// destructor. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//=============================================================================//

#ifndef C_PORTAL_BEAM_HELPER_H
#define C_PORTAL_BEAM_HELPER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"

class C_Beam;
class ITraceFilter;
class CGameTrace;
typedef CGameTrace trace_t;

// Retail stops following the trace after this many beams
#define MAX_PORTAL_BEAMS 10

class C_PortalBeamHelper
{
public:
	C_PortalBeamHelper();
	~C_PortalBeamHelper();

	// pBeamTemplate becomes the first beam and is owned (removed) by the helper
	void Init( C_Beam *pBeamTemplate );

	// Show or hide every beam
	void TurnOn();
	void TurnOff();

	// Traces from vStartPoint towards vEndPoint through any portals, placing one
	// beam per segment. pTrace receives the final segment's trace.
	void UpdatePoints( const Vector &vStartPoint, const Vector &vEndPoint, unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace );
	void UpdatePointDirection( const Vector &vStartPoint, const Vector &vDirection, unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace );

	int BeamCount() const { return m_beams.Count(); }
	C_Beam *GetBeam( int i ) const { return m_beams[i]; }
	C_Beam *GetFirstBeam() const { return m_beams.Count() ? m_beams.Head() : NULL; }
	C_Beam *GetLastBeam() const { return m_beams.Count() ? m_beams.Tail() : NULL; }

private:
	C_Beam *CreateBeam();

	C_Beam *m_pBeamTemplate;
	CUtlVector< C_Beam* > m_beams;
};

#endif // C_PORTAL_BEAM_HELPER_H
