//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "querycache.h"
#include "tier0/vprof.h"
#include "tier1/utlintrusivelist.h"
#include "datacache/imdlcache.h"
#include "vstdlib/jobthread.h"
#include "vstdlib/jobgraph_parallel.h"
#include "querycache_maintenance.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



#define QUERYCACHE_SIZE 1024

static QueryCacheEntry_t s_QCache[QUERYCACHE_SIZE];

#define QUERYCACHE_HASH_SIZE ( QUERYCACHE_SIZE  * 2 )

// elements available for cache reuse
static CUtlIntrusiveDList<QueryCacheEntry_t> s_VictimList;


static CUtlIntrusiveDList<QueryCacheEntry_t> s_HashChains[QUERYCACHE_HASH_SIZE];



static int s_nReplaceCtr = QUERYCACHE_SIZE - 1;
static int s_nTimeStampCounter = 0 ;
static int s_nNumCacheQueries = 0;
static int s_nNumCacheMisses = 0;
static int s_SuccessfulSpeculatives = 0;
static int s_WastedSpeculativeUpdates = 0;

void QueryCacheKey_t::ComputeHashIndex( void )
{
	unsigned int ret = ( unsigned int ) m_Type;
	for( int i = 0 ; i < m_nNumValidPoints; i++ )
	{
		ret += ( unsigned int ) m_pEntities[i].ToInt();
		ret += ( uintp ) m_nOffsetMode;
	}
	ret += *( ( uint32 *) &m_flMinimumUpdateInterval );
	ret += m_nTraceMask;
	m_nHashIdx = ret % QUERYCACHE_HASH_SIZE;
}


ConVar	sv_disable_querycache("sv_disable_querycache", "0", FCVAR_CHEAT, "debug - disable trace query cache" );
static ConVar sv_querycache_job_graph( "sv_querycache_job_graph", "0", 0,
	"Query-cache maintenance: 0 legacy batch, 1 serial job graph, 2 pooled job graph.", true, 0,
	true, 2 );

static QueryCacheEntry_t *FindOrAllocateCacheEntry( QueryCacheKey_t const &entry )
{
	QueryCacheEntry_t *pFound = NULL;
	// see if we find it
	for( QueryCacheEntry_t *pNode = s_HashChains[entry.m_nHashIdx].m_pHead; pNode; pNode = pNode->m_pNext )
	{
		if ( pNode->m_QueryParams.Matches( &entry ) )
		{
			pFound = pNode;
			break;
		}
	}
	if (! pFound )
	{
		pFound = s_VictimList.RemoveHead();
		if ( ! pFound )
		{
			// randomly replace one
			pFound = s_QCache + s_nReplaceCtr;
			s_nReplaceCtr--;
			if ( s_nReplaceCtr < 0 )
				s_nReplaceCtr = QUERYCACHE_SIZE - 1;
			if ( pFound->m_QueryParams.m_Type != EQUERY_INVALID )
			{
				s_HashChains[pFound->m_QueryParams.m_nHashIdx].RemoveNode( pFound );
			}
		}
		pFound->m_QueryParams = entry;
		s_HashChains[pFound->m_QueryParams.m_nHashIdx].AddToHead( pFound );
		pFound->m_bSpeculativelyDone = false;
		pFound->IssueQuery();
	}
	else
	{
		if ( sv_disable_querycache.GetInt() || 
			 ( gpGlobals->curtime - pFound->m_flLastUpdateTime >= 
			   pFound->m_QueryParams.m_flMinimumUpdateInterval ) )
		{
			pFound->m_bSpeculativelyDone = false;
			pFound->IssueQuery();
		}
		else
		{
			if ( pFound->m_bSpeculativelyDone )
				s_SuccessfulSpeculatives++;
		}
		
	}
	return pFound;
}

static QueryCacheEntry_t *FindOrAllocateCacheEntry( EQueryType_t nType,
													CBaseEntity *pEntity1, CBaseEntity *pEntity2,
													EEntityOffsetMode_t nMode1, EEntityOffsetMode_t nMode2,
													unsigned int nTraceMask )
{
	QueryCacheKey_t entry;
	entry.m_Type = nType;
	entry.m_pEntities[0] = pEntity1;
	entry.m_pEntities[1] = pEntity2;
	entry.m_nOffsetMode[0] = nMode1;
	entry.m_nOffsetMode[1] = nMode2;
	entry.m_nTraceMask = nTraceMask;
	entry.m_nNumValidPoints = 2;
	entry.ComputeHashIndex();
	return FindOrAllocateCacheEntry( entry );
}

bool QueryCacheKey_t::Matches( QueryCacheKey_t const *pNode ) const
{
	if (
		( pNode->m_Type != m_Type ) ||
		( pNode->m_nTraceMask != m_nTraceMask ) ||
		( pNode->m_pTraceFilterFunction != m_pTraceFilterFunction ) ||
		( pNode->m_nNumValidPoints != m_nNumValidPoints ) || 
		( pNode->m_flMinimumUpdateInterval != m_flMinimumUpdateInterval )
		)
		return false;
	for( int i = 0; i < m_nNumValidPoints; i++ )
	{
		if (
			( pNode->m_pEntities[i] != m_pEntities[i] ) ||
			( pNode->m_nOffsetMode[i] != m_nOffsetMode[i] )
			)
			return false;
	}
	return true;
}

static void CalculateOffsettedPosition( CBaseEntity *pEntity, EEntityOffsetMode_t nMode, Vector *pVecOut  )
{
	switch( nMode )
	{
		case EOFFSET_MODE_WORLDSPACE_CENTER:
			*pVecOut = pEntity->WorldSpaceCenter();
			break;

		case EOFFSET_MODE_EYEPOSITION:
			*pVecOut = pEntity->EyePosition();
			break;

		case EOFFSET_MODE_NONE:
			pVecOut->Init();
			break;
	}
}



typedef QueryCacheMaintenance::Item_t<QueryCacheEntry_t> QueryCacheUpdateItem_t;
typedef QueryCacheMaintenance::Split_t<QueryCacheEntry_t> QueryCacheUpdateRecord_t;

static void ProcessQueryCacheUpdate( QueryCacheUpdateRecord_t &workItem )
{
	// The caller owns the cache for this entire synchronous batch. Workers only
	// read pinned metadata and write their own decision slots; they never call
	// entities, trace filters, or mutate hash/victim lists and shared counters.
	QueryCacheMaintenance::ClassifySplit( workItem );
}

static void IssueQueryAtTime( QueryCacheEntry_t *pEntry, float flCurTime );

struct QueryCacheRefresh_t
{
	// Entity positions and even the default trace filter call gameplay code.
	// The commit preserves the serial reference's callback order on this thread.
	void operator()( QueryCacheEntry_t *pEntry, float flCurTime ) const
	{
		IssueQueryAtTime( pEntry, flCurTime );
	}
};


#define N_WAYS_TO_SPLIT_CACHE_UPDATE 8

void UpdateQueryCache( void )
{
	// Gather only entry pointers; the cache remains borrowed and unchanged until
	// every decision job joins. Keep one bounded buffer, with no heap allocation.
	QueryCacheUpdateItem_t items[QUERYCACHE_SIZE];
	QueryCacheUpdateRecord_t workList[N_WAYS_TO_SPLIT_CACHE_UPDATE];
	const int nItems = QueryCacheMaintenance::Gather( s_HashChains, ARRAYSIZE( s_HashChains ),
		N_WAYS_TO_SPLIT_CACHE_UPDATE, gpGlobals->curtime, items, QUERYCACHE_SIZE, workList );
	if ( nItems < 0 )
	{
		Error( "Query-cache entries exceed fixed storage\n" );
		return;
	}
	if ( nItems == 0 )
		return;

	const int nMaxParallel = sv_disable_querycache.GetBool() ? 0 : INT_MAX;
	const int nGraphMode = sv_querycache_job_graph.GetInt();
	if ( nGraphMode == 0 )
	{
		ParallelProcess( "ProcessQueryCacheUpdate", workList, N_WAYS_TO_SPLIT_CACHE_UPDATE,
			ProcessQueryCacheUpdate, NULL, NULL, nMaxParallel );
	}
	else
	{
		const jobsystem::BatchMode mode = nGraphMode == 1 ? jobsystem::BatchMode::Serial :
			jobsystem::BatchMode::Parallel;
		if ( !JobGraphParallelProcess( "ProcessQueryCacheUpdate", workList,
				 N_WAYS_TO_SPLIT_CACHE_UPDATE, ProcessQueryCacheUpdate, NULL, NULL, nMaxParallel,
				 NULL, mode ) )
		{
			Error( "Invalid query-cache maintenance job graph batch\n" );
		}
	}

	// The owner performs all gameplay callbacks and authoritative mutation. No
	// model-cache lock is moved between threads. Invalid query handles enter the
	// victim list during commit, before the ordered prepend of expired entries.
	MDLCACHE_CRITICAL_SECTION();
	CUtlIntrusiveDListWithTailPtr<QueryCacheEntry_t> killed[N_WAYS_TO_SPLIT_CACHE_UPDATE];
	QueryCacheRefresh_t refresh;
	QueryCacheMaintenance::Commit( workList, N_WAYS_TO_SPLIT_CACHE_UPDATE, s_HashChains, killed,
		s_VictimList, EQUERY_INVALID, s_WastedSpeculativeUpdates, refresh );
}

void InvalidateQueryCache( void )
{
	s_VictimList.RemoveAll();
	for( int i = 0; i < ARRAYSIZE( s_HashChains); i++ )
		s_HashChains[i].RemoveAll();
	// now, invalidate all cache entries and add them to the victims
	for( int i = 0; i < ARRAYSIZE( s_QCache ); i++ )
	{
		s_QCache[i].m_QueryParams.m_Type = EQUERY_INVALID;
		s_VictimList.AddToHead( s_QCache + i );
	}
}


static void IssueQueryAtTime( QueryCacheEntry_t *pEntry, float flCurTime )
{
	QueryCacheKey_t &params = pEntry->m_QueryParams;
	for ( int i = 0; i < params.m_nNumValidPoints; ++i )
	{
		CBaseEntity *pEntity = params.m_pEntities[i];
		if (! pEntity )
		{
			params.m_Type = EQUERY_INVALID;
			s_HashChains[params.m_nHashIdx].RemoveNode( pEntry );
			s_VictimList.AddToHead( pEntry );
			return;
		}
		CalculateOffsettedPosition( pEntity, params.m_nOffsetMode[i], &params.m_Points[i] );
	}
	CTraceFilterSimple filter( params.m_pEntities[2], params.m_nCollisionGroup,
		params.m_pTraceFilterFunction );
	trace_t result;
	s_nNumCacheMisses++;
	UTIL_TraceLine( params.m_Points[0], params.m_Points[1], params.m_nTraceMask, &filter, &result );
	pEntry->m_bResult = !result.DidHit();
	pEntry->m_flLastUpdateTime = flCurTime;
}

void QueryCacheEntry_t::IssueQuery( void )
{
	IssueQueryAtTime( this, gpGlobals->curtime );
}


bool IsLineOfSightBetweenTwoEntitiesClear( CBaseEntity *pSrcEntity,
										   EEntityOffsetMode_t nSrcOffsetMode,
										   CBaseEntity *pDestEntity,
										   EEntityOffsetMode_t nDestOffsetMode,
										   CBaseEntity *pSkipEntity,
										   int nCollisionGroup,
										   unsigned int nTraceMask,
										   ShouldHitFunc_t pTraceFilterCallback,
										   float flMinimumUpdateInterval )
{
	QueryCacheKey_t entry;
	entry.m_Type = EQUERY_ENTITY_LOS_CHECK;
	entry.m_pEntities[0] = pSrcEntity;
	entry.m_pEntities[1] = pDestEntity;
	entry.m_pEntities[2] = pSkipEntity;
	entry.m_nOffsetMode[0] = nSrcOffsetMode;
	entry.m_nOffsetMode[1] = nDestOffsetMode;
	entry.m_nOffsetMode[2] = EOFFSET_MODE_NONE;
	entry.m_nTraceMask = nTraceMask;
	entry.m_nNumValidPoints = 3;
	entry.m_nCollisionGroup = nCollisionGroup;
	entry.m_pTraceFilterFunction = pTraceFilterCallback;
	entry.m_flMinimumUpdateInterval = flMinimumUpdateInterval;
	entry.ComputeHashIndex();

	s_nNumCacheQueries++;
	QueryCacheEntry_t *pNode = FindOrAllocateCacheEntry( entry );
	pNode->m_bUsedSinceUpdated = true;
	return pNode->m_bResult;
}


#if defined( CLIENT_DLL )
CON_COMMAND_F( cl_querycache_stats, "Display status of the query cache (client only)", FCVAR_CHEAT )
#else
CON_COMMAND( sv_querycache_stats, "Display status of the query cache (client only)" )
#endif
{
#ifndef CLIENT_DLL
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;
#endif

	Warning( "%d queries, %d misses (%d free) suc spec = %d wasted spec=%d\n",
			 s_nNumCacheQueries, s_nNumCacheMisses, s_VictimList.Count(),
			 s_SuccessfulSpeculatives, s_WastedSpeculativeUpdates );
}


