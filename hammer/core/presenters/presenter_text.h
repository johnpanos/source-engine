//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Private text helpers of hammer.presenters: ASCII case folding,
//			case-insensitive matching and counted nouns. Presenter filters and
//			labels share these so every panel folds case the same way (ASCII
//			only; entity classes, key names and material paths are ASCII).
//
//=============================================================================//

#ifndef HAMMER_CORE_PRESENTERS_PRESENTER_TEXT_H
#define HAMMER_CORE_PRESENTERS_PRESENTER_TEXT_H

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::presenters::detail
{

inline char LowerChar( char c )
{
	return ( c >= 'A' && c <= 'Z' ) ? static_cast<char>( c - 'A' + 'a' ) : c;
}

inline std::string Lower( std::string_view text )
{
	std::string out( text );
	for ( char &c : out )
	{
		c = LowerChar( c );
	}
	return out;
}

inline bool EqualsNoCase( std::string_view a, std::string_view b )
{
	if ( a.size() != b.size() )
	{
		return false;
	}
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		if ( LowerChar( a[i] ) != LowerChar( b[i] ) )
		{
			return false;
		}
	}
	return true;
}

inline bool StartsWithNoCase( std::string_view text, std::string_view prefix )
{
	return text.size() >= prefix.size() && EqualsNoCase( text.substr( 0, prefix.size() ), prefix );
}

// True when 'needle' occurs in 'haystack' ignoring case; an empty needle matches.
inline bool ContainsNoCase( std::string_view haystack, std::string_view needle )
{
	return Lower( haystack ).find( Lower( needle ) ) != std::string::npos;
}

// Splits on ASCII whitespace, dropping empty words.
inline std::vector<std::string> Words( std::string_view text )
{
	std::vector<std::string> words;
	std::string current;
	for ( char c : text )
	{
		if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v' )
		{
			if ( !current.empty() )
			{
				words.push_back( current );
				current.clear();
			}
		}
		else
		{
			current.push_back( c );
		}
	}
	if ( !current.empty() )
	{
		words.push_back( current );
	}
	return words;
}

// "1 solid", "2 solids", "1 entity", "3 entities".
inline std::string Counted( std::size_t count, std::string_view singular, std::string_view plural )
{
	return std::to_string( count ) + " " + std::string( count == 1 ? singular : plural );
}

} // namespace hammer::presenters::detail

#endif // HAMMER_CORE_PRESENTERS_PRESENTER_TEXT_H
