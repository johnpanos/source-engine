//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render feature profiles and quirks (RFC 0001, "Capabilities, profiles,
//			and quirks").
//
//			RFC 0001 separates three categories the legacy hardware config mixes:
//
//			  1. device capabilities -- facts reported by a provider
//			     (RenderAdapterInfo / RenderDeviceCaps in render_backend.h);
//			  2. the feature profile -- application policy: the features the engine
//			     chooses to use on a device (RenderFeatureProfile below);
//			  3. quirks -- documented workarounds selected from backend, driver,
//			     adapter and version data (RenderQuirk below).
//
//			A composition root states its requirements in a RenderProfileRequest.
//			SelectRenderFeatureProfile combines that request with an adapter's facts
//			and a quirk table and returns either a profile or a structured error.
//			Portable code then asks the profile semantic questions ("is sampled sRGB
//			enabled?", "is the float normalization-cubemap workaround active?"); it
//			never asks which backend or driver it is running on.
//
//			Like render_backend.h this header is self-contained, C++11-compatible
//			(legacy material-system targets include it) and free of native types.
//
//=============================================================================//

#ifndef RENDER_RENDER_PROFILE_H
#define RENDER_RENDER_PROFILE_H

#include "render/render_backend.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace render
{

// ---------------------------------------------------------------------------
// Workarounds
// ---------------------------------------------------------------------------

// A named behavior change that exists only because some provider/driver needs
// it. Each value is enabled only by a quirk; portable code consults the selected
// profile instead of testing the backend identity that motivated the quirk.
enum class RenderWorkaround : uint32_t
{
	// Build normalization cubemaps as RGBA16F instead of 8-bit (BGRX8888 and
	// signed UVWQ8888) formats.
	kFloatNormalizationCubemaps = 0,
	kCount,
};

struct RenderWorkaroundSet
{
	uint32_t bits = 0;

	static uint32_t Bit( RenderWorkaround w )
	{
		return uint32_t( 1 ) << static_cast<uint32_t>( w );
	}
	static uint32_t KnownBits()
	{
		return ( uint32_t( 1 ) << static_cast<uint32_t>( RenderWorkaround::kCount ) ) - 1;
	}

	void Add( RenderWorkaround w ) { bits |= Bit( w ); }
	bool Has( RenderWorkaround w ) const { return ( bits & Bit( w ) ) != 0; }
	bool IsEmpty() const { return bits == 0; }

	bool operator==( const RenderWorkaroundSet &o ) const { return bits == o.bits; }
	bool operator!=( const RenderWorkaroundSet &o ) const { return bits != o.bits; }
};

// ---------------------------------------------------------------------------
// Quirks
// ---------------------------------------------------------------------------

// A documented workaround with the reason it exists and the provider range it
// affects. Quirk tables are static data owned by one policy module; profiles
// refer to their entries by pointer, so a table must outlive every profile
// selected from it.
//
// Selectors: a null string or a zero id matches anything, and numeric ranges are
// inclusive. A quirk must name at least one of backendId, driverApi or vendorId;
// a quirk that applies everywhere is product policy, not a quirk.
struct RenderQuirk
{
	const char *id;        // stable token, e.g. "gl.float-normalization-cubemaps"
	const char *reason;    // why the workaround exists; required
	const char *backendId; // RenderBackendId::id, or null for any provider
	const char *driverApi; // RenderAdapterInfo::driverApi, or null for any
	uint32_t vendorId;     // 0 for any vendor
	uint32_t deviceIdMin;
	uint32_t deviceIdMax;
	uint64_t driverVersionMin;
	uint64_t driverVersionMax;
	uint32_t removedFeatureBits;    // RenderFeatureSet bits the quirk disables
	uint32_t enabledWorkaroundBits; // RenderWorkaroundSet bits the quirk enables
};

static const uint32_t kRenderAnyDeviceIdMax = 0xFFFFFFFFu;
static const uint64_t kRenderAnyDriverVersionMax = 0xFFFFFFFFFFFFFFFFull;

// ---------------------------------------------------------------------------
// Requests, profiles and structured failure
// ---------------------------------------------------------------------------

// What a composition root asks for. Required features must be enabled or
// selection fails; preferred features are enabled when the adapter supports them
// and no quirk removes them.
struct RenderProfileRequest
{
	RenderFeatureSet required;
	RenderFeatureSet preferred;
};

// Requires nothing and prefers every feature the adapter supports. This is the
// material system's default for roots that state no request.
inline RenderProfileRequest PreferAvailableRenderFeatures()
{
	RenderProfileRequest request;
	request.preferred.bits = 0xFFFFFFFFu;
	return request;
}

static const uint32_t kRenderMaxAppliedQuirks = 16;

// The selected policy. It is a value: once selected it does not change, and it
// holds no pointer into provider memory (applied quirk pointers refer to the
// static quirk table).
struct RenderFeatureProfile
{
	RenderFeatureSet enabled;
	RenderWorkaroundSet workarounds;
	const RenderQuirk *appliedQuirks[kRenderMaxAppliedQuirks] = {};
	uint32_t appliedQuirkCount = 0;
};

enum class RenderProfileStatus : uint32_t
{
	kOk = 0,
	kInvalidQuirk,           // the quirk table violates the rules above
	kMissingRequiredFeature, // the adapter does not support a required feature
	kRemovedByQuirk,         // a quirk disabled a required feature
	kTooManyQuirks,          // more than kRenderMaxAppliedQuirks matched
	kInvalidProvider,        // the provider's adapter facts are inconsistent
	kInvalidAdapter,         // the requested adapter does not exist
	kClaimNotEnabled,        // a claimed feature is not enabled on the created device
};

struct RenderProfileError
{
	RenderProfileStatus status = RenderProfileStatus::kOk;
	// Valid for kMissingRequiredFeature and kRemovedByQuirk.
	RenderFeature feature = RenderFeature::kNeverSupported;
	// Valid for kInvalidQuirk and kRemovedByQuirk; points into the quirk table.
	const RenderQuirk *quirk = nullptr;
	char message[128] = { 0 };
};

// ---------------------------------------------------------------------------
// Selection
// ---------------------------------------------------------------------------

// Longest selector string compared; driverApi tokens are shorter than this.
static const size_t kRenderMaxSelectorLength = 64;

namespace profile_detail
{

inline bool SelectorMatches( const char *selector, const char *value )
{
	return !selector ||
	       ( value && contract_detail::EqualStrings( selector, value, kRenderMaxSelectorLength ) );
}

inline RenderFeature LowestFeature( uint32_t bits )
{
	for ( uint32_t bit = 0; bit < 32; ++bit )
	{
		if ( bits & ( uint32_t( 1 ) << bit ) )
			return static_cast<RenderFeature>( bit );
	}
	return RenderFeature::kNeverSupported;
}

inline bool Fail( RenderProfileError *error, RenderProfileStatus status, const char *message,
    RenderFeature feature = RenderFeature::kNeverSupported, const RenderQuirk *quirk = nullptr )
{
	if ( error )
	{
		error->status = status;
		error->feature = feature;
		error->quirk = quirk;
		std::snprintf( error->message, sizeof( error->message ), "%s%s%s", message,
		    quirk && quirk->id ? ": " : "", quirk && quirk->id ? quirk->id : "" );
	}
	return false;
}

} // namespace profile_detail

// True when a quirk entry obeys the table rules: an id and a reason, at least one
// provider selector, ordered ranges, and at least one known effect.
inline bool IsValidRenderQuirk( const RenderQuirk &quirk )
{
	return quirk.id && quirk.id[0] && quirk.reason && quirk.reason[0] &&
	       ( quirk.backendId || quirk.driverApi || quirk.vendorId != 0 ) &&
	       quirk.deviceIdMin <= quirk.deviceIdMax &&
	       quirk.driverVersionMin <= quirk.driverVersionMax &&
	       ( quirk.removedFeatureBits != 0 || quirk.enabledWorkaroundBits != 0 ) &&
	       ( quirk.enabledWorkaroundBits & ~RenderWorkaroundSet::KnownBits() ) == 0;
}

inline bool RenderQuirkMatches(
    const RenderQuirk &quirk, const char *backendId, const RenderAdapterInfo &adapter )
{
	return profile_detail::SelectorMatches( quirk.backendId, backendId ) &&
	       profile_detail::SelectorMatches( quirk.driverApi, adapter.driverApi ) &&
	       ( quirk.vendorId == 0 || quirk.vendorId == adapter.vendorId ) &&
	       adapter.deviceId >= quirk.deviceIdMin && adapter.deviceId <= quirk.deviceIdMax &&
	       adapter.driverVersion >= quirk.driverVersionMin &&
	       adapter.driverVersion <= quirk.driverVersionMax;
}

// A provider's claim for a device it created: every feature its adapter
// advertises is enabled on that device (`enabledOnDevice` is the provider's
// record of what its device creation enabled, from the device's own queries).
// A claim beyond it fails with kClaimNotEnabled naming the lowest such feature.
inline bool ValidateDeviceClaims( const RenderAdapterInfo &adapter,
    const RenderFeatureSet &enabledOnDevice, RenderProfileError *error )
{
	const uint32_t unclaimed = adapter.supportedFeatures.bits & ~enabledOnDevice.bits;
	if ( unclaimed )
		return profile_detail::Fail( error, RenderProfileStatus::kClaimNotEnabled,
		    "the adapter claims a feature its device did not enable",
		    profile_detail::LowestFeature( unclaimed ) );
	return true;
}

// Selects the profile for one adapter of the provider identified by backendId.
// The whole table is validated first, so an invalid entry fails every selection
// rather than only the adapters it happens to match. Quirks apply in table order.
// On failure 'out' is left unchanged.
inline bool SelectRenderFeatureProfile( const char *backendId, const RenderAdapterInfo &adapter,
    const RenderProfileRequest &request, const RenderQuirk *quirks, size_t quirkCount,
    RenderFeatureProfile *out, RenderProfileError *error )
{
	using profile_detail::Fail;
	if ( !out || !backendId || !backendId[0] || ( quirkCount && !quirks ) )
		return Fail( error, RenderProfileStatus::kInvalidProvider,
		    "selection needs a provider id, an output and a quirk table" );

	for ( size_t i = 0; i < quirkCount; ++i )
	{
		if ( !IsValidRenderQuirk( quirks[i] ) )
			return Fail( error, RenderProfileStatus::kInvalidQuirk, "invalid quirk entry",
			    RenderFeature::kNeverSupported, &quirks[i] );
	}

	const uint32_t supported = adapter.supportedFeatures.bits;
	const uint32_t missing = request.required.bits & ~supported;
	if ( missing )
		return Fail( error, RenderProfileStatus::kMissingRequiredFeature,
		    "adapter lacks a required feature", profile_detail::LowestFeature( missing ) );

	RenderFeatureProfile profile;
	uint32_t removed = 0;
	for ( size_t i = 0; i < quirkCount; ++i )
	{
		const RenderQuirk &quirk = quirks[i];
		if ( !RenderQuirkMatches( quirk, backendId, adapter ) )
			continue;
		const uint32_t removesRequired = quirk.removedFeatureBits & request.required.bits;
		if ( removesRequired )
			return Fail( error, RenderProfileStatus::kRemovedByQuirk,
			    "a quirk disables a required feature",
			    profile_detail::LowestFeature( removesRequired ), &quirk );
		if ( profile.appliedQuirkCount == kRenderMaxAppliedQuirks )
			return Fail(
			    error, RenderProfileStatus::kTooManyQuirks, "too many quirks match this adapter" );
		removed |= quirk.removedFeatureBits;
		profile.workarounds.bits |= quirk.enabledWorkaroundBits;
		profile.appliedQuirks[profile.appliedQuirkCount++] = &quirk;
	}

	profile.enabled.bits =
	    ( request.required.bits | request.preferred.bits ) & supported & ~removed;
	*out = profile;
	if ( error )
		*error = RenderProfileError();
	return true;
}

} // namespace render

#endif // RENDER_RENDER_PROFILE_H
