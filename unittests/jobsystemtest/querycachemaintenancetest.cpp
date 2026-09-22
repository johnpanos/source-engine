//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Query-cache maintenance conformance (RFC 0003 R30, Q-JOBS). The
//          production kernel must reproduce the original serial pass exactly
//          in legacy-loop, serial-graph and pooled-graph modes. See
//          querycache_harness.h for the oracle and the compared state.
//
//=============================================================================//

#include "querycache_harness.h"

#include <cstdio>

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const char *what, const std::string &detail )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL %s: %s\n", what, detail.c_str() );
	}
}

const qc::Config kConfigs[] = {
    // Production geometry: 1024 entries, 2048 chains, 8 splits.
    { 1024, 2048, 8, 160, 240, 160 },
    // Small cache: constant victim-list exhaustion and round-robin replacement.
    { 64, 128, 8, 48, 300, 90 },
    // Uneven chain count: the last split takes the remainder.
    { 48, 100, 8, 40, 300, 60 },
    // Fewer splits than workers and a one-chain split.
    { 40, 3, 3, 24, 200, 40 },
};

} // namespace

int main()
{
	int nFramesCompared = 0;
	for ( const qc::Config &cfg : kConfigs )
	{
		for ( uint32_t seed = 1; seed <= 6; ++seed )
		{
			struct Candidate
			{
				qc::Mode mode;
				int workers;
			};
			const Candidate candidates[] = {
			    { qc::Mode::LegacyLoop, 0 },
			    { qc::Mode::SerialGraph, 0 },
			    { qc::Mode::PooledGraph, 0 },
			    { qc::Mode::PooledGraph, 1 },
			    { qc::Mode::PooledGraph, 3 },
			    { qc::Mode::PooledGraph, 8 },
			};
			for ( const Candidate &candidate : candidates )
			{
				qc::ThreadBackend backend( candidate.workers );
				auto update = [&]( qc::Cache &cache )
				{
					return qc::KernelUpdate( cache, cfg.nSplits, candidate.mode, &backend,
					    &qc::KernelClassify, &qc::KernelCommit );
				};
				std::string why;
				int frame = -1;
				const int frames = qc::RunScenario( cfg, seed, update, &why, &frame );
				char what[160];
				std::snprintf( what, sizeof( what ),
				    "size=%d chains=%d splits=%d seed=%u mode=%s workers=%d frame=%d", cfg.nSize,
				    cfg.nChains, cfg.nSplits, seed, qc::ModeName( candidate.mode ),
				    candidate.workers, frame );
				Check( frames == cfg.nFrames, what, why );
				if ( frames > 0 )
					nFramesCompared += frames;
			}
		}
	}

	// The workload must actually exercise every commit path, or equality is vacuous.
	{
		const qc::Config &cfg = kConfigs[1];
		int refreshes = 0, invalidations = 0, wasted = 0, victimsUsed = 0, speculative = 0;
		auto update = [&]( qc::Cache &cache )
		{
			qc::ThreadBackend backend( 0 );
			const size_t before = cache.log.size();
			const int wastedBefore = cache.nWasted;
			const bool ok = qc::KernelUpdate( cache, cfg.nSplits, qc::Mode::LegacyLoop, &backend,
			    &qc::KernelClassify, &qc::KernelCommit );
			for ( size_t i = before; i < cache.log.size(); ++i )
				( cache.log[i].invalidated ? invalidations : refreshes )++;
			wasted += cache.nWasted - wastedBefore;
			victimsUsed += cache.victims.m_pHead ? 1 : 0;
			speculative = cache.nSuccessfulSpeculatives;
			return ok;
		};
		std::string why;
		int frame = -1;
		Check(
		    qc::RunScenario( cfg, 3, update, &why, &frame ) == cfg.nFrames, "coverage run", why );
		char detail[160];
		std::snprintf( detail, sizeof( detail ),
		    "refresh=%d invalidate=%d wasted=%d victims=%d speculative-hits=%d", refreshes,
		    invalidations, wasted, victimsUsed, speculative );
		Check( refreshes > 50 && invalidations > 5 && wasted > 20 && victimsUsed > 10 &&
		           speculative > 20,
		    "workload covers refresh, invalidation, wasted expiry, victim publication and "
		    "successful speculation",
		    detail );
		std::printf( "coverage: %s\n", detail );
	}

	std::printf(
	    "%d frames compared, %d checks, %d failures\n", nFramesCompared, g_checks, g_failures );
	return g_failures ? 1 : 0;
}
