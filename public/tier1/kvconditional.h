//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Evaluation of KeyValues conditional expressions, e.g. the tag in
//			"tall" "24" [$WIN32 && !($OSX || $GAMECONSOLE)]
//
//			The grammar is the later-branch one (C precedence):
//				or    := and { "||" and }
//				and   := unary { "&&" unary }
//				unary := "!" unary | "(" or ")" | "$" name | digits
//			A number is true unless it is 0. Symbol values come from the caller,
//			which owns the policy of what each platform or run-time symbol means.
//
//=============================================================================//

#ifndef KVCONDITIONAL_H
#define KVCONDITIONAL_H
#ifdef _WIN32
#pragma once
#endif

// Returns the value of a symbol, given without its leading '$'.
typedef bool ( *KVConditionalSymbolFn_t )( const char *pSymbol, void *pContext );

// Evaluates pExpression, which may keep its enclosing brackets ("[$X]").
// Returns false, leaving bResult false, when the expression is malformed.
bool EvaluateKVConditionalExpression(
    const char *pExpression, KVConditionalSymbolFn_t pfnSymbol, void *pContext, bool &bResult );

#endif // KVCONDITIONAL_H
