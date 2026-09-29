//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/instance_preview.h.
//
//=============================================================================//

#include "hammer/app/instance_preview.h"

#include "hammer/app/fragment_io.h"
#include "hammer/app/ops/instance_ops.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace hammer::app
{

using scene::ObjectId;

namespace
{

std::string Lower( std::string text )
{
	for ( char &c : text )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return text;
}

std::string Slashes( std::string text )
{
	std::replace( text.begin(), text.end(), '\\', '/' );
	return text;
}

std::string Join( const std::string &directory, const std::string &file )
{
	if ( directory.empty() )
	{
		return file;
	}
	return directory.back() == '/' ? directory + file : directory + "/" + file;
}

bool IsInstance( const scene::Entity &entity )
{
	return Lower( entity.classname ) == "func_instance";
}

// The identity of one placement: the file that holds the instance and the
// entity's class and keys (its id, group and editor state do not change
// what it draws).
std::string PlacementKey( const scene::Entity &instance, const std::string &base )
{
	std::string key = base;
	key += '\n';
	key += instance.classname;
	for ( const kvtext::KeyValue &kv : instance.keys )
	{
		key += '\n';
		key += kv.key;
		key += '\x1f';
		key += kv.value;
	}
	return key;
}

// Appends 'objects' to 'out' with fresh ids from 'next', rewriting owner and
// group references that name objects of 'objects' (others become invalid).
void AppendRemapped(
    std::vector<scene::MapObject> &out, std::vector<scene::MapObject> objects, std::uint64_t &next )
{
	std::unordered_map<std::uint64_t, ObjectId> fresh;
	const auto idOf = []( const scene::MapObject &o )
	{
		return std::visit(
		    []( const auto &value )
		    {
			    return value.id;
		    },
		    o );
	};
	for ( const scene::MapObject &o : objects )
	{
		fresh[idOf( o ).value] = ObjectId{ next++ };
	}
	const auto map = [&]( ObjectId id )
	{
		auto it = fresh.find( id.value );
		return it == fresh.end() ? ObjectId{} : it->second;
	};
	for ( scene::MapObject &o : objects )
	{
		std::visit(
		    [&]( auto &value )
		    {
			    value.id = map( value.id );
			    value.group = map( value.group );
		    },
		    o );
		if ( scene::Solid *solid = std::get_if<scene::Solid>( &o ) )
		{
			solid->owner = map( solid->owner );
		}
		out.push_back( std::move( o ) );
	}
}

} // namespace

InstancePreview::InstancePreview( const ports::IMapCodec &codec, const ports::IFileStore &store,
    const ports::IEntityCatalog *catalog, int maxDepth )
    : m_codec( codec ), m_store( store ), m_catalog( catalog ), m_maxDepth( maxDepth )
{
}

void InstancePreview::SetDocumentPath( const std::string &path )
{
	const std::string normalized = Slashes( path );
	if ( normalized != m_documentPath )
	{
		m_documentPath = normalized;
		Forget();
	}
}

void InstancePreview::SetSearchRoots( std::vector<std::string> roots )
{
	for ( std::string &root : roots )
	{
		root = Slashes( std::move( root ) );
	}
	if ( roots != m_roots )
	{
		m_roots = std::move( roots );
		Forget();
	}
}

void InstancePreview::Forget()
{
	m_files.clear();
	m_lookups.clear();
	m_placed.clear();
	++m_revision;
}

std::string InstancePreview::Resolve( const std::string &base, const std::string &file )
{
	const std::string key = base + '\n' + file;
	if ( auto it = m_lookups.find( key ); it != m_lookups.end() )
	{
		return it->second.resolved;
	}
	std::vector<std::string> candidates;
	if ( !base.empty() )
	{
		const std::size_t slash = base.rfind( '/' );
		candidates.push_back(
		    Join( slash == std::string::npos ? std::string() : base.substr( 0, slash ), file ) );
		const std::string lowered = Lower( base );
		const std::size_t maps = lowered.rfind( "/maps/" );
		if ( maps != std::string::npos )
		{
			candidates.push_back( base.substr( 0, maps + 6 ) + file );
		}
	}
	for ( const std::string &root : m_roots )
	{
		candidates.push_back( Join( root, file ) );
	}
	std::string resolved;
	for ( const std::string &candidate : candidates )
	{
		if ( m_store.Exists( candidate ) )
		{
			resolved = candidate;
			break;
		}
	}
	m_lookups[key] = Lookup{ base, file, resolved };
	return resolved;
}

const InstancePreview::FileEntry &InstancePreview::File( const std::string &path )
{
	auto [it, inserted] = m_files.try_emplace( path );
	FileEntry &entry = it->second;
	if ( !inserted )
	{
		return entry;
	}
	entry.read = m_store.Read( path, entry.bytes );
	auto loaded = LoadFragment( m_codec, m_store, path );
	++m_filesDecoded;
	if ( loaded.HasValue() )
	{
		entry.fragment = std::move( loaded.Value() );
	}
	else
	{
		entry.error = loaded.Error().message;
	}
	return entry;
}

ports::InstanceContent InstancePreview::Expand(
    const scene::Entity &instance, const std::string &base, std::vector<std::string> &stack )
{
	ports::InstanceContent content;
	const std::string file = ops::InstanceFile( instance );
	if ( file.empty() )
	{
		content.status = ports::InstanceStatus::NoFile;
		return content;
	}
	const std::string resolved = Resolve( base, file );
	if ( resolved.empty() )
	{
		content.status = ports::InstanceStatus::NotFound;
		content.detail = file;
		return content;
	}
	content.file = resolved;
	if ( std::find( stack.begin(), stack.end(), resolved ) != stack.end() ||
	     static_cast<int>( stack.size() ) >= m_maxDepth )
	{
		content.status = ports::InstanceStatus::Cycle;
		content.detail = resolved + " includes itself";
		return content;
	}
	const FileEntry &entry = File( resolved );
	if ( !entry.fragment )
	{
		content.status = ports::InstanceStatus::DecodeFailed;
		content.detail = entry.error;
		return content;
	}
	auto placed =
	    ops::PlaceInstanceContent( instance, *entry.fragment, "InstanceAuto1", m_catalog );
	++m_placements;
	if ( !placed.HasValue() )
	{
		content.status = ports::InstanceStatus::Rejected;
		content.detail = placed.Error();
		return content;
	}

	// Quick-hidden objects are not drawn; a hidden brush entity hides its
	// solids.
	std::vector<ObjectId> hiddenOwners;
	for ( const scene::MapObject &o : placed.Value().objects )
	{
		if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ); e && e->hidden )
		{
			hiddenOwners.push_back( e->id );
		}
	}
	std::vector<scene::MapObject> own;
	std::vector<scene::Entity> nested;
	for ( scene::MapObject &o : placed.Value().objects )
	{
		if ( const scene::Solid *s = std::get_if<scene::Solid>( &o ) )
		{
			if ( s->hidden || std::find( hiddenOwners.begin(), hiddenOwners.end(), s->owner ) !=
			                      hiddenOwners.end() )
			{
				continue;
			}
		}
		else if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			if ( e->hidden )
			{
				continue;
			}
			if ( IsInstance( *e ) )
			{
				nested.push_back( *e );
				continue;
			}
		}
		own.push_back( std::move( o ) );
	}
	std::uint64_t next = 1;
	AppendRemapped( content.objects, std::move( own ), next );
	stack.push_back( resolved );
	for ( const scene::Entity &child : nested )
	{
		ports::InstanceContent inner = Expand( child, resolved, stack );
		if ( inner.status != ports::InstanceStatus::Placed )
		{
			++content.nestedFailures;
			continue;
		}
		content.nestedFailures += inner.nestedFailures;
		AppendRemapped( content.objects, std::move( inner.objects ), next );
	}
	stack.pop_back();
	content.status = ports::InstanceStatus::Placed;
	return content;
}

std::shared_ptr<const ports::InstanceContent> InstancePreview::Content(
    const scene::Entity &instance )
{
	if ( !IsInstance( instance ) )
	{
		return nullptr;
	}
	const std::string key = PlacementKey( instance, m_documentPath );
	if ( auto it = m_placed.find( key ); it != m_placed.end() )
	{
		return it->second;
	}
	std::vector<std::string> stack;
	auto content =
	    std::make_shared<const ports::InstanceContent>( Expand( instance, m_documentPath, stack ) );
	m_placed.emplace( key, content );
	return content;
}

bool InstancePreview::Refresh()
{
	bool changed = false;
	for ( auto &[path, entry] : m_files )
	{
		std::string bytes;
		const bool read = m_store.Read( path, bytes );
		if ( read != entry.read || ( read && bytes != entry.bytes ) )
		{
			changed = true;
			break;
		}
	}
	if ( !changed )
	{
		const std::map<std::string, Lookup> lookups = m_lookups;
		for ( const auto &[key, lookup] : lookups )
		{
			// Repeat the lookup without its cached answer.
			m_lookups.erase( key );
			if ( Resolve( lookup.base, lookup.file ) != lookup.resolved )
			{
				changed = true;
				break;
			}
		}
	}
	if ( changed )
	{
		Forget();
	}
	return changed;
}

} // namespace hammer::app
