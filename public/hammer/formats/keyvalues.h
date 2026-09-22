//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMF/keyvalues document model, parser, writer, and semantic comparator
//			(RFC 0002, hammer.formats). Source's VMF is a nested keyvalues text
//			format: named blocks containing quoted "key" "value" pairs and child
//			blocks. This is the dependency-free foundation for the H2 persistence
//			slice and the HAM-CORPUS-001 versioned semantic comparator: no MFC,
//			tier0, PCH, or GPU.
//
//			The parser preserves unknown blocks/keys verbatim (nothing is dropped),
//			so round-tripping an unrecognized VMF chunk does not lose it. The
//			semantic comparator's normalization is explicit (see CompareKeyValues).
//
//=============================================================================//

#ifndef HAMMER_FORMATS_KEYVALUES_H
#define HAMMER_FORMATS_KEYVALUES_H

#include <cstddef>
#include <string>
#include <vector>

namespace hammer::formats
{

struct KeyValue
{
	std::string key;
	std::string value;
};

// A named block: ordered key/value pairs and ordered child blocks. The synthetic
// document root has an empty name and holds the top-level blocks as children.
struct KeyValueNode
{
	std::string name;
	std::vector<KeyValue> pairs;
	std::vector<KeyValueNode> children;

	// First value for a key, or nullptr. Duplicate keys keep their first here.
	const std::string *Find( const std::string &key ) const;
};

struct ParseResult
{
	bool ok = false;
	KeyValueNode root; // children are the top-level blocks
	std::string error; // human-readable diagnostic when !ok
	std::size_t errorLine = 0;
};

// Parses VMF/keyvalues text. Handles quoted "strings", bare block names, nested
// { } blocks, and // line comments. On malformed input, returns ok == false with
// a diagnostic and the 1-based line; the partial tree is not published.
ParseResult ParseKeyValues( const std::string &text );

// Serializes a document (its children as top-level blocks) with tab indentation.
// Round-trip stable: ParseKeyValues(WriteKeyValues(d)) reproduces d semantically.
std::string WriteKeyValues( const KeyValueNode &root );

struct CompareResult
{
	bool equal = false;
	std::string firstDivergence; // empty when equal
};

// Semantic comparison with explicit, enumerated normalization:
//  - Block NAME must match.
//  - Key/value pairs are compared as a multiset (order-insensitive, duplicates
//    significant) -- VMF key order within a block is not semantically meaningful.
//  - Child blocks are compared IN ORDER and recursively -- solid/side ordering is
//    treated as significant.
// Returns the first divergence (a path) when not equal. The comparator never
// discards a field to force a match.
CompareResult CompareKeyValues( const KeyValueNode &a, const KeyValueNode &b );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_KEYVALUES_H
