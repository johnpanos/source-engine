//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the VMF/keyvalues parser, writer, and semantic
//			comparator (RFC 0002). No tier0/MFC/PCH dependencies; headless-core.
//
//=============================================================================//

#include "kvtext/keyvalues.h"

#include <algorithm>
#include <cctype>

namespace kvtext
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
	kCondition, // a [tag]; text is the expression without brackets
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
			// A conditional tag: '[' to ']' on one line, spaces allowed.
			if ( c == '[' )
			{
				const std::size_t close = m_text.find_first_of( "]\n", m_pos );
				if ( close == std::string::npos || m_text[close] != ']' )
				{
					error = "unterminated conditional tag";
					errorLine = m_line;
					return false;
				}
				out.push_back( { TokenKind::kCondition,
				    m_text.substr( m_pos + 1, close - m_pos - 1 ), m_line } );
				m_pos = close + 1;
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
	Parser( const std::vector<Token> &tokens, bool closeBlocksAtEnd )
	    : m_tokens( tokens ), m_closeBlocksAtEnd( closeBlocksAtEnd )
	{
	}

	// Parses a block body into 'node' until a close brace or end of input.
	bool ParseBody(
	    KeyValueNode &node, bool atTopLevel, std::string &error, std::size_t &errorLine )
	{
		while ( true )
		{
			const Token &token = Peek();
			if ( token.kind == TokenKind::kEnd )
			{
				if ( atTopLevel || m_closeBlocksAtEnd )
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
			if ( token.kind == TokenKind::kCondition )
			{
				error = "unexpected conditional tag [" + token.text + "]";
				errorLine = token.line;
				return false;
			}

			// token is a string: either a block name (followed by '{') or a key.
			const Token first = Next();
			std::string blockCondition;
			if ( Peek().kind == TokenKind::kCondition )
			{
				blockCondition = Next().text;
				if ( Peek().kind != TokenKind::kOpen )
				{
					error = "conditional tag between key '" + first.text + "' and its value";
					errorLine = Peek().line;
					return false;
				}
			}
			const Token &after = Peek();
			if ( after.kind == TokenKind::kOpen )
			{
				Next(); // consume '{'
				KeyValueNode child;
				child.name = first.text;
				child.condition = std::move( blockCondition );
				if ( !ParseBody( child, false, error, errorLine ) )
				{
					return false;
				}
				if ( Peek().kind == TokenKind::kClose )
				{
					Next(); // consume '}'
				}
				else if ( !( m_closeBlocksAtEnd && Peek().kind == TokenKind::kEnd ) )
				{
					error = "expected '}' to close block '" + first.text + "'";
					errorLine = Peek().line;
					return false;
				}
				node.children.push_back( std::move( child ) );
				continue;
			}
			if ( after.kind == TokenKind::kString )
			{
				const Token value = Next();
				std::string condition;
				if ( Peek().kind == TokenKind::kCondition )
				{
					condition = Next().text;
				}
				node.pairs.push_back( { first.text, value.text, std::move( condition ) } );
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
	bool m_closeBlocksAtEnd = false;
	std::size_t m_index = 0;
};

void WriteNode( const KeyValueNode &node, int depth, std::string &out )
{
	const std::string indent( static_cast<std::size_t>( depth ), '\t' );
	out += indent;
	out += node.name;
	if ( !node.condition.empty() )
	{
		out += " [" + node.condition + "]";
	}
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
		out += '"';
		if ( !pair.condition.empty() )
		{
			out += " [" + pair.condition + "]";
		}
		out += "\n";
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
		flat.push_back( pair.key + std::string( 1, '\0' ) + pair.value + std::string( 1, '\0' ) +
		                pair.condition );
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
	if ( a.condition != b.condition )
	{
		divergence = path + " (" + a.name + "): conditional tag '" + a.condition + "' != '" +
		             b.condition + "'";
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

// Source's KeyValues conditional grammar (tier1/kvconditional.cpp):
// or := and ( "||" and )*, and := unary ( "&&" unary )*,
// unary := "!" unary | "(" or ")" | "$" name | digits.
class ConditionParser
{
public:
	ConditionParser( std::string_view text, const std::function<bool( std::string_view )> &symbol )
	    : m_text( text ), m_symbol( symbol )
	{
	}

	std::optional<bool> Evaluate()
	{
		const bool value = ParseOr();
		SkipSpaces();
		if ( m_error || m_pos != m_text.size() )
		{
			return std::nullopt;
		}
		return value;
	}

private:
	void SkipSpaces()
	{
		while ( m_pos < m_text.size() && ( m_text[m_pos] == ' ' || m_text[m_pos] == '\t' ) )
		{
			++m_pos;
		}
	}

	bool Accept( std::string_view token )
	{
		SkipSpaces();
		if ( m_text.substr( m_pos, token.size() ) != token )
		{
			return false;
		}
		m_pos += token.size();
		return true;
	}

	bool ParseOr()
	{
		bool value = ParseAnd();
		while ( !m_error && Accept( "||" ) )
		{
			const bool right = ParseAnd();
			value = value || right;
		}
		return value;
	}

	bool ParseAnd()
	{
		bool value = ParseUnary();
		while ( !m_error && Accept( "&&" ) )
		{
			const bool right = ParseUnary();
			value = value && right;
		}
		return value;
	}

	bool ParseUnary()
	{
		if ( Accept( "!" ) )
		{
			return !ParseUnary();
		}
		if ( Accept( "(" ) )
		{
			const bool value = ParseOr();
			if ( !Accept( ")" ) )
			{
				m_error = true;
			}
			return value;
		}
		SkipSpaces();
		const bool isSymbol = m_pos < m_text.size() && m_text[m_pos] == '$';
		if ( isSymbol )
		{
			++m_pos;
		}
		const std::size_t start = m_pos;
		while ( m_pos < m_text.size() &&
		        ( std::isalnum( static_cast<unsigned char>( m_text[m_pos] ) ) ||
		            m_text[m_pos] == '_' ) )
		{
			++m_pos;
		}
		const std::string_view name = m_text.substr( start, m_pos - start );
		if ( name.empty() )
		{
			m_error = true;
			return false;
		}
		if ( isSymbol )
		{
			return m_symbol ? m_symbol( name ) : false;
		}
		bool nonzero = false;
		for ( const char c : name )
		{
			if ( !std::isdigit( static_cast<unsigned char>( c ) ) )
			{
				m_error = true;
				return false;
			}
			nonzero = nonzero || c != '0';
		}
		return nonzero;
	}

	std::string_view m_text;
	const std::function<bool( std::string_view )> &m_symbol;
	std::size_t m_pos = 0;
	bool m_error = false;
};

} // namespace

std::optional<bool> EvaluateCondition(
    std::string_view expression, const std::function<bool( std::string_view )> &symbol )
{
	// Accept the tag with its brackets, as a caller may hold it.
	if ( !expression.empty() && expression.front() == '[' )
	{
		while ( !expression.empty() && ( expression.back() == ' ' || expression.back() == '\t' ) )
		{
			expression.remove_suffix( 1 );
		}
		if ( expression.size() < 2 || expression.back() != ']' )
		{
			return std::nullopt;
		}
		expression = expression.substr( 1, expression.size() - 2 );
	}
	return ConditionParser( expression, symbol ).Evaluate();
}

ParseResult ParseKeyValues( const std::string &text )
{
	return ParseKeyValues( text, ParseOptions{} );
}

ParseResult ParseKeyValues( const std::string &text, const ParseOptions &options )
{
	ParseResult result;
	std::vector<Token> tokens;
	Tokenizer tokenizer( text );
	if ( !tokenizer.Tokenize( tokens, result.error, result.errorLine ) )
	{
		result.ok = false;
		return result;
	}

	Parser parser( tokens, options.closeBlocksAtEnd );
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

} // namespace kvtext
