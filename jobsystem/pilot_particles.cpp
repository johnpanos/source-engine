//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Particle reference pilot implementation (RFC 0003, Phase D).
//
//=============================================================================//

#include "jobsystem/pilot_particles.h"

#include <cstring>

namespace jobsystem
{

// Resource domains for this subsystem's declarations.
enum : uint32_t
{
	kResControlPoints = 1,
	kResBone          = 2,
	kResSystemOut     = 3,
};

// Deterministic per-system function and commit digest, shared by the reference
// and the graph paths so all three modes must agree bit-for-bit.
static double SystemFunc( double controlPoint, double boneContribution )
{
	return controlPoint * 3.0 - boneContribution;
}

static uint64_t MixDigest( uint64_t h, double v )
{
	uint64_t bits;
	std::memcpy( &bits, &v, sizeof( bits ) );
	h ^= bits + 0x9e3779b97f4a7c15ull + ( h << 6 ) + ( h >> 2 );
	return h;
}

void ParticleReferenceUpdate( ParticleFrameState &s )
{
	const size_t n = s.inputs.size();

	// gather
	s.controlPoints = s.inputs;

	// bone producer
	double sum = 0.0;
	for ( double v : s.inputs ) sum += v;
	s.boneValue = sum * 0.5;

	// per-system compute (system 0 is attached to the bone)
	s.perSystemOut.assign( n, 0.0 );
	for ( size_t i = 0; i < n; ++i )
	{
		s.perSystemOut[i] = SystemFunc( s.controlPoints[i], ( i == 0 ) ? s.boneValue : 0.0 );
	}

	// commit: deterministic change digest over outputs in index order
	uint64_t h = 0;
	for ( size_t i = 0; i < n; ++i ) h = MixDigest( h, s.perSystemOut[i] );
	s.changeDigest = h;
}

void ParticlePilotContributor::Contribute( JobGraphBuilder &b, const FrameBuildContext &ctx )
{
	ParticleFrameState *s = m_state;
	const uint32_t n = (uint32_t)s->inputs.size();
	const uint32_t epoch = (uint32_t)ctx.frame.epoch;

	// Size outputs at build time so parallel compute writes never resize.
	s->perSystemOut.assign( n, 0.0 );
	s->controlPoints.assign( n, 0.0 );

	const ResourceVersion cpVer{ kResControlPoints, epoch, 0 };
	const ResourceVersion boneVer{ kResBone, epoch, 0 };

	// gather (serial): control points from captured inputs.
	JobDesc gd; gd.name = "particles.gather"; gd.executor = Executor::Compute();
	gd.function = [s]( JobRunContext & ) { s->controlPoints = s->inputs; };
	JobHandle gather = b.AddJob( gd );
	b.AddDependency( ctx.afterInputs, gather );
	b.Write( gather, cpVer );

	// bone producer (independent branch).
	JobDesc bd; bd.name = "particles.bone"; bd.executor = Executor::Compute();
	bd.function = [s]( JobRunContext & ) {
		double sum = 0.0; for ( double v : s->inputs ) sum += v; s->boneValue = sum * 0.5; };
	JobHandle bone = b.AddJob( bd );
	b.AddDependency( ctx.afterInputs, bone );
	b.Write( bone, boneVer );

	// per-system compute (parallel), each writing its own disjoint partition.
	std::vector<JobHandle> computes;
	computes.reserve( n );
	for ( uint32_t i = 0; i < n; ++i )
	{
		JobDesc cd; cd.name = "particles.compute"; cd.executor = Executor::Compute();
		cd.function = [s, i]( JobRunContext & ) {
			double boneContribution = ( i == 0 ) ? s->boneValue : 0.0;
			s->perSystemOut[i] = SystemFunc( s->controlPoints[i], boneContribution );
		};
		JobHandle c = b.AddJob( cd );
		b.AddDependency( gather, c );        // needs gathered control points
		b.Read( c, cpVer );
		b.Write( c, ResourceVersion{ kResSystemOut, epoch, i } ); // disjoint slot
		if ( i == 0 )
		{
			b.AddDependency( bone, c );      // attachment: system 0 waits on bone
			b.Read( c, boneVer );
		}
		computes.push_back( c );
	}

	// commit (serial): change digest over all outputs in index order.
	JobDesc md; md.name = "particles.commit"; md.executor = Executor::Compute();
	md.function = [s, n]( JobRunContext & ) {
		uint64_t h = 0;
		for ( uint32_t i = 0; i < n; ++i ) h = MixDigest( h, s->perSystemOut[i] );
		s->changeDigest = h;
	};
	JobHandle commit = b.AddJob( md );
	for ( uint32_t i = 0; i < n; ++i )
	{
		b.AddDependency( computes[i], commit );
		b.Read( commit, ResourceVersion{ kResSystemOut, epoch, i } );
	}
	// Publish phase must wait for the commit.
	b.AddDependency( commit, ctx.beforeCommit );
}

} // namespace jobsystem
