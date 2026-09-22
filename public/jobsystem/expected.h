//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Minimal recoverable-result type for the dependency-aware job system
//          (RFC 0003). This is a deliberately small, job-system-scoped value
//          type. It is NOT the engine-wide result vocabulary; RFC 0001 rank 2 /
//          AGENTS.md R05 owns that. When R05 lands, this MUST converge onto it
//          rather than remain a second authority. `std::expected` is C++23 and
//          is intentionally not used (see RFC 0006 / AGENTS.md C++ policy).
//
//=============================================================================//

#ifndef JOBSYSTEM_EXPECTED_H
#define JOBSYSTEM_EXPECTED_H

#ifdef _WIN32
#pragma once
#endif

#include <utility>
#include <new>

namespace jobsystem
{

// Tag used to construct the error alternative unambiguously.
template < typename E >
struct Unexpected
{
	explicit Unexpected( E value ) : m_error( std::move( value ) ) {}
	E m_error;
};

template < typename E >
Unexpected< E > MakeUnexpected( E value )
{
	return Unexpected< E >( std::move( value ) );
}

// Expected<T, E>: holds either a value T or an error E, never both, always one.
// Value semantics; no dynamic allocation; no exceptions required by callers.
template < typename T, typename E >
class [[nodiscard]] Expected
{
public:
	Expected( T value ) : m_bHasValue( true ) { new ( &m_value ) T( std::move( value ) ); }
	Expected( Unexpected< E > err ) : m_bHasValue( false ) { new ( &m_error ) E( std::move( err.m_error ) ); }

	Expected( const Expected &other ) : m_bHasValue( other.m_bHasValue )
	{
		if ( m_bHasValue ) new ( &m_value ) T( other.m_value );
		else               new ( &m_error ) E( other.m_error );
	}
	Expected( Expected &&other ) noexcept : m_bHasValue( other.m_bHasValue )
	{
		if ( m_bHasValue ) new ( &m_value ) T( std::move( other.m_value ) );
		else               new ( &m_error ) E( std::move( other.m_error ) );
	}
	Expected &operator=( Expected other ) noexcept
	{
		Destroy();
		m_bHasValue = other.m_bHasValue;
		if ( m_bHasValue ) new ( &m_value ) T( std::move( other.m_value ) );
		else               new ( &m_error ) E( std::move( other.m_error ) );
		return *this;
	}
	~Expected() { Destroy(); }

	bool HasValue() const { return m_bHasValue; }
	explicit operator bool() const { return m_bHasValue; }

	// Precondition: HasValue(). Callers gate on HasValue()/operator bool first.
	T &Value()             { return m_value; }
	const T &Value() const { return m_value; }

	// Precondition: !HasValue().
	E &Error()             { return m_error; }
	const E &Error() const { return m_error; }

	// Non-throwing accessor with a caller-supplied fallback.
	T ValueOr( T fallback ) const { return m_bHasValue ? m_value : std::move( fallback ); }

private:
	void Destroy()
	{
		if ( m_bHasValue ) m_value.~T();
		else               m_error.~E();
	}

	union
	{
		T m_value;
		E m_error;
	};
	bool m_bHasValue;
};

// Specialization for a value-less success (T = void): success carries no data.
template < typename E >
class [[nodiscard]] Expected< void, E >
{
public:
	Expected() : m_bHasValue( true ) {}
	Expected( Unexpected< E > err ) : m_bHasValue( false ), m_error( std::move( err.m_error ) ) {}

	bool HasValue() const { return m_bHasValue; }
	explicit operator bool() const { return m_bHasValue; }

	E &Error()             { return m_error; }
	const E &Error() const { return m_error; }

	static Expected Ok() { return Expected(); }

private:
	bool m_bHasValue;
	E m_error{};
};

} // namespace jobsystem

#endif // JOBSYSTEM_EXPECTED_H
