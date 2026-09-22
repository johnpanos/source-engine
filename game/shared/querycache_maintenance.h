//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Query-cache maintenance pass: gather, independent classification,
//          and ordered commit. Tier0-free C++11 so the conformance suite runs
//          the same code the game instantiates (querycache.cpp).
//
//=============================================================================//
#ifndef QUERYCACHE_MAINTENANCE_H
#define QUERYCACHE_MAINTENANCE_H
#ifdef _WIN32
#pragma once
#endif

// Observable contract, matching the original serial pass over hash chains:
//  - Chains are split into nSplits contiguous ranges of nChains / nSplits, the
//    last taking the remainder; entries are visited in chain, then list order.
//  - An entry used since its last update refreshes once its interval elapses
//    (>=); an unused entry expires once the interval is exceeded (>), and counts
//    as wasted if it was speculatively refreshed.
//  - Refresh callbacks (entity positions, trace filters) run on the calling
//    thread in visiting order. A refresh may itself retire its entry to the
//    victim list immediately (invalid entity handle).
//  - Each split's expired entries are prepended to its killed list as they are
//    committed; after every split commits, killed lists are prepended to the
//    victim list in split order.
// Classification only reads entry fields and writes the entry's own item slot,
// so it may run concurrently across splits. Everything else is owner-thread.
namespace QueryCacheMaintenance
{

enum Action_t
{
	ACTION_KEEP,
	ACTION_REFRESH,
	ACTION_EXPIRE,
	ACTION_EXPIRE_WASTED,
};

inline Action_t Classify( bool bUsedSinceUpdated, bool bSpeculativelyDone, float flElapsed,
	float flMinimumUpdateInterval )
{
	if ( bUsedSinceUpdated )
		return flElapsed >= flMinimumUpdateInterval ? ACTION_REFRESH : ACTION_KEEP;
	if ( flElapsed > flMinimumUpdateInterval )
		return bSpeculativelyDone ? ACTION_EXPIRE_WASTED : ACTION_EXPIRE;
	return ACTION_KEEP;
}

template <class Entry> struct Item_t
{
	Entry *m_pEntry;
	Action_t m_Action;
};

template <class Entry> struct Split_t
{
	Item_t<Entry> *m_pItems;
	int m_nItems;
	float m_flCurTime;
};

// Records every entry in pChains[0..nChains) into pItems, partitioned into
// nSplits contiguous chain ranges. Returns the total item count, or -1 if the
// cache holds more than nCapacity entries (nothing may be classified then).
template <class Entry, class Chain>
int Gather( const Chain *pChains, int nChains, int nSplits, float flCurTime,
	Item_t<Entry> *pItems, int nCapacity, Split_t<Entry> *pSplits )
{
	int nItems = 0;
	const int nChainsPerSplit = nChains / nSplits;
	for ( int i = 0; i < nSplits; ++i )
	{
		Split_t<Entry> &split = pSplits[i];
		split.m_pItems = pItems + nItems;
		split.m_nItems = 0;
		split.m_flCurTime = flCurTime;
		// Equal ranges; the last split also takes the remainder.
		const int nFirstChain = i * nChainsPerSplit;
		const int nLastChain = i == nSplits - 1 ? nChains : nFirstChain + nChainsPerSplit;
		for ( int chain = nFirstChain; chain < nLastChain; ++chain )
		{
			for ( Entry *pEntry = pChains[chain].m_pHead; pEntry; pEntry = pEntry->m_pNext )
			{
				if ( nItems == nCapacity )
					return -1;
				pItems[nItems].m_pEntry = pEntry;
				pItems[nItems].m_Action = ACTION_KEEP;
				++nItems;
				++split.m_nItems;
			}
		}
	}
	return nItems;
}

// Worker-safe: reads the split's entries and writes only their item slots.
template <class Entry> void ClassifySplit( Split_t<Entry> &split )
{
	for ( int i = 0; i < split.m_nItems; ++i )
	{
		Item_t<Entry> &item = split.m_pItems[i];
		const Entry &entry = *item.m_pEntry;
		item.m_Action = Classify( entry.m_bUsedSinceUpdated, entry.m_bSpeculativelyDone,
			split.m_flCurTime - entry.m_flLastUpdateTime,
			entry.m_QueryParams.m_flMinimumUpdateInterval );
	}
}

// Owner thread only. refresh( pEntry, flCurTime ) issues the query. pKilled has
// nSplits entries and must start empty. PrependDListWithTailToDList is found
// with the list types (tier1 in the game, a test double in the suite).
template <class Entry, class ChainList, class KilledList, class VictimList, class Type,
	class Refresh>
void Commit( Split_t<Entry> *pSplits, int nSplits, ChainList *pChains, KilledList *pKilled,
	VictimList &victims, Type invalidType, int &nWastedSpeculativeUpdates, Refresh &refresh )
{
	for ( int i = 0; i < nSplits; ++i )
	{
		const Split_t<Entry> &split = pSplits[i];
		for ( int j = 0; j < split.m_nItems; ++j )
		{
			const Item_t<Entry> &item = split.m_pItems[j];
			Entry *pEntry = item.m_pEntry;
			if ( item.m_Action == ACTION_REFRESH )
			{
				refresh( pEntry, split.m_flCurTime );
				pEntry->m_bUsedSinceUpdated = false;
				pEntry->m_bSpeculativelyDone = true;
			}
			else if ( item.m_Action == ACTION_EXPIRE || item.m_Action == ACTION_EXPIRE_WASTED )
			{
				if ( item.m_Action == ACTION_EXPIRE_WASTED )
					++nWastedSpeculativeUpdates;
				pEntry->m_QueryParams.m_Type = invalidType;
				pChains[pEntry->m_QueryParams.m_nHashIdx].RemoveNode( pEntry );
				pKilled[i].AddToHead( pEntry );
			}
		}
	}
	for ( int i = 0; i < nSplits; ++i )
		PrependDListWithTailToDList( pKilled[i], victims );
}

} // namespace QueryCacheMaintenance

#endif // QUERYCACHE_MAINTENANCE_H
