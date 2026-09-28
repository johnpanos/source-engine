//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMF/keyvalues document model, parser, writer, and semantic comparator
//			(RFC 0002, content.keyvalues-text). Source's VMF is a nested keyvalues text
//			format: named blocks containing quoted "key" "value" pairs and child
//			blocks. This is the dependency-free foundation for the H2 persistence
//			slice and the HAM-CORPUS-001 versioned semantic comparator: no MFC,
//			tier0, PCH, or GPU.
//
//			The parser preserves unknown blocks/keys verbatim (nothing is dropped),
//			so round-tripping an unrecognized VMF chunk does not lose it. The
//			semantic comparator's normalization is explicit (see CompareKeyValues).
//
//			Conditional tags ("key" "value" [$WIN32], "block" [!$X360] { }) are
//			kept on the pair or block they follow, as Source's KeyValues reads
//			them; EvaluateCondition() evaluates one against caller symbols.
//
//=============================================================================//

#ifndef KVTEXT_KEYVALUES_H
#define KVTEXT_KEYVALUES_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace kvtext
{

struct KeyValue
{
	// Constructors, so the two-field { key, value } form stays valid everywhere.
	KeyValue() = default;
	KeyValue( std::string keyText, std::string valueText, std::string conditionText = {} )
	    : key( std::move( keyText ) ), value( std::move( valueText ) ),
	      condition( std::move( conditionText ) )
	{
	}

	std::string key;
	std::string value;
	std::string condition; // the tag's expression without brackets; empty when none

	friend bool operator==( const KeyValue &, const KeyValue & ) = default;
};

// A named block: ordered key/value pairs and ordered child blocks. The synthetic
// document root has an empty name and holds the top-level blocks as children.
struct KeyValueNode
{
	std::string name;
	std::string condition; // the block's conditional tag, as KeyValue::condition
	std::vector<KeyValue> pairs;
	std::vector<KeyValueNode> children;

	// First value for a key, or nullptr. Duplicate keys keep their first here.
	const std::string *Find( const std::string &key ) const;

	// Exact structural equality (names, pair order and child order all count).
	// CompareKeyValues is the semantic comparator with a divergence report.
	friend bool operator==( const KeyValueNode &, const KeyValueNode & ) = default;
};

struct ParseResult
{
	bool ok = false;
	KeyValueNode root; // children are the top-level blocks
	std::string error; // human-readable diagnostic when !ok
	std::size_t errorLine = 0;
};

// Parses VMF/keyvalues text. Handles quoted "strings", bare block names, nested
// { } blocks, // line comments and [conditional] tags after a value or before a
// block's '{' (a tag between a key and its value is an error, as in Source). On
// malformed input, returns ok == false with a diagnostic and the 1-based line;
// the partial tree is not published.
ParseResult ParseKeyValues( const std::string &text );

struct ParseOptions
{
	// End of input closes every open block, as Source's KeyValues reader does
	// (it stops at the end and keeps what it read). VMF parsing stays strict.
	bool closeBlocksAtEnd = false;
};

ParseResult ParseKeyValues( const std::string &text, const ParseOptions &options );

// Serializes a document (its children as top-level blocks) with tab indentation.
// Round-trip stable: ParseKeyValues(WriteKeyValues(d)) reproduces d semantically.
std::string WriteKeyValues( const KeyValueNode &root );

struct CompareResult
{
	bool equal = false;
	std::string firstDivergence; // empty when equal
};

// Evaluates a conditional tag's expression ("$WIN32 && !$OSX", brackets
// optional): '!', '&&', '||', parentheses, $SYMBOL (resolved by `symbol`, given
// the name without '$') and integer literals (nonzero is true), the grammar of
// Source's KeyValues. nullopt when the expression is malformed.
std::optional<bool> EvaluateCondition(
    std::string_view expression, const std::function<bool( std::string_view )> &symbol );

// Semantic comparison with explicit, enumerated normalization:
//  - Block NAME and conditional tag must match.
//  - Key/value pairs (with their tags) are compared as a multiset
//    (order-insensitive, duplicates significant) -- VMF key order within a
//    block is not semantically meaningful.
//  - Child blocks are compared IN ORDER and recursively -- solid/side ordering is
//    treated as significant.
// Returns the first divergence (a path) when not equal. The comparator never
// discards a field to force a match.
CompareResult CompareKeyValues( const KeyValueNode &a, const KeyValueNode &b );

} // namespace kvtext

#endif // KVTEXT_KEYVALUES_H
