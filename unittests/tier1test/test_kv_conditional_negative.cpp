//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the KeyValues conditional oracle
//          (kv_conditional_checks.h). Each broken evaluator differs from the
//          real one in one plausible way; the oracle must fail every one of
//          them while the real evaluator passes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/tier1test/kv_conditional_checks.h"

#include <cctype>
#include <cstdio>
#include <string>

namespace
{

// The SDK 2013 evaluator: substring matching of one symbol, with an unknown
// symbol false even when negated ([!$GAMECONSOLE] rejected every retail font).
bool LegacySubstringEvaluator(
    const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext, bool &bResult )
{
	bResult = false;
	if ( !pExpression )
		return false;
	std::string text( pExpression );
	for ( char &c : text )
		c = (char)std::toupper( (unsigned char)c );
	size_t start = ( !text.empty() && text[0] == '[' ) ? 1 : 0;
	const bool bNot = text.size() > start && text[start] == '!';
	static const char *const s_Symbols[] = {
	    "DECK", "X360", "WIN32", "WINDOWS", "OSX", "LINUX", "POSIX" };
	for ( const char *pSymbol : s_Symbols )
	{
		if ( text.find( std::string( "$" ) + pSymbol ) != std::string::npos )
		{
			bResult = ( pfnSymbol && pfnSymbol( pSymbol, pContext ) ) != bNot;
			return true;
		}
	}
	return true;
}

// Groups everything after the first && as its right operand, so
// "a && b || c" reads as "a && (b || c)" (no operator precedence).
bool RightGroupingEvaluator(
    const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext, bool &bResult )
{
	if ( !pExpression )
		return EvaluateKVConditionalExpression( pExpression, pfnSymbol, pContext, bResult );
	std::string text( pExpression );
	const size_t op = text.find( "&&" );
	const size_t close = text.rfind( ']' );
	if ( op != std::string::npos && close != std::string::npos && close > op )
	{
		text.insert( close, ")" );
		text.insert( op + 2, "(" );
	}
	return EvaluateKVConditionalExpression( text.c_str(), pfnSymbol, pContext, bResult );
}

// Stops at the first blank, as the SDK 2013 tokenizer split "[$A && $B]".
bool SplitAtBlankEvaluator(
    const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext, bool &bResult )
{
	if ( !pExpression )
		return EvaluateKVConditionalExpression( pExpression, pfnSymbol, pContext, bResult );
	std::string text( pExpression );
	const size_t blank = text.find( ' ', 1 );
	if ( blank != std::string::npos && text[0] == '[' )
		text = text.substr( 0, blank ) + "]";
	return EvaluateKVConditionalExpression( text.c_str(), pfnSymbol, pContext, bResult );
}

// Never reports a syntax error.
bool AcceptsMalformedEvaluator(
    const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext, bool &bResult )
{
	EvaluateKVConditionalExpression( pExpression, pfnSymbol, pContext, bResult );
	return true;
}

// Resolves symbols without regard to negation.
bool IgnoresNotEvaluator(
    const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext, bool &bResult )
{
	if ( !pExpression )
		return EvaluateKVConditionalExpression( pExpression, pfnSymbol, pContext, bResult );
	std::string text;
	for ( const char *p = pExpression; *p; ++p )
	{
		if ( *p != '!' )
			text += *p;
	}
	return EvaluateKVConditionalExpression( text.c_str(), pfnSymbol, pContext, bResult );
}

struct BrokenEvaluator
{
	const char *name;
	kvcondtest::EvaluatorFn evaluate;
};

} // namespace

int main()
{
	unsigned long checks = 0;
	unsigned long failures = 0;

	// The real evaluator must pass, or the detections below mean nothing.
	kvcondtest::Tally real;
	real.verbose = true;
	kvcondtest::CheckAll( EvaluateKVConditionalExpression, real );
	++checks;
	if ( real.failures != 0 )
	{
		++failures;
		std::printf( "FAIL: real evaluator fails %lu of %lu checks\n", real.failures, real.checks );
	}

	static const BrokenEvaluator s_Broken[] = {
	    { "SDK 2013 substring evaluator", LegacySubstringEvaluator },
	    { "no operator precedence", RightGroupingEvaluator },
	    { "split at the first blank", SplitAtBlankEvaluator },
	    { "accepts malformed expressions", AcceptsMalformedEvaluator },
	    { "ignores negation", IgnoresNotEvaluator },
	};

	for ( const BrokenEvaluator &broken : s_Broken )
	{
		kvcondtest::Tally tally;
		tally.verbose = false;
		kvcondtest::CheckAll( broken.evaluate, tally );
		++checks;
		if ( tally.failures == 0 )
		{
			++failures;
			std::printf( "FAIL: oracle did not detect: %s\n", broken.name );
		}
		else
		{
			std::printf(
			    "detected: %s (%lu of %lu checks)\n", broken.name, tally.failures, tally.checks );
		}
	}

	return testing::ReportConformance( checks, failures );
}
