//========= Portal 2 reconstruction ============================================//
//
// Purpose: Histogram of challenge mode leaderboard scores ("buckets") used by
//          the leaderboard graphs, and its KeyValues form ("bucketizedData")
//          returned by the IPortal2Leaderboards/GetBucketizedData web API.
//
// Reconstructed from the retail Linux client.so (stripped i386; decompiler
// output). The class name is not recorded in the binary; this one follows the
// file name. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL2_LEADERBOARD_BUCKETIZER_H
#define PORTAL2_LEADERBOARD_BUCKETIZER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"

class KeyValues;

// A score range is split into at most this many buckets.
#define LEADERBOARD_MAX_BUCKETS	41

class CLeaderboardBucketizer
{
public:
	CLeaderboardBucketizer();
	~CLeaderboardBucketizer();

	void Purge();

	void SetEntryInfo( int nFirstEntry, int nNumEntries ) { m_nFirstEntry = nFirstEntry; m_nNumEntries = nNumEntries; }

	// Clears the buckets and covers [nMin, nMax] with one bucket per score,
	// up to LEADERBOARD_MAX_BUCKETS.
	void SetRange( int nMin, int nMax );

	// Counts one score; scores above the maximum are ignored.
	void AddScore( unsigned int nScore );

	void SetBuckets( const CUtlVector< int > &buckets );

	// Fills the buckets from a leaderboard download
	// ("totalLeaderBoardEntryCount", "leaderboardEntries"/*/"score").
	void AddScoresFromKeyValues( KeyValues *pLeaderboard );

	// Writes "bucketCount", "min", "max", "numEntries" and "buckets" and
	// dumps the result with DevMsg.
	bool WriteToKeyValues( KeyValues *pOut ) const;

	// Replaces the counts with a smooth sine-shaped curve peaking at the
	// weighted mean score (at least a quarter of the range past the first
	// non-empty bucket).
	void SmoothBuckets();

	int GetBucketCount() const { return m_Buckets.Count(); }
	int GetBucket( int nIndex ) const { return m_Buckets[ nIndex ]; }
	int GetMin() const { return m_nMin; }
	int GetMax() const { return m_nMax; }
	int GetNumEntries() const { return m_nNumEntries; }

private:
	CUtlVector< int > m_Buckets;
	int m_nMin;
	int m_nMax;
	int m_nFirstEntry;
	int m_nNumEntries;
};

#endif // PORTAL2_LEADERBOARD_BUCKETIZER_H
