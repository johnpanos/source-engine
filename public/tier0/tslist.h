//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// LIFO from disassembly of Windows API and http://perso.wanadoo.fr/gmem/evenements/jim2002/articles/L17_Fober.pdf
// FIFO from http://perso.wanadoo.fr/gmem/evenements/jim2002/articles/L17_Fober.pdf
//
//=============================================================================

#ifndef TSLIST_H
#define TSLIST_H

#if defined( _WIN32 )
#pragma once
// Suppress this spurious warning:
// warning C4700: uninitialized local variable 'oldHead' used
#pragma warning( push )
#pragma warning( disable : 4700 )
#endif

#if defined( USE_NATIVE_SLIST )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#include "tier0/memalloc.h"
#include "tier0/memdbgoff.h"


//-----------------------------------------------------------------------------

#if defined( PLATFORM_64BITS )

#if defined (PLATFORM_WINDOWS) 
//typedef __m128i int128;
//inline int128 int128_zero()	{ return _mm_setzero_si128(); }
#else
typedef __int128_t int128;
#define int128_zero() 0
#endif// PLATFORM_WINDOWS

#define TSLIST_HEAD_ALIGNMENT 16
#define TSLIST_NODE_ALIGNMENT 16

#ifdef POSIX
inline bool ThreadInterlockedAssignIf128( int128 volatile * pDest, const int128 &value, const int128 &comparand ) 
{
    // We do not want the original comparand modified by the swap
    // so operate on a local copy.
    int128 local_comparand = comparand;
	return __sync_bool_compare_and_swap( pDest, local_comparand, value );
}
#endif

inline bool ThreadInterlockedAssignIf64x128( volatile int128 *pDest, const int128 &value, const int128 &comperand )
{
	return ThreadInterlockedAssignIf128( pDest, value, comperand );
}
#else
#define TSLIST_HEAD_ALIGNMENT 8
#define TSLIST_NODE_ALIGNMENT 8
inline bool ThreadInterlockedAssignIf64x128( volatile int64 *pDest, const int64 value, const int64 comperand )
{
	return ThreadInterlockedAssignIf64( pDest, value, comperand );
}
#endif

#ifdef _MSC_VER
#define TSLIST_HEAD_ALIGN DECL_ALIGN(TSLIST_HEAD_ALIGNMENT)
#define TSLIST_NODE_ALIGN DECL_ALIGN(TSLIST_NODE_ALIGNMENT)
#define TSLIST_HEAD_ALIGN_POST
#define TSLIST_NODE_ALIGN_POST
#elif defined( GNUC )
#define TSLIST_HEAD_ALIGN 
#define TSLIST_NODE_ALIGN 
#define TSLIST_HEAD_ALIGN_POST DECL_ALIGN(TSLIST_HEAD_ALIGNMENT)
#define TSLIST_NODE_ALIGN_POST DECL_ALIGN(TSLIST_NODE_ALIGNMENT)
#else
#error
#endif

//-----------------------------------------------------------------------------

PLATFORM_INTERFACE bool RunTSQueueTests( int nListSize = 10000, int nTests = 1 );
PLATFORM_INTERFACE bool RunTSListTests( int nListSize = 10000, int nTests = 1 );

//-----------------------------------------------------------------------------
// Lock free list.
//-----------------------------------------------------------------------------
//#define USE_NATIVE_SLIST

#ifdef USE_NATIVE_SLIST
typedef SLIST_ENTRY TSLNodeBase_t;
typedef SLIST_HEADER TSLHead_t;
#else
struct TSLIST_NODE_ALIGN TSLNodeBase_t
{
	TSLNodeBase_t *Next; // name to match Windows
} TSLIST_NODE_ALIGN_POST;

union TSLIST_HEAD_ALIGN TSLHead_t
{
	struct Value_t
	{
		TSLNodeBase_t *Next;
		// <sergiy> Depth must be in the least significant halfword when atomically loading into register,
		//          to avoid carrying digits from Sequence. Carrying digits from Depth to Sequence is ok,
		//          because Sequence can be pretty much random. We could operate on both of them separately,
		//          but it could perhaps (?) lead to problems with store forwarding. I don't know 'cause I didn't 
		//          performance-test or design original code, I'm just making it work on PowerPC.
#ifdef VALVE_BIG_ENDIAN
		uint16	Sequence;
		uint16   Depth;
#else
		uint16   Depth;
		uint16	Sequence;
#endif
#ifdef PLATFORM_64BITS
		uint32   Padding;
#endif
	} value;

	struct Value32_t
	{
		TSLNodeBase_t *Next_do_not_use_me;
		uint32   DepthAndSequence;
	} value32;

#ifdef PLATFORM_64BITS
	int128 value64x128;
#else
	int64 value64x128;
#endif
} TSLIST_HEAD_ALIGN_POST;

#endif

//-------------------------------------
class CTSListBase
{
public:

	// override new/delete so we can guarantee 8-byte aligned allocs
	static void * operator new(size_t size)
	{
		CTSListBase *pNode = (CTSListBase *)MemAlloc_AllocAlignedFileLine( size, TSLIST_HEAD_ALIGNMENT, __FILE__, __LINE__ );
		return pNode;
	}

	static void * operator new(size_t size, int nBlockUse, const char *pFileName, int nLine)
	{
		CTSListBase *pNode = (CTSListBase *)MemAlloc_AllocAlignedFileLine( size, TSLIST_HEAD_ALIGNMENT, pFileName, nLine );
		return pNode;
	}

	static void operator delete(void *p)
	{
		MemAlloc_FreeAligned( p );
	}

	static void operator delete(void *p, int nBlockUse, const char *pFileName, int nLine)
	{
		MemAlloc_FreeAligned( p );
	}

private:
	// These ain't gonna work
	static void * operator new[]( size_t size );
	static void operator delete[]( void *p );

public:

	CTSListBase()
	{
		if ( ((size_t)&m_Head) % TSLIST_HEAD_ALIGNMENT != 0 )
		{
			Error( "CTSListBase: Misaligned list\n" );
			DebuggerBreak();
		}

#ifdef USE_NATIVE_SLIST
		InitializeSListHead( &m_Head );
#elif defined(PLATFORM_64BITS)
		m_Head.value64x128 = int128_zero();
#else
		m_Head.value64x128 = (int64)0;
#endif
	}

	~CTSListBase()
	{
		Detach();
	}

	TSLNodeBase_t *Push( TSLNodeBase_t *pNode )
	{
#ifdef _DEBUG
		if ( (size_t)pNode % TSLIST_NODE_ALIGNMENT != 0 )
		{
			Error( "CTSListBase: Misaligned node\n" );
			DebuggerBreak();
		}
#endif

#ifdef USE_NATIVE_SLIST
		return (TSLNodeBase_t *)InterlockedPushEntrySList( &m_Head, pNode );
#else
		TSLHead_t oldHead;
		TSLHead_t newHead;


#ifdef PLATFORM_64BITS
		newHead.value.Padding = 0;
#endif
		for ( ;; )
		{
			oldHead.value64x128 = m_Head.value64x128;
			pNode->Next = oldHead.value.Next;
			newHead.value.Next = pNode;

			newHead.value32.DepthAndSequence = oldHead.value32.DepthAndSequence + 0x10001;


			if ( ThreadInterlockedAssignIf64x128( &m_Head.value64x128, newHead.value64x128, oldHead.value64x128 ) )
			{
				break;
			}
			ThreadPause();
		};

		return (TSLNodeBase_t *)oldHead.value.Next;
#endif
	}

	// TODO(nillerusr): fix asan issue later
	NO_ASAN TSLNodeBase_t *Pop()
	{
#ifdef USE_NATIVE_SLIST
		TSLNodeBase_t *pNode = (TSLNodeBase_t *)InterlockedPopEntrySList( &m_Head );
		return pNode;
#else
		TSLHead_t oldHead;
		TSLHead_t newHead;

#ifdef PLATFORM_64BITS
		newHead.value.Padding = 0;
#endif
		for ( ;; )
		{
			oldHead.value64x128 = m_Head.value64x128;
			if ( !oldHead.value.Next )
				return NULL;

			newHead.value.Next = oldHead.value.Next->Next;
			newHead.value32.DepthAndSequence = oldHead.value32.DepthAndSequence - 1;


			if ( ThreadInterlockedAssignIf64x128( &m_Head.value64x128, newHead.value64x128, oldHead.value64x128 ) )
			{
				break;
			}
			ThreadPause();
		};

		return (TSLNodeBase_t *)oldHead.value.Next;
#endif
	}

	TSLNodeBase_t *Detach()
	{
#ifdef USE_NATIVE_SLIST
		TSLNodeBase_t *pBase = (TSLNodeBase_t *)InterlockedFlushSList( &m_Head );
		return pBase;
#else
		TSLHead_t oldHead;
		TSLHead_t newHead;

#ifdef PLATFORM_64BITS
		newHead.value.Padding = 0;
#endif
		do
		{
			ThreadPause();

			oldHead.value64x128 = m_Head.value64x128;
			if ( !oldHead.value.Next )
				return NULL;

			newHead.value.Next = NULL;
			// <sergiy> the reason for AND'ing it instead of poking a short into memory 
			//          is probably to avoid store forward issues, but I'm not sure because
			//          I didn't construct this code. In any case, leaving it as is on big-endian
			newHead.value32.DepthAndSequence = oldHead.value32.DepthAndSequence & 0xffff0000;

		} while ( !ThreadInterlockedAssignIf64x128( &m_Head.value64x128, newHead.value64x128, oldHead.value64x128 ) );

		return (TSLNodeBase_t *)oldHead.value.Next;
#endif
	}

	TSLHead_t *AccessUnprotected()
	{
		return &m_Head;
	}

	int Count() const
	{
#ifdef USE_NATIVE_SLIST
		return QueryDepthSList( const_cast<TSLHead_t*>(&m_Head) );
#else
		return m_Head.value.Depth;
#endif
	}

private:
	TSLHead_t m_Head;
} TSLIST_HEAD_ALIGN_POST;

//-------------------------------------

template <typename T>
class TSLIST_HEAD_ALIGN CTSSimpleList : public CTSListBase
{
public:
	void Push( T *pNode )
	{
		Assert( sizeof(T) >= sizeof(TSLNodeBase_t) );
		CTSListBase::Push( (TSLNodeBase_t *)pNode );
	}

	T *Pop()
	{
		return (T *)CTSListBase::Pop();
	}
} TSLIST_HEAD_ALIGN_POST;

//-------------------------------------
// this is a replacement for CTSList<> and CObjectPool<> that does not
// have a per-item, per-alloc new/delete overhead
// similar to CTSSimpleList except that it allocates it's own pool objects
// and frees them on destruct.  Also it does not overlay the TSNodeBase_t memory
// on T's memory
template< class T >
class TSLIST_HEAD_ALIGN CTSPool : public CTSListBase
{
	// packs the node and the item (T) into a single struct and pools those
	struct TSLIST_NODE_ALIGN simpleTSPoolStruct_t : public TSLNodeBase_t
	{
		T elem;
	} TSLIST_NODE_ALIGN_POST;

public:

	~CTSPool()
	{
		Purge();
	}

	void Purge()
	{
		simpleTSPoolStruct_t *pNode = NULL;
		while ( 1 )
		{
			pNode = (simpleTSPoolStruct_t *)CTSListBase::Pop();
			if ( !pNode )
				break;
			delete pNode;
		}
	}

	void PutObject( T *pInfo )
	{
		char *pElem = (char *)pInfo;
		pElem -= offsetof( simpleTSPoolStruct_t, elem );
		simpleTSPoolStruct_t *pNode = (simpleTSPoolStruct_t *)pElem;

		CTSListBase::Push( pNode );
	}

	T *GetObject()
	{
		simpleTSPoolStruct_t *pNode = (simpleTSPoolStruct_t *)CTSListBase::Pop();
		if ( !pNode )
		{
			pNode = new simpleTSPoolStruct_t;
		}
		return &pNode->elem;
	}

	// omg windows sdk - why do you #define GetObject()?
	FORCEINLINE T *Get()
	{
		return GetObject();
	}
} TSLIST_HEAD_ALIGN_POST;
//-------------------------------------

template <typename T>
class TSLIST_HEAD_ALIGN CTSList : public CTSListBase
{
public:
	struct TSLIST_NODE_ALIGN Node_t : public TSLNodeBase_t
	{
		Node_t() = default;
		Node_t( const T &init ) : elem( init ) {}
		T elem;

		// override new/delete so we can guarantee 8-byte aligned allocs
		static void * operator new(size_t size)
		{
			Node_t *pNode = (Node_t *)MemAlloc_AllocAlignedFileLine( size, TSLIST_NODE_ALIGNMENT, __FILE__, __LINE__ );
			return pNode;
		}

		// override new/delete so we can guarantee 8-byte aligned allocs
		static void * operator new(size_t size, int nBlockUse, const char *pFileName, int nLine)
		{
			Node_t *pNode = (Node_t *)MemAlloc_AllocAlignedFileLine( size, TSLIST_NODE_ALIGNMENT, pFileName, nLine );
			return pNode;
		}

		static void operator delete(void *p)
		{
			MemAlloc_FreeAligned( p );
		}
		static void operator delete(void *p, int nBlockUse, const char *pFileName, int nLine)
		{
			MemAlloc_FreeAligned( p );
		}

	} TSLIST_NODE_ALIGN_POST;

	~CTSList()
	{
		Purge();
	}

	void Purge()
	{
		Node_t *pCurrent = Detach();
		Node_t *pNext;
		while ( pCurrent )
		{
			pNext = (Node_t *)pCurrent->Next;
			delete pCurrent;
			pCurrent = pNext;
		}
	}

	void RemoveAll()
	{
		Purge();
	}

	Node_t *Push( Node_t *pNode )
	{
		return (Node_t *)CTSListBase::Push( pNode );
	}

	Node_t *Pop()
	{
		return (Node_t *)CTSListBase::Pop();
	}

	void PushItem( const T &init )
	{
		Push( new Node_t( init ) );
	}

	bool PopItem( T *pResult )
	{
		Node_t *pNode = Pop();
		if ( !pNode )
			return false;
		*pResult = pNode->elem;
		delete pNode;
		return true;
	}

	Node_t *Detach()
	{
		return (Node_t *)CTSListBase::Detach();
	}

} TSLIST_HEAD_ALIGN_POST;

//-------------------------------------

template <typename T>
class TSLIST_HEAD_ALIGN CTSListWithFreeList : public CTSListBase
{
public:
	struct TSLIST_NODE_ALIGN Node_t : public TSLNodeBase_t
	{
		Node_t() = default;
		Node_t( const T &init ) : elem( init ) {}

		T elem;
	} TSLIST_NODE_ALIGN_POST;

	~CTSListWithFreeList()
	{
		Purge();
	}

	void Purge()
	{
		Node_t *pCurrent = Detach();
		Node_t *pNext;
		while ( pCurrent )
		{
			pNext = (Node_t *)pCurrent->Next;
			delete pCurrent;
			pCurrent = pNext;
		}
		pCurrent = (Node_t *)m_FreeList.Detach();
		while ( pCurrent )
		{
			pNext = (Node_t *)pCurrent->Next;
			delete pCurrent;
			pCurrent = pNext;
		}
	}

	void RemoveAll()
	{
		Node_t *pCurrent = Detach();
		Node_t *pNext;
		while ( pCurrent )
		{
			pNext = (Node_t *)pCurrent->Next;
			m_FreeList.Push( pCurrent );
			pCurrent = pNext;
		}
	}

	Node_t *Push( Node_t *pNode )
	{
		return (Node_t *)CTSListBase::Push( pNode );
	}

	Node_t *Pop()
	{
		return (Node_t *)CTSListBase::Pop();
	}

	void PushItem( const T &init )
	{
		Node_t *pNode = (Node_t *)m_FreeList.Pop();
		if ( !pNode )
		{
			pNode = new Node_t;
		}
		pNode->elem = init;
		Push( pNode );
	}

	bool PopItem( T *pResult )
	{
		Node_t *pNode = Pop();
		if ( !pNode )
			return false;
		*pResult = pNode->elem;
		m_FreeList.Push( pNode );
		return true;
	}

	Node_t *Detach()
	{
		return (Node_t *)CTSListBase::Detach();
	}

	void FreeNode( Node_t *pNode )
	{
		m_FreeList.Push( pNode );
	}

private:
	CTSListBase m_FreeList;
} TSLIST_HEAD_ALIGN_POST;

//-----------------------------------------------------------------------------
// Thread-safe FIFO queue (multi-producer, multi-consumer, unbounded).
//
// The former lock-free implementation was unsound: Push read the tail without
// synchronization, its help path could install a recycled node's free-list link
// as the tail, and the pNext CAS had no ABA tag. It crashed the legacy queue
// tests. Structure changes are now serialized by a mutex; a lock-free design
// needs native stress, sanitizer and performance evidence before replacing it.
//
// Contract (unchanged for callers):
//  - FIFO per queue; each pushed element is popped exactly once.
//  - Push(Node_t *) transfers the node to the queue and returns the previous
//    tail node (for identification only). Pop() returns a node the caller now
//    owns, carrying the popped element; delete it or hand it to FreeNode().
//    Node memory is never read after it leaves the queue, so it may be freed.
//  - PushItem/PopItem recycle nodes through a private free list. Element copies
//    happen under the lock, so T may be any copyable type.
//  - Count() is an atomic observation that may be stale when it returns.
//  - Purge, RemoveAll and ValidateQueue require that no other thread uses the
//    queue concurrently.
//-----------------------------------------------------------------------------

template <typename T, bool bTestOptimizer = false>
class TSLIST_HEAD_ALIGN CTSQueue
{
public:

	// override new/delete so we can guarantee 8-byte aligned allocs
	static void * operator new(size_t size)
	{
		CTSQueue *pNode = (CTSQueue *)MemAlloc_AllocAlignedFileLine( size, TSLIST_HEAD_ALIGNMENT, __FILE__, __LINE__ );
		return pNode;
	}

	// override new/delete so we can guarantee 8-byte aligned allocs
	static void * operator new(size_t size, int nBlockUse, const char *pFileName, int nLine)
	{
		CTSQueue *pNode = (CTSQueue *)MemAlloc_AllocAlignedFileLine( size, TSLIST_HEAD_ALIGNMENT, pFileName, nLine );
		return pNode;
	}

	static void operator delete(void *p)
	{
		MemAlloc_FreeAligned( p );
	}

	static void operator delete(void *p, int nBlockUse, const char *pFileName, int nLine)
	{
		MemAlloc_FreeAligned( p );
	}

private:
	// These ain't gonna work
	static void * operator new[]( size_t size ) throw()
	{
		return NULL;
	}

	static void operator delete []( void *p )
	{
	}

public:

	struct TSLIST_NODE_ALIGN Node_t
	{
		// override new/delete so we can guarantee 8-byte aligned allocs
		static void * operator new(size_t size)
		{
			Node_t *pNode = (Node_t *)MemAlloc_AllocAlignedFileLine( size, TSLIST_HEAD_ALIGNMENT, __FILE__, __LINE__ );
			return pNode;
		}

		static void * operator new(size_t size, int nBlockUse, const char *pFileName, int nLine)
		{
			Node_t *pNode = (Node_t *)MemAlloc_AllocAlignedFileLine( size, TSLIST_HEAD_ALIGNMENT, pFileName, nLine );
			return pNode;
		}

		static void operator delete(void *p)
		{
			MemAlloc_FreeAligned( p );
		}

		static void operator delete(void *p, int nBlockUse, const char *pFileName, int nLine)
		{
			MemAlloc_FreeAligned( p );
		}

		Node_t() = default;
		Node_t( const T &init ) : elem( init ) {}

		Node_t *pNext;
		T elem;
	} TSLIST_NODE_ALIGN_POST;

	CTSQueue()
	{
		m_Count = 0;
		m_pFreeNodes = NULL;
		m_pHead = m_pTail = new Node_t; // list always contains a dummy node
		m_pHead->pNext = NULL;
	}

	~CTSQueue()
	{
		Purge();
		Assert( m_Count == 0 );
		Assert( m_pHead == m_pTail );
		delete m_pHead;
	}

	void Purge()
	{
		Node_t *pNode;
		while ( ( pNode = Pop() ) != NULL )
		{
			delete pNode;
		}

		AUTO_LOCK( m_Mutex );
		while ( ( pNode = m_pFreeNodes ) != NULL )
		{
			m_pFreeNodes = pNode->pNext;
			delete pNode;
		}
		Assert( m_Count == 0 );
		Assert( m_pHead == m_pTail && m_pHead->pNext == NULL );
	}

	void RemoveAll()
	{
		AUTO_LOCK( m_Mutex );
		Node_t *pNode;
		while ( ( pNode = PopNodeLocked( NULL ) ) != NULL )
		{
			FreeNodeLocked( pNode );
		}
	}

	bool ValidateQueue()
	{
		AUTO_LOCK( m_Mutex );
		int nNodes = 0;
		Node_t *pLast = m_pHead;
		for ( Node_t *pNode = m_pHead->pNext; pNode; pNode = pNode->pNext )
		{
			++nNodes;
			pLast = pNode;
		}

		const bool bResult =
		    ( pLast == m_pTail ) && ( m_pTail->pNext == NULL ) && ( nNodes == m_Count );
		if ( !bResult )
		{
			DebuggerBreakIfDebugging();
			Msg( "Corrupt CTSQueueDetected" );
		}
		return bResult;
	}

	Node_t *Push( Node_t *pNode )
	{
#ifdef _DEBUG
		if ( (size_t)pNode % TSLIST_NODE_ALIGNMENT != 0 )
		{
			Error( "CTSListBase: Misaligned node\n" );
			DebuggerBreak();
		}
#endif
		AUTO_LOCK( m_Mutex );
		return PushNodeLocked( pNode );
	}

	Node_t *Pop()
	{
		AUTO_LOCK( m_Mutex );
		return PopNodeLocked( NULL );
	}

	void FreeNode( Node_t *pNode )
	{
		AUTO_LOCK( m_Mutex );
		FreeNodeLocked( pNode );
	}

	void PushItem( const T &init )
	{
		{
			AUTO_LOCK( m_Mutex );
			Node_t *pNode = m_pFreeNodes;
			if ( pNode )
			{
				m_pFreeNodes = pNode->pNext;
				pNode->elem = init;
				PushNodeLocked( pNode );
				return;
			}
		}

		// Allocate outside the lock; the node is private until it is linked.
		Push( new Node_t( init ) );
	}

	bool PopItem( T *pResult )
	{
		AUTO_LOCK( m_Mutex );
		Node_t *pNode = PopNodeLocked( pResult );
		if ( !pNode )
			return false;
		FreeNodeLocked( pNode );
		return true;
	}

	int Count() const
	{
		return m_Count;
	}

private:
	Node_t *PushNodeLocked( Node_t *pNode )
	{
		pNode->pNext = NULL;
		Node_t *pOldTail = m_pTail;
		pOldTail->pNext = pNode;
		m_pTail = pNode;
		++m_Count;
		return pOldTail;
	}

	// Unlinks the dummy head. Its successor becomes the new dummy and the popped
	// element is returned in the old dummy (and/or copied to pResult).
	Node_t *PopNodeLocked( T *pResult )
	{
		Node_t *pHead = m_pHead;
		Node_t *pNext = pHead->pNext;
		if ( !pNext )
			return NULL;

		if ( pResult )
			*pResult = pNext->elem;
		else
			pHead->elem = pNext->elem;
		m_pHead = pNext;
		pHead->pNext = NULL;
		--m_Count;
		return pHead;
	}

	void FreeNodeLocked( Node_t *pNode )
	{
		pNode->pNext = m_pFreeNodes;
		m_pFreeNodes = pNode;
	}

	CThreadMutex m_Mutex;
	Node_t *m_pHead;
	Node_t *m_pTail;
	Node_t *m_pFreeNodes;
	CInterlockedInt m_Count;
} TSLIST_NODE_ALIGN_POST;

#if defined( _WIN32 )
// Suppress this spurious warning:
// warning C4700: uninitialized local variable 'oldHead' used
#pragma warning( pop )
#endif

#endif // TSLIST_H
