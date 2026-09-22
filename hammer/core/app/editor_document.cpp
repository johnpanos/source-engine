//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the headless editor document (RFC 0002). No
//			tier0/MFC/PCH dependencies; headless-core profile.
//
//=============================================================================//

#include "hammer/app/editor_document.h"

namespace hammer::app
{

EditorDocument::EditorDocument()
{
	ResetTo( formats::KeyValueNode{} );
}

void EditorDocument::ResetTo( formats::KeyValueNode content )
{
	m_content = std::move( content );
	m_history = DocumentHistory();
	m_snapshots.clear();
	m_snapshots.push_back( m_content );
	m_history.MarkSaved(); // a freshly loaded/created document is unmodified
}

void EditorDocument::CommitSnapshot( formats::KeyValueNode next )
{
	// Advance history by one unit, then align the snapshot list so snapshots[pos]
	// holds the content at each reachable history position (dropping any redo tail).
	const std::size_t oldPos = m_history.Position();
	m_history.Commit( true );
	m_snapshots.resize( oldPos + 1 );
	m_snapshots.push_back( next );
	m_content = std::move( next );
}

bool EditorDocument::LoadFromText( const std::string &vmf, std::string &error )
{
	// Build a detached candidate; publish only on success.
	const formats::ParseResult parsed = formats::ParseKeyValues( vmf );
	if ( !parsed.ok )
	{
		error = parsed.error;
		return false;
	}
	ResetTo( parsed.root );
	return true;
}

bool EditorDocument::Load( ports::IFileStore &store, const std::string &path, std::string &error )
{
	std::string text;
	if ( !store.Read( path, text ) )
	{
		error = "could not read '" + path + "'";
		return false;
	}
	return LoadFromText( text, error );
}

SaveStatus EditorDocument::Save( ports::IFileStore &store, const std::string &path )
{
	const std::string text = formats::WriteKeyValues( m_content );
	const SaveStatus status = SaveDocument( store, path, text );
	if ( status == SaveStatus::kOk )
	{
		m_history.MarkSaved();
	}
	return status;
}

bool EditorDocument::SetFirstBlockKey( const std::string &key, const std::string &value )
{
	if ( m_content.children.empty() )
	{
		return false; // nothing to edit -> no-op
	}

	formats::KeyValueNode candidate = m_content;
	formats::KeyValueNode &block = candidate.children[0];

	bool replaced = false;
	for ( formats::KeyValue &pair : block.pairs )
	{
		if ( pair.key == key )
		{
			pair.value = value;
			replaced = true;
			break;
		}
	}
	if ( !replaced )
	{
		block.pairs.push_back( { key, value } );
	}

	// A no-op edit (value unchanged) adds no history and does not mark modified.
	if ( formats::CompareKeyValues( m_content, candidate ).equal )
	{
		return false;
	}

	CommitSnapshot( std::move( candidate ) );
	return true;
}

const std::string &EditorDocument::EntityName( std::size_t index ) const
{
	static const std::string kEmpty;
	if ( index >= m_content.children.size() )
	{
		return kEmpty;
	}
	return m_content.children[index].name;
}

const std::string &EditorDocument::EntityClassName( std::size_t index ) const
{
	static const std::string kEmpty;
	if ( index >= m_content.children.size() )
	{
		return kEmpty;
	}
	const std::string *classname = m_content.children[index].Find( "classname" );
	return classname != nullptr ? *classname : kEmpty;
}

PropertyValue EditorDocument::AggregateProperty( const EntitySelection &selection,
    const std::string &key ) const
{
	PropertyValue aggregate = PropertyValue::Unset();
	for ( const std::size_t index : selection.Indices() )
	{
		if ( index >= m_content.children.size() )
		{
			continue; // stale/out-of-range index: not a contributor
		}
		const std::string *value = m_content.children[index].Find( key );
		if ( value == nullptr )
		{
			continue; // entity does not define the key: not a contributor
		}
		aggregate = aggregate.AddContributor( *value );
	}
	return aggregate;
}

bool EditorDocument::SetPropertyOnSelection( const EntitySelection &selection,
    const std::string &key, const std::string &value )
{
	if ( selection.Empty() )
	{
		return false; // nothing selected -> no-op
	}

	formats::KeyValueNode candidate = m_content;
	for ( const std::size_t index : selection.Indices() )
	{
		if ( index >= candidate.children.size() )
		{
			continue; // ignore stale/out-of-range indices
		}
		formats::KeyValueNode &block = candidate.children[index];

		bool replaced = false;
		for ( formats::KeyValue &pair : block.pairs )
		{
			if ( pair.key == key )
			{
				pair.value = value;
				replaced = true;
				break;
			}
		}
		if ( !replaced )
		{
			block.pairs.push_back( { key, value } );
		}
	}

	// One history unit for the whole multi-entity edit; a no-op records nothing.
	if ( formats::CompareKeyValues( m_content, candidate ).equal )
	{
		return false;
	}

	CommitSnapshot( std::move( candidate ) );
	return true;
}

bool EditorDocument::Undo()
{
	if ( !m_history.Undo() )
	{
		return false;
	}
	m_content = m_snapshots[m_history.Position()];
	return true;
}

bool EditorDocument::Redo()
{
	if ( !m_history.Redo() )
	{
		return false;
	}
	m_content = m_snapshots[m_history.Position()];
	return true;
}

} // namespace hammer::app
