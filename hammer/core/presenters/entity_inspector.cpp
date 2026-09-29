//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/entity_inspector.h.
//
//=============================================================================//

#include "hammer/presenters/entity_inspector.h"

#include "hammer/app/ops/entity_ops.h"
#include "hammer/presenters/object_label.h"
#include "hammer/scene/map_queries.h"
#include "presenter_text.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <set>

namespace hammer::presenters
{

using app::EditError;
using app::EditErrorCode;
using scene::ObjectId;

namespace
{

bool IsIntegerText( const std::string &token )
{
	std::size_t i = 0;
	if ( i < token.size() && ( token[i] == '-' || token[i] == '+' ) )
	{
		++i;
	}
	if ( i == token.size() )
	{
		return false;
	}
	for ( ; i < token.size(); ++i )
	{
		if ( token[i] < '0' || token[i] > '9' )
		{
			return false;
		}
	}
	return true;
}

std::optional<double> ParseFinite( const std::string &token )
{
	if ( token.empty() )
	{
		return std::nullopt;
	}
	errno = 0;
	char *end = nullptr;
	const double value = std::strtod( token.c_str(), &end );
	if ( end != token.c_str() + token.size() || errno == ERANGE || !std::isfinite( value ) )
	{
		return std::nullopt;
	}
	return value;
}

// Every whitespace-separated token as a finite number, or nothing.
std::optional<std::vector<double>> ParseNumbers( const std::string &value )
{
	std::vector<double> numbers;
	for ( const std::string &word : detail::Words( value ) )
	{
		const std::optional<double> number = ParseFinite( word );
		if ( !number )
		{
			return std::nullopt;
		}
		numbers.push_back( *number );
	}
	return numbers;
}

long long ParseFlags( const std::string &text )
{
	const std::vector<std::string> words = detail::Words( text );
	if ( words.size() != 1 || !IsIntegerText( words[0] ) )
	{
		return 0;
	}
	return std::strtoll( words[0].c_str(), nullptr, 10 );
}

bool IsNumericType( ports::KeyType type )
{
	switch ( type )
	{
	case ports::KeyType::Integer:
	case ports::KeyType::Float:
	case ports::KeyType::Boolean:
	case ports::KeyType::Flags:
	case ports::KeyType::Color255:
	case ports::KeyType::Color1:
	case ports::KeyType::Angle:
	case ports::KeyType::Vector:
		return true;
	default:
		return false;
	}
}

std::optional<std::string> TextProblem( const std::string &text, const char *what )
{
	if ( text.find( '"' ) != std::string::npos )
	{
		return std::string( what ) + " cannot contain '\"'";
	}
	if ( text.find_first_of( "\r\n" ) != std::string::npos )
	{
		return std::string( what ) + " cannot contain a line break";
	}
	return std::nullopt;
}

} // namespace

std::optional<std::string> ValidateKeyValue(
    const ports::KeyDefinition *definition, const std::string &value, bool allowFreeChoices )
{
	if ( std::optional<std::string> problem = TextProblem( value, "a value" ) )
	{
		return problem;
	}
	if ( !definition )
	{
		return std::nullopt;
	}
	const ports::KeyType type = definition->type;
	const std::vector<std::string> words = detail::Words( value );
	if ( IsNumericType( type ) && words.empty() )
	{
		return std::string( "a value is required" );
	}
	switch ( type )
	{
	case ports::KeyType::Integer:
		if ( words.size() != 1 || !IsIntegerText( words[0] ) )
		{
			return std::string( "expected an integer" );
		}
		return std::nullopt;
	case ports::KeyType::Float:
		if ( words.size() != 1 || !ParseFinite( words[0] ) )
		{
			return std::string( "expected a number" );
		}
		return std::nullopt;
	case ports::KeyType::Boolean:
		if ( words.size() != 1 || ( words[0] != "0" && words[0] != "1" ) )
		{
			return std::string( "expected 0 or 1" );
		}
		return std::nullopt;
	case ports::KeyType::Flags:
		if ( words.size() != 1 || !IsIntegerText( words[0] ) || words[0][0] == '-' )
		{
			return std::string( "expected a non-negative integer" );
		}
		return std::nullopt;
	case ports::KeyType::Choices:
		if ( allowFreeChoices )
		{
			return std::nullopt;
		}
		for ( const ports::KeyChoice &choice : definition->choices )
		{
			if ( choice.value == value )
			{
				return std::nullopt;
			}
		}
		return std::string( "not one of the choices" );
	case ports::KeyType::Color255:
	{
		const std::optional<std::vector<double>> numbers = ParseNumbers( value );
		if ( !numbers || ( numbers->size() != 3 && numbers->size() != 4 ) )
		{
			return std::string( "expected \"r g b\" or \"r g b brightness\"" );
		}
		bool allMinusOne = true;
		bool inRange = true;
		for ( std::size_t i = 0; i < 3; ++i )
		{
			allMinusOne = allMinusOne && ( *numbers )[i] == -1.0;
			inRange = inRange && IsIntegerText( words[i] ) && ( *numbers )[i] >= 0.0 &&
			          ( *numbers )[i] <= 255.0;
		}
		if ( !inRange && !allMinusOne )
		{
			return std::string( "color components must be integers in 0..255" );
		}
		return std::nullopt;
	}
	case ports::KeyType::Color1:
	{
		const std::optional<std::vector<double>> numbers = ParseNumbers( value );
		if ( !numbers || ( numbers->size() != 3 && numbers->size() != 4 ) )
		{
			return std::string( "expected three or four numbers" );
		}
		return std::nullopt;
	}
	case ports::KeyType::Angle:
	case ports::KeyType::Vector:
	{
		const std::optional<std::vector<double>> numbers = ParseNumbers( value );
		if ( !numbers || numbers->size() != 3 )
		{
			return std::string( type == ports::KeyType::Angle ? "expected \"pitch yaw roll\""
			                                                  : "expected \"x y z\"" );
		}
		return std::nullopt;
	}
	case ports::KeyType::SideList:
		for ( const std::string &word : words )
		{
			if ( !IsIntegerText( word ) || word[0] == '-' || word[0] == '+' )
			{
				return std::string( "expected side ids" );
			}
		}
		return std::nullopt;
	default:
		return std::nullopt;
	}
}

namespace
{

// The text a colour row's picker reads: its draft, else its Single value,
// else its default when the value is Unset (or 'mixedDefault' allows it).
const std::string *ColorSource( const KeyRow &row, bool mixedDefault )
{
	if ( row.drafted )
	{
		return &row.draftValue;
	}
	if ( row.value.IsSingle() )
	{
		return &row.value.Value();
	}
	if ( row.value.IsUnset() || mixedDefault )
	{
		return &row.defaultValue;
	}
	return nullptr;
}

bool IsColorType( ports::KeyType type )
{
	return type == ports::KeyType::Color255 || type == ports::KeyType::Color1;
}

} // namespace

std::optional<KeyColor> ColorOfRow( const KeyRow &row )
{
	if ( !IsColorType( row.type ) )
	{
		return std::nullopt;
	}
	const std::string *text = ColorSource( row, false );
	if ( !text )
	{
		return std::nullopt;
	}
	const std::optional<std::vector<double>> numbers = ParseNumbers( *text );
	if ( !numbers || numbers->size() < 3 )
	{
		return std::nullopt;
	}
	const double scale = row.type == ports::KeyType::Color255 ? 255.0 : 1.0;
	auto channel = [&]( double v )
	{
		return std::clamp( v / scale, 0.0, 1.0 );
	};
	return KeyColor{
	    channel( ( *numbers )[0] ), channel( ( *numbers )[1] ), channel( ( *numbers )[2] ) };
}

std::string ValueWithColor( const KeyRow &row, const KeyColor &color )
{
	if ( !IsColorType( row.type ) )
	{
		return std::string();
	}
	std::string out;
	for ( double v : { color.r, color.g, color.b } )
	{
		const double c = std::clamp( std::isfinite( v ) ? v : 0.0, 0.0, 1.0 );
		char buf[32];
		if ( row.type == ports::KeyType::Color255 )
		{
			std::snprintf( buf, sizeof( buf ), "%d", static_cast<int>( std::lround( c * 255.0 ) ) );
		}
		else
		{
			std::snprintf( buf, sizeof( buf ), "%.4g", std::round( c * 10000.0 ) / 10000.0 );
		}
		out += ( out.empty() ? "" : " " ) + std::string( buf );
	}
	if ( const std::string *text = ColorSource( row, true ) )
	{
		const std::vector<std::string> words = detail::Words( *text );
		for ( std::size_t i = 3; i < words.size(); ++i )
		{
			out += " " + words[i];
		}
	}
	return out;
}

EntityInspector::EntityInspector( app::EditSession &session, const ports::IEntityCatalog *catalog )
    : m_session( session ), m_catalog( catalog )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    OnEvent( event );
	    } );
	m_guard = m_session.AddSelectionGuard(
	    [this]()
	    {
		    return OnGuard();
	    } );
	Rebuild();
}

EntityInspector::~EntityInspector() = default;

void EntityInspector::OnEvent( const app::SessionEvent &event )
{
	if ( event.kind == app::SessionEventKind::Replaced )
	{
		// The guard already committed or vetoed; anything left is stale.
		m_draft.clear();
		m_draftTargets.clear();
	}
	Rebuild();
}

bool EntityInspector::OnGuard()
{
	if ( m_draft.empty() )
	{
		return true;
	}
	if ( std::optional<std::string> problem = DraftProblem() )
	{
		m_lastError = "The selection was kept: " + *problem;
		Bump();
		return false;
	}
	Result applied = ApplyDraft();
	if ( !applied )
	{
		m_lastError = "The selection was kept: the pending property edits could not be applied (" +
		              applied.Error().message + ")";
		Bump();
		return false;
	}
	m_draft.clear();
	m_draftTargets.clear();
	Bump();
	return true;
}

EntityInspector::Result EntityInspector::ApplyDraft()
{
	const std::vector<ObjectId> targets = m_draftTargets;
	const std::vector<std::pair<std::string, std::string>> values = m_draft;
	if ( !m_draftWorld && app::ops::EntitiesOf( m_session.Document(), targets ).empty() )
	{
		return foundation::MakeUnexpected(
		    EditError{ EditErrorCode::Rejected, "the drafted entities no longer exist" } );
	}
	const bool world = m_draftWorld;
	const ports::IEntityCatalog *catalog = m_catalog;
	auto committed = m_session.Execute( world ? "Edit world properties" : "Edit properties",
	    [&]( scene::DocumentEdit &edit ) -> app::EditResult
	    {
		    for ( const auto &entry : values )
		    {
			    app::EditResult r;
			    if ( world )
			    {
				    r = app::ops::SetWorldKey( edit, entry.first, entry.second );
			    }
			    else if ( detail::EqualsNoCase( entry.first, "targetname" ) )
			    {
				    for ( ObjectId id : targets )
				    {
					    r = app::ops::RenameEntity( edit, id, entry.second, true, catalog );
					    if ( !r && r.Error().code != EditErrorCode::Nothing )
					    {
						    return r;
					    }
				    }
				    r = {};
			    }
			    else
			    {
				    r = app::ops::SetKey( edit, targets, entry.first, entry.second );
			    }
			    if ( !r && r.Error().code != EditErrorCode::Nothing )
			    {
				    return r;
			    }
		    }
		    return {};
	    } );
	if ( !committed )
	{
		return foundation::MakeUnexpected( committed.Error() );
	}
	return {};
}

EntityInspector::Result EntityInspector::RefuseInWorld()
{
	return Fail( EditError{ EditErrorCode::Rejected, "not available for the world" } );
}

EntityInspector::Result EntityInspector::Fail( EditError error )
{
	m_lastError = error.message;
	Bump();
	return foundation::MakeUnexpected( std::move( error ) );
}

EntityInspector::Result EntityInspector::Run(
    const std::string &label, const app::EditSession::Operation &operation )
{
	auto committed = m_session.Execute( label, operation );
	if ( !committed )
	{
		return Fail( committed.Error() );
	}
	return {};
}

void EntityInspector::ClearError()
{
	if ( !m_lastError.empty() )
	{
		m_lastError.clear();
		Bump();
	}
}

void EntityInspector::SetSmartEdit( bool on )
{
	if ( m_smartEdit != on )
	{
		m_smartEdit = on;
		Rebuild();
	}
}

void EntityInspector::SetAllowFreeChoices( bool allow )
{
	if ( m_allowFreeChoices != allow )
	{
		m_allowFreeChoices = allow;
		Rebuild();
	}
}

const ports::KeyDefinition *EntityInspector::DefinitionOf( const std::string &key ) const
{
	for ( const ports::KeyDefinition *def : m_schema )
	{
		if ( detail::EqualsNoCase( def->key, key ) )
		{
			return def;
		}
	}
	return nullptr;
}

std::optional<std::string> EntityInspector::DraftProblem() const
{
	std::vector<std::string> errors = DraftErrors();
	if ( errors.empty() )
	{
		return std::nullopt;
	}
	std::string joined;
	for ( const std::string &line : errors )
	{
		joined += ( joined.empty() ? "" : "; " ) + line;
	}
	return joined;
}

std::vector<std::string> EntityInspector::DraftErrors() const
{
	std::vector<std::string> errors;
	for ( const auto &entry : m_draft )
	{
		if ( std::optional<std::string> problem =
		         ValidateKeyValue( DefinitionOf( entry.first ), entry.second, m_allowFreeChoices ) )
		{
			errors.push_back( entry.first + ": " + *problem );
		}
	}
	return errors;
}

EntityInspector::Result EntityInspector::SetDraft(
    const std::string &key, const std::string &value )
{
	if ( key.empty() || key == "classname" || key == "id" )
	{
		return Fail(
		    EditError{ EditErrorCode::Rejected, "'" + key + "' cannot be edited as a key" } );
	}
	if ( std::optional<std::string> problem = TextProblem( key, "a key name" ) )
	{
		return Fail( EditError{ EditErrorCode::Rejected, *problem } );
	}
	if ( const ports::KeyDefinition *def = DefinitionOf( key ); def && def->readOnly )
	{
		return Fail( EditError{ EditErrorCode::Rejected, "'" + key + "' is read-only" } );
	}
	if ( m_draft.empty() )
	{
		m_draftTargets = m_entities;
		m_draftWorld = m_world;
	}
	bool found = false;
	for ( auto &entry : m_draft )
	{
		if ( entry.first == key )
		{
			entry.second = value;
			found = true;
		}
	}
	if ( !found )
	{
		m_draft.emplace_back( key, value );
	}
	Rebuild();
	return {};
}

void EntityInspector::ClearDraft( const std::string &key )
{
	const std::size_t before = m_draft.size();
	std::erase_if( m_draft,
	    [&]( const auto &entry )
	    {
		    return entry.first == key;
	    } );
	if ( m_draft.empty() )
	{
		m_draftTargets.clear();
	}
	if ( m_draft.size() != before )
	{
		Rebuild();
	}
}

void EntityInspector::Cancel()
{
	if ( !m_draft.empty() )
	{
		m_draft.clear();
		m_draftTargets.clear();
		Rebuild();
	}
}

EntityInspector::Result EntityInspector::Commit()
{
	if ( m_draft.empty() )
	{
		return Fail( EditError{ EditErrorCode::Nothing, "no pending property edits" } );
	}
	if ( std::optional<std::string> problem = DraftProblem() )
	{
		return Fail( EditError{ EditErrorCode::Rejected, *problem } );
	}
	Result applied = ApplyDraft();
	if ( !applied )
	{
		return Fail( applied.Error() );
	}
	m_draft.clear();
	m_draftTargets.clear();
	Rebuild();
	return {};
}

EntityInspector::Result EntityInspector::Settle()
{
	if ( m_draft.empty() )
	{
		return {};
	}
	if ( std::optional<std::string> problem = DraftProblem() )
	{
		m_draft.clear();
		m_draftTargets.clear();
		m_lastError = "Pending property edits were discarded: " + *problem;
		Rebuild();
		return foundation::MakeUnexpected( EditError{ EditErrorCode::Rejected, m_lastError } );
	}
	return Commit();
}

EntityInspector::Result EntityInspector::SetClass( const std::string &classname )
{
	if ( m_world )
	{
		return RefuseInWorld();
	}
	const std::vector<ObjectId> targets = m_entities;
	return Run( "Change class to " + classname,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetClass( edit, targets, classname, m_catalog );
	    } );
}

EntityInspector::Result EntityInspector::SetFlag( long long bit, bool on )
{
	if ( m_world )
	{
		return RefuseInWorld();
	}
	std::string name = std::to_string( bit );
	for ( const FlagRow &row : m_flags )
	{
		if ( row.bit == bit && !row.label.empty() )
		{
			name = row.label;
		}
	}
	const std::vector<ObjectId> targets = m_entities;
	return Run( std::string( on ? "Set flag " : "Clear flag " ) + name,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::SetSpawnFlag( edit, targets, bit, on );
	    } );
}

EntityInspector::Result EntityInspector::RemoveKey( const std::string &key )
{
	if ( m_world )
	{
		return RefuseInWorld();
	}
	const std::vector<ObjectId> targets = m_entities;
	return Run( "Remove key " + key,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::RemoveKey( edit, targets, key );
	    } );
}

EntityInspector::Result EntityInspector::AddOutput( const scene::Connection &connection )
{
	if ( m_world )
	{
		return RefuseInWorld();
	}
	const std::vector<ObjectId> targets = m_entities;
	return Run( "Add output " + connection.output,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::AddConnection( edit, targets, connection );
	    } );
}

EntityInspector::Result EntityInspector::ReplaceOutput(
    ObjectId entity, std::size_t index, const scene::Connection &connection )
{
	if ( std::find( m_entities.begin(), m_entities.end(), entity ) == m_entities.end() )
	{
		return Fail( EditError{ EditErrorCode::Rejected, "not an inspected entity" } );
	}
	return Run( "Edit output " + connection.output,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::ReplaceConnection( edit, entity, index, connection );
	    } );
}

EntityInspector::Result EntityInspector::RemoveOutput( ObjectId entity, std::size_t index )
{
	const scene::Entity *e = m_session.Document().FindEntity( entity );
	if ( !e || std::find( m_entities.begin(), m_entities.end(), entity ) == m_entities.end() )
	{
		return Fail( EditError{ EditErrorCode::Rejected, "not an inspected entity" } );
	}
	if ( index >= e->connections.size() )
	{
		return Fail( EditError{ EditErrorCode::Rejected, "no such connection" } );
	}
	return Run( "Remove output " + e->connections[index].output,
	    [&]( scene::DocumentEdit &edit )
	    {
		    return app::ops::RemoveConnectionAt( edit, entity, index );
	    } );
}

void EntityInspector::Rebuild()
{
	const scene::MapDocument &doc = m_session.Document();
	std::vector<ObjectId> entities =
	    app::ops::EntitiesOf( doc, m_session.CurrentSelection().objects );
	const bool world = entities.empty();
	if ( !m_draft.empty() && ( world != m_draftWorld || entities != m_draftTargets ) )
	{
		m_draft.clear();
		m_draftTargets.clear();
		m_lastError = "Pending property edits were discarded: the inspected entities changed";
	}
	m_entities = std::move( entities );
	m_world = world;

	std::vector<const scene::Entity *> ents;
	// The key lists shown: each entity's keys, or the worldspawn's.
	std::vector<const std::vector<kvtext::KeyValue> *> keyLists;
	for ( ObjectId id : m_entities )
	{
		ents.push_back( doc.FindEntity( id ) );
		keyLists.push_back( &ents.back()->keys );
	}
	if ( world )
	{
		keyLists.push_back( &doc.Settings().worldKeys );
	}
	auto valueOf = []( const std::vector<kvtext::KeyValue> &keys,
	                   const std::string &key ) -> const std::string *
	{
		for ( const kvtext::KeyValue &kv : keys )
		{
			if ( kv.key == key )
			{
				return &kv.value;
			}
		}
		return nullptr;
	};

	// Class and schema.
	m_class = app::PropertyValue::Unset();
	for ( const scene::Entity *e : ents )
	{
		m_class = m_class.AddContributor( e->classname );
	}
	// Class names compare case-insensitively (the catalog's rule).
	if ( m_class.IsMixed() )
	{
		bool same = true;
		for ( const scene::Entity *e : ents )
		{
			same = same && detail::EqualsNoCase( e->classname, ents.front()->classname );
		}
		if ( same )
		{
			m_class = app::PropertyValue::Single( ents.front()->classname );
		}
	}
	if ( world )
	{
		m_class = app::PropertyValue::Single( "worldspawn" );
	}
	m_classInfo =
	    ( m_catalog && m_class.IsSingle() ) ? m_catalog->Find( m_class.Value() ) : nullptr;
	m_schema.clear();
	if ( world && m_classInfo )
	{
		for ( const ports::KeyDefinition &def : m_classInfo->keys )
		{
			m_schema.push_back( &def );
		}
	}
	if ( m_catalog && !ents.empty() )
	{
		std::vector<const ports::EntityClassInfo *> infos;
		for ( const scene::Entity *e : ents )
		{
			infos.push_back( m_catalog->Find( e->classname ) );
		}
		if ( std::find( infos.begin(), infos.end(), nullptr ) == infos.end() )
		{
			for ( const ports::KeyDefinition &def : infos.front()->keys )
			{
				bool everywhere = true;
				for ( const ports::EntityClassInfo *info : infos )
				{
					everywhere = everywhere && info->FindKey( def.key ) != nullptr;
				}
				if ( everywhere )
				{
					m_schema.push_back( &def );
				}
			}
		}
	}
	// Class choices (see the header): the kind is 0 point, 1 brush, 2 none.
	int kind = world ? 2 : -1;
	for ( ObjectId id : m_entities )
	{
		const int brush = scene::EntitySolids( doc, id ).empty() ? 0 : 1;
		kind = kind == -1 || kind == brush ? brush : 2;
	}
	if ( kind != m_kindListed )
	{
		m_kindListed = kind;
		m_kindClasses.clear();
		for ( const std::string &name :
		    ( m_catalog && kind != 2 ) ? m_catalog->ClassNames() : std::vector<std::string>() )
		{
			const ports::EntityClassInfo *info = m_catalog->Find( name );
			if ( info && !detail::EqualsNoCase( name, "worldspawn" ) &&
			     ( info->kind == ports::EntityClassKind::Solid ) == ( kind == 1 ) )
			{
				m_kindClasses.push_back( name );
			}
		}
	}
	m_classChoices.clear();
	if ( m_class.IsSingle() && !world &&
	     std::none_of( m_kindClasses.begin(), m_kindClasses.end(),
	         [&]( const std::string &name )
	         {
		         return detail::EqualsNoCase( name, m_class.Value() );
	         } ) )
	{
		m_classChoices.push_back( m_class.Value() );
	}
	m_classChoices.insert( m_classChoices.end(), m_kindClasses.begin(), m_kindClasses.end() );

	m_flagsKey.clear();
	const ports::KeyDefinition *flagsDef = nullptr;
	for ( const ports::KeyDefinition *def : m_schema )
	{
		if ( def->type == ports::KeyType::Flags )
		{
			flagsDef = def;
			m_flagsKey = def->key;
			break;
		}
	}

	// Rows.
	auto makeRow = [&]( const std::string &key, const ports::KeyDefinition *def ) -> KeyRow
	{
		KeyRow row;
		row.key = key;
		row.displayName = key;
		if ( def )
		{
			if ( m_smartEdit && !def->displayName.empty() )
			{
				row.displayName = def->displayName;
			}
			row.help = def->help;
			row.type = def->type;
			row.typeName = def->typeName;
			row.choices = def->choices;
			row.readOnly = def->readOnly;
			row.inSchema = true;
			row.defaultValue = def->defaultValue;
		}
		for ( const std::vector<kvtext::KeyValue> *keys : keyLists )
		{
			if ( const std::string *v = valueOf( *keys, key ) )
			{
				++row.presentCount;
				row.value = row.value.AddContributor( *v );
			}
		}
		if ( row.presentCount > 0 && row.presentCount < keyLists.size() )
		{
			row.value = app::PropertyValue::Mixed();
		}
		row.differsFromDefault =
		    row.inSchema ? ( row.value.IsMixed() ||
		                       ( row.value.IsSingle() && row.value.Value() != row.defaultValue ) )
		                 : !row.value.IsUnset();
		if ( row.value.IsSingle() )
		{
			row.valueLabel = row.value.Value();
			for ( const ports::KeyChoice &choice : row.choices )
			{
				if ( row.type == ports::KeyType::Choices && choice.value == row.value.Value() &&
				     !choice.label.empty() )
				{
					row.valueLabel = choice.label;
				}
			}
		}
		for ( const auto &entry : m_draft )
		{
			if ( entry.first == key )
			{
				row.drafted = true;
				row.draftValue = entry.second;
				row.draftError = ValidateKeyValue( def, entry.second, m_allowFreeChoices )
				                     .value_or( std::string() );
			}
		}
		return row;
	};

	m_rows.clear();
	std::set<std::string> shown;
	if ( m_smartEdit )
	{
		for ( const ports::KeyDefinition *def : m_schema )
		{
			if ( def->type == ports::KeyType::Flags )
			{
				continue;
			}
			m_rows.push_back( makeRow( def->key, def ) );
			shown.insert( detail::Lower( def->key ) );
		}
		if ( flagsDef )
		{
			shown.insert( detail::Lower( flagsDef->key ) );
		}
	}
	for ( const std::vector<kvtext::KeyValue> *keys : keyLists )
	{
		for ( const kvtext::KeyValue &kv : *keys )
		{
			if ( shown.insert( detail::Lower( kv.key ) ).second )
			{
				m_rows.push_back( makeRow( kv.key, DefinitionOf( kv.key ) ) );
			}
		}
	}
	for ( const auto &entry : m_draft )
	{
		if ( shown.insert( detail::Lower( entry.first ) ).second )
		{
			m_rows.push_back( makeRow( entry.first, DefinitionOf( entry.first ) ) );
		}
	}

	// Flags.
	m_flags.clear();
	if ( flagsDef && !ents.empty() )
	{
		long long defaultBits = 0;
		for ( const ports::KeyChoice &choice : flagsDef->choices )
		{
			if ( choice.defaultOn )
			{
				defaultBits |= ParseFlags( choice.value );
			}
		}
		if ( !flagsDef->defaultValue.empty() )
		{
			defaultBits = ParseFlags( flagsDef->defaultValue );
		}
		std::vector<long long> values;
		for ( const scene::Entity *e : ents )
		{
			const std::string *v = e->Key( flagsDef->key );
			values.push_back( v ? ParseFlags( *v ) : defaultBits );
		}
		for ( const ports::KeyChoice &choice : flagsDef->choices )
		{
			FlagRow row;
			row.bit = ParseFlags( choice.value );
			if ( row.bit <= 0 )
			{
				continue;
			}
			row.label = choice.label;
			row.defaultOn = choice.defaultOn;
			std::size_t on = 0;
			for ( long long v : values )
			{
				on += ( v & row.bit ) ? 1 : 0;
			}
			row.state = on == 0               ? FlagState::Off
			            : on == values.size() ? FlagState::On
			                                  : FlagState::Mixed;
			m_flags.push_back( row );
		}
	}

	// Outputs and inputs.
	m_outputs.clear();
	m_inputs.clear();
	for ( std::size_t n = 0; n < ents.size(); ++n )
	{
		const scene::Entity &e = *ents[n];
		const ports::EntityClassInfo *info = m_catalog ? m_catalog->Find( e.classname ) : nullptr;
		for ( std::size_t i = 0; i < e.connections.size(); ++i )
		{
			OutputRow row;
			row.entity = m_entities[n];
			row.index = i;
			row.entityLabel = ObjectLabel( doc, m_entities[n] );
			row.connection = e.connections[i];
			row.targetValid = IsProceduralTarget( row.connection.target );
			for ( const auto &other : doc.Entities() )
			{
				row.targetValid =
				    row.targetValid || TargetNamesEntity( row.connection.target, other.second );
			}
			row.outputKnown = !info || info->HasOutput( row.connection.output );
			m_outputs.push_back( std::move( row ) );
		}
	}
	for ( const auto &source : doc.Entities() )
	{
		for ( std::size_t i = 0; i < source.second.connections.size(); ++i )
		{
			const scene::Connection &c = source.second.connections[i];
			for ( std::size_t n = 0; n < ents.size(); ++n )
			{
				if ( m_entities[n] == source.first || !TargetNamesEntity( c.target, *ents[n] ) )
				{
					continue;
				}
				const ports::EntityClassInfo *info =
				    m_catalog ? m_catalog->Find( ents[n]->classname ) : nullptr;
				InputRow row;
				row.source = source.first;
				row.index = i;
				row.sourceLabel = ObjectLabel( doc, source.first );
				row.target = m_entities[n];
				row.targetLabel = ObjectLabel( doc, m_entities[n] );
				row.connection = c;
				row.inputKnown = !info || info->HasInput( c.input );
				m_inputs.push_back( std::move( row ) );
			}
		}
	}
	Bump();
}

} // namespace hammer::presenters
