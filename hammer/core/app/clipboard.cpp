//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/clipboard.h.
//
//=============================================================================//

#include "hammer/app/clipboard.h"

#include "hammer/app/ops/transform_ops.h"
#include "hammer/app/ops/visgroup_ops.h"
#include "hammer/scene/map_queries.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <set>
#include <sstream>

namespace hammer::app
{

using scene::ObjectId;

namespace
{

std::string Lower( std::string_view text )
{
	std::string out( text );
	for ( char &c : out )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return out;
}

scene::EditorInfo &EditorOf( scene::MapObject &object )
{
	return std::visit(
	    []( auto &o ) -> scene::EditorInfo &
	    {
		    return o.editor;
	    },
	    object );
}

ObjectId &GroupOf( scene::MapObject &object )
{
	return std::visit(
	    []( auto &o ) -> ObjectId &
	    {
		    return o.group;
	    },
	    object );
}

bool Finite( const mapgeometry::Vec3d &v )
{
	return std::isfinite( v.x ) && std::isfinite( v.y ) && std::isfinite( v.z );
}

// Legacy IncrementStringName: bump the trailing number, or append "1".
std::string Increment( const std::string &name )
{
	std::size_t pos = name.size();
	while ( pos > 0 && std::isdigit( static_cast<unsigned char>( name[pos - 1] ) ) )
	{
		--pos;
	}
	if ( pos == name.size() )
	{
		return name + "1";
	}
	// Digits beyond what fits are treated as a fresh counter (legacy atoi overflow).
	const std::string digits = name.substr( pos );
	long long value = 0;
	for ( char c : digits )
	{
		value = value < 100000000000000LL ? value * 10 + ( c - '0' ) : value;
	}
	return name.substr( 0, pos ) + std::to_string( value + 1 );
}

std::string FixName(
    const std::string &name, const PasteOptions &options, const std::set<std::string> &taken )
{
	using NameFix = PasteOptions::NameFix;
	if ( options.nameFix == NameFix::Keep )
	{
		return name;
	}
	std::string out =
	    options.nameFix == NameFix::Prefix ? options.nameText + name : name + options.nameText;
	if ( out.empty() )
	{
		out = "entity";
	}
	while ( taken.count( Lower( out ) ) )
	{
		out = Increment( out );
	}
	return out;
}

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() && Lower( a ) == Lower( b );
}

// Rewrites references to renamed names inside one pasted entity.
void RenameReferences(
    scene::Entity &e, const std::vector<std::pair<std::string, std::string>> &renames )
{
	auto mapped = [&]( const std::string &value ) -> const std::string *
	{
		for ( const auto &[from, to] : renames )
		{
			if ( EqualsNoCase( value, from ) )
			{
				return &to;
			}
		}
		return nullptr;
	};
	for ( kvtext::KeyValue &kv : e.keys )
	{
		if ( const std::string *to = mapped( kv.value ) )
		{
			kv.value = *to;
		}
	}
	for ( scene::Connection &c : e.connections )
	{
		if ( const std::string *to = mapped( c.target ) )
		{
			c.target = *to;
		}
		if ( const std::string *to = mapped( c.parameter ) )
		{
			c.parameter = *to;
		}
	}
}

// Remaps the side ids in an overlay "sides" value; nothing when the value is
// not a list of ids or nothing changes.
std::optional<std::string> RemapSides(
    const std::string &value, const std::map<std::uint32_t, std::uint32_t> &sides )
{
	std::istringstream in( value );
	std::string token;
	std::vector<std::string> out;
	bool changed = false;
	while ( in >> token )
	{
		if ( token.empty() ||
		     !std::all_of( token.begin(), token.end(),
		         []( char c )
		         {
			         return std::isdigit( static_cast<unsigned char>( c ) );
		         } ) ||
		     token.size() > 9 )
		{
			return std::nullopt;
		}
		const auto it = sides.find( static_cast<std::uint32_t>( std::stoul( token ) ) );
		if ( it != sides.end() )
		{
			token = std::to_string( it->second );
			changed = true;
		}
		out.push_back( token );
	}
	if ( !changed )
	{
		return std::nullopt;
	}
	std::string joined;
	for ( std::size_t i = 0; i < out.size(); ++i )
	{
		joined += ( i ? " " : "" ) + out[i];
	}
	return joined;
}

void CollectVisgroups(
    const std::vector<scene::Visgroup> &tree, std::vector<const scene::Visgroup *> &out )
{
	for ( const scene::Visgroup &v : tree )
	{
		out.push_back( &v );
		CollectVisgroups( v.children, out );
	}
}

} // namespace

std::string FixedPasteName(
    const std::string &name, const PasteOptions &options, const std::vector<std::string> &taken )
{
	return FixName( name, options, std::set<std::string>( taken.begin(), taken.end() ) );
}

MapFragment Copy( const scene::DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	MapFragment fragment;
	const std::vector<ObjectId> all = scene::ExpandObjects( doc, ids );
	const std::set<ObjectId> inSet( all.begin(), all.end() );
	std::set<int> visgroups;
	auto keep = [&]( ObjectId ref )
	{
		return inSet.count( ref ) ? ref : ObjectId();
	};
	auto noteVisgroups = [&]( scene::EditorInfo &info )
	{
		std::erase_if( info.visgroupIds,
		    [&]( int v )
		    {
			    return !scene::FindVisgroup( doc.Settings().visgroups, v ).visgroup;
		    } );
		visgroups.insert( info.visgroupIds.begin(), info.visgroupIds.end() );
	};
	for ( ObjectId id : all )
	{
		if ( const scene::Solid *s = doc.FindSolid( id ) )
		{
			scene::Solid copy = *s;
			copy.owner = keep( copy.owner );
			copy.group = keep( copy.group );
			noteVisgroups( copy.editor );
			fragment.objects.emplace_back( std::move( copy ) );
		}
		else if ( const scene::Entity *e = doc.FindEntity( id ) )
		{
			scene::Entity copy = *e;
			copy.group = keep( copy.group );
			noteVisgroups( copy.editor );
			fragment.objects.emplace_back( std::move( copy ) );
		}
		else if ( const scene::Group *g = doc.FindGroup( id ) )
		{
			scene::Group copy = *g;
			copy.group = keep( copy.group );
			noteVisgroups( copy.editor );
			fragment.objects.emplace_back( std::move( copy ) );
		}
	}
	for ( int v : visgroups )
	{
		scene::Visgroup def = *scene::FindVisgroup( doc.Settings().visgroups, v ).visgroup;
		def.children.clear();
		fragment.visgroups.push_back( std::move( def ) );
	}
	fragment.bounds = scene::ObjectsBounds( doc, all );
	return fragment;
}

EditResult Paste( scene::DocumentEdit &edit, const MapFragment &fragment,
    const PasteOptions &options, std::vector<ObjectId> *created )
{
	if ( fragment.Empty() )
	{
		return NothingToDo( "the clipboard is empty" );
	}
	if ( options.copies < 1 || options.copies > 1024 )
	{
		return Reject( "the number of copies must be between 1 and 1024" );
	}
	if ( !Finite( options.offset ) || ( options.rotation && !Finite( *options.rotation ) ) )
	{
		return Reject( "the paste offset and angles must be finite" );
	}

	// Fragment-local structure.
	std::set<ObjectId> local;
	for ( const scene::MapObject &o : fragment.objects )
	{
		local.insert( scene::IdOf( o ) );
	}

	// Transform every copy first, so a degenerate one refuses before staging.
	const mapgeometry::Vec3d center =
	    fragment.bounds ? fragment.bounds->Center() : mapgeometry::Vec3d();
	const bool rotates = options.rotation && *options.rotation != mapgeometry::Vec3d();
	std::vector<std::vector<scene::MapObject>> copies;
	for ( int k = 1; k <= options.copies; ++k )
	{
		const double n = static_cast<double>( k );
		mapgeometry::Affine xf = mapgeometry::Affine::Translation( options.offset * n );
		if ( rotates )
		{
			const mapgeometry::Vec3d a = *options.rotation * n;
			xf = mapgeometry::Compose( xf,
			    mapgeometry::Affine::About( mapgeometry::AngleMatrix( a.x, a.y, a.z ), center ) );
		}
		std::vector<scene::MapObject> objects;
		for ( const scene::MapObject &o : fragment.objects )
		{
			if ( const scene::Solid *s = std::get_if<scene::Solid>( &o ) )
			{
				std::optional<scene::Solid> moved =
				    ops::TransformedSolid( *s, xf, ops::TransformOptions{ true } );
				if ( !moved )
				{
					return Reject( "the paste transform would make a solid degenerate" );
				}
				objects.emplace_back( std::move( *moved ) );
			}
			else if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
			{
				objects.emplace_back( ops::TransformedEntity( *e, xf ) );
			}
			else
			{
				objects.push_back( o );
			}
		}
		copies.push_back( std::move( objects ) );
	}

	// Visgroups: match by id and name, then by name, else create.
	std::map<int, int> visgroupMap;
	for ( const scene::Visgroup &def : fragment.visgroups )
	{
		const scene::Visgroup *same =
		    scene::FindVisgroup( edit.Settings().visgroups, def.id ).visgroup;
		if ( same && same->name == def.name )
		{
			visgroupMap[def.id] = def.id;
			continue;
		}
		std::vector<const scene::Visgroup *> all;
		CollectVisgroups( edit.Settings().visgroups, all );
		const auto byName = std::find_if( all.begin(), all.end(),
		    [&]( const scene::Visgroup *v )
		    {
			    return v->name == def.name;
		    } );
		if ( byName != all.end() )
		{
			visgroupMap[def.id] = ( *byName )->id;
			continue;
		}
		scene::Visgroup made;
		made.id = ops::NextVisgroupId( edit );
		made.name = def.name.empty() ? std::string( "visgroup" ) : def.name;
		made.color = def.color;
		visgroupMap[def.id] = made.id;
		edit.MutableSettings().visgroups.push_back( std::move( made ) );
	}

	// Names in use in the target (lower case), for the name fix.
	std::set<std::string> taken;
	for ( ObjectId id : edit.EntityIds() )
	{
		const std::string_view name = edit.FindEntity( id )->Name();
		if ( !name.empty() )
		{
			taken.insert( Lower( name ) );
		}
	}
	std::vector<std::string> definedNames;
	for ( const scene::MapObject &o : fragment.objects )
	{
		if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			const std::string name( e->Name() );
			if ( !name.empty() && std::none_of( definedNames.begin(), definedNames.end(),
			                          [&]( const std::string &n )
			                          {
				                          return EqualsNoCase( n, name );
			                          } ) )
			{
				definedNames.push_back( name );
			}
		}
	}

	for ( std::vector<scene::MapObject> &objects : copies )
	{
		// The name fix for this copy.
		std::vector<std::pair<std::string, std::string>> renames;
		if ( options.nameFix != PasteOptions::NameFix::Keep )
		{
			for ( const std::string &name : definedNames )
			{
				const std::string fixed = FixName( name, options, taken );
				taken.insert( Lower( fixed ) );
				renames.emplace_back( name, fixed );
			}
		}

		ObjectId wrapper;
		if ( options.group )
		{
			wrapper = edit.Add( scene::Group{} );
			if ( created )
			{
				created->push_back( wrapper );
			}
		}

		// New identities.
		std::map<ObjectId, ObjectId> ids;
		std::map<std::uint32_t, std::uint32_t> sides;
		for ( scene::MapObject &o : objects )
		{
			ids[scene::IdOf( o )] = edit.AllocateId();
		}
		for ( scene::MapObject &o : objects )
		{
			const ObjectId fragmentId = scene::IdOf( o );
			std::visit(
			    [&]( auto &value )
			    {
				    value.id = ids.at( fragmentId );
			    },
			    o );
			if ( scene::Solid *s = std::get_if<scene::Solid>( &o ) )
			{
				s->vmfId = edit.AllocateVmfId();
				for ( scene::Side &side : s->sides )
				{
					const std::uint32_t fresh = edit.AllocateVmfId();
					sides.emplace( side.vmfId, fresh );
					side.vmfId = fresh;
				}
				s->owner = s->owner.IsValid() ? ids.at( s->owner ) : ObjectId();
			}
			else if ( scene::Entity *e = std::get_if<scene::Entity>( &o ) )
			{
				e->vmfId = edit.AllocateVmfId();
			}
			else
			{
				std::get<scene::Group>( o ).vmfId = edit.AllocateVmfId();
			}
			// A brush entity's solids belong to the entity, not to a group.
			ObjectId &group = GroupOf( o );
			const scene::Solid *asSolid = std::get_if<scene::Solid>( &o );
			if ( group.IsValid() && local.count( group ) )
			{
				group = ids.at( group );
			}
			else if ( asSolid && asSolid->owner.IsValid() )
			{
				group = ObjectId();
			}
			else
			{
				group = wrapper;
				if ( created && !wrapper.IsValid() )
				{
					created->push_back( ids.at( fragmentId ) );
				}
			}
			scene::EditorInfo &info = EditorOf( o );
			std::vector<int> mapped;
			for ( int v : info.visgroupIds )
			{
				const auto it = visgroupMap.find( v );
				if ( it != visgroupMap.end() &&
				     std::find( mapped.begin(), mapped.end(), it->second ) == mapped.end() )
				{
					mapped.push_back( it->second );
				}
			}
			info.visgroupIds = std::move( mapped );
		}
		for ( scene::MapObject &o : objects )
		{
			if ( scene::Entity *e = std::get_if<scene::Entity>( &o ) )
			{
				RenameReferences( *e, renames );
				for ( kvtext::KeyValue &kv : e->keys )
				{
					if ( EqualsNoCase( kv.key, "sides" ) || EqualsNoCase( kv.key, "sides2" ) )
					{
						if ( std::optional<std::string> remapped = RemapSides( kv.value, sides ) )
						{
							kv.value = *remapped;
						}
					}
				}
			}
			edit.Put( std::move( o ) );
		}
	}
	return {};
}

EditResult Duplicate( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids,
    const mapgeometry::Vec3d &offset, std::vector<ObjectId> *created )
{
	const MapFragment fragment = Copy( edit, ids );
	if ( fragment.Empty() )
	{
		return NothingToDo( "nothing selected to duplicate" );
	}
	PasteOptions options;
	options.offset = offset;
	return Paste( edit, fragment, options, created );
}

} // namespace hammer::app
