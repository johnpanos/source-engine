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
	std::string name;     // "name" as a string, for CPU-vendor matching
	int nameAsInt = 0;    // "name" as an integer (atoi; 0 when not numeric)
	int vendorId = -1;    // "VendorID"
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
	int dxLevel = 0;    // already an actual level (see ClosestActualDxLevel)
	int maxDxLevel = 0; // the adapter's maximum
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
inline char AsciiLower( char c )
{
	return ( c >= 'A' && c <= 'Z' ) ? static_cast<char>( c - 'A' + 'a' ) : c;
}

inline bool ContainsNoCase( const std::string &haystack, const char *needle )
{
	const std::string::size_type length = std::char_traits<char>::length( needle );
	if ( length == 0 )
		return true;
	for ( std::string::size_type start = 0; start + length <= haystack.size(); ++start )
	{
		std::string::size_type i = 0;
		while ( i < length && AsciiLower( haystack[start + i] ) == AsciiLower( needle[i] ) )
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

// The card group for a device: VendorID equals the vendor and
// MinDeviceID <= device <= MaxDeviceID (first in file order). It also carries
// the device's MaxDXLevel/DXLevel.
[[nodiscard]] inline std::optional<size_t> FindCardGroup(
    const std::vector<DxSupportGroupFacts> &groups, int vendorId, int deviceId )
{
	return dxsupport_detail::FindFirst( groups,
	    [&]( const DxSupportGroupFacts &g )
	    {
		    return g.vendorId == vendorId && deviceId >= g.minDeviceId && deviceId <= g.maxDeviceId;
	    } );
}

// The device groups for a level, in application order: the dx-level group
// ("name" as an integer equals the level), then the card group. A card group
// spanning every device (0..0xffff) is a vendor catch-all, so the dx-level-and-
// vendor group (same name, VendorID equal) applies after it; a narrower card
// group applies last and the dx-level-and-vendor group is skipped. Both the
// recommended configuration and the hardware caps apply this order.
[[nodiscard]] inline std::vector<size_t> ResolveDeviceGroupOrder(
    const std::vector<DxSupportGroupFacts> &groups, int dxLevel, int vendorId, int deviceId )
{
	using dxsupport_detail::FindFirst;
	std::vector<size_t> order;
	const std::optional<size_t> dxLevelGroup = FindFirst( groups,
	    [&]( const DxSupportGroupFacts &g )
	    {
		    return g.nameAsInt == dxLevel;
	    } );
	const std::optional<size_t> dxLevelVendorGroup = FindFirst( groups,
	    [&]( const DxSupportGroupFacts &g )
	    {
		    return g.nameAsInt == dxLevel && g.vendorId == vendorId;
	    } );
	const std::optional<size_t> cardGroup = FindCardGroup( groups, vendorId, deviceId );
	const bool cardIsCatchAll = cardGroup && groups[*cardGroup].minDeviceId == 0 &&
	                            groups[*cardGroup].maxDeviceId == 0xffff;
	for ( const std::optional<size_t> &group :
	    { dxLevelGroup, cardGroup, cardIsCatchAll ? dxLevelVendorGroup : std::nullopt } )
	{
		if ( group )
			order.push_back( *group );
	}
	return order;
}

// The recommended configuration: the device groups (ResolveDeviceGroupOrder),
// then the CPU group ("AMD"/"Intel" in the name, [min, max) megahertz), then
// the system-RAM group ([min, max) megabytes). The video-memory group
// ([min, max) megatexels, compared with memory in MB) only contributes a
// picmip floor, and only at the maximum level or under 100 MB.
[[nodiscard]] inline DxSupportPlan ResolveRecommendedConfig(
    const std::vector<DxSupportGroupFacts> &groups, const DxSupportQuery &query )
{
	using dxsupport_detail::FindFirst;
	DxSupportPlan plan;
	plan.dxLevel = query.dxLevel;
	plan.applyOrder =
	    ResolveDeviceGroupOrder( groups, query.dxLevel, query.vendorId, query.deviceId );

	const auto apply = [&]( const std::optional<size_t> &group )
	{
		if ( group )
			plan.applyOrder.push_back( *group );
	};
	apply( FindFirst( groups,
	    [&]( const DxSupportGroupFacts &g )
	    {
		    if ( !g.hasName )
			    return false;
		    const bool vendorMatches = query.cpuIsAmd
		                                   ? dxsupport_detail::ContainsNoCase( g.name, "AMD" )
		                                   : dxsupport_detail::ContainsNoCase( g.name, "Intel" );
		    return vendorMatches && g.minMegahertz != -1 && g.maxMegahertz != -1 &&
		           g.minMegahertz <= query.cpuMegahertz && query.cpuMegahertz < g.maxMegahertz;
	    } ) );
	apply( FindFirst( groups,
	    [&]( const DxSupportGroupFacts &g )
	    {
		    return g.minMegabytes != -1 && g.maxMegabytes != -1 &&
		           g.minMegabytes <= query.systemRamMegabytes &&
		           query.systemRamMegabytes < g.maxMegabytes;
	    } ) );

	const int videoMemoryMegabytes = query.videoMemoryBytes / ( 1024 * 1024 );
	const std::optional<size_t> videoMemoryGroup = FindFirst( groups,
	    [&]( const DxSupportGroupFacts &g )
	    {
		    return g.minMegatexels != -1 && g.maxMegatexels != -1 &&
		           g.minMegatexels <= videoMemoryMegabytes &&
		           videoMemoryMegabytes < g.maxMegatexels;
	    } );
	if ( videoMemoryGroup && query.videoMemoryBytes > 0 && groups[*videoMemoryGroup].picmip &&
	     ( query.dxLevel == query.maxDxLevel || videoMemoryMegabytes < 100 ) )
		plan.picmipFloor = groups[*videoMemoryGroup].picmip;
	return plan;
}
} // namespace render

#endif // RENDER_DXSUPPORT_POLICY_H
