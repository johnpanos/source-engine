//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A small JSON value, reader and writer for the Hammer MCP adapter
//			(RFC 0002, hammer.adapters.mcp). Private to the adapter, its only
//			consumer. Numbers keep their source text, so a request id is echoed
//			exactly; objects keep member order. The reader bounds nesting depth
//			and rejects trailing text, invalid escapes and unpaired surrogates.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_MCP_JSON_VALUE_H
#define HAMMER_ADAPTERS_MCP_JSON_VALUE_H

#include "foundation/expected.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace hammer::adapters::mcp
{

class JsonValue
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

	JsonValue() = default;
	static JsonValue Bool( bool value );
	// `text` must be a valid JSON number; it is written back verbatim.
	static JsonValue Number( std::string text );
	static JsonValue Number( long long value );
	static JsonValue String( std::string value );
	static JsonValue Array();
	static JsonValue Object();

	Kind GetKind() const { return m_kind; }
	bool IsNull() const { return m_kind == Kind::Null; }
	bool AsBool() const { return m_bool; }
	// The number's source text, or the string's value.
	const std::string &Text() const { return m_text; }
	const std::vector<JsonValue> &Items() const { return m_items; }
	const std::vector<std::pair<std::string, JsonValue>> &Members() const { return m_members; }

	// Object member lookup; nullptr when absent or not an object.
	const JsonValue *Find( std::string_view key ) const;

	JsonValue &Push( JsonValue value );
	JsonValue &Set( std::string key, JsonValue value );

	// Compact, single-line UTF-8 JSON (control characters are escaped).
	std::string Write() const;

private:
	void WriteTo( std::string &out ) const;

	Kind m_kind = Kind::Null;
	bool m_bool = false;
	std::string m_text;
	std::vector<JsonValue> m_items;
	std::vector<std::pair<std::string, JsonValue>> m_members;
};

struct JsonError
{
	size_t offset = 0;
	std::string detail;
};

inline constexpr int kMaxJsonDepth = 64; // nested arrays and objects

[[nodiscard]] foundation::Expected<JsonValue, JsonError> ParseJson( std::string_view text );

} // namespace hammer::adapters::mcp

#endif // HAMMER_ADAPTERS_MCP_JSON_VALUE_H
