//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/object_label.h.
//
//=============================================================================//

#include "hammer/presenters/object_label.h"

#include "hammer/scene/map_queries.h"
#include "presenter_text.h"

namespace hammer::presenters
{

std::string ObjectLabel( const scene::DocumentReader &doc, scene::ObjectId id )
{
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		const std::string_view name = e->Name();
		return name.empty() ? e->classname : std::string( name );
	}
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		return "solid " + std::to_string( s->vmfId );
	}
	if ( const scene::Group *g = doc.FindGroup( id ) )
	{
		return "group " + std::to_string( g->vmfId );
	}
	return std::string();
}

bool IsProceduralTarget( std::string_view target )
{
	return !target.empty() && target.front() == '!';
}

bool TargetNamesEntity( std::string_view target, const scene::Entity &entity )
{
	if ( target.empty() || IsProceduralTarget( target ) )
	{
		return false;
	}
	const std::string_view name = entity.Name();
	if ( !name.empty() && scene::NameMatches( target, name ) )
	{
		return true;
	}
	return detail::EqualsNoCase( target, entity.classname );
}

} // namespace hammer::presenters
