//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the FGD entity-schema parser. See
//			public/hammer/formats/fgd.h.
//
//=============================================================================//

#include "hammer/formats/fgd.h"

#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

enum class TokType
{
	At,   // @Word  (text holds the word after '@', e.g. "PointClass")
	Word, // a bare run: identifier, number, path, "+", etc.
	Str,  // a "quoted string" (text without the quotes)
	LParen,
	RParen,
	LBracket,
	RBracket,
	Colon,
	Equals,
	Comma,
	End,
};

struct Tok
{
	TokType type = TokType::End;
	std::string text;
	std::size_t line = 0;
};

bool IsSpecial( char c )
{
	return c == '(' || c == ')' || c == '[' || c == ']' || c == ':' || c == '=' || c == ',' ||
	       c == '"' || c == '@';
}

// Tokenizes FGD text. Comments ("// ... EOL") are skipped. Never fails; malformed
// structure is diagnosed by the parser.
std::vector<Tok> Tokenize( const std::string &text )
{
	std::vector<Tok> toks;
	std::size_t i = 0;
	std::size_t line = 1;
	const std::size_t n = text.size();

	while ( i < n )
	{
		const char c = text[i];
		if ( c == '\n' )
		{
			++line;
			++i;
			continue;
		}
		if ( std::isspace( static_cast<unsigned char>( c ) ) )
		{
			++i;
			continue;
		}
		if ( c == '/' && i + 1 < n && text[i + 1] == '/' )
		{
			while ( i < n && text[i] != '\n' )
			{
				++i;
			}
			continue;
		}
		if ( c == '@' )
		{
			++i;
			std::string word;
			while ( i < n && !std::isspace( static_cast<unsigned char>( text[i] ) ) &&
			        !IsSpecial( text[i] ) )
			{
				word += text[i++];
			}
			toks.push_back( { TokType::At, word, line } );
			continue;
		}
		if ( c == '"' )
		{
			++i; // opening quote
			std::string str;
			while ( i < n && text[i] != '"' )
			{
				if ( text[i] == '\n' )
				{
					++line;
				}
				str += text[i++];
			}
			if ( i < n )
			{
				++i; // closing quote
			}
			toks.push_back( { TokType::Str, str, line } );
			continue;
		}
		switch ( c )
		{
		case '(':
			toks.push_back( { TokType::LParen, "(", line } );
			++i;
			continue;
		case ')':
			toks.push_back( { TokType::RParen, ")", line } );
			++i;
			continue;
		case '[':
			toks.push_back( { TokType::LBracket, "[", line } );
			++i;
			continue;
		case ']':
			toks.push_back( { TokType::RBracket, "]", line } );
			++i;
			continue;
		case ':':
			toks.push_back( { TokType::Colon, ":", line } );
			++i;
			continue;
		case '=':
			toks.push_back( { TokType::Equals, "=", line } );
			++i;
			continue;
		case ',':
			toks.push_back( { TokType::Comma, ",", line } );
			++i;
			continue;
		default:
			break;
		}
		// A bare word: run until whitespace, a special, or a "//" comment.
		std::string word;
		while ( i < n && !std::isspace( static_cast<unsigned char>( text[i] ) ) &&
		        !IsSpecial( text[i] ) )
		{
			if ( text[i] == '/' && i + 1 < n && text[i + 1] == '/' )
			{
				break;
			}
			word += text[i++];
		}
		toks.push_back( { TokType::Word, word, line } );
	}

	toks.push_back( { TokType::End, "", line } );
	return toks;
}

std::string Lower( std::string s )
{
	for ( char &c : s )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return s;
}

EntityKind KindFromKeyword( const std::string &kw )
{
	const std::string k = Lower( kw );
	if ( k == "pointclass" )
	{
		return EntityKind::Point;
	}
	if ( k == "solidclass" )
	{
		return EntityKind::Solid;
	}
	if ( k == "baseclass" )
	{
		return EntityKind::Base;
	}
	if ( k == "keyvalueclass" )
	{
		return EntityKind::KeyValue;
	}
	if ( k == "npcclass" )
	{
		return EntityKind::Npc;
	}
	if ( k == "filterclass" )
	{
		return EntityKind::Filter;
	}
	if ( k == "moveclass" )
	{
		return EntityKind::Move;
	}
	return EntityKind::Other;
}

bool IsClassKeyword( const std::string &kw )
{
	const std::string k = Lower( kw );
	return k == "pointclass" || k == "solidclass" || k == "baseclass" || k == "keyvalueclass" ||
	       k == "npcclass" || k == "filterclass" || k == "moveclass";
}

// Recursive-descent parser over the token stream.
class Parser
{
public:
	explicit Parser( std::vector<Tok> toks ) : m_toks( std::move( toks ) ) {}

	FgdParseResult Parse()
	{
		FgdParseResult result;
		while ( Peek().type != TokType::End )
		{
			const Tok &t = Peek();
			if ( t.type == TokType::At )
			{
				if ( IsClassKeyword( t.text ) )
				{
					EntityClass cls;
					if ( !ParseClass( cls ) )
					{
						result.error = m_error;
						result.errorLine = m_errorLine;
						return result;
					}
					result.classes.push_back( std::move( cls ) );
				}
				else
				{
					SkipDirective();
				}
			}
			else
			{
				Fail( "expected a top-level @-directive", t.line );
				result.error = m_error;
				result.errorLine = m_errorLine;
				return result;
			}
		}
		result.ok = true;
		return result;
	}

private:
	const Tok &Peek( std::size_t ahead = 0 ) const
	{
		const std::size_t idx = m_pos + ahead;
		return idx < m_toks.size() ? m_toks[idx] : m_toks.back();
	}
	const Tok &Next() { return m_toks[m_pos < m_toks.size() - 1 ? m_pos++ : m_pos]; }

	void Fail( const std::string &msg, std::size_t line )
	{
		if ( m_error.empty() )
		{
			m_error = msg;
			m_errorLine = line;
		}
		m_failed = true;
	}

	// Skips a non-class top-level directive: optional (...) args and/or a [ ... ]
	// body, or a run of tokens up to the next top-level @.
	void SkipDirective()
	{
		Next(); // the @word
		while ( true )
		{
			const Tok &t = Peek();
			if ( t.type == TokType::End || t.type == TokType::At )
			{
				return;
			}
			if ( t.type == TokType::LBracket )
			{
				SkipBracketBody();
				return;
			}
			Next();
		}
	}

	void SkipBracketBody()
	{
		int depth = 0;
		do
		{
			const Tok &t = Next();
			if ( t.type == TokType::LBracket )
			{
				++depth;
			}
			else if ( t.type == TokType::RBracket )
			{
				--depth;
			}
			else if ( t.type == TokType::End )
			{
				return;
			}
		} while ( depth > 0 );
	}

	// Reads a value: a quoted string (with "a" + "b" concatenation) or a bare word.
	std::string ReadValue()
	{
		if ( Peek().type == TokType::Str )
		{
			std::string s = Next().text;
			while ( Peek().type == TokType::Word && Peek().text == "+" &&
			        Peek( 1 ).type == TokType::Str )
			{
				Next(); // '+'
				s += Next().text;
			}
			return s;
		}
		if ( Peek().type == TokType::Word )
		{
			return Next().text;
		}
		return std::string();
	}

	bool ParseClass( EntityClass &out )
	{
		const Tok &kw = Next(); // @ClassKind
		out.kind = KindFromKeyword( kw.text );

		// Helpers until '='.
		while ( Peek().type != TokType::Equals )
		{
			const Tok &t = Peek();
			if ( t.type == TokType::End || t.type == TokType::At )
			{
				Fail( "unterminated class header (missing '=')", t.line );
				return false;
			}
			if ( t.type == TokType::Word )
			{
				const std::string helper = Lower( t.text );
				Next();
				if ( Peek().type == TokType::LParen )
				{
					std::vector<std::string> args;
					if ( !ReadParenArgs( args ) )
					{
						return false;
					}
					if ( helper == "base" )
					{
						for ( std::string &a : args )
						{
							out.bases.push_back( std::move( a ) );
						}
					}
				}
				// A helper word without parens (e.g. a modifier) is simply ignored.
			}
			else
			{
				Next(); // stray punctuation in the header; ignore defensively
			}
		}
		Next(); // '='

		if ( Peek().type != TokType::Word )
		{
			Fail( "expected class name after '='", Peek().line );
			return false;
		}
		out.name = Next().text;

		if ( Peek().type == TokType::Colon )
		{
			Next();
			out.description = ReadValue();
		}

		if ( Peek().type != TokType::LBracket )
		{
			Fail( "expected '[' to open class body", Peek().line );
			return false;
		}
		Next(); // '['

		while ( Peek().type != TokType::RBracket )
		{
			if ( Peek().type == TokType::End )
			{
				Fail( "unterminated class body (missing ']')", Peek().line );
				return false;
			}
			if ( !ParseMember( out ) )
			{
				return false;
			}
		}
		Next(); // ']'
		return true;
	}

	// Reads "( a, b, c )" into args (words and strings; commas optional-tolerant).
	bool ReadParenArgs( std::vector<std::string> &args )
	{
		Next(); // '('
		while ( Peek().type != TokType::RParen )
		{
			const Tok &t = Peek();
			if ( t.type == TokType::End )
			{
				Fail( "unterminated '(' argument list", t.line );
				return false;
			}
			if ( t.type == TokType::Word || t.type == TokType::Str )
			{
				args.push_back( Next().text );
			}
			else
			{
				Next(); // comma or stray token
			}
		}
		Next(); // ')'
		return true;
	}

	bool ParseMember( EntityClass &cls )
	{
		// input/output declarations: parsed and skipped (not stored this slice).
		if ( Peek().type == TokType::Word &&
		     ( Lower( Peek().text ) == "input" || Lower( Peek().text ) == "output" ) )
		{
			Next();                             // input/output
			if ( Peek().type == TokType::Word ) // io name
			{
				Next();
			}
			if ( Peek().type == TokType::LParen )
			{
				std::vector<std::string> args;
				if ( !ReadParenArgs( args ) )
				{
					return false;
				}
			}
			if ( Peek().type == TokType::Colon )
			{
				Next();
				ReadValue(); // help text, ignored
			}
			return true;
		}

		if ( Peek().type != TokType::Word )
		{
			Fail( "expected a property name in class body", Peek().line );
			return false;
		}

		FgdProperty prop;
		prop.name = Next().text;

		if ( Peek().type != TokType::LParen )
		{
			Fail( "expected '(' after property name '" + prop.name + "'", Peek().line );
			return false;
		}
		std::vector<std::string> typeArgs;
		if ( !ReadParenArgs( typeArgs ) )
		{
			return false;
		}
		if ( !typeArgs.empty() )
		{
			prop.type = typeArgs.front();
		}

		// Optional modifiers (e.g. 'readonly', 'report') before ':' or '=': skip
		// bare words that are not the start of the next member ("word (").
		while ( Peek().type == TokType::Word && Peek( 1 ).type != TokType::LParen &&
		        Peek( 1 ).type != TokType::End )
		{
			Next();
		}

		// Colon-separated fields: display, default, help (any may be empty).
		int field = 0;
		while ( Peek().type == TokType::Colon )
		{
			Next(); // ':'
			std::string value;
			const bool nextIsMember =
			    Peek().type == TokType::Word && Peek( 1 ).type == TokType::LParen;
			if ( ( Peek().type == TokType::Str || Peek().type == TokType::Word ) && !nextIsMember )
			{
				value = ReadValue();
			}
			if ( field == 0 )
			{
				prop.displayName = value;
			}
			else if ( field == 1 )
			{
				prop.defaultValue = value;
			}
			else if ( field == 2 )
			{
				prop.help = value;
			}
			++field;
		}

		// Optional choices/flags body.
		if ( Peek().type == TokType::Equals )
		{
			Next(); // '='
			if ( Peek().type != TokType::LBracket )
			{
				Fail( "expected '[' after '=' in property '" + prop.name + "'", Peek().line );
				return false;
			}
			Next(); // '['
			while ( Peek().type != TokType::RBracket )
			{
				if ( Peek().type == TokType::End )
				{
					Fail( "unterminated choices list in '" + prop.name + "'", Peek().line );
					return false;
				}
				FgdChoice choice;
				choice.value = ReadValue();
				if ( Peek().type == TokType::Colon )
				{
					Next();
					choice.label = ReadValue();
				}
				// A flags entry has a third ": <default>" field; consume it.
				if ( Peek().type == TokType::Colon )
				{
					Next();
					ReadValue();
				}
				prop.choices.push_back( std::move( choice ) );
			}
			Next(); // ']'
		}

		cls.properties.push_back( std::move( prop ) );
		return true;
	}

	std::vector<Tok> m_toks;
	std::size_t m_pos = 0;
	std::string m_error;
	std::size_t m_errorLine = 0;
	bool m_failed = false;
};

const EntityClass *FindClass( const std::vector<EntityClass> &classes, const std::string &name )
{
	for ( const EntityClass &c : classes )
	{
		if ( c.name == name )
		{
			return &c;
		}
	}
	return nullptr;
}

// Merges 'prop' into 'out', overriding any existing property with the same name.
void MergeProperty( std::vector<FgdProperty> &out, const FgdProperty &prop )
{
	for ( FgdProperty &existing : out )
	{
		if ( existing.name == prop.name )
		{
			existing = prop;
			return;
		}
	}
	out.push_back( prop );
}

void CollectResolved( const std::vector<EntityClass> &classes, const std::string &name,
    std::vector<FgdProperty> &out, std::vector<std::string> &visiting )
{
	for ( const std::string &v : visiting )
	{
		if ( v == name )
		{
			return; // cycle guard
		}
	}
	const EntityClass *cls = FindClass( classes, name );
	if ( cls == nullptr )
	{
		return;
	}
	visiting.push_back( name );
	for ( const std::string &base : cls->bases )
	{
		CollectResolved( classes, base, out, visiting );
	}
	for ( const FgdProperty &prop : cls->properties )
	{
		MergeProperty( out, prop );
	}
	visiting.pop_back();
}

} // namespace

FgdParseResult ParseFgd( const std::string &text )
{
	Parser parser( Tokenize( text ) );
	return parser.Parse();
}

std::optional<EntityClass> ResolveClass(
    const std::vector<EntityClass> &classes, const std::string &name )
{
	const EntityClass *cls = FindClass( classes, name );
	if ( cls == nullptr )
	{
		return std::nullopt;
	}
	EntityClass resolved = *cls;
	resolved.properties.clear();
	std::vector<std::string> visiting;
	CollectResolved( classes, name, resolved.properties, visiting );
	return resolved;
}

} // namespace hammer::formats
