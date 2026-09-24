//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.dxsupport-policy.v1 — recommended-configuration group
//          selection and order against hand-derived expectations for a
//          dxsupport.cfg-shaped group list.
//
//===========================================================================//

#include "render/dxsupport_policy.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <utility>
#include <vector>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;

[[maybe_unused]] void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

render::DxSupportGroupFacts Named( const char *name, int nameAsInt )
{
	render::DxSupportGroupFacts group;
	group.hasName = true;
	group.name = name;
	group.nameAsInt = nameAsInt;
	return group;
}

render::DxSupportGroupFacts Card( const char *name, int vendor, int minDevice, int maxDevice )
{
	render::DxSupportGroupFacts group = Named( name, 0 );
	group.vendorId = vendor;
	group.minDeviceId = minDevice;
	group.maxDeviceId = maxDevice;
	return group;
}

enum GroupIndex : size_t
{
	kDx90,
	kDx95,
	kDx95Amd,
	kCpuAmd,
	kCpuIntel,
	kMemorySmall,
	kMemoryLarge,
	kVidMemSmall,
	kVidMemLarge,
	kAmdCatchAll,
	kNvidiaNarrow,
	kNvidiaCatchAll,
	kDx95Nvidia,
	kGroupCount,
};

// The shape of the shipped file: dx-level rows, a dx-level-and-vendor row,
// CPU/memory/video-memory rows, then card rows (a vendor catch-all for AMD, a
// narrow range before a catch-all for NVIDIA).
std::vector<render::DxSupportGroupFacts> Groups()
{
	std::vector<render::DxSupportGroupFacts> groups( kGroupCount );
	groups[kDx90] = Named( "90", 90 );
	groups[kDx95] = Named( "95", 95 );
	groups[kDx95Amd] = Named( "95", 95 );
	groups[kDx95Amd].vendorId = 0x1002;
	groups[kCpuAmd] = Named( "AMD", 0 );
	groups[kCpuAmd].minMegahertz = 0;
	groups[kCpuAmd].maxMegahertz = 1000000;
	groups[kCpuIntel] = Named( "Intel", 0 );
	groups[kCpuIntel].minMegahertz = 0;
	groups[kCpuIntel].maxMegahertz = 1000000;
	groups[kMemorySmall] = Named( "memory [0-250)", 0 );
	groups[kMemorySmall].minMegabytes = 0;
	groups[kMemorySmall].maxMegabytes = 250;
	groups[kMemoryLarge] = Named( "memory [256-512)", 0 );
	groups[kMemoryLarge].minMegabytes = 256;
	groups[kMemoryLarge].maxMegabytes = 512;
	groups[kVidMemSmall] = Named( "vid mem [0-128)", 0 );
	groups[kVidMemSmall].minMegatexels = 0;
	groups[kVidMemSmall].maxMegatexels = 128;
	groups[kVidMemSmall].picmip = 2;
	groups[kVidMemLarge] = Named( "vid mem [256-infinity)", 0 );
	groups[kVidMemLarge].minMegatexels = 256;
	groups[kVidMemLarge].maxMegatexels = 100000;
	groups[kAmdCatchAll] = Card( "AMD any", 0x1002, 0x0000, 0xffff );
	groups[kNvidiaNarrow] = Card( "NVIDIA narrow", 0x10de, 0x1000, 0x1fff );
	groups[kNvidiaCatchAll] = Card( "NVIDIA any", 0x10de, 0x0000, 0xffff );
	groups[kDx95Nvidia] = Named( "95", 95 );
	groups[kDx95Nvidia].vendorId = 0x10de;
	return groups;
}

render::DxSupportQuery Query( int vendor, int device )
{
	render::DxSupportQuery query;
	query.dxLevel = 95;
	query.maxDxLevel = 95;
	query.vendorId = vendor;
	query.deviceId = device;
	query.cpuMegahertz = 3000;
	query.cpuIsAmd = true;
	query.systemRamMegabytes = 32768;
	query.videoMemoryBytes = 2047 * 1024 * 1024;
	return query;
}

bool OrderIs( const render::DxSupportPlan &plan, std::vector<size_t> expected )
{
	return plan.applyOrder == expected;
}
} // namespace

int main()
{
	const std::vector<render::DxSupportGroupFacts> groups = Groups();

	// AMD 0x744c (a current Radeon): catch-all card row, so the dx-level-and-vendor
	// row applies after it. AMD CPU; no memory row matches 32 GB.
	render::DxSupportPlan amd = render::ResolveRecommendedConfig( groups, Query( 0x1002, 0x744c ) );
#ifdef DXSUPPORT_SEEDED_CATCH_ALL_ORDER
	// Seeded defect: the dx-level-and-vendor row applied before the catch-all card.
	std::swap( amd.applyOrder[1], amd.applyOrder[2] );
	return testing::ReportConformance(
	    1, OrderIs( amd, { kDx95, kAmdCatchAll, kDx95Amd, kCpuAmd } ) ? 1 : 0 );
#else
	Check( OrderIs( amd, { kDx95, kAmdCatchAll, kDx95Amd, kCpuAmd } ),
	    "a vendor catch-all card applies before the dx-level-and-vendor group" );
	Check( amd.dxLevel == 95 && !amd.picmipFloor, "large video memory adds no picmip floor" );

	// NVIDIA 0x1234 falls in the narrow row, which applies last; the
	// dx-level-and-vendor row is skipped. First match wins over the catch-all.
	render::DxSupportQuery nvidiaQuery = Query( 0x10de, 0x1234 );
	nvidiaQuery.cpuIsAmd = false;
	nvidiaQuery.systemRamMegabytes = 300;
	const render::DxSupportPlan nvidia = render::ResolveRecommendedConfig( groups, nvidiaQuery );
	Check( OrderIs( nvidia, { kDx95, kNvidiaNarrow, kCpuIntel, kMemoryLarge } ),
	    "a narrow card range applies last and suppresses the dx-level-and-vendor group" );

	// NVIDIA 0x2684 only matches the catch-all.
	const render::DxSupportPlan nvidiaNew =
	    render::ResolveRecommendedConfig( groups, Query( 0x10de, 0x2684 ) );
	Check( OrderIs( nvidiaNew, { kDx95, kNvidiaCatchAll, kDx95Nvidia, kCpuAmd } ),
	    "a device outside the narrow range uses the vendor catch-all and vendor dx row" );

	// Unknown vendor (Intel GPU 0x8086 not listed): only the dx-level row.
	render::DxSupportQuery unknown = Query( 0x8086, 0x56a0 );
	unknown.dxLevel = 90;
	const render::DxSupportPlan intel = render::ResolveRecommendedConfig( groups, unknown );
	Check(
	    OrderIs( intel, { kDx90, kCpuAmd } ), "an unlisted card gets the dx-level and CPU rows" );

	// Small video memory adds a picmip floor at the maximum level, and under
	// 100 MB at any level; not otherwise.
	render::DxSupportQuery smallVram = Query( 0x1002, 0x744c );
	smallVram.videoMemoryBytes = 64 * 1024 * 1024;
	const render::DxSupportPlan smallPlan = render::ResolveRecommendedConfig( groups, smallVram );
	Check( smallPlan.picmipFloor && *smallPlan.picmipFloor == 2,
	    "video memory under 128 MB at the maximum level floors picmip" );
	smallVram.dxLevel = 90;
	Check( render::ResolveRecommendedConfig( groups, smallVram ).picmipFloor.has_value(),
	    "video memory under 100 MB floors picmip below the maximum level too" );
	smallVram.videoMemoryBytes = 120 * 1024 * 1024;
	Check( !render::ResolveRecommendedConfig( groups, smallVram ).picmipFloor,
	    "100-128 MB below the maximum level adds no floor" );
	smallVram.videoMemoryBytes = 0;
	smallVram.dxLevel = 95;
	Check( !render::ResolveRecommendedConfig( groups, smallVram ).picmipFloor,
	    "unknown video memory adds no floor" );

	render::DxSupportQuery tinyRam = Query( 0x1002, 0x744c );
	tinyRam.systemRamMegabytes = 250;
	Check( OrderIs( render::ResolveRecommendedConfig( groups, tinyRam ),
	           { kDx95, kAmdCatchAll, kDx95Amd, kCpuAmd } ),
	    "memory ranges are half-open (250 is outside [0, 250))" );

	// The device-group order alone is what hardware caps apply (ReadHardwareCaps).
	Check( render::ResolveDeviceGroupOrder( groups, 95, 0x1002, 0x744c ) ==
	               std::vector<size_t>{ kDx95, kAmdCatchAll, kDx95Amd } &&
	           render::ResolveDeviceGroupOrder( groups, 95, 0x10de, 0x1234 ) ==
	               std::vector<size_t>{ kDx95, kNvidiaNarrow },
	    "device group order is the recommended configuration's leading groups" );
	Check(
	    render::FindCardGroup( groups, 0x10de, 0x1234 ) == std::optional<size_t>( kNvidiaNarrow ) &&
	        render::FindCardGroup( groups, 0x10de, 0x2684 ) ==
	            std::optional<size_t>( kNvidiaCatchAll ) &&
	        !render::FindCardGroup( groups, 0x8086, 0x56a0 ),
	    "the card group is the first vendor match whose range holds the device" );

	Check( render::ClosestActualDxLevel( 95, false, 80 ) == 95 &&
	           render::ClosestActualDxLevel( 94, false, 80 ) == 90 &&
	           render::ClosestActualDxLevel( 98, false, 80 ) == 95 &&
	           render::ClosestActualDxLevel( 95, true, 80 ) == 92 &&
	           render::ClosestActualDxLevel( 70, false, 80 ) == 80,
	    "requested levels map to the actual levels a device manager runs" );
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
