//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/map_check.h.
//
//=============================================================================//

#include "hammer/app/map_check.h"

#include "hammer/app/ops/visgroup_ops.h"
#include "hammer/scene/document_index.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <set>

namespace hammer::app
{

using scene::ObjectId;
using Code = MapProblem::Code;
using Severity = MapProblem::Severity;

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

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() && Lower( a ) == Lower( b );
}

std::string Join( const std::vector<std::string> &items )
{
	std::string out;
	for ( std::size_t i = 0; i < items.size(); ++i )
	{
		out += ( i ? ", " : "" ) + items[i];
	}
	return out;
}

bool IsSpecialTarget( std::string_view name )
{
	static const char *const kSpecial[] = {
	    "!self", "!activator", "!caller", "!player", "!pvsplayer", "!speechtarget", "!picker" };
	for ( const char *s : kSpecial )
	{
		if ( EqualsNoCase( name, s ) )
		{
			return true;
		}
	}
	return false;
}

const ports::EntityClassInfo *ClassOf(
    const ports::IEntityCatalog *catalog, const scene::Entity &e )
{
	return catalog && !e.classname.empty() ? catalog->Find( e.classname ) : nullptr;
}

// A reference that names no entity.
bool NamesNothing( const scene::DocumentIndex &index, const std::string &target )
{
	return !IsSpecialTarget( target ) && !index.AnyEntityNamed( target );
}

// --- Per-object detectors (shared by CheckMap and FixProblem) ----------------

std::vector<std::string> MissingTargetKeys(
    const scene::DocumentIndex &index, const scene::Entity &e, const ports::IEntityCatalog *catalog )
{
	const ports::EntityClassInfo *info = ClassOf( catalog, e );
	std::vector<std::string> out;
	for ( const kvtext::KeyValue &kv : e.keys )
	{
		bool namesEntities = false;
		if ( info )
		{
			const ports::KeyDefinition *def = info->FindKey( kv.key );
			namesEntities = def && def->type == ports::KeyType::TargetDestination;
		}
		else
		{
			namesEntities = EqualsNoCase( kv.key, "target" );
		}
		if ( namesEntities && !kv.value.empty() && NamesNothing( index, kv.value ) &&
		     std::find( out.begin(), out.end(), kv.key ) == out.end() )
		{
			out.push_back( kv.key );
		}
	}
	return out;
}

// Indices of the connections that fail 'code' (one of the connection codes).
std::vector<std::size_t> BadConnections( const scene::DocumentReader &doc,
    const scene::DocumentIndex &index, const scene::Entity &e, const ports::IEntityCatalog *catalog,
    Code code )
{
	std::vector<std::size_t> out;
	const ports::EntityClassInfo *info = ClassOf( catalog, e );
	for ( std::size_t i = 0; i < e.connections.size(); ++i )
	{
		const scene::Connection &c = e.connections[i];
		bool bad = false;
		if ( code == Code::ConnectionMissingTarget )
		{
			bad = c.target.empty() || NamesNothing( index, c.target );
		}
		else if ( code == Code::UnknownOutput )
		{
			bad = info && !info->HasOutput( c.output );
		}
		else if ( code == Code::UnknownInput && catalog && !IsSpecialTarget( c.target ) )
		{
			const std::vector<ObjectId> targets = index.EntitiesNamed( c.target );
			bool allKnown = !targets.empty();
			bool declared = false;
			for ( ObjectId t : targets )
			{
				const ports::EntityClassInfo *ti = ClassOf( catalog, *doc.FindEntity( t ) );
				allKnown = allKnown && ti;
				declared = declared || ( ti && ti->HasInput( c.input ) );
			}
			bad = allKnown && !declared;
		}
		if ( bad )
		{
			out.push_back( i );
		}
	}
	return out;
}

bool AllowedWithoutSchema( std::string_view key )
{
	static const char *const kAllow[] = { "origin", "angles", "angle", "targetname", "spawnflags" };
	for ( const char *k : kAllow )
	{
		if ( EqualsNoCase( key, k ) )
		{
			return true;
		}
	}
	return false;
}

std::vector<std::string> UnusedKeys( const scene::Entity &e, const ports::IEntityCatalog *catalog )
{
	std::vector<std::string> out;
	const ports::EntityClassInfo *info = ClassOf( catalog, e );
	if ( !info || EqualsNoCase( e.classname, "multi_manager" ) )
	{
		return out;
	}
	for ( const kvtext::KeyValue &kv : e.keys )
	{
		if ( !AllowedWithoutSchema( kv.key ) && !info->FindKey( kv.key ) &&
		     std::none_of( out.begin(), out.end(),
		         [&]( const std::string &k )
		         {
			         return EqualsNoCase( k, kv.key );
		         } ) )
		{
			out.push_back( kv.key );
		}
	}
	return out;
}

// Keys that appear more than once (case-insensitively), by first appearance.
std::vector<std::string> DuplicateKeys( const scene::Entity &e )
{
	std::vector<std::string> out;
	std::map<std::string, int> seen;
	for ( const kvtext::KeyValue &kv : e.keys )
	{
		if ( ++seen[Lower( kv.key )] == 2 )
		{
			out.push_back( kv.key );
		}
	}
	return out;
}

bool SolidIsInvalid( const scene::Solid &s )
{
	return !scene::NormalizeSides( s ).has_value();
}

mapgeometry::Vec3d UnitNormal( const scene::Side &side )
{
	return side.Plane().normal;
}

bool SameDirection( const mapgeometry::Vec3d &a, const mapgeometry::Vec3d &b )
{
	return std::fabs( a.x - b.x ) < 1.0e-6 && std::fabs( a.y - b.y ) < 1.0e-6 &&
	       std::fabs( a.z - b.z ) < 1.0e-6;
}

// Describes a valid solid's redundant sides; empty when there are none.
std::string RedundantSides( const scene::Solid &s )
{
	std::vector<std::string> notes;
	for ( std::size_t i = 0; i < s.sides.size(); ++i )
	{
		for ( std::size_t j = i + 1; j < s.sides.size(); ++j )
		{
			if ( SameDirection( UnitNormal( s.sides[i] ), UnitNormal( s.sides[j] ) ) )
			{
				notes.push_back( "sides " + std::to_string( s.sides[i].vmfId ) + " and " +
				                 std::to_string( s.sides[j].vmfId ) + " share a plane direction" );
			}
		}
	}
	const mapgeometry::BrushSolid geometry = scene::BuildGeometry( s );
	std::set<int> bounding;
	for ( const mapgeometry::BrushFace &face : geometry.faces )
	{
		bounding.insert( face.sourcePlane );
	}
	for ( std::size_t i = 0; i < s.sides.size(); ++i )
	{
		if ( !bounding.count( static_cast<int>( i ) ) )
		{
			notes.push_back( "side " + std::to_string( s.sides[i].vmfId ) + " bounds no face" );
		}
	}
	return Join( notes );
}

std::vector<std::string> MissingMaterials(
    const scene::Solid &s, const ports::IMaterialInfo &materials )
{
	std::vector<std::string> out;
	for ( const scene::Side &side : s.sides )
	{
		const std::string &m = side.texture.material;
		if ( !materials.Exists( m ) && std::find( out.begin(), out.end(), m ) == out.end() )
		{
			out.push_back( m );
		}
	}
	return out;
}

// Side ids of 'solid' already used by an earlier side (solids in id order).
std::vector<std::size_t> DuplicateSideIndices(
    const scene::DocumentReader &doc, const scene::DocumentIndex &index, ObjectId solid )
{
	const scene::Solid *s = doc.FindSolid( solid );
	std::vector<std::size_t> out;
	if ( !s )
	{
		return out;
	}
	std::set<std::uint32_t> own;
	for ( std::size_t i = 0; i < s->sides.size(); ++i )
	{
		const std::uint32_t side = s->sides[i].vmfId;
		const std::vector<ObjectId> &users = index.SolidsWithSide( side );
		const bool earlierSolid = !users.empty() && users.front() < solid;
		if ( !own.insert( side ).second || earlierSolid )
		{
			out.push_back( i );
		}
	}
	return out;
}

std::uint32_t VmfIdOf( const scene::DocumentReader &doc, ObjectId id )
{
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		return s->vmfId;
	}
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		return e->vmfId;
	}
	return doc.FindGroup( id ) ? doc.FindGroup( id )->vmfId : 0;
}

// True when an object of the same kind with a smaller id has the same VMF id.
bool HasEarlierVmfTwin( const scene::DocumentReader &doc, const scene::DocumentIndex &index, ObjectId id )
{
	const std::optional<scene::ObjectKind> kind = doc.KindOf( id );
	if ( !kind )
	{
		return false;
	}
	const std::vector<ObjectId> &twins = index.WithVmfId( *kind, VmfIdOf( doc, id ) );
	return !twins.empty() && twins.front() < id;
}

const scene::EditorInfo *EditorOf( const scene::DocumentReader &doc, ObjectId id )
{
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		return &s->editor;
	}
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		return &e->editor;
	}
	if ( const scene::Group *g = doc.FindGroup( id ) )
	{
		return &g->editor;
	}
	return nullptr;
}

scene::EditorInfo *MutableEditor( scene::DocumentEdit &edit, ObjectId id )
{
	if ( scene::Solid *s = edit.MutableSolid( id ) )
	{
		return &s->editor;
	}
	if ( scene::Entity *e = edit.MutableEntity( id ) )
	{
		return &e->editor;
	}
	if ( scene::Group *g = edit.MutableGroup( id ) )
	{
		return &g->editor;
	}
	return nullptr;
}

std::vector<int> UndefinedVisgroups( const scene::DocumentReader &doc, ObjectId id )
{
	std::vector<int> out;
	for ( int v : EditorOf( doc, id )->visgroupIds )
	{
		if ( !scene::FindVisgroup( doc.Settings().visgroups, v ).visgroup &&
		     std::find( out.begin(), out.end(), v ) == out.end() )
		{
			out.push_back( v );
		}
	}
	return out;
}

// Hidden by visgroups with no defined visgroup on it or its containers.
bool HiddenWithoutVisgroup( const scene::DocumentReader &doc, ObjectId id )
{
	if ( EditorOf( doc, id )->visgroupShown )
	{
		return false;
	}
	ObjectId at = id;
	for ( int guard = 0; guard < 4096 && at.IsValid(); ++guard )
	{
		for ( int v : EditorOf( doc, at )->visgroupIds )
		{
			if ( scene::FindVisgroup( doc.Settings().visgroups, v ).visgroup )
			{
				return false;
			}
		}
		at = scene::ContainerOf( doc, at );
	}
	return true;
}

bool Outside( const scene::Box &box )
{
	for ( int axis = 0; axis < 3; ++axis )
	{
		if ( std::fabs( mapgeometry::Component( box.mins, axis ) ) > kMapCoordinateLimit ||
		     std::fabs( mapgeometry::Component( box.maxs, axis ) ) > kMapCoordinateLimit )
		{
			return true;
		}
	}
	return false;
}

bool IsFixable( Code code )
{
	switch ( code )
	{
	case Code::EmptyBrushEntity:
	case Code::MissingTarget:
	case Code::ConnectionMissingTarget:
	case Code::UnknownOutput:
	case Code::UnknownInput:
	case Code::UnusedKeyvalues:
	case Code::DuplicateKeys:
	case Code::InvalidSolid:
	case Code::DuplicatePlanes:
	case Code::DuplicateSideId:
	case Code::DuplicateObjectId:
	case Code::UndefinedVisgroup:
	case Code::HiddenWithoutVisgroup:
	case Code::EmptyGroup:
		return true;
	default:
		return false;
	}
}

Severity SeverityOf( Code code )
{
	switch ( code )
	{
	case Code::UnknownClass:
	case Code::MissingTarget:
	case Code::ConnectionMissingTarget:
	case Code::UnknownOutput:
	case Code::UnknownInput:
	case Code::UnusedKeyvalues:
	case Code::DuplicateKeys:
	case Code::MissingMaterial:
	case Code::UndefinedVisgroup:
	case Code::HiddenWithoutVisgroup:
	case Code::EmptyGroup:
		return Severity::Warning;
	default:
		return Severity::Error;
	}
}

std::string Describe( const scene::DocumentReader &doc, ObjectId id )
{
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		return "solid " + std::to_string( s->vmfId );
	}
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		std::string out = ( e->classname.empty() ? std::string( "entity" ) : e->classname ) + " " +
		                  std::to_string( e->vmfId );
		if ( !e->Name().empty() )
		{
			out += " '" + std::string( e->Name() ) + "'";
		}
		return out;
	}
	if ( const scene::Group *g = doc.FindGroup( id ) )
	{
		return "group " + std::to_string( g->vmfId );
	}
	return "object";
}

std::vector<std::string> ConnectionTexts(
    const scene::Entity &e, const std::vector<std::size_t> &indices )
{
	std::vector<std::string> out;
	for ( std::size_t i : indices )
	{
		const scene::Connection &c = e.connections[i];
		out.push_back( c.output + " -> " + c.target + "." + c.input );
	}
	return out;
}

// Whether 'code' still applies to 'id' (the fix precondition).
bool Applies(
    const scene::DocumentReader &doc, Code code, ObjectId id, const ports::IEntityCatalog *catalog )
{
	const scene::DocumentIndex index( doc );
	const scene::Entity *e = doc.FindEntity( id );
	const scene::Solid *s = doc.FindSolid( id );
	switch ( code )
	{
	case Code::EmptyBrushEntity:
	{
		const ports::EntityClassInfo *info = e ? ClassOf( catalog, *e ) : nullptr;
		return info && info->kind == ports::EntityClassKind::Solid &&
		       index.EntitySolids( id ).empty();
	}
	case Code::MissingTarget:
		return e && !MissingTargetKeys( index, *e, catalog ).empty();
	case Code::ConnectionMissingTarget:
	case Code::UnknownOutput:
	case Code::UnknownInput:
		return e && !BadConnections( doc, index, *e, catalog, code ).empty();
	case Code::UnusedKeyvalues:
		return e && !UnusedKeys( *e, catalog ).empty();
	case Code::DuplicateKeys:
		return e && !DuplicateKeys( *e ).empty();
	case Code::InvalidSolid:
		return s && SolidIsInvalid( *s );
	case Code::DuplicatePlanes:
		return s && !SolidIsInvalid( *s ) && !RedundantSides( *s ).empty();
	case Code::DuplicateSideId:
		return s && !DuplicateSideIndices( doc, index, id ).empty();
	case Code::DuplicateObjectId:
		return HasEarlierVmfTwin( doc, index, id );
	case Code::UndefinedVisgroup:
		return EditorOf( doc, id ) && !UndefinedVisgroups( doc, id ).empty();
	case Code::HiddenWithoutVisgroup:
		return EditorOf( doc, id ) && HiddenWithoutVisgroup( doc, id );
	case Code::EmptyGroup:
		return doc.FindGroup( id ) && index.GroupMembers( id ).empty();
	default:
		return false;
	}
}

} // namespace

const char *MapProblemCodeName( Code code )
{
	switch ( code )
	{
	case Code::NoPlayerStart:
		return "no-player-start";
	case Code::EmptyClassname:
		return "empty-classname";
	case Code::UnknownClass:
		return "unknown-class";
	case Code::EmptyBrushEntity:
		return "empty-brush-entity";
	case Code::PointEntityWithSolids:
		return "point-entity-with-solids";
	case Code::MissingTarget:
		return "missing-target";
	case Code::ConnectionMissingTarget:
		return "connection-missing-target";
	case Code::UnknownOutput:
		return "unknown-output";
	case Code::UnknownInput:
		return "unknown-input";
	case Code::UnusedKeyvalues:
		return "unused-keyvalues";
	case Code::DuplicateKeys:
		return "duplicate-keys";
	case Code::InvalidSolid:
		return "invalid-solid";
	case Code::DuplicatePlanes:
		return "duplicate-planes";
	case Code::MissingMaterial:
		return "missing-material";
	case Code::DuplicateSideId:
		return "duplicate-side-id";
	case Code::DuplicateObjectId:
		return "duplicate-object-id";
	case Code::UndefinedVisgroup:
		return "undefined-visgroup";
	case Code::HiddenWithoutVisgroup:
		return "hidden-without-visgroup";
	case Code::EmptyGroup:
		return "empty-group";
	case Code::OutsideMapBounds:
		return "outside-map-bounds";
	}
	return "unknown";
}

std::vector<MapProblem> CheckMap( const scene::DocumentReader &doc,
    const ports::IEntityCatalog *catalog, const ports::IMaterialInfo *materials )
{
	std::vector<MapProblem> out;
	auto add = [&]( Code code, std::string message, std::vector<ObjectId> objects )
	{
		MapProblem p;
		p.code = code;
		p.severity = SeverityOf( code );
		p.message = std::move( message );
		p.objects = std::move( objects );
		p.fixable = IsFixable( code );
		out.push_back( std::move( p ) );
	};

	const scene::DocumentIndex index( doc );

	// Entities.
	bool playerStart = false;
	for ( ObjectId id : doc.EntityIds() )
	{
		const scene::Entity &e = *doc.FindEntity( id );
		const std::string what = Describe( doc, id );
		if ( Lower( e.classname ).rfind( "info_player_", 0 ) == 0 )
		{
			playerStart = true;
		}
		if ( e.classname.empty() )
		{
			add( Code::EmptyClassname, "entity " + std::to_string( e.vmfId ) + " has no class",
			    { id } );
		}
		const ports::EntityClassInfo *info = ClassOf( catalog, e );
		if ( catalog && !e.classname.empty() && !info )
		{
			add( Code::UnknownClass,
			    what + ": the class '" + e.classname + "' is not in the game data", { id } );
		}
		if ( info )
		{
			const bool brush = info->kind == ports::EntityClassKind::Solid;
			const bool hasSolids = !index.EntitySolids( id ).empty();
			if ( brush && !hasSolids )
			{
				add( Code::EmptyBrushEntity, what + " is a brush entity with no solids", { id } );
			}
			if ( !brush && hasSolids )
			{
				add( Code::PointEntityWithSolids, what + " is a point entity class but owns solids",
				    { id } );
			}
		}
		if ( const std::vector<std::string> keys = MissingTargetKeys( index, e, catalog );
		    !keys.empty() )
		{
			add( Code::MissingTarget, what + ": " + Join( keys ) + " names no entity", { id } );
		}
		for ( Code code :
		    { Code::ConnectionMissingTarget, Code::UnknownOutput, Code::UnknownInput } )
		{
			const std::vector<std::size_t> bad = BadConnections( doc, index, e, catalog, code );
			if ( bad.empty() )
			{
				continue;
			}
			const char *why = code == Code::ConnectionMissingTarget ? "target no entity"
			                  : code == Code::UnknownOutput ? "use outputs the class does not have"
			                                                : "use inputs no target has";
			add(
			    code, what + ": outputs " + Join( ConnectionTexts( e, bad ) ) + " " + why, { id } );
		}
		if ( const std::vector<std::string> keys = UnusedKeys( e, catalog ); !keys.empty() )
		{
			add( Code::UnusedKeyvalues, what + ": keys not in the class: " + Join( keys ), { id } );
		}
		if ( const std::vector<std::string> keys = DuplicateKeys( e ); !keys.empty() )
		{
			add( Code::DuplicateKeys, what + ": keys set more than once: " + Join( keys ), { id } );
		}
		if ( index.EntitySolids( id ).empty() )
		{
			if ( const std::optional<mapgeometry::Vec3d> origin = e.Origin();
			    origin && Outside( scene::PointBox( *origin ) ) )
			{
				add( Code::OutsideMapBounds, what + " is outside the map bounds", { id } );
			}
		}
	}
	if ( !playerStart )
	{
		add( Code::NoPlayerStart, "the map has no player start (info_player_*)", {} );
	}

	// Solids.
	for ( ObjectId id : doc.SolidIds() )
	{
		const scene::Solid &s = *doc.FindSolid( id );
		const std::string what = Describe( doc, id );
		const bool invalid = SolidIsInvalid( s );
		if ( invalid )
		{
			add( Code::InvalidSolid, what + " does not form a closed convex solid", { id } );
		}
		else
		{
			if ( const std::string redundant = RedundantSides( s ); !redundant.empty() )
			{
				add( Code::DuplicatePlanes, what + ": " + redundant, { id } );
			}
			if ( const std::optional<scene::Box> bounds = scene::SolidBounds( s );
			    bounds && Outside( *bounds ) )
			{
				add( Code::OutsideMapBounds, what + " is outside the map bounds", { id } );
			}
		}
		if ( materials )
		{
			if ( const std::vector<std::string> missing = MissingMaterials( s, *materials );
			    !missing.empty() )
			{
				add( Code::MissingMaterial, what + ": missing materials " + Join( missing ),
				    { id } );
			}
		}
		if ( const std::vector<std::size_t> dup = DuplicateSideIndices( doc, index, id ); !dup.empty() )
		{
			std::vector<std::string> ids;
			for ( std::size_t i : dup )
			{
				ids.push_back( std::to_string( s.sides[i].vmfId ) );
			}
			add( Code::DuplicateSideId, what + ": side ids already used: " + Join( ids ), { id } );
		}
	}

	// Every object.
	std::vector<ObjectId> all = doc.SolidIds();
	for ( const std::vector<ObjectId> &more : { doc.EntityIds(), doc.GroupIds() } )
	{
		all.insert( all.end(), more.begin(), more.end() );
	}
	std::sort( all.begin(), all.end() );
	for ( ObjectId id : all )
	{
		const std::string what = Describe( doc, id );
		if ( HasEarlierVmfTwin( doc, index, id ) )
		{
			add( Code::DuplicateObjectId, what + ": the id is used by another object of its kind",
			    { id } );
		}
		if ( const std::vector<int> undefined = UndefinedVisgroups( doc, id ); !undefined.empty() )
		{
			std::vector<std::string> ids;
			for ( int v : undefined )
			{
				ids.push_back( std::to_string( v ) );
			}
			add( Code::UndefinedVisgroup, what + ": visgroups " + Join( ids ) + " are not defined",
			    { id } );
		}
		if ( HiddenWithoutVisgroup( doc, id ) )
		{
			add( Code::HiddenWithoutVisgroup, what + " is hidden by visgroups but belongs to none",
			    { id } );
		}
		if ( doc.FindGroup( id ) && index.GroupMembers( id ).empty() )
		{
			add( Code::EmptyGroup, what + " is empty", { id } );
		}
	}

	std::stable_sort( out.begin(), out.end(),
	    []( const MapProblem &a, const MapProblem &b )
	    {
		    if ( a.code != b.code )
		    {
			    return a.code < b.code;
		    }
		    const ObjectId ia = a.objects.empty() ? ObjectId() : a.objects.front();
		    const ObjectId ib = b.objects.empty() ? ObjectId() : b.objects.front();
		    if ( a.objects.empty() != b.objects.empty() )
		    {
			    return a.objects.empty();
		    }
		    if ( ia != ib )
		    {
			    return ia < ib;
		    }
		    return a.message < b.message;
	    } );
	return out;
}

EditResult FixProblem(
    scene::DocumentEdit &edit, const MapProblem &problem, const ports::IEntityCatalog *catalog )
{
	if ( !IsFixable( problem.code ) )
	{
		return Reject(
		    std::string( "'" ) + MapProblemCodeName( problem.code ) + "' has no automatic fix" );
	}
	if ( problem.objects.empty() ||
	     !Applies( edit, problem.code, problem.objects.front(), catalog ) )
	{
		return NothingToDo( "the problem no longer applies" );
	}
	const ObjectId id = problem.objects.front();
	switch ( problem.code )
	{
	case Code::EmptyBrushEntity:
		edit.Remove( id );
		break;
	case Code::MissingTarget:
		for ( const std::string &key : MissingTargetKeys( scene::DocumentIndex( edit ), *edit.FindEntity( id ), catalog ) )
		{
			edit.MutableEntity( id )->RemoveKey( key );
		}
		break;
	case Code::ConnectionMissingTarget:
	case Code::UnknownOutput:
	case Code::UnknownInput:
	{
		const std::vector<std::size_t> bad =
		    BadConnections( edit, scene::DocumentIndex( edit ), *edit.FindEntity( id ), catalog, problem.code );
		std::vector<scene::Connection> &connections = edit.MutableEntity( id )->connections;
		for ( auto it = bad.rbegin(); it != bad.rend(); ++it )
		{
			connections.erase( connections.begin() + static_cast<std::ptrdiff_t>( *it ) );
		}
		break;
	}
	case Code::UnusedKeyvalues:
	{
		const std::vector<std::string> keys = UnusedKeys( *edit.FindEntity( id ), catalog );
		std::erase_if( edit.MutableEntity( id )->keys,
		    [&]( const kvtext::KeyValue &kv )
		    {
			    return std::any_of( keys.begin(), keys.end(),
			        [&]( const std::string &k )
			        {
				        return EqualsNoCase( k, kv.key );
			        } );
		    } );
		break;
	}
	case Code::DuplicateKeys:
	{
		std::set<std::string> seen;
		std::erase_if( edit.MutableEntity( id )->keys,
		    [&]( const kvtext::KeyValue &kv )
		    {
			    return !seen.insert( Lower( kv.key ) ).second;
		    } );
		break;
	}
	case Code::InvalidSolid:
	{
		const ObjectId owner = edit.FindSolid( id )->owner;
		edit.Remove( id );
		if ( owner.IsValid() && edit.FindEntity( owner ) &&
		     scene::EntitySolids( edit, owner ).empty() )
		{
			edit.Remove( owner );
		}
		break;
	}
	case Code::DuplicatePlanes:
	{
		std::optional<scene::Solid> normalized = scene::NormalizeSides( *edit.FindSolid( id ) );
		std::vector<scene::Side> kept;
		for ( scene::Side &side : normalized->sides )
		{
			const bool twin = std::any_of( kept.begin(), kept.end(),
			    [&]( const scene::Side &k )
			    {
				    return SameDirection( UnitNormal( k ), UnitNormal( side ) );
			    } );
			if ( !twin )
			{
				kept.push_back( std::move( side ) );
			}
		}
		normalized->sides = std::move( kept );
		if ( SolidIsInvalid( *normalized ) )
		{
			return Reject( "the solid's sides cannot be normalized" );
		}
		*edit.MutableSolid( id ) = std::move( *normalized );
		break;
	}
	case Code::DuplicateSideId:
	{
		const std::vector<std::size_t> dup = DuplicateSideIndices( edit, scene::DocumentIndex( edit ), id );
		scene::Solid *s = edit.MutableSolid( id );
		for ( std::size_t i : dup )
		{
			s->sides[i].vmfId = edit.AllocateVmfId();
		}
		break;
	}
	case Code::DuplicateObjectId:
	{
		const std::uint32_t fresh = edit.AllocateVmfId();
		if ( scene::Solid *s = edit.MutableSolid( id ) )
		{
			s->vmfId = fresh;
		}
		else if ( scene::Entity *e = edit.MutableEntity( id ) )
		{
			e->vmfId = fresh;
		}
		else if ( scene::Group *g = edit.MutableGroup( id ) )
		{
			g->vmfId = fresh;
		}
		break;
	}
	case Code::UndefinedVisgroup:
	{
		const std::vector<int> undefined = UndefinedVisgroups( edit, id );
		std::erase_if( MutableEditor( edit, id )->visgroupIds,
		    [&]( int v )
		    {
			    return std::find( undefined.begin(), undefined.end(), v ) != undefined.end();
		    } );
		break;
	}
	case Code::HiddenWithoutVisgroup:
	{
		int visgroup = 0;
		for ( const scene::Visgroup &v : edit.Settings().visgroups )
		{
			if ( v.name == kCheckHiddenVisgroupName )
			{
				visgroup = v.id;
				break;
			}
		}
		if ( visgroup == 0 )
		{
			if ( EditResult made =
			         ops::CreateVisgroup( edit, kCheckHiddenVisgroupName, 0, visgroup );
			    !made )
			{
				return made;
			}
		}
		MutableEditor( edit, id )->visgroupIds.push_back( visgroup );
		break;
	}
	case Code::EmptyGroup:
	{
		ObjectId at = id;
		while ( at.IsValid() && edit.FindGroup( at ) && scene::GroupMembers( edit, at ).empty() )
		{
			const ObjectId parent = edit.FindGroup( at )->group;
			edit.Remove( at );
			at = parent;
		}
		break;
	}
	default:
		return Reject( "no fix" );
	}
	return {};
}

EditResult FixAll( scene::DocumentEdit &edit, const ports::IEntityCatalog *catalog,
    const ports::IMaterialInfo *materials )
{
	bool fixedAny = false;
	for ( int pass = 0; pass < 32; ++pass )
	{
		bool fixed = false;
		for ( const MapProblem &problem : CheckMap( edit, catalog, materials ) )
		{
			if ( !problem.fixable )
			{
				continue;
			}
			const EditResult result = FixProblem( edit, problem, catalog );
			if ( result )
			{
				fixed = true;
			}
			else if ( result.Error().code != EditErrorCode::Nothing )
			{
				return result;
			}
		}
		if ( !fixed )
		{
			break;
		}
		fixedAny = true;
	}
	if ( !fixedAny )
	{
		return NothingToDo( "no fixable problems" );
	}
	return {};
}

} // namespace hammer::app
