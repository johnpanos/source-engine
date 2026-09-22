//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the VMF/keyvalues parser, writer, and semantic
//			comparator (RFC 0002). No tier0/MFC/PCH dependencies; headless-core.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"

#include <algorithm>

namespace hammer::formats
{

const std::string *KeyValueNode::Find( const std::string &key ) const
{
	for ( const KeyValue &pair : pairs )
	{
		if ( pair.key == key )
		{
			return &pair.value;
		}
	}
	return nullptr;
}

namespace
{

enum class TokenKind
{
	kString,
	kOpen,
	kClose,
	kEnd,
};

struct Token
{
	TokenKind kind = TokenKind::kEnd;
	std::string text;
	std::size_t line = 0;
};

class Tokenizer
{
public:
	explicit Tokenizer( const std::string &text ) : m_text( text ) {}

	// Produces the token stream, or sets 'error'/'errorLine' and returns false.
	bool Tokenize( std::vector<Token> &out, std::string &error, std::size_t &errorLine )
	{
		while ( m_pos < m_text.size() )
		{
			const char c = m_text[m_pos];
			if ( c == '\n' )
			{
				++m_line;
				++m_pos;
				continue;
			}
			if ( c == ' ' || c == '\t' || c == '\r' )
			{
				++m_pos;
				continue;
			}
			if ( c == '/' && m_pos + 1 < m_text.size() && m_text[m_pos + 1] == '/' )
			{
				while ( m_pos < m_text.size() && m_text[m_pos] != '\n' )
				{
					++m_pos;
				}
				continue;
			}
			if ( c == '{' )
			{
				out.push_back( { TokenKind::kOpen, "{", m_line } );
				++m_pos;
				continue;
			}
			if ( c == '}' )
			{
				out.push_back( { TokenKind::kClose, "}", m_line } );
				++m_pos;
				continue;
			}
			if ( c == '"' )
			{
				const std::size_t startLine = m_line;
				++m_pos;
				std::string value;
				while ( m_pos < m_text.size() && m_text[m_pos] != '"' )
				{
					if ( m_text[m_pos] == '\n' )
					{
						++m_line;
					}
					value.push_back( m_text[m_pos] );
					++m_pos;
				}
				if ( m_pos >= m_text.size() )
				{
					error = "unterminated quoted string";
					errorLine = startLine;
					return false;
				}
				++m_pos; // closing quote
				out.push_back( { TokenKind::kString, value, startLine } );
				continue;
			}
			// Bare word (block name): up to whitespace/brace/quote.
			std::string word;
			const std::size_t startLine = m_line;
			while ( m_pos < m_text.size() )
			{
				const char w = m_text[m_pos];
				if ( w == ' ' || w == '\t' || w == '\r' || w == '\n' || w == '{' || w == '}' ||
				     w == '"' )
				{
					break;
				}
				word.push_back( w );
				++m_pos;
			}
			out.push_back( { TokenKind::kString, word, startLine } );
		}
		out.push_back( { TokenKind::kEnd, "", m_line } );
		return true;
	}

private:
	const std::string &m_text;
	std::size_t m_pos = 0;
	std::size_t m_line = 1;
};

class Parser
{
public:
	Parser( const std::vector<Token> &tokens ) : m_tokens( tokens ) {}

	// Parses a block body into 'node' until a close brace or end of input.
	bool ParseBody(
	    KeyValueNode &node, bool atTopLevel, std::string &error, std::size_t &errorLine )
	{
		while ( true )
		{
			const Token &token = Peek();
			if ( token.kind == TokenKind::kEnd )
			{
				if ( atTopLevel )
				{
					return true;
				}
				error = "unexpected end of input inside a block";
				errorLine = token.line;
				return false;
			}
			if ( token.kind == TokenKind::kClose )
			{
				if ( atTopLevel )
				{
					error = "unexpected '}' at top level";
					errorLine = token.line;
					return false;
				}
				return true; // caller consumes the close brace
			}
			if ( token.kind == TokenKind::kOpen )
			{
				error = "expected a name or key before '{'";
				errorLine = token.line;
				return false;
			}

			// token is a string: either a block name (followed by '{') or a key.
			const Token first = Next();
			const Token &after = Peek();
			if ( after.kind == TokenKind::kOpen )
			{
				Next(); // consume '{'
				KeyValueNode child;
				child.name = first.text;
				if ( !ParseBody( child, false, error, errorLine ) )
				{
					return false;
				}
				if ( Peek().kind != TokenKind::kClose )
				{
					error = "expected '}' to close block '" + first.text + "'";
					errorLine = Peek().line;
					return false;
				}
				Next(); // consume '}'
				node.children.push_back( std::move( child ) );
				continue;
			}
			if ( after.kind == TokenKind::kString )
			{
				const Token value = Next();
				node.pairs.push_back( { first.text, value.text } );
				continue;
			}
			error = "expected a value or '{' after '" + first.text + "'";
			errorLine = after.line;
			return false;
		}
	}

private:
	const Token &Peek() const { return m_tokens[m_index]; }
	const Token &Next() { return m_tokens[m_index++]; }

	const std::vector<Token> &m_tokens;
	std::size_t m_index = 0;
};

void WriteNode( const KeyValueNode &node, int depth, std::string &out )
{
	const std::string indent( static_cast<std::size_t>( depth ), '\t' );
	out += indent;
	out += node.name;
	out += "\n";
	out += indent;
	out += "{\n";

	const std::string inner( static_cast<std::size_t>( depth + 1 ), '\t' );
	for ( const KeyValue &pair : node.pairs )
	{
		out += inner;
		out += '"';
		out += pair.key;
		out += "\" \"";
		out += pair.value;
		out += "\"\n";
	}
	for ( const KeyValueNode &child : node.children )
	{
		WriteNode( child, depth + 1, out );
	}

	out += indent;
	out += "}\n";
}

std::vector<std::string> SortedPairs( const KeyValueNode &node )
{
	std::vector<std::string> flat;
	flat.reserve( node.pairs.size() );
	for ( const KeyValue &pair : node.pairs )
	{
		flat.push_back( pair.key + std::string( 1, '\0' ) + pair.value );
	}
	std::sort( flat.begin(), flat.end() );
	return flat;
}

bool CompareNode(
    const KeyValueNode &a, const KeyValueNode &b, const std::string &path, std::string &divergence )
{
	if ( a.name != b.name )
	{
		divergence = path + ": block name '" + a.name + "' != '" + b.name + "'";
		return false;
	}

	const std::vector<std::string> pairsA = SortedPairs( a );
	const std::vector<std::string> pairsB = SortedPairs( b );
	if ( pairsA != pairsB )
	{
		divergence = path + " (" + a.name + "): key/value pairs differ";
		return false;
	}

	if ( a.children.size() != b.children.size() )
	{
		divergence = path + " (" + a.name + "): child block count differs";
		return false;
	}
	for ( std::size_t i = 0; i < a.children.size(); ++i )
	{
		const std::string childPath =
		    path + "/" + a.children[i].name + "[" + std::to_string( i ) + "]";
		if ( !CompareNode( a.children[i], b.children[i], childPath, divergence ) )
		{
			return false;
		}
	}
	return true;
}

} // namespace

ParseResult ParseKeyValues( const std::string &text )
{
	ParseResult result;
	std::vector<Token> tokens;
	Tokenizer tokenizer( text );
	if ( !tokenizer.Tokenize( tokens, result.error, result.errorLine ) )
	{
		result.ok = false;
		return result;
	}

	Parser parser( tokens );
	KeyValueNode root;
	if ( !parser.ParseBody( root, true, result.error, result.errorLine ) )
	{
		result.ok = false;
		return result;
	}
	result.ok = true;
	result.root = std::move( root );
	return result;
}

std::string WriteKeyValues( const KeyValueNode &root )
{
	std::string out;
	for ( const KeyValueNode &child : root.children )
	{
		WriteNode( child, 0, out );
	}
	return out;
}

CompareResult CompareKeyValues( const KeyValueNode &a, const KeyValueNode &b )
{
	CompareResult result;
	std::string divergence;
	// Compare the top-level block lists in order (wrapped through a root compare).
	KeyValueNode rootA;
	KeyValueNode rootB;
	rootA.children = a.children;
	rootB.children = b.children;
	if ( CompareNode( rootA, rootB, "", divergence ) )
	{
		result.equal = true;
		return result;
	}
	result.equal = false;
	result.firstDivergence = divergence;
	return result;
}

} // namespace hammer::formats
