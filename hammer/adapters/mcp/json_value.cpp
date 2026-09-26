//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: JSON value, reader and writer for the Hammer MCP adapter.
//
//=============================================================================//

#include "json_value.h"

#include <cstdio>

namespace hammer::adapters::mcp
{

JsonValue JsonValue::Bool( bool value )
{
	JsonValue v;
	v.m_kind = Kind::Bool;
	v.m_bool = value;
	return v;
}

JsonValue JsonValue::Number( std::string text )
{
	JsonValue v;
	v.m_kind = Kind::Number;
	v.m_text = std::move( text );
	return v;
}

JsonValue JsonValue::Number( long long value )
{
	return Number( std::to_string( value ) );
}

JsonValue JsonValue::String( std::string value )
{
	JsonValue v;
	v.m_kind = Kind::String;
	v.m_text = std::move( value );
	return v;
}

JsonValue JsonValue::Array()
{
	JsonValue v;
	v.m_kind = Kind::Array;
	return v;
}

JsonValue JsonValue::Object()
{
	JsonValue v;
	v.m_kind = Kind::Object;
	return v;
}

const JsonValue *JsonValue::Find( std::string_view key ) const
{
	if ( m_kind != Kind::Object )
		return nullptr;
	for ( const auto &member : m_members )
	{
		if ( member.first == key )
			return &member.second;
	}
	return nullptr;
}

JsonValue &JsonValue::Push( JsonValue value )
{
	m_items.push_back( std::move( value ) );
	return m_items.back();
}

JsonValue &JsonValue::Set( std::string key, JsonValue value )
{
	for ( auto &member : m_members )
	{
		if ( member.first == key )
		{
			member.second = std::move( value );
			return member.second;
		}
	}
	m_members.emplace_back( std::move( key ), std::move( value ) );
	return m_members.back().second;
}

namespace
{

void WriteString( std::string &out, const std::string &text )
{
	out += '"';
	for ( const char c : text )
	{
		switch ( c )
		{
		case '"':
			out += "\\\"";
			break;
		case '\\':
			out += "\\\\";
			break;
		case '\n':
			out += "\\n";
			break;
		case '\r':
			out += "\\r";
			break;
		case '\t':
			out += "\\t";
			break;
		default:
			if ( static_cast<unsigned char>( c ) < 0x20 )
			{
				char buf[8];
				std::snprintf( buf, sizeof( buf ), "\\u%04x", static_cast<unsigned char>( c ) );
				out += buf;
			}
			else
			{
				out += c;
			}
		}
	}
	out += '"';
}

class Reader
{
public:
	explicit Reader( std::string_view text ) : m_text( text ) {}

	foundation::Expected<JsonValue, JsonError> Document()
	{
		JsonValue value;
		if ( !Value( value, 0 ) )
			return foundation::MakeUnexpected( m_error );
		SkipSpace();
		if ( m_pos != m_text.size() )
			return foundation::MakeUnexpected( JsonError{ m_pos, "trailing text" } );
		return value;
	}

private:
	bool Fail( std::string detail )
	{
		m_error = JsonError{ m_pos, std::move( detail ) };
		return false;
	}

	void SkipSpace()
	{
		while ( m_pos < m_text.size() && ( m_text[m_pos] == ' ' || m_text[m_pos] == '\t' ||
		                                     m_text[m_pos] == '\n' || m_text[m_pos] == '\r' ) )
			++m_pos;
	}

	bool Literal( std::string_view word )
	{
		if ( m_text.substr( m_pos, word.size() ) != word )
			return Fail( "invalid literal" );
		m_pos += word.size();
		return true;
	}

	bool Value( JsonValue &out, int depth )
	{
		SkipSpace();
		if ( m_pos >= m_text.size() )
			return Fail( "unexpected end of input" );
		const char c = m_text[m_pos];
		if ( ( c == '{' || c == '[' ) && depth >= kMaxJsonDepth )
			return Fail( "more than " + std::to_string( kMaxJsonDepth ) + " nested containers" );
		if ( c == '{' )
			return ObjectValue( out, depth );
		if ( c == '[' )
			return ArrayValue( out, depth );
		if ( c == '"' )
		{
			std::string text;
			if ( !StringValue( text ) )
				return false;
			out = JsonValue::String( std::move( text ) );
			return true;
		}
		if ( c == 't' )
		{
			out = JsonValue::Bool( true );
			return Literal( "true" );
		}
		if ( c == 'f' )
		{
			out = JsonValue::Bool( false );
			return Literal( "false" );
		}
		if ( c == 'n' )
		{
			out = JsonValue();
			return Literal( "null" );
		}
		return NumberValue( out );
	}

	bool Digits()
	{
		const size_t start = m_pos;
		while ( m_pos < m_text.size() && m_text[m_pos] >= '0' && m_text[m_pos] <= '9' )
			++m_pos;
		return m_pos > start;
	}

	bool NumberValue( JsonValue &out )
	{
		const size_t start = m_pos;
		if ( m_text[m_pos] == '-' )
			++m_pos;
		if ( m_pos < m_text.size() && m_text[m_pos] == '0' )
			++m_pos;
		else if ( !Digits() )
			return Fail( "invalid value" );
		if ( m_pos < m_text.size() && m_text[m_pos] == '.' )
		{
			++m_pos;
			if ( !Digits() )
				return Fail( "invalid number" );
		}
		if ( m_pos < m_text.size() && ( m_text[m_pos] == 'e' || m_text[m_pos] == 'E' ) )
		{
			++m_pos;
			if ( m_pos < m_text.size() && ( m_text[m_pos] == '+' || m_text[m_pos] == '-' ) )
				++m_pos;
			if ( !Digits() )
				return Fail( "invalid number" );
		}
		out = JsonValue::Number( std::string( m_text.substr( start, m_pos - start ) ) );
		return true;
	}

	bool Hex4( unsigned &out )
	{
		if ( m_pos + 4 > m_text.size() )
			return Fail( "truncated \\u escape" );
		out = 0;
		for ( int i = 0; i < 4; ++i )
		{
			const char c = m_text[m_pos++];
			out <<= 4;
			if ( c >= '0' && c <= '9' )
				out |= static_cast<unsigned>( c - '0' );
			else if ( c >= 'a' && c <= 'f' )
				out |= static_cast<unsigned>( c - 'a' + 10 );
			else if ( c >= 'A' && c <= 'F' )
				out |= static_cast<unsigned>( c - 'A' + 10 );
			else
				return Fail( "invalid \\u escape" );
		}
		return true;
	}

	static void AppendUtf8( std::string &out, unsigned cp )
	{
		if ( cp < 0x80 )
		{
			out += static_cast<char>( cp );
		}
		else if ( cp < 0x800 )
		{
			out += static_cast<char>( 0xC0 | ( cp >> 6 ) );
			out += static_cast<char>( 0x80 | ( cp & 0x3F ) );
		}
		else if ( cp < 0x10000 )
		{
			out += static_cast<char>( 0xE0 | ( cp >> 12 ) );
			out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
			out += static_cast<char>( 0x80 | ( cp & 0x3F ) );
		}
		else
		{
			out += static_cast<char>( 0xF0 | ( cp >> 18 ) );
			out += static_cast<char>( 0x80 | ( ( cp >> 12 ) & 0x3F ) );
			out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
			out += static_cast<char>( 0x80 | ( cp & 0x3F ) );
		}
	}

	bool StringValue( std::string &out )
	{
		++m_pos; // opening quote
		while ( true )
		{
			if ( m_pos >= m_text.size() )
				return Fail( "unterminated string" );
			const char c = m_text[m_pos++];
			if ( c == '"' )
				return true;
			if ( static_cast<unsigned char>( c ) < 0x20 )
				return Fail( "control character in string" );
			if ( c != '\\' )
			{
				out += c;
				continue;
			}
			if ( m_pos >= m_text.size() )
				return Fail( "unterminated string" );
			const char e = m_text[m_pos++];
			switch ( e )
			{
			case '"':
			case '\\':
			case '/':
				out += e;
				break;
			case 'b':
				out += '\b';
				break;
			case 'f':
				out += '\f';
				break;
			case 'n':
				out += '\n';
				break;
			case 'r':
				out += '\r';
				break;
			case 't':
				out += '\t';
				break;
			case 'u':
			{
				unsigned cp = 0;
				if ( !Hex4( cp ) )
					return false;
				if ( cp >= 0xDC00 && cp <= 0xDFFF )
					return Fail( "unpaired low surrogate" );
				if ( cp >= 0xD800 && cp <= 0xDBFF )
				{
					unsigned low = 0;
					if ( m_text.substr( m_pos, 2 ) != "\\u" )
						return Fail( "unpaired high surrogate" );
					m_pos += 2;
					if ( !Hex4( low ) )
						return false;
					if ( low < 0xDC00 || low > 0xDFFF )
						return Fail( "unpaired high surrogate" );
					cp = 0x10000 + ( ( cp - 0xD800 ) << 10 ) + ( low - 0xDC00 );
				}
				AppendUtf8( out, cp );
				break;
			}
			default:
				return Fail( "invalid escape" );
			}
		}
	}

	bool ArrayValue( JsonValue &out, int depth )
	{
		++m_pos;
		out = JsonValue::Array();
		SkipSpace();
		if ( m_pos < m_text.size() && m_text[m_pos] == ']' )
		{
			++m_pos;
			return true;
		}
		while ( true )
		{
			JsonValue item;
			if ( !Value( item, depth + 1 ) )
				return false;
			out.Push( std::move( item ) );
			SkipSpace();
			if ( m_pos >= m_text.size() )
				return Fail( "unterminated array" );
			const char c = m_text[m_pos++];
			if ( c == ']' )
				return true;
			if ( c != ',' )
				return Fail( "expected ',' or ']'" );
		}
	}

	bool ObjectValue( JsonValue &out, int depth )
	{
		++m_pos;
		out = JsonValue::Object();
		SkipSpace();
		if ( m_pos < m_text.size() && m_text[m_pos] == '}' )
		{
			++m_pos;
			return true;
		}
		while ( true )
		{
			SkipSpace();
			if ( m_pos >= m_text.size() || m_text[m_pos] != '"' )
				return Fail( "expected a member name" );
			std::string key;
			if ( !StringValue( key ) )
				return false;
			SkipSpace();
			if ( m_pos >= m_text.size() || m_text[m_pos] != ':' )
				return Fail( "expected ':'" );
			++m_pos;
			JsonValue value;
			if ( !Value( value, depth + 1 ) )
				return false;
			if ( out.Find( key ) )
				return Fail( "duplicate member \"" + key + "\"" );
			out.Set( std::move( key ), std::move( value ) );
			SkipSpace();
			if ( m_pos >= m_text.size() )
				return Fail( "unterminated object" );
			const char c = m_text[m_pos++];
			if ( c == '}' )
				return true;
			if ( c != ',' )
				return Fail( "expected ',' or '}'" );
		}
	}

	std::string_view m_text;
	size_t m_pos = 0;
	JsonError m_error;
};

} // namespace

void JsonValue::WriteTo( std::string &out ) const
{
	switch ( m_kind )
	{
	case Kind::Null:
		out += "null";
		break;
	case Kind::Bool:
		out += m_bool ? "true" : "false";
		break;
	case Kind::Number:
		out += m_text;
		break;
	case Kind::String:
		WriteString( out, m_text );
		break;
	case Kind::Array:
		out += '[';
		for ( size_t i = 0; i < m_items.size(); ++i )
		{
			if ( i )
				out += ',';
			m_items[i].WriteTo( out );
		}
		out += ']';
		break;
	case Kind::Object:
		out += '{';
		for ( size_t i = 0; i < m_members.size(); ++i )
		{
			if ( i )
				out += ',';
			WriteString( out, m_members[i].first );
			out += ':';
			m_members[i].second.WriteTo( out );
		}
		out += '}';
		break;
	}
}

std::string JsonValue::Write() const
{
	std::string out;
	WriteTo( out );
	return out;
}

foundation::Expected<JsonValue, JsonError> ParseJson( std::string_view text )
{
	return Reader( text ).Document();
}

} // namespace hammer::adapters::mcp
