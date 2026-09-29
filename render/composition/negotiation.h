//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's capability negotiation (RFC 0016 K10
//			"Capability negotiation"): the configured features against the
//			device's capabilities. A feature the device can run is composed; one
//			it cannot run is replaced by its declared fallback
//			(FeatureRequirements::fallback) only when the product profile
//			declares that substitution ("feature=fallback"), and every
//			substitution is reported by name. Anything else fails composition
//			with a structured result.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_NEGOTIATION_H
#define RENDER_COMPOSITION_NEGOTIATION_H

#include "render/device/facts.h"
#include "render/frame/feature.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace render::composition
{

enum class NegotiationStatus : std::uint8_t
{
	kOk,
	kUnknownFeature,    // a name the product does not link
	kMissingCapability, // the device lacks a requirement and the feature has no fallback
	kUndeclaredFallback // the feature has a fallback the profile does not declare
};

struct Substitution
{
	std::string feature;  // requested
	std::string fallback; // composed instead
	device::Capability lacking = device::Capability::kCompute;
};

struct Negotiation
{
	NegotiationStatus status = NegotiationStatus::kOk;
	std::string message; // names the feature, fallback and capability on failure
	std::vector<std::unique_ptr<frame::IRenderFeature>> features;
	std::vector<Substitution> substitutions;
};

using FeatureFactory = std::function<std::unique_ptr<frame::IRenderFeature>( std::string_view )>;

// Seeded bad compositions (render.composition.capabilities sensitivity only;
// RenderCore_Create never sets them): each breaks one rule the suite's oracle
// must catch.
struct NegotiationFaults
{
	bool acceptUndeclared = false; // substitutes without the profile's declaration
	bool silent = false;           // substitutes without reporting it
};

// `features` and `declared` are comma-separated: feature names in frame
// order, and the profile's "feature=fallback" substitutions.
Negotiation Negotiate( std::string_view features, std::string_view declared,
    device::CapabilitySet have, const FeatureFactory &create, NegotiationFaults faults = {} );

// "skinning -> skinning-cpu (lacks compute)", comma-separated.
std::string DescribeSubstitutions( const std::vector<Substitution> &substitutions );

} // namespace render::composition

#endif // RENDER_COMPOSITION_NEGOTIATION_H
