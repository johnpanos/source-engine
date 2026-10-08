//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Path value types (RFC 0001 rank 5, "Path representation and
//			encoding"; roadmap R11).
//
//			VirtualPath is a validated UTF-8, '/'-separated, relative engine
//			name: assets, search-path-relative names, manifests, module search
//			patterns. NativePath is an opaque platform path that keeps the native
//			representation (bytes on POSIX, UTF-16 on Windows). Neither converts
//			implicitly to the other. Portable code combines them through methods
//			(a NativePath base joined with a VirtualPath) and never reads native
//			storage; only platform backends do, through NativePathAccess
//			(platform/native_path/native_path_access.h, which archlint confines
//			to backends). Display text is an explicit, lossy conversion.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_PATH_TYPES_H
#define PLATFORM_CONTRACTS_PATH_TYPES_H

// Contract header: standard library and foundation vocabulary only. No
// tier0/tier1, no native SDK, no OS-selection macros.
#include "foundation/expected.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace platform
{

enum class PathError
{
	kEmpty = 0,
	kEmbeddedNul,
	kInvalidUtf8,
	kBackslash,       // a native separator inside a virtual path
	kAbsolute,        // a leading '/' or a drive prefix in a virtual path
	kEmptySegment,    // "a//b" or a trailing '/'
	kDotSegment,      // a "." or ".." segment
	kNotRepresentable // valid text the native encoding cannot hold
};

namespace detail
{

// Decodes one UTF-8 scalar at `i`, rejecting overlong forms, surrogates and
// values past U+10FFFF. Returns false on malformed input.
inline bool DecodeUtf8( std::string_view text, std::size_t &i, char32_t &out )
{
	const unsigned char c = static_cast<unsigned char>( text[i] );
	int extra = 0;
	char32_t cp = 0;
	if ( c < 0x80 )
	{
		out = c;
		++i;
		return true;
	}
	if ( c >= 0xc2 && c <= 0xdf )
	{
		extra = 1;
		cp = c & 0x1f;
	}
	else if ( c >= 0xe0 && c <= 0xef )
	{
		extra = 2;
		cp = c & 0x0f;
	}
	else if ( c >= 0xf0 && c <= 0xf4 )
	{
		extra = 3;
		cp = c & 0x07;
	}
	else
	{
		return false;
	}
	for ( int k = 1; k <= extra; ++k )
	{
		if ( i + k >= text.size() )
		{
			return false;
		}
		const unsigned char cc = static_cast<unsigned char>( text[i + k] );
		if ( ( cc & 0xc0 ) != 0x80 )
		{
			return false;
		}
		cp = ( cp << 6 ) | ( cc & 0x3f );
	}
	if ( ( extra == 2 && ( cp < 0x800 || ( cp >= 0xd800 && cp <= 0xdfff ) ) ) ||
	     ( extra == 3 && ( cp < 0x10000 || cp > 0x10ffff ) ) )
	{
		return false;
	}
	out = cp;
	i += static_cast<std::size_t>( extra ) + 1;
	return true;
}

inline bool IsValidUtf8( std::string_view text )
{
	std::size_t i = 0;
	char32_t cp = 0;
	while ( i < text.size() )
	{
		if ( !DecodeUtf8( text, i, cp ) )
		{
			return false;
		}
	}
	return true;
}

inline void AppendUtf8( std::string &out, char32_t cp )
{
	if ( cp < 0x80 )
	{
		out += static_cast<char>( cp );
	}
	else if ( cp < 0x800 )
	{
		out += static_cast<char>( 0xc0 | ( cp >> 6 ) );
		out += static_cast<char>( 0x80 | ( cp & 0x3f ) );
	}
	else if ( cp < 0x10000 )
	{
		out += static_cast<char>( 0xe0 | ( cp >> 12 ) );
		out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3f ) );
		out += static_cast<char>( 0x80 | ( cp & 0x3f ) );
	}
	else
	{
		out += static_cast<char>( 0xf0 | ( cp >> 18 ) );
		out += static_cast<char>( 0x80 | ( ( cp >> 12 ) & 0x3f ) );
		out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3f ) );
		out += static_cast<char>( 0x80 | ( cp & 0x3f ) );
	}
}

} // namespace detail

// A validated, relative engine name. Invariants: non-empty, valid UTF-8, no
// NUL, no '\', no leading '/', no drive prefix, no empty, "." or ".." segment.
class VirtualPath
{
public:
	[[nodiscard]] static foundation::Expected<VirtualPath, PathError> Parse( std::string_view text )
	{
		if ( text.empty() )
		{
			return foundation::MakeUnexpected( PathError::kEmpty );
		}
		if ( text.find( '\0' ) != std::string_view::npos )
		{
			return foundation::MakeUnexpected( PathError::kEmbeddedNul );
		}
		if ( !detail::IsValidUtf8( text ) )
		{
			return foundation::MakeUnexpected( PathError::kInvalidUtf8 );
		}
		if ( text.find( '\\' ) != std::string_view::npos )
		{
			return foundation::MakeUnexpected( PathError::kBackslash );
		}
		if ( text.front() == '/' || ( text.size() >= 2 && text[1] == ':' ) )
		{
			return foundation::MakeUnexpected( PathError::kAbsolute );
		}
		std::size_t start = 0;
		for ( ;; )
		{
			const std::size_t slash = text.find( '/', start );
			const std::string_view segment = text.substr(
			    start, slash == std::string_view::npos ? std::string_view::npos : slash - start );
			if ( segment.empty() )
			{
				return foundation::MakeUnexpected( PathError::kEmptySegment );
			}
			if ( segment == "." || segment == ".." )
			{
				return foundation::MakeUnexpected( PathError::kDotSegment );
			}
			if ( slash == std::string_view::npos )
			{
				break;
			}
			start = slash + 1;
		}
		VirtualPath path;
		path.m_text.assign( text );
		return path;
	}

	const std::string &String() const { return m_text; }

	// The segments in order.
	std::vector<std::string_view> Segments() const
	{
		std::vector<std::string_view> out;
		std::string_view text = m_text;
		std::size_t start = 0;
		for ( ;; )
		{
			const std::size_t slash = text.find( '/', start );
			out.push_back( text.substr(
			    start, slash == std::string_view::npos ? std::string_view::npos : slash - start ) );
			if ( slash == std::string_view::npos )
			{
				return out;
			}
			start = slash + 1;
		}
	}

	// `this` followed by `child`.
	VirtualPath Join( const VirtualPath &child ) const
	{
		VirtualPath path;
		path.m_text = m_text + "/" + child.m_text;
		return path;
	}

	bool operator==( const VirtualPath &other ) const { return m_text == other.m_text; }

private:
	VirtualPath() = default;
	std::string m_text;
};

// How a NativePath stores its text. A provider chooses it; portable code only
// passes NativePaths along and joins VirtualPaths onto them.
enum class NativePathFlavor : std::uint8_t
{
	kNone = 0,     // the empty path
	kPosixBytes,   // uninterpreted bytes, '/' separators
	kWindowsUtf16, // UTF-16 code units, '\' separators (also accepts '/')
};

class NativePathAccess;

class NativePath
{
public:
	NativePath() = default;

	NativePathFlavor Flavor() const { return m_flavor; }
	bool IsEmpty() const { return m_flavor == NativePathFlavor::kNone; }

	// `this` with `child` appended under it, in the native representation.
	// Fails with kNotRepresentable when this path is empty (a resolver joins
	// only onto real roots).
	[[nodiscard]] foundation::Expected<NativePath, PathError> Join( const VirtualPath &child ) const
	{
		NativePath out = *this;
		switch ( m_flavor )
		{
		case NativePathFlavor::kPosixBytes:
			if ( out.m_bytes.empty() || out.m_bytes.back() != '/' )
			{
				out.m_bytes += '/';
			}
			out.m_bytes += child.String();
			return out;
		case NativePathFlavor::kWindowsUtf16:
		{
			if ( out.m_units.empty() ||
			     ( out.m_units.back() != u'\\' && out.m_units.back() != u'/' ) )
			{
				out.m_units += u'\\';
			}
			const std::string &text = child.String();
			std::size_t i = 0;
			char32_t cp = 0;
			while ( i < text.size() )
			{
				detail::DecodeUtf8( text, i, cp ); // VirtualPath is valid UTF-8
				if ( cp == U'/' )
				{
					out.m_units += u'\\';
				}
				else if ( cp >= 0x10000 )
				{
					const char32_t v = cp - 0x10000;
					out.m_units += static_cast<char16_t>( 0xd800 + ( v >> 10 ) );
					out.m_units += static_cast<char16_t>( 0xdc00 + ( v & 0x3ff ) );
				}
				else
				{
					out.m_units += static_cast<char16_t>( cp );
				}
			}
			return out;
		}
		case NativePathFlavor::kNone:
		default:
			return foundation::MakeUnexpected( PathError::kNotRepresentable );
		}
	}

	// Lossy display text: UTF-8, with U+FFFD for bytes or code units that do
	// not decode. Never fed back into a path.
	std::string ToDisplayString() const
	{
		std::string out;
		if ( m_flavor == NativePathFlavor::kPosixBytes )
		{
			std::size_t i = 0;
			while ( i < m_bytes.size() )
			{
				char32_t cp = 0;
				if ( !detail::DecodeUtf8( m_bytes, i, cp ) )
				{
					detail::AppendUtf8( out, 0xfffd );
					++i;
					continue;
				}
				detail::AppendUtf8( out, cp );
			}
		}
		else if ( m_flavor == NativePathFlavor::kWindowsUtf16 )
		{
			for ( std::size_t i = 0; i < m_units.size(); ++i )
			{
				const char16_t u = m_units[i];
				if ( u >= 0xd800 && u <= 0xdbff && i + 1 < m_units.size() &&
				     m_units[i + 1] >= 0xdc00 && m_units[i + 1] <= 0xdfff )
				{
					detail::AppendUtf8( out, 0x10000 + ( ( char32_t( u ) - 0xd800 ) << 10 ) +
					                             ( m_units[i + 1] - 0xdc00 ) );
					++i;
				}
				else if ( u >= 0xd800 && u <= 0xdfff )
				{
					detail::AppendUtf8( out, 0xfffd );
				}
				else
				{
					detail::AppendUtf8( out, u );
				}
			}
		}
		return out;
	}

	bool operator==( const NativePath &other ) const
	{
		return m_flavor == other.m_flavor && m_bytes == other.m_bytes && m_units == other.m_units;
	}

private:
	friend class NativePathAccess;
	NativePathFlavor m_flavor = NativePathFlavor::kNone;
	std::string m_bytes;    // kPosixBytes
	std::u16string m_units; // kWindowsUtf16
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_PATH_TYPES_H
