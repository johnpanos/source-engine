//========= Copyright 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Single-player shared memory registry (see public/ispsharedmemory.h),
//          ported from the CS:GO-era engine. Spaces are reference counted and
//          freed with their last reference.
//
//=============================================================================//

#include "ispsharedmemory.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "tier0/dbg.h"

#include "tier0/memdbgon.h"

class CSPSharedMemoryManager;

class CSPSharedMemory : public ISPSharedMemory
{
public:
	CSPSharedMemory( CSPSharedMemoryManager *pManager, const char *pszName, int nEntNum )
		: m_pManager( pManager ), m_Name( pszName ), m_nEntNum( nEntNum ), m_pMemory( NULL ), m_Size( 0 ), m_nRefCount( 0 )
	{
	}

	~CSPSharedMemory( void )
	{
		Assert( m_nRefCount == 0 );
		delete[] m_pMemory;
	}

	virtual bool Init( size_t iSize )
	{
		Assert( iSize != 0 );
		Assert( ( m_Size == 0 ) || ( m_Size == iSize ) );

		if ( m_Size != 0 )
			return false;

		m_pMemory = new uint8[iSize];
		m_Size = iSize;
		memset( m_pMemory, 0, iSize );
		return true;
	}

	virtual uint8 *Base( void )
	{
		Assert( m_pMemory != NULL );
		return m_pMemory;
	}

	virtual size_t Size( void ) { return m_Size; }

	virtual void AddRef( void ) { ++m_nRefCount; }
	virtual void Release( void );

	CSPSharedMemoryManager *m_pManager;
	CUtlString m_Name;
	int m_nEntNum;
	uint8 *m_pMemory;
	size_t m_Size;
	unsigned int m_nRefCount;
};

class CSPSharedMemoryManager : public ISPSharedMemoryManager
{
public:
	~CSPSharedMemoryManager( void )
	{
		for ( int i = m_SharedSpaces.Count(); --i >= 0; )
		{
			Warning( "Shared memory space %s (%i) still has %u references.\n",
				m_SharedSpaces[i]->m_Name.String(), m_SharedSpaces[i]->m_nEntNum, m_SharedSpaces[i]->m_nRefCount );
		}
		// Process exit: the game modules that held these are already unloaded.
		m_SharedSpaces.RemoveAll();
	}

	virtual ISPSharedMemory *GetSharedMemory( const char *szName, int nEntNum )
	{
		for ( int i = m_SharedSpaces.Count(); --i >= 0; )
		{
			if ( m_SharedSpaces[i]->m_nEntNum == nEntNum && m_SharedSpaces[i]->m_Name == szName )
			{
				m_SharedSpaces[i]->AddRef();
				return m_SharedSpaces[i];
			}
		}

		CSPSharedMemory *pNew = new CSPSharedMemory( this, szName, nEntNum );
		m_SharedSpaces.AddToTail( pNew );
		pNew->AddRef();
		return pNew;
	}

	void Remove( CSPSharedMemory *pSpace )
	{
		m_SharedSpaces.FindAndFastRemove( pSpace );
	}

private:
	CUtlVector<CSPSharedMemory *> m_SharedSpaces;
};

void CSPSharedMemory::Release( void )
{
	Assert( m_nRefCount > 0 );
	if ( --m_nRefCount == 0 )
	{
		m_pManager->Remove( this );
		delete this;
	}
}

EXPOSE_SINGLE_INTERFACE( CSPSharedMemoryManager, ISPSharedMemoryManager, VENGINE_SPSHAREDMEMORY_INTERFACE_VERSION );
