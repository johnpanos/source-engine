//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: foundation.json (RFC 0027): the one JSON value, reader and writer,
//			shared by product profiles, kiln --json and hammer_cli --mcp
//			(extracted from the Hammer MCP adapter, its first consumer).
//			Numbers keep their source text, so a request id is echoed exactly;
//			objects keep member order. The reader bounds nesting depth and
//			rejects trailing text, duplicate members, invalid escapes and
//			unpaired surrogates. A Value is a plain value: no global state,
//			safe to share across threads while unmodified.
//
//=============================================================================//

#ifndef FOUNDATION_JSON_H
#define FOUNDATION_JSON_H

#include "foundation/expected.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace foundation::json
{

class Value
{
public:
	enum class Kind
	{
		Null,
		Bool,
		Number,
		String,
		Array,
		Object,
	};

	Value() = default;
	static Value Bool( bool value );
	// `text` must be a valid JSON number; it is written back verbatim.
	static Value Number( std::string text );
	static Value Number( long long value );
	static Value String( std::string value );
	static Value Array();
	static Value Object();

	Kind GetKind() const { return m_kind; }
	bool IsNull() const { return m_kind == Kind::Null; }
	bool IsBool() const { return m_kind == Kind::Bool; }
	bool IsNumber() const { return m_kind == Kind::Number; }
	bool IsString() const { return m_kind == Kind::String; }
	bool IsArray() const { return m_kind == Kind::Array; }
	bool IsObject() const { return m_kind == Kind::Object; }
	bool AsBool() const { return m_bool; }
	// The number's source text, or the string's value.
	const std::string &Text() const { return m_text; }
	const std::vector<Value> &Items() const { return m_items; }
	const std::vector<std::pair<std::string, Value>> &Members() const { return m_members; }

	// Object member lookup; nullptr when absent or not an object.
	const Value *Find( std::string_view key ) const;
	Value *Find( std::string_view key );
	// Removes an object member; false when absent.
	bool Remove( std::string_view key );
	// The string member's value, or nullptr when absent or not a string.
	const std::string *FindString( std::string_view key ) const;

	// The returned reference is valid until the next Push or Set on this
	// value (the storage may move); do not hold it across them.
	Value &Push( Value value );
	Value &Set( std::string key, Value value );

	// Compact, single-line UTF-8 JSON (control characters are escaped).
	std::string Write() const;
	// Multi-line JSON indented by `indent` spaces, members in order.
	std::string WritePretty( int indent = 2 ) const;

	// Structural equality: kinds, member order-insensitive objects, numbers
	// compared by value when both parse as doubles, else by text.
	friend bool operator==( const Value &a, const Value &b );

private:
	void WriteTo( std::string &out, int indent, int level ) const;

	Kind m_kind = Kind::Null;
	bool m_bool = false;
	std::string m_text;
	std::vector<Value> m_items;
	std::vector<std::pair<std::string, Value>> m_members;
};

struct ParseError
{
	size_t offset = 0;
	std::string detail;
};

inline constexpr int kMaxDepth = 64; // nested arrays and objects

[[nodiscard]] foundation::Expected<Value, ParseError> Parse( std::string_view text );

} // namespace foundation::json

#endif // FOUNDATION_JSON_H
