//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Evaluation of KeyValues conditional expressions. Deliberately free
//			of tier0 so the parser can be tested on its own.
//
//=============================================================================//

#include "tier1/kvconditional.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace
{

const int MAX_CONDITIONAL_SYMBOL = 64;

class CConditionalParser
{
public:
	CConditionalParser( const char *pBegin, const char *pEnd, KVConditionalSymbolFn_t pfnSymbol, void *pContext )
		: m_p( pBegin ), m_pEnd( pEnd ), m_pfnSymbol( pfnSymbol ), m_pContext( pContext ), m_bError( false )
	{
	}

	bool Evaluate( bool &bResult )
	{
		const bool bValue = ParseOr();
		SkipSpaces();
		if ( m_bError || m_p != m_pEnd )
			return false;
		bResult = bValue;
		return true;
	}

private:
	void SkipSpaces()
	{
		while ( m_p < m_pEnd && ( *m_p == ' ' || *m_p == '\t' ) )
			++m_p;
	}

	bool Accept( const char *pToken )
	{
		SkipSpaces();
		const size_t nLen = strlen( pToken );
		if ( (size_t)( m_pEnd - m_p ) < nLen || strncmp( m_p, pToken, nLen ) )
			return false;
		m_p += nLen;
		return true;
	}

	bool ParseOr()
	{
		bool bValue = ParseAnd();
		while ( !m_bError && Accept( "||" ) )
		{
			const bool bRight = ParseAnd();
			bValue = bValue || bRight;
		}
		return bValue;
	}

	bool ParseAnd()
	{
		bool bValue = ParseUnary();
		while ( !m_bError && Accept( "&&" ) )
		{
			const bool bRight = ParseUnary();
			bValue = bValue && bRight;
		}
		return bValue;
	}

	bool ParseUnary()
	{
		if ( Accept( "!" ) )
			return !ParseUnary();

		if ( Accept( "(" ) )
		{
			const bool bValue = ParseOr();
			if ( !Accept( ")" ) )
				m_bError = true;
			return bValue;
		}

		SkipSpaces();
		const bool bSymbol = ( m_p < m_pEnd && *m_p == '$' );
		if ( bSymbol )
			++m_p;

		char name[MAX_CONDITIONAL_SYMBOL];
		int nLen = 0;
		while ( m_p < m_pEnd && ( isalnum( (unsigned char)*m_p ) || *m_p == '_' ) )
		{
			if ( nLen >= MAX_CONDITIONAL_SYMBOL - 1 )
				return Fail();
			name[nLen++] = *m_p++;
		}
		name[nLen] = 0;

		if ( nLen == 0 )
			return Fail();

		if ( bSymbol )
			return m_pfnSymbol ? m_pfnSymbol( name, m_pContext ) : false;

		for ( int i = 0; i < nLen; ++i )
		{
			if ( !isdigit( (unsigned char)name[i] ) )
				return Fail();
		}
		return atoi( name ) != 0;
	}

	bool Fail()
	{
		m_bError = true;
		return false;
	}

	const char *m_p;
	const char *m_pEnd;
	KVConditionalSymbolFn_t m_pfnSymbol;
	void *m_pContext;
	bool m_bError;
};

} // namespace

bool EvaluateKVConditionalExpression( const char *pExpression, KVConditionalSymbolFn_t pfnSymbol,
	void *pContext, bool &bResult )
{
	bResult = false;
	if ( !pExpression )
		return false;

	const char *pBegin = pExpression;
	const char *pEnd = pExpression + strlen( pExpression );

	// Accept the tag as the tokenizer returns it, brackets included.
	if ( *pBegin == '[' )
	{
		++pBegin;
		while ( pEnd > pBegin && ( pEnd[-1] == ' ' || pEnd[-1] == '\t' ) )
			--pEnd;
		if ( pEnd == pBegin || pEnd[-1] != ']' )
			return false;
		--pEnd;
	}

	CConditionalParser parser( pBegin, pEnd, pfnSymbol, pContext );
	return parser.Evaluate( bResult );
}
