//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Allocation-free recoverable result vocabulary for strict C++20
//          first-party targets.
//
//=============================================================================//

#ifndef FOUNDATION_EXPECTED_H
#define FOUNDATION_EXPECTED_H

#ifdef _WIN32
#pragma once
#endif

#include <cassert>
#include <concepts>
#include <new>
#include <type_traits>
#include <utility>

namespace foundation
{

template < typename E >
struct Unexpected
{
	template < typename U >
	requires std::constructible_from< E, U && >
	explicit Unexpected( U &&value ) : m_Error( std::forward< U >( value ) )
	{
	}

	E m_Error;
};

template < typename E >
Unexpected< std::decay_t< E > > MakeUnexpected( E &&error )
{
	return Unexpected< std::decay_t< E > >( std::forward< E >( error ) );
}

template < typename T, typename E >
class [[nodiscard]] Expected
{
public:
	template < typename U = T >
	requires std::constructible_from< T, U && >
	Expected( U &&value ) : m_HasValue( true )
	{
		new ( &m_Storage.m_Value ) T( std::forward< U >( value ) );
	}

	template < typename U >
	requires std::constructible_from< E, U && >
	Expected( Unexpected< U > error ) : m_HasValue( false )
	{
		new ( &m_Storage.m_Error ) E( std::move( error.m_Error ) );
	}

	Expected( const Expected &other )
	requires( std::copy_constructible< T > &&std::copy_constructible< E > )
	    : m_HasValue( other.m_HasValue )
	{
		CopyConstruct( other );
	}

	Expected( const Expected & )
	requires( !std::copy_constructible< T > || !std::copy_constructible< E > ) = delete;

	Expected( Expected &&other ) noexcept( std::is_nothrow_move_constructible_v< T > &&
	                                      std::is_nothrow_move_constructible_v< E > )
	    : m_HasValue( other.m_HasValue )
	{
		MoveConstruct( std::move( other ) );
	}

	Expected &operator=( const Expected &other )
	requires( std::copy_constructible< T > &&std::copy_constructible< E > )
	{
		if ( this == &other )
			return *this;

		Destroy();
		m_HasValue = other.m_HasValue;
		CopyConstruct( other );
		return *this;
	}

	Expected &operator=( Expected &&other ) noexcept( std::is_nothrow_move_constructible_v< T > &&
	                                                std::is_nothrow_move_constructible_v< E > )
	{
		if ( this == &other )
			return *this;

		Destroy();
		m_HasValue = other.m_HasValue;
		MoveConstruct( std::move( other ) );
		return *this;
	}

	~Expected() { Destroy(); }

	bool HasValue() const noexcept { return m_HasValue; }
	explicit operator bool() const noexcept { return m_HasValue; }

	T &Value() &
	{
		assert( m_HasValue );
		return m_Storage.m_Value;
	}

	const T &Value() const &
	{
		assert( m_HasValue );
		return m_Storage.m_Value;
	}

	T &&Value() &&
	{
		assert( m_HasValue );
		return std::move( m_Storage.m_Value );
	}

	E &Error() &
	{
		assert( !m_HasValue );
		return m_Storage.m_Error;
	}

	const E &Error() const &
	{
		assert( !m_HasValue );
		return m_Storage.m_Error;
	}

	E &&Error() &&
	{
		assert( !m_HasValue );
		return std::move( m_Storage.m_Error );
	}

	template < typename U >
	T ValueOr( U &&fallback ) const &
	{
		return m_HasValue ? m_Storage.m_Value : T( std::forward< U >( fallback ) );
	}

private:
	union Storage
	{
		Storage() {}
		~Storage() {}

		T m_Value;
		E m_Error;
	};

	void CopyConstruct( const Expected &other )
	{
		if ( m_HasValue )
			new ( &m_Storage.m_Value ) T( other.m_Storage.m_Value );
		else
			new ( &m_Storage.m_Error ) E( other.m_Storage.m_Error );
	}

	void MoveConstruct( Expected &&other )
	{
		if ( m_HasValue )
			new ( &m_Storage.m_Value ) T( std::move( other.m_Storage.m_Value ) );
		else
			new ( &m_Storage.m_Error ) E( std::move( other.m_Storage.m_Error ) );
	}

	void Destroy() noexcept
	{
		if ( m_HasValue )
			m_Storage.m_Value.~T();
		else
			m_Storage.m_Error.~E();
	}

	Storage m_Storage;
	bool m_HasValue;
};

template < typename E >
class [[nodiscard]] Expected< void, E >
{
public:
	Expected() = default;

	template < typename U >
	requires std::constructible_from< E, U && >
	Expected( Unexpected< U > error ) : m_HasValue( false ), m_Error( std::move( error.m_Error ) )
	{
	}

	bool HasValue() const noexcept { return m_HasValue; }
	explicit operator bool() const noexcept { return m_HasValue; }

	E &Error() &
	{
		assert( !m_HasValue );
		return m_Error;
	}

	const E &Error() const &
	{
		assert( !m_HasValue );
		return m_Error;
	}

	E &&Error() &&
	{
		assert( !m_HasValue );
		return std::move( m_Error );
	}

	static Expected Ok() { return Expected(); }

private:
	bool m_HasValue = true;
	E m_Error{};
};

} // namespace foundation

#endif // FOUNDATION_EXPECTED_H
