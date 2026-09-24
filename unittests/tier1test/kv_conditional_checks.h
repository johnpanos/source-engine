//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Oracle for KeyValues conditional expressions (tier1/kvconditional.h).
//          The cases are the expression shapes retail Portal 2 resource files
//          use, the SDK 2013 single-symbol forms, and malformed input.
//
//=============================================================================//
#ifndef KV_CONDITIONAL_CHECKS_H
#define KV_CONDITIONAL_CHECKS_H

#include "tier1/kvconditional.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace kvcondtest
{

typedef bool (*EvaluatorFn)( const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext,
	bool &bResult );

struct Tally
{
	unsigned long checks = 0;
	unsigned long failures = 0;
	bool verbose = true;

	void Check( bool condition, const char *what )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			if ( verbose )
				std::printf( "FAIL: %s\n", what );
		}
	}
};

// A desktop Linux, English, widescreen high-definition profile. Every other
// symbol (GAMECONSOLE, PS3, OSX, TURKISH, ...) is false, as unset symbols are.
inline bool TestSymbol( const char *pSymbol, void *pContext )
{
	int *pCalls = static_cast<int *>( pContext );
	if ( pCalls )
		++*pCalls;

	static const char *const s_TrueSymbols[] = { "WIN32", "LINUX", "POSIX", "ENGLISH", "WIN32WIDE", "WIN32HIDEF" };
	for ( const char *pTrue : s_TrueSymbols )
	{
		if ( !std::strcmp( pSymbol, pTrue ) )
			return true;
	}
	return false;
}

struct Case
{
	const char *expression;
	bool valid;
	bool value;
};

inline void CheckAll( EvaluatorFn evaluate, Tally &tally )
{
	static const Case s_Cases[] = {
		// SDK 2013 single-symbol forms keep their meaning.
		{ "[$WIN32]", true, true },
		{ "[!$WIN32]", true, false },
		{ "[$X360]", true, false },
		{ "[!$X360]", true, true },
		{ "[$LINUX]", true, true },
		{ "[!$OSX]", true, true },

		// A negated unset symbol is true: [!$GAMECONSOLE] guards most retail fonts.
		{ "[!$GAMECONSOLE]", true, true },
		{ "[!$TURKISH]", true, true },
		{ "[$TURKISH]", true, false },

		// Compound retail expressions, with and without spaces.
		{ "[$PS3 && !$INPUTSWAPAB]", true, false },
		{ "[!$GAMECONSOLE && $WIN32WIDE]", true, true },
		{ "[!$GAMECONSOLE && !$WIN32WIDE]", true, false },
		{ "[$WIN32 && $ENGLISH]", true, true },
		{ "[$WIN32 && !$ENGLISH]", true, false },
		{ "[$OSX||$LINUX]", true, true },
		{ "[$GAMECONSOLE && ($GAMECONSOLEWIDE && !$ANAMORPHIC)]", true, false },
		{ "[$GAMECONSOLE && (!$GAMECONSOLEWIDE || $ANAMORPHIC)]", true, false },
		{ "[!($PS3 && ($JAPANESE || $TCHINESE || $SCHINESE || $KOREAN))]", true, true },
		{ "[($WIN32 && $WIN32HIDEF) || ($GAMECONSOLE && ($GAMECONSOLEWIDE && $GAMECONSOLEHIDEF))]", true, true },
		{ "[($WIN32 && !$WIN32HIDEF) || ($GAMECONSOLE && !($GAMECONSOLEWIDE && $GAMECONSOLEHIDEF))]", true, false },
		{ "[!$GAMECONSOLE && $WIN32HIDEF && !$OSX]", true, true },

		// C precedence: && binds tighter than ||.
		{ "[$GAMECONSOLE && $WIN32 || $LINUX]", true, true },
		{ "[$WIN32 || $GAMECONSOLE && $PS3]", true, true },
		{ "[!$WIN32 || $LINUX]", true, true },

		// Numeric constants.
		{ "[0]", true, false },
		{ "[1]", true, true },
		{ "[12]", true, true },

		// The brackets are optional, and surrounding blanks are ignored.
		{ "$WIN32 && !$OSX", true, true },
		{ "[ $WIN32 ]", true, true },
		{ "[$WIN32] ", true, true },

		// Malformed expressions are rejected and read as false.
		{ "[$!LINUX]", false, false }, // SDK 2013 content; was false there too
		{ "[$WIN32", false, false },
		{ "[$WIN32 &&]", false, false },
		{ "[($WIN32]", false, false },
		{ "[$WIN32)]", false, false },
		{ "[$WIN32 & $LINUX]", false, false },
		{ "[$WIN32 $LINUX]", false, false },
		{ "[]", false, false },
		{ "[$]", false, false },
		{ "[WIN32]", false, false },
		{ "[!]", false, false },
		{ "[$WIN32]x", false, false },
	};

	for ( const Case &test : s_Cases )
	{
		bool bResult = !test.value; // must be overwritten
		const bool bValid = evaluate( test.expression, TestSymbol, nullptr, bResult );
		const std::string what = std::string( test.expression ) + ( test.valid ? " is valid" : " is rejected" );
		tally.Check( bValid == test.valid, what.c_str() );
		const std::string value = std::string( test.expression ) + ( test.value ? " is true" : " is false" );
		tally.Check( bResult == test.value, value.c_str() );
	}

	// A symbol longer than the parser's buffer is rejected, not truncated.
	const std::string longSymbol = "[$" + std::string( 200, 'A' ) + "]";
	bool bResult = true;
	tally.Check( !evaluate( longSymbol.c_str(), TestSymbol, nullptr, bResult ) && !bResult, "oversized symbol is rejected" );

	// No expression at all.
	bResult = true;
	tally.Check( !evaluate( nullptr, TestSymbol, nullptr, bResult ) && !bResult, "null expression is rejected" );

	// Symbols reach the resolver without their '$', once each.
	int nCalls = 0;
	evaluate( "[$WIN32 && !$OSX]", TestSymbol, &nCalls, bResult );
	tally.Check( nCalls == 2, "each symbol is resolved once" );

	// Without a resolver every symbol is false.
	tally.Check( evaluate( "[!$WIN32]", nullptr, nullptr, bResult ) && bResult, "missing resolver reads symbols as false" );
}

} // namespace kvcondtest

#endif // KV_CONDITIONAL_CHECKS_H
