//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's capability negotiation; see negotiation.h.
//
//=============================================================================//

#include "negotiation.h"

#include <optional>

namespace render::composition
{

namespace
{

// Calls visit( item ) for each non-empty comma-separated item.
template <typename Visit> void ForEachItem( std::string_view list, const Visit &visit )
{
	while ( !list.empty() )
	{
		const std::size_t comma = list.find( ',' );
		const std::string_view item = list.substr( 0, comma );
		list = comma == std::string_view::npos ? std::string_view() : list.substr( comma + 1 );
		if ( !item.empty() )
			visit( item );
	}
}

bool Declared( std::string_view declared, std::string_view feature, std::string_view fallback )
{
	bool found = false;
	ForEachItem( declared,
	    [&]( std::string_view item )
	    {
		    const std::size_t equals = item.find( '=' );
		    found |= equals != std::string_view::npos && item.substr( 0, equals ) == feature &&
		             item.substr( equals + 1 ) == fallback;
	    } );
	return found;
}

} // namespace

Negotiation Negotiate( std::string_view features, std::string_view declared,
    device::CapabilitySet have, const FeatureFactory &create, NegotiationFaults faults )
{
	Negotiation result;
	auto fail = [&]( NegotiationStatus status, std::string message )
	{
		if ( result.status == NegotiationStatus::kOk )
		{
			result.status = status;
			result.message = std::move( message );
		}
	};
	ForEachItem( features,
	    [&]( std::string_view name )
	    {
		    if ( result.status != NegotiationStatus::kOk )
			    return;
		    std::unique_ptr<frame::IRenderFeature> feature = create( name );
		    if ( !feature )
			    return fail( NegotiationStatus::kUnknownFeature,
			        "render feature '" + std::string( name ) + "' is not linked in this product" );
		    const frame::FeatureRequirements requirements = feature->Requirements();
		    const std::optional<device::Capability> missing =
		        device::FirstMissing( have, requirements.required );
		    if ( !missing )
		    {
			    result.features.push_back( std::move( feature ) );
			    return;
		    }
		    const std::string lacking = device::CapabilityName( *missing );
		    if ( !requirements.fallback )
			    return fail( NegotiationStatus::kMissingCapability,
			        "render feature '" + std::string( name ) + "' requires " + lacking +
			            ", which the device lacks" );
		    const std::string fallback = requirements.fallback;
		    if ( !faults.acceptUndeclared && !Declared( declared, name, fallback ) )
			    return fail( NegotiationStatus::kUndeclaredFallback,
			        "render feature '" + std::string( name ) + "' requires " + lacking +
			            ", which the device lacks; its fallback '" + fallback +
			            "' is not declared by the product profile (" + std::string( name ) + "=" +
			            fallback + ")" );
		    std::unique_ptr<frame::IRenderFeature> substitute = create( fallback );
		    if ( !substitute )
			    return fail( NegotiationStatus::kUnknownFeature,
			        "render feature '" + fallback + "', the fallback of '" + std::string( name ) +
			            "', is not linked in this product" );
		    if ( const std::optional<device::Capability> alsoMissing =
		             device::FirstMissing( have, substitute->Requirements().required ) )
			    return fail( NegotiationStatus::kMissingCapability,
			        "render feature '" + fallback + "', the fallback of '" + std::string( name ) +
			            "', requires " + device::CapabilityName( *alsoMissing ) +
			            ", which the device lacks" );
		    if ( !faults.silent )
			    result.substitutions.push_back( { std::string( name ), fallback, *missing } );
		    result.features.push_back( std::move( substitute ) );
	    } );
	if ( result.status != NegotiationStatus::kOk )
		result.features.clear();
	return result;
}

std::string DescribeSubstitutions( const std::vector<Substitution> &substitutions )
{
	std::string text;
	for ( const Substitution &substitution : substitutions )
	{
		if ( !text.empty() )
			text += ", ";
		text += substitution.feature + " -> " + substitution.fallback + " (lacks " +
		        device::CapabilityName( substitution.lacking ) + ")";
	}
	return text;
}

} // namespace render::composition
