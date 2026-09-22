#ifndef PHYSICS_OBJECT_HASH_H
#define PHYSICS_OBJECT_HASH_H

#include "vphysics/object_hash.h"
#include "vphysics/collision_set.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

// Object-pair hash and collision set for the Box3D provider.
//
// These are pure bookkeeping contracts (which object pairs / index pairs are
// allowed to collide) with no simulation dependency, so they are implemented
// fully rather than stubbed: game code stores its collision-exception hash here
// during init and queries it on every entity trace. A null hash (the old stub)
// is dereferenced unconditionally by CTraceFilterEntity, so a correct, live
// implementation is required for a level to run.

//-----------------------------------------------------------------------------
// Symmetric set of object pairs, with per-object adjacency for iteration.
//-----------------------------------------------------------------------------
class CObjectPairHashBox3D : public IPhysicsObjectPairHash
{
public:
	virtual ~CObjectPairHashBox3D() {}

	virtual void AddObjectPair( void *pObject0, void *pObject1 ) override
	{
		if ( IsObjectPairInHash( pObject0, pObject1 ) )
			return;
		m_adjacency[pObject0].push_back( pObject1 );
		if ( pObject0 != pObject1 )
			m_adjacency[pObject1].push_back( pObject0 );
	}

	virtual void RemoveObjectPair( void *pObject0, void *pObject1 ) override
	{
		EraseNeighbor( pObject0, pObject1 );
		if ( pObject0 != pObject1 )
			EraseNeighbor( pObject1, pObject0 );
	}

	virtual bool IsObjectPairInHash( void *pObject0, void *pObject1 ) override
	{
		auto it = m_adjacency.find( pObject0 );
		if ( it == m_adjacency.end() )
			return false;
		for ( void *neighbor : it->second )
		{
			if ( neighbor == pObject1 )
				return true;
		}
		return false;
	}

	virtual void RemoveAllPairsForObject( void *pObject0 ) override
	{
		auto it = m_adjacency.find( pObject0 );
		if ( it == m_adjacency.end() )
			return;
		// Drop the back-references held by each neighbor, then this object.
		for ( void *neighbor : it->second )
		{
			if ( neighbor != pObject0 )
				EraseNeighbor( neighbor, pObject0 );
		}
		m_adjacency.erase( it );
	}

	virtual bool IsObjectInHash( void *pObject0 ) override
	{
		auto it = m_adjacency.find( pObject0 );
		return it != m_adjacency.end() && !it->second.empty();
	}

	virtual int GetPairCountForObject( void *pObject0 ) override
	{
		auto it = m_adjacency.find( pObject0 );
		return it == m_adjacency.end() ? 0 : (int)it->second.size();
	}

	virtual int GetPairListForObject( void *pObject0, int nMaxCount, void **ppObjectList ) override
	{
		auto it = m_adjacency.find( pObject0 );
		if ( it == m_adjacency.end() )
			return 0;
		int count = 0;
		for ( void *neighbor : it->second )
		{
			if ( count >= nMaxCount )
				break;
			ppObjectList[count++] = neighbor;
		}
		return count;
	}

private:
	void EraseNeighbor( void *pObject, void *pNeighbor )
	{
		auto it = m_adjacency.find( pObject );
		if ( it == m_adjacency.end() )
			return;
		std::vector<void *> &list = it->second;
		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			if ( list[i] == pNeighbor )
			{
				list[i] = list.back();
				list.pop_back();
				break;
			}
		}
		if ( list.empty() )
			m_adjacency.erase( it );
	}

	std::unordered_map<void *, std::vector<void *>> m_adjacency;
};

//-----------------------------------------------------------------------------
// Collision set: which index pairs collide. Defaults to all disabled, matching
// the documented IPhysicsCollisionSet contract.
//-----------------------------------------------------------------------------
class CCollisionSetBox3D : public IPhysicsCollisionSet
{
public:
	virtual ~CCollisionSetBox3D() {}
	virtual void EnableCollisions( int index0, int index1 ) override { m_enabled.insert( Key( index0, index1 ) ); }
	virtual void DisableCollisions( int index0, int index1 ) override { m_enabled.erase( Key( index0, index1 ) ); }
	virtual bool ShouldCollide( int index0, int index1 ) override { return m_enabled.count( Key( index0, index1 ) ) != 0; }

private:
	static unsigned long long Key( int a, int b )
	{
		if ( a > b )
		{
			int t = a; a = b; b = t;
		}
		return ( (unsigned long long)(unsigned int)a << 32 ) | (unsigned int)b;
	}

	std::unordered_set<unsigned long long> m_enabled;
};

#endif // PHYSICS_OBJECT_HASH_H
