//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Particle reference pilot (RFC 0003, Phase D; subsystem matrix row
//          "Particle simulation").
//
//          A faithful, self-contained model of the particle update seam
//          (CParticleMgr::UpdateNewEffects): a serial control-point gather, a
//          parallel per-system compute over disjoint output partitions, and a
//          serial change-detection commit. It also models one attachment
//          dependency (a control point attached to a bone must wait for that
//          bone's producer), matching the RFC render-frame example.
//
//          The subsystem owns its state; the contributor only expresses the
//          work as graph nodes with declared resources. The reference update is
//          the legacy path used for three-mode equivalence comparison. This is
//          NOT wired into game/client/particlemgr.cpp yet: that migration needs
//          the engine build and captured workloads (see RFC 0003 progress).
//
//=============================================================================//

#ifndef JOBSYSTEM_PILOT_PARTICLES_H
#define JOBSYSTEM_PILOT_PARTICLES_H

#ifdef _WIN32
#pragma once
#endif

#include <cstdint>
#include <vector>

#include "jobsystem/frame_graph.h"

namespace jobsystem
{

// Per-frame particle state, owned by the subsystem (not the coordinator).
struct ParticleFrameState
{
	// Captured immutable inputs for the frame (control-point sources).
	std::vector<double> inputs;

	// Produced during the frame:
	double              boneValue = 0.0;   // bone attachment producer
	std::vector<double> controlPoints;     // serial gather
	std::vector<double> perSystemOut;      // parallel, one disjoint slot per system
	uint64_t            changeDigest = 0;  // serial commit result
};

// The legacy serial reference over captured inputs. Deterministic; used as the
// equivalence oracle for the graph paths.
void ParticleReferenceUpdate( ParticleFrameState &s );

// Expresses ParticleReferenceUpdate as gather/compute/commit graph nodes.
class ParticlePilotContributor : public IFrameContributor
{
public:
	explicit ParticlePilotContributor( ParticleFrameState *state ) : m_state( state ) {}

	const char *Name() const override { return "particles"; }
	void Contribute( JobGraphBuilder &builder, const FrameBuildContext &ctx ) override;

private:
	ParticleFrameState *m_state;
};

} // namespace jobsystem

#endif // JOBSYSTEM_PILOT_PARTICLES_H
