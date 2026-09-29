//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Presentation policies picking and extraction share (RFC 0002,
//			hammer.viewport). See public/hammer/viewport/view_policy.h.
//
//=============================================================================//

#include "hammer/viewport/view_policy.h"

#include "hammer/scene/map_queries.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>

namespace hammer::viewport
{

using mapgeometry::Vec3d;

bool IsShown(
    const VisibilityPredicate &predicate, const scene::DocumentReader &doc, scene::ObjectId id )
{
	return predicate ? predicate( doc, id ) : scene::IsVisible( doc, id );
}

VisibilityPredicate ShowEverything()
{
	return []( const scene::DocumentReader &doc, scene::ObjectId id )
	{
		return doc.KindOf( id ).has_value();
	};
}

std::optional<scene::Box> EntityMarkerBox(
    const scene::Entity &entity, const ports::IEntityCatalog *catalog, double pointHalfSize )
{
	const std::optional<Vec3d> origin = entity.Origin();
	if ( !origin )
	{
		return std::nullopt;
	}
	if ( catalog )
	{
		const ports::EntityClassInfo *info = catalog->Find( entity.classname );
		if ( info && info->boxMins && info->boxMaxs )
		{
			scene::Box box{ *origin + *info->boxMins, *origin + *info->boxMaxs };
			// A schema with swapped corners still gives a proper box.
			scene::Box ordered = scene::PointBox( box.mins );
			ordered.Extend( box.maxs );
			return ordered;
		}
	}
	const double half = std::isfinite( pointHalfSize ) ? std::fabs( pointHalfSize ) : 0.0;
	const Vec3d extent( half, half, half );
	return scene::Box{ *origin - extent, *origin + extent };
}

bool IsStudioModelPath( std::string_view path )
{
	constexpr std::string_view kExtension = ".mdl";
	if ( path.size() <= kExtension.size() )
	{
		return false;
	}
	const std::string_view tail = path.substr( path.size() - kExtension.size() );
	return std::equal( tail.begin(), tail.end(), kExtension.begin(),
	    []( char a, char b )
	    {
		    return std::tolower( static_cast<unsigned char>( a ) ) == b;
	    } );
}

namespace
{

// The value of 'key' in any case (the engine matches keys without regard to
// case); nothing when absent.
const std::string *KeyNoCase( const scene::Entity &entity, std::string_view key )
{
	for ( const kvtext::KeyValue &kv : entity.keys )
	{
		if ( kv.key.size() == key.size() &&
		     std::equal( kv.key.begin(), kv.key.end(), key.begin(),
		         []( char a, char b )
		         {
			         return std::tolower( static_cast<unsigned char>( a ) ) ==
			                std::tolower( static_cast<unsigned char>( b ) );
		         } ) )
		{
			return &kv.value;
		}
	}
	return nullptr;
}

} // namespace

ModelKeys ReadModelKeys( const scene::Entity &entity )
{
	ModelKeys keys;
	const std::string *defaultAnim = KeyNoCase( entity, "DefaultAnim" );
	if ( defaultAnim && !defaultAnim->empty() )
	{
		keys.sequence = *defaultAnim;
	}
	if ( const std::string *sequence = KeyNoCase( entity, "sequence" );
	    sequence && !sequence->empty() )
	{
		int value = 0;
		const char *end = sequence->data() + sequence->size();
		if ( std::from_chars( sequence->data(), end, value ).ptr == end )
		{
			keys.sequenceIndex = value >= 0 ? value : 0;
		}
		else if ( keys.sequence.empty() )
		{
			keys.sequence = *sequence;
		}
	}
	if ( const std::string *skin = entity.Key( "skin" ) )
	{
		int value = 0;
		const char *end = skin->data() + skin->size();
		if ( std::from_chars( skin->data(), end, value ).ptr == end && value >= 0 )
		{
			keys.skin = value;
		}
	}
	if ( const std::string *scale = entity.Key( "modelscale" ) )
	{
		double value = 0.0;
		const char *end = scale->data() + scale->size();
		if ( std::from_chars( scale->data(), end, value ).ptr == end && std::isfinite( value ) &&
		     value > 0.0 )
		{
			keys.scale = value;
		}
	}
	if ( const std::string *color = entity.Key( "rendercolor" ) )
	{
		int channels[3] = {};
		const char *at = color->data();
		const char *end = color->data() + color->size();
		bool ok = true;
		for ( int &channel : channels )
		{
			while ( at < end && *at == ' ' )
			{
				++at;
			}
			const std::from_chars_result parsed = std::from_chars( at, end, channel );
			if ( parsed.ec != std::errc() || channel < 0 || channel > 255 )
			{
				ok = false;
				break;
			}
			at = parsed.ptr;
		}
		while ( at < end && *at == ' ' )
		{
			++at;
		}
		if ( ok && at == end )
		{
			keys.renderColor = scene::Rgb{ channels[0], channels[1], channels[2] };
		}
	}
	return keys;
}

std::optional<scene::Rgb> CatalogColor(
    const ports::IEntityCatalog *catalog, std::string_view classname )
{
	if ( !catalog )
	{
		return std::nullopt;
	}
	const ports::EntityClassInfo *info = catalog->Find( classname );
	if ( !info || !info->color )
	{
		return std::nullopt;
	}
	const auto channel = []( double value )
	{
		if ( !std::isfinite( value ) )
		{
			return 0;
		}
		return static_cast<int>( std::clamp( std::round( value ), 0.0, 255.0 ) );
	};
	return scene::Rgb{
	    channel( info->color->x ), channel( info->color->y ), channel( info->color->z ) };
}

scene::Rgb SolidColor( const scene::DocumentReader &doc, const scene::Solid &solid,
    const ports::IEntityCatalog *catalog )
{
	if ( solid.editor.color )
	{
		return *solid.editor.color;
	}
	if ( solid.owner.IsValid() )
	{
		if ( const scene::Entity *owner = doc.FindEntity( solid.owner ) )
		{
			return CatalogColor( catalog, owner->classname ).value_or( kDefaultEntityColor );
		}
	}
	return kDefaultWorldColor;
}

scene::Rgb EntityColor( const scene::Entity &entity, const ports::IEntityCatalog *catalog )
{
	if ( const std::optional<scene::Rgb> color = CatalogColor( catalog, entity.classname ) )
	{
		return *color;
	}
	return entity.editor.color.value_or( kDefaultEntityColor );
}

void AppendUniqueEdges( std::vector<WorldEdge> &edges, const std::vector<Vec3d> &polygon )
{
	const std::size_t count = polygon.size();
	if ( count < 2 )
	{
		return;
	}
	for ( std::size_t i = 0; i < count; ++i )
	{
		const Vec3d &a = polygon[i];
		const Vec3d &b = polygon[( i + 1 ) % count];
		if ( mapgeometry::NearlyEqual( a, b, kEdgeEpsilon ) )
		{
			continue;
		}
		const bool known = std::any_of( edges.begin(), edges.end(),
		    [&]( const WorldEdge &edge )
		    {
			    return ( mapgeometry::NearlyEqual( edge.a, a, kEdgeEpsilon ) &&
			               mapgeometry::NearlyEqual( edge.b, b, kEdgeEpsilon ) ) ||
			           ( mapgeometry::NearlyEqual( edge.a, b, kEdgeEpsilon ) &&
			               mapgeometry::NearlyEqual( edge.b, a, kEdgeEpsilon ) );
		    } );
		if ( !known )
		{
			edges.push_back( { a, b } );
		}
	}
}

std::vector<WorldEdge> UniqueEdges( const mapgeometry::BrushSolid &solid )
{
	std::vector<WorldEdge> edges;
	for ( const mapgeometry::BrushFace &face : solid.faces )
	{
		AppendUniqueEdges( edges, face.vertices );
	}
	return edges;
}

} // namespace hammer::viewport
