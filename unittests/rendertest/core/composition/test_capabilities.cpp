//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition.capabilities (RFC 0016 K10 "Capability
//			negotiation"):
//
//			C1 an independent oracle judges negotiations: each requested
//			   feature is composed as itself when the device has what it
//			   requires, else as its fallback only when the profile declares
//			   "feature=fallback", and every substitution is reported by name;
//			   a feature with no fallback fails kMissingCapability, and an
//			   undeclared fallback fails kUndeclaredFallback, naming it;
//			C2 two seeded bad compositions (NegotiationFaults) each fail the
//			   oracle: one substitutes an undeclared fallback, one substitutes
//			   silently;
//			C3 RenderCore_Create with compute masked off (the product profile's
//			   -render-masked-capabilities) composes CPU skinning when the
//			   profile declares skinning=skinning-cpu and reports it in
//			   RenderCoreResult; without the declaration it fails
//			   RENDER_CORE_UNDECLARED_FALLBACK naming the fallback; an unknown
//			   capability name fails RENDER_CORE_INVALID_CONFIG.
//
//			The device is the null adapter, or the OpenGL one when built with
//			RENDER_CORE_GL (render.composition.capabilities.gl).
//
//=============================================================================//

#include "../../../../render/composition/negotiation.h"
#include "render/pass/skinning/feature.h"
#include "testing/checks.h"

#include <cstring>
#include <string>
#include <vector>

// Last: it reaches legacy headers that define min and max.
#include "render/composition/render_core.h"

namespace
{

using namespace render;
using render::composition::Negotiation;
using render::composition::NegotiationFaults;
using render::composition::NegotiationStatus;

#if defined( RENDER_CORE_GL )
constexpr const char *kDevice = "gl";
#else
constexpr const char *kDevice = "null";
#endif

// Test features: "grid" needs nothing; "traced" needs ray query and has no
// fallback; the skinning pair is the product's.
class TestFeature final : public frame::IRenderFeature
{
public:
	TestFeature( const char *name, frame::FeatureRequirements requirements )
	    : m_Name( name ), m_Requirements( requirements )
	{
	}
	const char *Name() const override { return m_Name; }
	frame::FeatureRequirements Requirements() const override { return m_Requirements; }
	void AddPasses( frame::FeatureContext & ) override {}

private:
	const char *m_Name;
	frame::FeatureRequirements m_Requirements;
};

std::unique_ptr<frame::IRenderFeature> Catalog( std::string_view name )
{
	if ( name == "grid" )
		return std::make_unique<TestFeature>( "grid", frame::FeatureRequirements{} );
	if ( name == "traced" )
	{
		frame::FeatureRequirements requirements;
		requirements.required = { device::Capability::kRayQuery };
		return std::make_unique<TestFeature>( "traced", requirements );
	}
	if ( name == pass::skinning::kSkinningFeature )
		return pass::skinning::CreateSkinningFeature();
	if ( name == pass::skinning::kCpuSkinningFeature )
		return pass::skinning::CreateCpuSkinningFeature();
	return nullptr;
}

std::vector<std::string> Split( std::string_view list, char separator )
{
	std::vector<std::string> items;
	while ( !list.empty() )
	{
		const std::size_t at = list.find( separator );
		if ( at != 0 )
			items.emplace_back( list.substr( 0, at ) );
		list = at == std::string_view::npos ? std::string_view() : list.substr( at + 1 );
	}
	return items;
}

// The oracle: what a negotiation of `requested` against `have` must yield,
// written from the rule, not from Negotiate. Returns the first violation,
// or empty.
std::string Judge( std::string_view requested, std::string_view declared,
    device::CapabilitySet have, const Negotiation &got )
{
	const std::vector<std::string> names = Split( requested, ',' );
	const std::vector<std::string> declarations = Split( declared, ',' );
	std::vector<std::string> expectedComposed;
	std::vector<std::string> expectedSubstituted; // "feature>fallback"
	NegotiationStatus expected = NegotiationStatus::kOk;
	std::string failing;
	for ( const std::string &name : names )
	{
		std::unique_ptr<frame::IRenderFeature> feature = Catalog( name );
		const frame::FeatureRequirements requirements = feature->Requirements();
		if ( !device::FirstMissing( have, requirements.required ) )
		{
			expectedComposed.push_back( name );
			continue;
		}
		if ( !requirements.fallback )
		{
			expected = NegotiationStatus::kMissingCapability;
			failing = name;
			break;
		}
		const std::string declaration = name + "=" + requirements.fallback;
		bool isDeclared = false;
		for ( const std::string &item : declarations )
			isDeclared |= item == declaration;
		if ( !isDeclared )
		{
			expected = NegotiationStatus::kUndeclaredFallback;
			failing = requirements.fallback;
			break;
		}
		expectedComposed.push_back( requirements.fallback );
		expectedSubstituted.push_back( name + ">" + requirements.fallback );
	}
	if ( got.status != expected )
		return expected == NegotiationStatus::kUndeclaredFallback
		           ? "an undeclared fallback was taken"
		           : "the status is not the rule's";
	if ( expected != NegotiationStatus::kOk )
	{
		if ( got.message.find( "'" + failing + "'" ) == std::string::npos )
			return "the failure does not name '" + failing + "'";
		return got.features.empty() ? "" : "a failed negotiation left features";
	}
	if ( got.features.size() != expectedComposed.size() )
		return "the composed features are not the requested ones";
	for ( std::size_t i = 0; i < expectedComposed.size(); ++i )
	{
		if ( expectedComposed[i] != got.features[i]->Name() )
			return "feature " + std::to_string( i ) + " is " + got.features[i]->Name() + ", not " +
			       expectedComposed[i];
	}
	std::vector<std::string> reported;
	for ( const composition::Substitution &substitution : got.substitutions )
		reported.push_back( substitution.feature + ">" + substitution.fallback );
	if ( reported != expectedSubstituted )
		return "a substitution was not reported (silent)";
	return {};
}

void NegotiationClauses( testing::Checks &checks )
{
	const device::CapabilitySet all = device::CapabilitySet::All();
	const device::CapabilitySet noCompute =
	    device::CapabilitySet::All().Remove( device::Capability::kCompute );
	struct Case
	{
		const char *what;
		const char *features;
		const char *declared;
		device::CapabilitySet have;
	};
	const Case cases[] = {
	    { "a device with compute keeps GPU skinning", "grid,skinning", "skinning=skinning-cpu",
	        all },
	    { "without compute a declared fallback is taken and reported", "grid,skinning",
	        "skinning=skinning-cpu", noCompute },
	    { "without compute an undeclared fallback fails, naming it", "grid,skinning", "",
	        noCompute },
	    { "a declaration of another fallback does not count", "skinning", "skinning=grid",
	        noCompute },
	    { "a feature lacking a capability with no fallback fails, naming it", "grid,traced",
	        "traced=grid", device::CapabilitySet::All().Remove( device::Capability::kRayQuery ) },
	};
	for ( const Case &c : cases )
	{
		const Negotiation got = composition::Negotiate( c.features, c.declared, c.have, Catalog );
		const std::string violation = Judge( c.features, c.declared, c.have, got );
		checks.That( violation.empty(), std::string( "composition.C1 " ) + c.what +
		                                    ( violation.empty() ? "" : ": " + violation ) );
	}
	const Negotiation reported =
	    composition::Negotiate( "grid,skinning", "skinning=skinning-cpu", noCompute, Catalog );
	checks.That( composition::DescribeSubstitutions( reported.substitutions ) ==
	                 "skinning -> skinning-cpu (lacks compute)",
	    "composition.C1 the substitution is reported as feature, fallback and capability" );

	// C2: seeded bad compositions must each fail the oracle.
	NegotiationFaults undeclared;
	undeclared.acceptUndeclared = true;
	const Negotiation badUndeclared =
	    composition::Negotiate( "grid,skinning", "", noCompute, Catalog, undeclared );
	checks.That( !Judge( "grid,skinning", "", noCompute, badUndeclared ).empty(),
	    "composition.C2 a composition that takes an undeclared fallback is caught" );
	NegotiationFaults silent;
	silent.silent = true;
	const Negotiation badSilent = composition::Negotiate(
	    "grid,skinning", "skinning=skinning-cpu", noCompute, Catalog, silent );
	checks.That( !Judge( "grid,skinning", "skinning=skinning-cpu", noCompute, badSilent ).empty(),
	    "composition.C2 a composition that substitutes silently is caught" );
}

void CoreClauses( testing::Checks &checks )
{
	RenderCoreConfig config;
	config.device = kDevice;
	config.features = "skinning,present";
	config.fallbacks = "skinning=skinning-cpu";
	config.maskedCapabilities = "compute,storage-buffers";
	RenderCoreResult result;
	RenderCore *core = RenderCore_Create( &config, &result );
	const std::string device = kDevice;
	checks.That( core != nullptr && result.status == RENDER_CORE_OK,
	    "composition.C3 " + device + " with compute masked composes (" + result.message + ")" );
	checks.That(
	    result.substitutionCount == 1 &&
	        std::strcmp( result.substitutions, "skinning -> skinning-cpu (lacks compute)" ) == 0,
	    "composition.C3 " + device + ": the result names the CPU skinning substitution" );
	if ( core )
	{
		const RenderCoreBinding *binding = RenderCore_GetBinding( core );
		checks.That( binding && binding->device &&
		                 !binding->device->Facts().capabilities.Has( device::Capability::kCompute ),
		    "composition.C3 " + device + ": the composed device does not claim compute" );
		RenderCore_Destroy( core );
	}

	config.fallbacks = "";
	core = RenderCore_Create( &config, &result );
	checks.That( !core && result.status == RENDER_CORE_UNDECLARED_FALLBACK &&
	                 std::strstr( result.message, "'skinning-cpu'" ) != nullptr,
	    "composition.C3 " + device + ": an undeclared fallback fails, naming it" );
	if ( core )
		RenderCore_Destroy( core );

	config.fallbacks = "skinning=skinning-cpu";
	config.maskedCapabilities = "compute,warp-drive";
	core = RenderCore_Create( &config, &result );
	checks.That( !core && result.status == RENDER_CORE_INVALID_CONFIG &&
	                 std::strstr( result.message, "'warp-drive'" ) != nullptr,
	    "composition.C3 an unknown masked capability fails, naming it" );
	if ( core )
		RenderCore_Destroy( core );

	config.maskedCapabilities = "";
	core = RenderCore_Create( &config, &result );
	checks.That( core && result.substitutionCount == 0 && result.substitutions[0] == '\0',
	    "composition.C3 " + device + " with compute keeps GPU skinning and reports nothing" );
	if ( core )
		RenderCore_Destroy( core );
}

} // namespace

int main()
{
	testing::Checks checks;
	NegotiationClauses( checks );
	CoreClauses( checks );
	return checks.Report();
}
