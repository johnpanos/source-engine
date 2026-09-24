//========= Portal 2 port ======================================================//
//
// Purpose: Growable array used by the blobulator API.
//
// Clean-room implementation of the blobulator interface the Portal 2 paint
// blob renderer uses (not Valve's blobulator library). Member names follow the
// 2010 retail DWARF declarations (common/blobulator/SmartArray.h): the data
// pointer is `a`, the element count `size` and the allocation `capacity`.
// T must be trivially copyable: elements are moved with memcpy.
//
// Template parameters:
//   bZeroNew   - zero memory added by ensureCapacity()
//   nAlignment - byte alignment of the storage (0 or a power of two)
//
//=============================================================================//

#ifndef BLOBULATOR_SMARTARRAY_H
#define BLOBULATOR_SMARTARRAY_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include "tier0/memalloc.h"
#include <string.h>

template <class T, bool bZeroNew, int nAlignment> class SmartArray
{
public:
	T *a;
	int size;
	int capacity;

	SmartArray() : a( NULL ), size( 0 ), capacity( 0 ) {}

	~SmartArray()
	{
		if ( a )
			MemAlloc_FreeAligned( a );
	}

	void ensureCapacity( int nCapacity )
	{
		if ( nCapacity <= capacity )
			return;

		int nNewCapacity = capacity > 0 ? capacity : 8;
		while ( nNewCapacity < nCapacity )
			nNewCapacity *= 2;

		const size_t nAlign = nAlignment > 0 ? nAlignment : 16;
		T *pNew = (T *)MemAlloc_AllocAligned( nNewCapacity * sizeof( T ), nAlign );
		if ( a )
		{
			memcpy( (void *)pNew, a, size * sizeof( T ) );
			MemAlloc_FreeAligned( a );
		}
		if ( bZeroNew )
			memset( (void *)( pNew + capacity ), 0, ( nNewCapacity - capacity ) * sizeof( T ) );

		a = pNew;
		capacity = nNewCapacity;
	}

	void ensureAdditionalCapacity( int nAdditional ) { ensureCapacity( size + nAdditional ); }

	void resize( int nSize )
	{
		ensureCapacity( nSize );
		size = nSize;
	}

	T &operator[]( int i )
	{
		Assert( i >= 0 && i < capacity );
		return a[i];
	}
	const T &operator[]( int i ) const
	{
		Assert( i >= 0 && i < capacity );
		return a[i];
	}

	void put( int i, const T &val )
	{
		ensureCapacity( i + 1 );
		a[i] = val;
		if ( i >= size )
			size = i + 1;
	}

	void push( const T &val ) { pushAutoSize( val ); }
	T &push() { return pushAutoSize(); }
	void pushAutoSize( const T &val ) { pushAutoSize() = val; }
	T &pushAutoSize()
	{
		ensureCapacity( size + 1 );
		return a[size++];
	}
	void push2AutoSize( const T &v1, const T &v2 )
	{
		pushAutoSize( v1 );
		pushAutoSize( v2 );
	}
	void push2( const T &v1, const T &v2 ) { push2AutoSize( v1, v2 ); }

	T &pop()
	{
		Assert( size > 0 );
		return a[--size];
	}

	T &last()
	{
		Assert( size > 0 );
		return a[size - 1];
	}

	// Removes element i, keeping the order of the rest.
	void remove( int i )
	{
		Assert( i >= 0 && i < size );
		memmove( (void *)( a + i ), a + i + 1, ( size - i - 1 ) * sizeof( T ) );
		--size;
	}

	T removeAndReturn( int i )
	{
		T val = a[i];
		remove( i );
		return val;
	}

	int find( const T &val ) const
	{
		for ( int i = 0; i < size; ++i )
		{
			if ( a[i] == val )
				return i;
		}
		return -1;
	}

	bool findAndRemove( const T &val )
	{
		int i = find( val );
		if ( i < 0 )
			return false;
		remove( i );
		return true;
	}

	void swap( int i, int j )
	{
		T tmp = a[i];
		a[i] = a[j];
		a[j] = tmp;
	}

	void swapData( SmartArray &other )
	{
		T *pA = a;
		int nSize = size, nCapacity = capacity;
		a = other.a;
		size = other.size;
		capacity = other.capacity;
		other.a = pA;
		other.size = nSize;
		other.capacity = nCapacity;
	}

	// Stable insertion-merge sort ordered by C::IsLessThan( x, y ).
	template <class C> void sort()
	{
		if ( size < 2 )
			return;

		T *pTmp =
		    (T *)MemAlloc_AllocAligned( size * sizeof( T ), nAlignment > 0 ? nAlignment : 16 );
		mergeSort<C>( a, pTmp, 0, size );
		MemAlloc_FreeAligned( pTmp );
	}

private:
	template <class C> static void mergeSort( T *pData, T *pTmp, int nBegin, int nEnd )
	{
		if ( nEnd - nBegin < 2 )
			return;

		int nMid = ( nBegin + nEnd ) / 2;
		mergeSort<C>( pData, pTmp, nBegin, nMid );
		mergeSort<C>( pData, pTmp, nMid, nEnd );

		int i = nBegin, j = nMid, k = nBegin;
		while ( i < nMid && j < nEnd )
		{
			if ( C::IsLessThan( pData[j], pData[i] ) )
				memcpy( (void *)&pTmp[k++], &pData[j++], sizeof( T ) );
			else
				memcpy( (void *)&pTmp[k++], &pData[i++], sizeof( T ) );
		}
		while ( i < nMid )
			memcpy( (void *)&pTmp[k++], &pData[i++], sizeof( T ) );
		while ( j < nEnd )
			memcpy( (void *)&pTmp[k++], &pData[j++], sizeof( T ) );
		memcpy( (void *)( pData + nBegin ), pTmp + nBegin, ( nEnd - nBegin ) * sizeof( T ) );
	}

	SmartArray( const SmartArray & );
	SmartArray &operator=( const SmartArray & );
};

#endif // BLOBULATOR_SMARTARRAY_H
