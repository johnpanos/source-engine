//========= Portal 2 reconstruction ============================================//
//
// Purpose: Challenge mode leaderboard score histogram; see
//          portal2_leaderboard_bucketizer.h.
//
// Reconstructed from the retail Linux client.so (stripped i386; decompiler
// output). AddScoresFromKeyValues, WriteToKeyValues and SmoothBuckets have no
// callers in the retail client. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_leaderboard_bucketizer.h"
#include "tier1/KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLeaderboardBucketizer::CLeaderboardBucketizer()
{
	m_nMin = 0;
	m_nMax = 0;
	m_nFirstEntry = 0;
	m_nNumEntries = 0;
}

CLeaderboardBucketizer::~CLeaderboardBucketizer()
{
	Purge();
}

void CLeaderboardBucketizer::Purge()
{
	m_Buckets.Purge();
}

void CLeaderboardBucketizer::SetRange( int nMin, int nMax )
{
	m_Buckets.RemoveAll();
	m_nMax = nMax;
	m_nMin = nMin;
	if ( nMin == nMax + 1 )
		return;

	unsigned int nBuckets = MIN( (unsigned int)( nMax + 1 - nMin ), (unsigned int)LEADERBOARD_MAX_BUCKETS );
	m_Buckets.SetCount( nBuckets );
	for ( int i = 0; i < m_Buckets.Count(); ++i )
	{
		m_Buckets[ i ] = 0;
	}
}

void CLeaderboardBucketizer::AddScore( unsigned int nScore )
{
	if ( nScore > (unsigned int)m_nMax || m_Buckets.Count() == 0 )
		return;

	float flLastBucket = (float)(unsigned int)( m_Buckets.Count() - 1 );
	float flMax = (float)(unsigned int)m_nMax;
	float flMin = (float)(unsigned int)m_nMin;
	float flScore = (float)nScore;

	unsigned int nBucket;
	if ( flMax == flMin )
	{
		nBucket = ( flScore - flMax < 0.0f ) ? 0 : (unsigned int)flLastBucket;
	}
	else
	{
		float flFraction = ( flScore - flMin ) / ( flMax - flMin );
		nBucket = (unsigned int)( flLastBucket * clamp( flFraction, 0.0f, 1.0f ) );
	}

	m_Buckets[ nBucket ]++;
}

void CLeaderboardBucketizer::SetBuckets( const CUtlVector< int > &buckets )
{
	m_Buckets.Purge();
	m_Buckets.AddVectorToTail( buckets );
}

void CLeaderboardBucketizer::AddScoresFromKeyValues( KeyValues *pLeaderboard )
{
	unsigned int nTotal = (unsigned int)pLeaderboard->GetInt( "totalLeaderBoardEntryCount", 0 );
	KeyValues *pEntries = pLeaderboard->FindKey( "leaderboardEntries" );
	KeyValues *pEntry = pEntries ? pEntries->GetFirstTrueSubKey() : NULL;
	if ( !nTotal || !pEntry )
		return;

	unsigned int nAdded = 0;
	do
	{
		++nAdded;
		AddScore( (unsigned int)pEntry->GetInt( "score", 0 ) );
		pEntry = pEntry->GetNextKey();
	} while ( nAdded < nTotal && pEntry );
}

bool CLeaderboardBucketizer::WriteToKeyValues( KeyValues *pOut ) const
{
	if ( !pOut || !m_Buckets.Count() )
		return false;

	pOut->SetInt( "bucketCount", m_Buckets.Count() );
	pOut->SetInt( "min", m_nMin );
	pOut->SetInt( "max", m_nMax );
	pOut->SetInt( "numEntries", m_nNumEntries );

	KeyValues *pBuckets = new KeyValues( "buckets" );
	for ( int i = 0; i < m_Buckets.Count(); ++i )
	{
		char szName[8];
		V_snprintf( szName, sizeof( szName ), "%d", i );
		KeyValues *pBucket = new KeyValues( szName );
		pBucket->SetInt( "playerCount", m_Buckets[ i ] );
		pBuckets->AddSubKey( pBucket );
	}
	pOut->AddSubKey( pBuckets );

	KeyValuesDumpAsDevMsg( pOut, 0, 1 );
	return true;
}

void CLeaderboardBucketizer::SmoothBuckets()
{
	int nBuckets = m_Buckets.Count();
	if ( !nBuckets )
		return;

	int nTotal = 0;
	if ( nBuckets > 0 )
	{
		unsigned int nWeighted = 0;
		int nFirst = 0;
		for ( int i = 0; i < nBuckets; ++i )
		{
			nWeighted += m_Buckets[ i ] * i;
			if ( nFirst == 0 && m_Buckets[ i ] != 0 )
			{
				nFirst = i;
			}
		}

		unsigned int nPeak = MAX( nWeighted / (unsigned int)m_nNumEntries, (unsigned int)( ( nBuckets >> 2 ) + nFirst ) );
		float flStep = 1.0f / (float)(int)( nPeak - nFirst );

		for ( int i = 0; i < nBuckets; ++i )
		{
			int nValue = 0;
			if ( i >= nFirst )
			{
				float flX = (float)( i - nFirst ) * flStep;
				float flAngle = flX * ( M_PI_F * 0.5f );
				if ( flX >= 1.0f )
				{
					flAngle = ( flAngle - M_PI_F * 0.5f ) * 0.6f + M_PI_F * 0.5f;
				}
				if ( flAngle >= M_PI_F * 0.75f )
				{
					flAngle = ( flAngle - M_PI_F * 0.75f ) * 0.4f + M_PI_F * 0.75f;
				}
				if ( (double)flAngle < M_PI )
				{
					nValue = (int)( sinf( flAngle ) * 50.0f );
					nTotal += nValue;
				}
			}
			m_Buckets[ i ] = nValue;
		}
	}

	m_nNumEntries = nTotal;
}
