//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: When a particle's step crosses one of its collision planes
//          (particle_collision.cpp). Tier0-free C++11 so the conformance suite
//          (unittests/clienttest/test_particle_plane_crossing.cpp) runs the
//          same predicate the client instantiates.
//
//=============================================================================//
#ifndef PARTICLE_PLANE_CROSSING_H
#define PARTICLE_PLANE_CROSSING_H

namespace ParticlePlaneCrossing
{

// A step from signed plane distance flStart to flEnd hits the plane when it
// goes from in front of it (beyond flEpsilon) to on or behind it, or from on
// the plane (within flEpsilon) to behind it. The second case catches a
// particle that starts on a surface (an effect origin quantized onto it by the
// network) and moves into it; a particle behind the plane never hits it. On a
// hit, *pFraction is the step's fraction at the plane (0 for a particle
// already on it).
inline bool Hits( float flStart, float flEnd, float flEpsilon, float *pFraction )
{
	bool bHit;
	if ( flStart > flEpsilon )
		bHit = flEnd <= flEpsilon;
	else
		bHit = flStart >= -flEpsilon && flEnd < -flEpsilon;
	if ( bHit )
		*pFraction = flStart > 0.0f ? flStart / ( flStart - flEnd ) : 0.0f;
	return bHit;
}

} // namespace ParticlePlaneCrossing

#endif // PARTICLE_PLANE_CROSSING_H
