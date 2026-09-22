//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Query-cache maintenance equivalence harness (RFC 0003, R30).
//
// Two independent copies of a query cache are driven by the same generated
// workload. The reference copy runs a transcription of the original serial
// UpdateQueryCache/ProcessQueryCacheUpdate (4e4039d7, sv_disable_querycache
// ordering). The candidate copy runs game/shared/querycache_maintenance.h, the
// kernel querycache.cpp instantiates, with classification executed by the
// legacy loop, the deterministic serial graph, or the pooled graph on real
// threads. After every frame the complete observable state is compared:
// entry fields (bit-exact times), hash-chain and victim-list order and links,
// counters, and the ordered log of refresh callbacks.
//
// Code the migration did not change (lookup/allocation, IssueQuery, the
// intrusive-list operations) is transcribed once here and shared by both
// copies, so a divergence can only come from the maintenance pass under test.
//
//=============================================================================//
#ifndef QUERYCACHE_HARNESS_H
#define QUERYCACHE_HARNESS_H

#include "game/shared/querycache_maintenance.h"
#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace qc
{

enum
{
	TYPE_INVALID = 0,
	TYPE_TRACELINE,
	TYPE_ENTITY_LOS_CHECK
};
enum
{
	OFFSET_WORLDSPACE_CENTER,
	OFFSET_EYEPOSITION,
	OFFSET_NONE
};
constexpr int MAXPNTS = 3;

struct Vec
{
	float x = 0, y = 0, z = 0;
};

// Entity handles are nonzero ids; 0 is an empty handle. A deleted entity's
// handle keeps its value (EHANDLE equality) but resolves to null.
struct Key
{
	int m_Type = TYPE_INVALID;
	int m_nNumValidPoints = 0;
	Vec m_Points[MAXPNTS];
	int m_pEntities[MAXPNTS] = { 0, 0, 0 };
	int m_nOffsetMode[MAXPNTS] = { 0, 0, 0 };
	unsigned m_nTraceMask = 0;
	unsigned m_nHashIdx = 0;
	int m_nCollisionGroup = 0;
	int m_nFilter = 0;
	float m_flMinimumUpdateInterval = 0;

	bool Matches( const Key &other ) const
	{
		if ( other.m_Type != m_Type || other.m_nTraceMask != m_nTraceMask ||
			 other.m_nFilter != m_nFilter || other.m_nNumValidPoints != m_nNumValidPoints ||
			 other.m_flMinimumUpdateInterval != m_flMinimumUpdateInterval )
			return false;
		for ( int i = 0; i < m_nNumValidPoints; ++i )
		{
			if ( other.m_pEntities[i] != m_pEntities[i] ||
				 other.m_nOffsetMode[i] != m_nOffsetMode[i] )
				return false;
		}
		return true;
	}
};

struct Entry
{
	Entry *m_pNext = nullptr;
	Entry *m_pPrev = nullptr;
	Key m_QueryParams;
	float m_flLastUpdateTime = 0;
	bool m_bUsedSinceUpdated = false;
	bool m_bSpeculativelyDone = false;
	bool m_bResult = false;
};

// Semantics copied from public/tier1/utlintrusivelist.h (AddToDHead,
// RemoveFromDList, AddToDHeadWithTailPtr, RemoveHead, PrependDListWithTailToDList).
struct DList
{
	Entry *m_pHead = nullptr;

	void AddToHead( Entry *which )
	{
		which->m_pNext = m_pHead;
		if ( m_pHead )
			m_pHead->m_pPrev = which;
		which->m_pPrev = nullptr;
		m_pHead = which;
	}
	void RemoveNode( Entry *which )
	{
		if ( which->m_pPrev )
		{
			which->m_pPrev->m_pNext = which->m_pNext;
			if ( which->m_pNext )
				which->m_pNext->m_pPrev = which->m_pPrev;
		}
		else if ( m_pHead == which )
		{
			m_pHead = which->m_pNext;
			if ( m_pHead )
				m_pHead->m_pPrev = nullptr;
		}
		which->m_pNext = which->m_pPrev = nullptr;
	}
	Entry *RemoveHead()
	{
		Entry *ret = m_pHead;
		if ( ret )
		{
			m_pHead = ret->m_pNext;
			if ( m_pHead )
				m_pHead->m_pPrev = nullptr;
		}
		return ret;
	}
	void RemoveAll() { m_pHead = nullptr; }
};

struct DListWithTail : DList
{
	Entry *m_pTailPtr = nullptr;

	void AddToHead( Entry *which )
	{
		which->m_pNext = m_pHead;
		if ( m_pHead )
			m_pHead->m_pPrev = which;
		else
			m_pTailPtr = which;
		which->m_pPrev = nullptr;
		m_pHead = which;
	}
};

inline void PrependDListWithTailToDList( DListWithTail &src, DList &dest )
{
	if ( src.m_pHead )
	{
		src.m_pTailPtr->m_pNext = dest.m_pHead;
		if ( dest.m_pHead )
			dest.m_pHead->m_pPrev = src.m_pTailPtr;
		dest.m_pHead = src.m_pHead;
	}
}

inline uint32_t Mix( uint32_t h )
{
	h ^= h >> 16;
	h *= 0x7feb352du;
	h ^= h >> 15;
	h *= 0x846ca68bu;
	h ^= h >> 16;
	return h;
}

inline uint32_t FloatBits( float f )
{
	uint32_t bits;
	std::memcpy( &bits, &f, sizeof( bits ) );
	return bits;
}

// Entities move every frame and may be deleted; traces are a pure function of
// the endpoints and mask, so both copies observe identical gameplay answers.
struct World
{
	int nEntities = 0;
	std::vector<bool> alive;
	uint32_t frame = 0;

	bool Resolve( int handle ) const
	{
		return handle > 0 && handle <= nEntities && alive[handle];
	}
	Vec Position( int handle, int mode ) const
	{
		Vec v;
		const uint32_t h = Mix( uint32_t( handle ) * 2654435761u + frame / 3 );
		v.x = float( h & 1023 ) * 0.5f;
		v.y = float( ( h >> 10 ) & 1023 ) * 0.5f;
		v.z = float( ( h >> 20 ) & 255 ) + ( mode == OFFSET_EYEPOSITION ? 64.0f : 0.0f );
		return v;
	}
	bool TraceClear( const Vec &a, const Vec &b, unsigned mask, int filter ) const
	{
		uint32_t h = FloatBits( a.x ) ^ Mix( FloatBits( a.y ) ) ^ Mix( FloatBits( a.z ) + 1 );
		h = Mix( h ^ FloatBits( b.x ) ^ Mix( FloatBits( b.y ) + 2 ) ^ Mix( FloatBits( b.z ) + 3 ) );
		h = Mix( h ^ mask ^ uint32_t( filter ) * 97u );
		return ( h & 3 ) != 0;
	}
};

struct Callback
{
	int entry;
	uint32_t time;
	bool invalidated;
};

struct Cache
{
	std::vector<Entry> entries;
	std::vector<DList> chains;
	DList victims;
	int nReplaceCtr = 0;
	int nQueries = 0;
	int nMisses = 0;
	int nSuccessfulSpeculatives = 0;
	int nWasted = 0;
	float curtime = 0;
	const World *world = nullptr;
	std::vector<Callback> log;

	void Init( int nSize, int nChains, const World &w )
	{
		entries.assign( nSize, Entry() );
		chains.assign( nChains, DList() );
		victims.RemoveAll();
		nReplaceCtr = nSize - 1;
		world = &w;
		// InvalidateQueryCache
		for ( int i = 0; i < nSize; ++i )
		{
			entries[i].m_QueryParams.m_Type = TYPE_INVALID;
			victims.AddToHead( &entries[i] );
		}
	}

	int Index( const Entry *e ) const { return int( e - entries.data() ); }

	// Unchanged IssueQueryAtTime (reference: IssueQuery at gpGlobals->curtime).
	void IssueQuery( Entry *e, float flTime )
	{
		Key &params = e->m_QueryParams;
		for ( int i = 0; i < params.m_nNumValidPoints; ++i )
		{
			if ( !world->Resolve( params.m_pEntities[i] ) )
			{
				params.m_Type = TYPE_INVALID;
				chains[params.m_nHashIdx].RemoveNode( e );
				victims.AddToHead( e );
				log.push_back( { Index( e ), FloatBits( flTime ), true } );
				return;
			}
			params.m_Points[i] = world->Position( params.m_pEntities[i], params.m_nOffsetMode[i] );
		}
		++nMisses;
		e->m_bResult = world->TraceClear( params.m_Points[0], params.m_Points[1],
			params.m_nTraceMask, params.m_nFilter );
		e->m_flLastUpdateTime = flTime;
		log.push_back( { Index( e ), FloatBits( flTime ), false } );
	}

	// Unchanged FindOrAllocateCacheEntry + IsLineOfSightBetweenTwoEntitiesClear.
	bool Lookup( Key key )
	{
		key.m_Type = TYPE_ENTITY_LOS_CHECK;
		key.m_nNumValidPoints = 3;
		unsigned h = unsigned( key.m_Type );
		for ( int i = 0; i < key.m_nNumValidPoints; ++i )
			h += unsigned( key.m_pEntities[i] ) + 0x40u; // + address term, fixed here
		h += FloatBits( key.m_flMinimumUpdateInterval ) + key.m_nTraceMask;
		key.m_nHashIdx = h % unsigned( chains.size() );
		++nQueries;

		Entry *found = nullptr;
		for ( Entry *node = chains[key.m_nHashIdx].m_pHead; node; node = node->m_pNext )
		{
			if ( node->m_QueryParams.Matches( key ) )
			{
				found = node;
				break;
			}
		}
		if ( !found )
		{
			found = victims.RemoveHead();
			if ( !found )
			{
				found = &entries[nReplaceCtr];
				if ( --nReplaceCtr < 0 )
					nReplaceCtr = int( entries.size() ) - 1;
				if ( found->m_QueryParams.m_Type != TYPE_INVALID )
					chains[found->m_QueryParams.m_nHashIdx].RemoveNode( found );
			}
			found->m_QueryParams = key;
			chains[key.m_nHashIdx].AddToHead( found );
			found->m_bSpeculativelyDone = false;
			IssueQuery( found, curtime );
		}
		else if ( curtime - found->m_flLastUpdateTime >=
				  found->m_QueryParams.m_flMinimumUpdateInterval )
		{
			found->m_bSpeculativelyDone = false;
			IssueQuery( found, curtime );
		}
		else if ( found->m_bSpeculativelyDone )
		{
			++nSuccessfulSpeculatives;
		}
		found->m_bUsedSinceUpdated = true;
		return found->m_bResult;
	}
};

// ---------------------------------------------------------------------------
// Reference: original serial maintenance pass, transcribed.
struct LegacyRecord
{
	int m_nStartHashChain;
	int m_nNumHashChainsToUpdate;
	DListWithTail m_KilledList;
};

inline void LegacyProcess( Cache &c, LegacyRecord &workItem )
{
	const float flCurTime = c.curtime;
	for ( int i = 0; i < workItem.m_nNumHashChainsToUpdate; i++ )
	{
		Entry *pNext;
		for ( Entry *pEntry = c.chains[i + workItem.m_nStartHashChain].m_pHead; pEntry;
			  pEntry = pNext )
		{
			pNext = pEntry->m_pNext;
			if ( pEntry->m_bUsedSinceUpdated )
			{
				if ( flCurTime - pEntry->m_flLastUpdateTime >=
					 pEntry->m_QueryParams.m_flMinimumUpdateInterval )
				{
					c.IssueQuery( pEntry, c.curtime );
					pEntry->m_bUsedSinceUpdated = false;
					pEntry->m_bSpeculativelyDone = true;
				}
			}
			else
			{
				if ( flCurTime - pEntry->m_flLastUpdateTime >
					 pEntry->m_QueryParams.m_flMinimumUpdateInterval )
				{
					if ( pEntry->m_bSpeculativelyDone && ( !pEntry->m_bUsedSinceUpdated ) )
						c.nWasted++;
					pEntry->m_QueryParams.m_Type = TYPE_INVALID;
					c.chains[pEntry->m_QueryParams.m_nHashIdx].RemoveNode( pEntry );
					workItem.m_KilledList.AddToHead( pEntry );
				}
			}
		}
	}
}

inline void LegacyUpdate( Cache &c, int nSplits )
{
	std::vector<LegacyRecord> workList( nSplits );
	const int nChains = int( c.chains.size() );
	int nCurEntry = 0;
	for ( int i = 0; i < nSplits; i++ )
	{
		workList[i].m_nStartHashChain = nCurEntry;
		if ( i != nSplits - 1 )
			workList[i].m_nNumHashChainsToUpdate = nChains / nSplits;
		else
			workList[i].m_nNumHashChainsToUpdate = nChains - nCurEntry;
		nCurEntry += nChains / nSplits;
	}
	// sv_disable_querycache ordering: every record on the caller, ascending.
	for ( int i = 0; i < nSplits; i++ )
		LegacyProcess( c, workList[i] );
	for ( int i = 0; i < nSplits; i++ )
		PrependDListWithTailToDList( workList[i].m_KilledList, c.victims );
}

// ---------------------------------------------------------------------------
// Candidate: the production kernel.
enum class Mode
{
	LegacyLoop,
	SerialGraph,
	PooledGraph
};

inline const char *ModeName( Mode mode )
{
	switch ( mode )
	{
	case Mode::LegacyLoop: return "legacy-loop";
	case Mode::SerialGraph: return "serial-graph";
	case Mode::PooledGraph: return "pooled-graph";
	}
	return "?";
}

// Real threads per dispatch; the return is the join barrier.
class ThreadBackend final : public jobsystem::IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ) {}
	void ParallelFor( int count, const std::function<void( int )> &body ) override
	{
		std::vector<std::thread> threads;
		for ( int index = 1; index < count; ++index )
			threads.emplace_back( body, index );
		if ( count > 0 )
			body( 0 );
		for ( auto &thread : threads )
			thread.join();
	}
	int WorkerCount() const override { return m_workers; }

private:
	int m_workers;
};

typedef QueryCacheMaintenance::Item_t<Entry> Item;
typedef QueryCacheMaintenance::Split_t<Entry> Split;

struct Refresh
{
	Cache *cache;
	void operator()( Entry *e, float t ) const { cache->IssueQuery( e, t ); }
};

struct ClassifyContext
{
	Split *splits;
	void ( *classify )( Split & );
};

inline void ClassifyIndex( void *context, unsigned index )
{
	ClassifyContext &ctx = *static_cast<ClassifyContext *>( context );
	ctx.classify( ctx.splits[index] );
}

inline void KernelClassify( Split &split ) { QueryCacheMaintenance::ClassifySplit( split ); }

// Returns false only if the batch could not be constructed (a harness error).
// classify and commit are replaceable so the sensitivity suite can prove the
// comparator rejects plausible migration defects.
template <class CommitFn>
bool KernelUpdate( Cache &c, int nSplits, Mode mode, jobsystem::IWorkerBackend *backend,
	void ( *classify )( Split & ), CommitFn commit )
{
	std::vector<Item> items( c.entries.size() );
	std::vector<Split> splits( nSplits );
	const int nItems = QueryCacheMaintenance::Gather( c.chains.data(), int( c.chains.size() ),
		nSplits, c.curtime, items.data(), int( items.size() ), splits.data() );
	if ( nItems < 0 )
		return false;
	if ( nItems == 0 )
		return true;
	ClassifyContext ctx = { splits.data(), classify };
	if ( mode == Mode::LegacyLoop )
	{
		for ( int i = 0; i < nSplits; ++i )
			classify( splits[i] );
	}
	else
	{
		jobsystem::BatchDesc desc;
		desc.name = "ProcessQueryCacheUpdate";
		desc.context = &ctx;
		desc.count = unsigned( nSplits );
		desc.process = &ClassifyIndex;
		desc.maxParticipants = 0x7fffffffu;
		const jobsystem::BatchMode batchMode = mode == Mode::SerialGraph ?
			jobsystem::BatchMode::Serial : jobsystem::BatchMode::Parallel;
		if ( !jobsystem::ExecuteParallelBatch( desc, backend, batchMode ) )
			return false;
	}
	commit( c, splits.data(), nSplits );
	return true;
}

inline void KernelCommit( Cache &c, Split *splits, int nSplits )
{
	std::vector<DListWithTail> killed( nSplits );
	Refresh refresh = { &c };
	QueryCacheMaintenance::Commit( splits, nSplits, c.chains.data(), killed.data(), c.victims,
		int( TYPE_INVALID ), c.nWasted, refresh );
}

// ---------------------------------------------------------------------------
// Observable-state comparison. Returns an empty string when equal, else the
// first divergence.
inline std::string ListOrder( const Cache &c, const DList &list, bool &linksOk )
{
	std::string out;
	const Entry *prev = nullptr;
	int guard = 0;
	for ( const Entry *e = list.m_pHead; e; e = e->m_pNext )
	{
		if ( e->m_pPrev != prev || ++guard > int( c.entries.size() ) )
		{
			linksOk = false;
			break;
		}
		out += std::to_string( c.Index( e ) );
		out += ',';
		prev = e;
	}
	return out;
}

inline std::string Compare( const Cache &a, const Cache &b )
{
	char buf[256];
	if ( a.nQueries != b.nQueries || a.nMisses != b.nMisses ||
		 a.nSuccessfulSpeculatives != b.nSuccessfulSpeculatives || a.nWasted != b.nWasted ||
		 a.nReplaceCtr != b.nReplaceCtr )
	{
		std::snprintf( buf, sizeof( buf ),
			"counters queries %d/%d misses %d/%d spec %d/%d wasted %d/%d replace %d/%d",
			a.nQueries, b.nQueries, a.nMisses, b.nMisses, a.nSuccessfulSpeculatives,
			b.nSuccessfulSpeculatives, a.nWasted, b.nWasted, a.nReplaceCtr, b.nReplaceCtr );
		return buf;
	}
	if ( a.log.size() != b.log.size() )
	{
		std::snprintf( buf, sizeof( buf ), "callback count %zu/%zu", a.log.size(), b.log.size() );
		return buf;
	}
	for ( size_t i = 0; i < a.log.size(); ++i )
	{
		const Callback &x = a.log[i], &y = b.log[i];
		if ( x.entry != y.entry || x.time != y.time || x.invalidated != y.invalidated )
		{
			std::snprintf( buf, sizeof( buf ), "callback %zu: entry %d/%d time %08x/%08x inv %d/%d",
				i, x.entry, y.entry, x.time, y.time, x.invalidated, y.invalidated );
			return buf;
		}
	}
	for ( size_t i = 0; i < a.entries.size(); ++i )
	{
		const Entry &x = a.entries[i], &y = b.entries[i];
		const Key &p = x.m_QueryParams, &q = y.m_QueryParams;
		bool same = p.m_Type == q.m_Type && p.m_nHashIdx == q.m_nHashIdx &&
			x.m_bUsedSinceUpdated == y.m_bUsedSinceUpdated &&
			x.m_bSpeculativelyDone == y.m_bSpeculativelyDone && x.m_bResult == y.m_bResult &&
			FloatBits( x.m_flLastUpdateTime ) == FloatBits( y.m_flLastUpdateTime ) &&
			p.Matches( q );
		for ( int k = 0; same && k < MAXPNTS; ++k )
		{
			same = FloatBits( p.m_Points[k].x ) == FloatBits( q.m_Points[k].x ) &&
				FloatBits( p.m_Points[k].y ) == FloatBits( q.m_Points[k].y ) &&
				FloatBits( p.m_Points[k].z ) == FloatBits( q.m_Points[k].z );
		}
		if ( !same )
		{
			std::snprintf( buf, sizeof( buf ), "entry %zu fields differ", i );
			return buf;
		}
	}
	for ( size_t i = 0; i < a.chains.size(); ++i )
	{
		bool okA = true, okB = true;
		if ( ListOrder( a, a.chains[i], okA ) != ListOrder( b, b.chains[i], okB ) || !okA || !okB )
		{
			std::snprintf( buf, sizeof( buf ), "hash chain %zu order/links differ", i );
			return buf;
		}
	}
	bool okA = true, okB = true;
	if ( ListOrder( a, a.victims, okA ) != ListOrder( b, b.victims, okB ) || !okA || !okB )
		return "victim list order/links differ";
	return std::string();
}

// ---------------------------------------------------------------------------
struct Config
{
	int nSize;
	int nChains;
	int nSplits;
	int nEntities;
	int nFrames;
	int nLookupsPerFrame;
};

struct Workload
{
	uint32_t state;
	uint32_t Next()
	{
		state = Mix( state + 0x9e3779b9u );
		return state;
	}
};

inline Key MakeKey( Workload &w, int nEntities )
{
	// Production default 0.2 plus exactly representable intervals, so tick-
	// quantized time reaches the refresh/expiry boundaries exactly.
	static const float kIntervals[] = { 0.1f, 0.2f, 0.125f, 0.25f };
	Key key;
	key.m_pEntities[0] = 1 + int( w.Next() % uint32_t( nEntities ) );
	key.m_pEntities[1] = 1 + int( w.Next() % uint32_t( nEntities ) );
	// Mostly valid skip entities; an empty skip handle exercises the unchanged
	// invalidate-on-issue path (third point resolves to null).
	key.m_pEntities[2] = ( w.Next() % 16 ) ? 1 + int( w.Next() % uint32_t( nEntities ) ) : 0;
	key.m_nOffsetMode[0] = int( w.Next() % 2 );
	key.m_nOffsetMode[1] = int( w.Next() % 2 );
	key.m_nOffsetMode[2] = OFFSET_NONE;
	key.m_nTraceMask = ( w.Next() % 4 ) ? 0x4600400bu : 0x0000400bu;
	key.m_nCollisionGroup = int( w.Next() % 3 );
	key.m_nFilter = int( w.Next() % 2 );
	key.m_flMinimumUpdateInterval = kIntervals[w.Next() % 4];
	return key;
}

// Runs one seed; update( cache ) performs the candidate maintenance pass.
// Returns the number of frames compared, or -1 with *pWhy set on divergence.
template <class Update>
int RunScenario( const Config &cfg, uint32_t seed, Update update, std::string *pWhy,
	int *pFrame )
{
	World worldA, worldB;
	for ( World *w : { &worldA, &worldB } )
	{
		w->nEntities = cfg.nEntities;
		w->alive.assign( cfg.nEntities + 1, true );
	}
	Cache ref, cand;
	ref.Init( cfg.nSize, cfg.nChains, worldA );
	cand.Init( cfg.nSize, cfg.nChains, worldB );
	Workload wl = { seed };
	std::vector<Key> pool( size_t( cfg.nSize ) * 3 / 2 );
	for ( Key &key : pool )
		key = MakeKey( wl, cfg.nEntities );
	// Time advances in whole 1/64 s ticks (exact in float), with jitter and
	// occasional long hitches that expire many entries at once.
	int tick = 64;
	for ( int frame = 0; frame < cfg.nFrames; ++frame )
	{
		tick += ( wl.Next() % 23 ) ? 1 + int( wl.Next() % 4 ) : 25 + int( wl.Next() % 45 );
		const float curtime = float( tick ) * ( 1.0f / 64.0f );
		const uint32_t kill = wl.Next();
		const uint32_t revive = wl.Next();
		for ( World *w : { &worldA, &worldB } )
		{
			w->frame = uint32_t( frame );
			if ( kill % 5 == 0 )
				w->alive[1 + kill / 5 % uint32_t( cfg.nEntities )] = false;
			if ( revive % 7 == 0 )
				w->alive[1 + revive / 7 % uint32_t( cfg.nEntities )] = true;
		}
		ref.curtime = cand.curtime = curtime;
		const int nLookups = int( wl.Next() % uint32_t( cfg.nLookupsPerFrame + 1 ) );
		for ( int i = 0; i < nLookups; ++i )
		{
			// Half the lookups hit a hot eighth of the pool, so keys recur while
			// cached (hits, successful speculation) as AI queries do.
			const uint32_t pick = wl.Next();
			const size_t hot = pool.size() / 8 + 1;
			const Key &key = pool[( pick & 1 ) ? ( pick >> 1 ) % hot : ( pick >> 1 ) % pool.size()];
			const bool a = ref.Lookup( key );
			const bool b = cand.Lookup( key );
			if ( a != b )
			{
				*pWhy = "lookup result differs";
				*pFrame = frame;
				return -1;
			}
		}
		LegacyUpdate( ref, cfg.nSplits );
		if ( !update( cand ) )
		{
			*pWhy = "candidate batch construction failed";
			*pFrame = frame;
			return -1;
		}
		const std::string diff = Compare( ref, cand );
		if ( !diff.empty() )
		{
			*pWhy = diff;
			*pFrame = frame;
			return -1;
		}
	}
	return cfg.nFrames;
}

} // namespace qc

#endif // QUERYCACHE_HARNESS_H
