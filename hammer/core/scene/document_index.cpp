//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/document_index.h.
//
//=============================================================================//

#include "hammer/scene/document_index.h"

#include <algorithm>
#include <cctype>

namespace hammer::scene
{

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

} // namespace

DocumentIndex::DocumentIndex( const DocumentReader &doc )
{
	// Id-ordered iteration keeps every list in id order without sorting.
	for ( ObjectId id : doc.SolidIds() )
	{
		const Solid &s = *doc.FindSolid( id );
		if ( s.owner.IsValid() )
		{
			m_entitySolids[s.owner].push_back( id );
		}
		else
		{
			m_groupMembers[s.group].push_back( id );
		}
		m_vmfIds[{ static_cast<int>( ObjectKind::Solid ), s.vmfId }].push_back( id );
		for ( const Side &side : s.sides )
		{
			m_sideIds[side.vmfId].push_back( id );
		}
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		const Entity &e = *doc.FindEntity( id );
		m_groupMembers[e.group].push_back( id );
		m_vmfIds[{ static_cast<int>( ObjectKind::Entity ), e.vmfId }].push_back( id );
		const std::string_view name = e.Name();
		if ( !name.empty() )
		{
			m_names[Lower( name )].push_back( id );
		}
	}
	for ( ObjectId id : doc.GroupIds() )
	{
		const Group &g = *doc.FindGroup( id );
		if ( g.group != id )
		{
			m_groupMembers[g.group].push_back( id );
		}
		m_vmfIds[{ static_cast<int>( ObjectKind::Group ), g.vmfId }].push_back( id );
	}
	// Members mix kinds; GroupMembers returns them in id order.
	for ( auto &entry : m_groupMembers )
	{
		std::sort( entry.second.begin(), entry.second.end() );
	}
	for ( auto &entry : m_names )
	{
		std::sort( entry.second.begin(), entry.second.end() );
	}
}

const std::vector<ObjectId> &DocumentIndex::EntitySolids( ObjectId entity ) const
{
	const auto it = m_entitySolids.find( entity );
	return it == m_entitySolids.end() ? m_none : it->second;
}

const std::vector<ObjectId> &DocumentIndex::GroupMembers( ObjectId group ) const
{
	const auto it = m_groupMembers.find( group );
	return it == m_groupMembers.end() ? m_none : it->second;
}

std::vector<ObjectId> DocumentIndex::EntitiesNamed( std::string_view pattern ) const
{
	std::vector<ObjectId> out;
	if ( pattern.empty() )
	{
		return out;
	}
	if ( pattern.back() != '*' )
	{
		const auto it = m_names.find( Lower( pattern ) );
		if ( it != m_names.end() )
		{
			out = it->second;
		}
		return out;
	}
	const std::string prefix = Lower( pattern.substr( 0, pattern.size() - 1 ) );
	for ( auto it = m_names.lower_bound( prefix ); it != m_names.end() && it->first.rfind( prefix, 0 ) == 0; ++it )
	{
		out.insert( out.end(), it->second.begin(), it->second.end() );
	}
	std::sort( out.begin(), out.end() );
	return out;
}

bool DocumentIndex::AnyEntityNamed( std::string_view pattern ) const
{
	if ( pattern.empty() )
	{
		return false;
	}
	if ( pattern.back() != '*' )
	{
		return m_names.count( Lower( pattern ) ) > 0;
	}
	const std::string prefix = Lower( pattern.substr( 0, pattern.size() - 1 ) );
	const auto it = m_names.lower_bound( prefix );
	return it != m_names.end() && it->first.rfind( prefix, 0 ) == 0;
}

const std::vector<ObjectId> &DocumentIndex::WithVmfId( ObjectKind kind, std::uint32_t vmfId ) const
{
	const auto it = m_vmfIds.find( { static_cast<int>( kind ), vmfId } );
	return it == m_vmfIds.end() ? m_none : it->second;
}

const std::vector<ObjectId> &DocumentIndex::SolidsWithSide( std::uint32_t sideId ) const
{
	const auto it = m_sideIds.find( sideId );
	return it == m_sideIds.end() ? m_none : it->second;
}

} // namespace hammer::scene
