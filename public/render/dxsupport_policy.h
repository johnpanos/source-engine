//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Which dxsupport.cfg groups make up a recommended configuration, and
//          in what order they apply (render.dxsupport-policy.v1).
//
//          dxsupport.cfg is a list of groups. A recommended configuration is
//          the concatenation of the matching groups' values, later groups
//          overriding earlier ones. This header owns the matching and ordering
//          rules the D3D9 device manager has always used
//          (CShaderDeviceMgrBase::GetRecommendedConfigurationInfo); callers
//          describe each group's facts, apply the returned order to their own
//          key/value store, then apply the picmip floor.
//
//===========================================================================//

#ifndef RENDER_DXSUPPORT_POLICY_H
#define RENDER_DXSUPPORT_POLICY_H

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace render
{
// What the policy reads from one group. Integer facts are -1 when the group
// lacks the key; hex facts use strtol base 16 as the file format defines.
struct DxSupportGroupFacts
{
	bool hasName = false;
	std::string name;    // "name" as a string, for CPU-vendor matching
	int nameAsInt = 0;   // "name" as an integer (atoi; 0 when not numeric)
	int vendorId = -1;   // "VendorID"
	int minDeviceId = -1; // "MinDeviceID"
	int maxDeviceId = -1; // "MaxDeviceID"
	int minMegahertz = -1;
	int maxMegahertz = -1;
	int minMegabytes = -1;
	int maxMegabytes = -1;
	int minMegatexels = -1;
	int maxMegatexels = -1;
	std::optional<int> picmip; // "ConVar.mat_picmip"
};

struct DxSupportQuery
{
	int dxLevel = 0;         // already an actual level (see ClosestActualDxLevel)
	int maxDxLevel = 0;      // the adapter's maximum
	int vendorId = 0;
	int deviceId = 0;
	int cpuMegahertz = 0;
	bool cpuIsAmd = false;
	int systemRamMegabytes = 0;
	int videoMemoryBytes = 0; // the device's reported texture memory; 0 = unknown
};

struct DxSupportPlan
{
	// Group indices whose values are copied, in order; later ones override.
	std::vector<size_t> applyOrder;
	// When set, ConVar.mat_picmip becomes max( floor, the layered value ).
	std::optional<int> picmipFloor;
	// ConVar.mat_dxlevel is always forced to this level last.
	int dxLevel = 0;
};

// The levels a device manager can actually run, for a requested level.
[[nodiscard]] constexpr int ClosestActualDxLevel( int dxLevel, bool openGL, int absoluteMinimum )
{
	if ( dxLevel < absoluteMinimum )
		return absoluteMinimum;
	if ( dxLevel == 80 )
		return 80;
	if ( dxLevel <= 89 )
		return 81;
	if ( openGL )
		return ( dxLevel <= 90 ) ? 90 : 92;
	if ( dxLevel <= 94 )
		return 90;
	if ( dxLevel <= 99 )
		return 95;
	return 100;
}

namespace dxsupport_detail
{
inline bool ContainsNoCase( const std::string &haystack, const char *needle )
{
	const std::string::size_type length = std::char_traits<char>::length( needle );
	if ( length == 0 )
		return true;
	for ( std::string::size_type start = 0; start + length <= haystack.size(); ++start )
	{
		std::string::size_type i = 0;
		while ( i < length && std::tolower( static_cast<unsigned char>( haystack[start + i] ) ) ==
		                          std::tolower( static_cast<unsigned char>( needle[i] ) ) )
			++i;
		if ( i == length )
			return true;
	}
	return false;
}

template <typename Predicate>
std::optional<size_t> FindFirst( const std::vector<DxSupportGroupFacts> &groups, Predicate matches )
{
	for ( size_t i = 0; i < groups.size(); ++i )
	{
		if ( matches( groups[i] ) )
			return i;
	}
	return std::nullopt;
}
} // namespace dxsupport_detail

// Each lookup takes the first matching group, in file order:
// - dx level: "name" as an integer equals the level;
// - dx level and vendor: that, and VendorID equals the vendor;
// - card: VendorID equals the vendor and MinDeviceID <= device <= MaxDeviceID.
// A card group spanning every device (0..0xffff) is a vendor catch-all: it
// applies before the dx-level-and-vendor group. Any narrower card group applies
// last and the dx-level-and-vendor group is skipped. CPU ("AMD"/"Intel" in the
// name, [min, max) megahertz) then system RAM ([min, max) megabytes) groups
// follow. The video-memory group ([min, max) megatexels, compared with memory
// in MB) only contributes a picmip floor, and only at the maximum level or
// under 100 MB.
[[nodiscard]] inline DxSupportPlan ResolveRecommendedConfig(
    const std::vector<DxSupportGroupFacts> &groups, const DxSupportQuery &query )
{
	using dxsupport_detail::FindFirst;
	DxSupportPlan plan;
	plan.dxLevel = query.dxLevel;

	const std::optional<size_t> dxLevelGroup =
	    FindFirst( groups, [&]( const DxSupportGroupFacts &g ) { return g.nameAsInt == query.dxLevel; } );
	const std::optional<size_t> dxLevelVendorGroup =
	    FindFirst( groups, [&]( const DxSupportGroupFacts &g ) {
		    return g.nameAsInt == query.dxLevel && g.vendorId == query.vendorId;
	    } );
	const std::optional<size_t> cardGroup = FindFirst( groups, [&]( const DxSupportGroupFacts &g ) {
		return g.vendorId == query.vendorId && query.deviceId >= g.minDeviceId &&
		       query.deviceId <= g.maxDeviceId;
	} );

	const auto apply = [&]( const std::optional<size_t> &group ) {
		if ( group )
			plan.applyOrder.push_back( *group );
	};
	const bool cardIsCatchAll =
	    cardGroup && groups[*cardGroup].minDeviceId == 0 && groups[*cardGroup].maxDeviceId == 0xffff;
	apply( dxLevelGroup );
	apply( cardGroup );
	if ( cardIsCatchAll )
		apply( dxLevelVendorGroup );

	apply( FindFirst( groups, [&]( const DxSupportGroupFacts &g ) {
		if ( !g.hasName )
			return false;
		const bool vendorMatches = query.cpuIsAmd ? dxsupport_detail::ContainsNoCase( g.name, "AMD" )
		                                          : dxsupport_detail::ContainsNoCase( g.name, "Intel" );
		return vendorMatches && g.minMegahertz != -1 && g.maxMegahertz != -1 &&
		       g.minMegahertz <= query.cpuMegahertz && query.cpuMegahertz < g.maxMegahertz;
	} ) );
	apply( FindFirst( groups, [&]( const DxSupportGroupFacts &g ) {
		return g.minMegabytes != -1 && g.maxMegabytes != -1 &&
		       g.minMegabytes <= query.systemRamMegabytes &&
		       query.systemRamMegabytes < g.maxMegabytes;
	} ) );

	const int videoMemoryMegabytes = query.videoMemoryBytes / ( 1024 * 1024 );
	const std::optional<size_t> videoMemoryGroup =
	    FindFirst( groups, [&]( const DxSupportGroupFacts &g ) {
		    return g.minMegatexels != -1 && g.maxMegatexels != -1 &&
		           g.minMegatexels <= videoMemoryMegabytes && videoMemoryMegabytes < g.maxMegatexels;
	    } );
	if ( videoMemoryGroup && query.videoMemoryBytes > 0 && groups[*videoMemoryGroup].picmip &&
	     ( query.dxLevel == query.maxDxLevel || videoMemoryMegabytes < 100 ) )
		plan.picmipFloor = groups[*videoMemoryGroup].picmip;
	return plan;
}
} // namespace render

#endif // RENDER_DXSUPPORT_POLICY_H
