//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the devices a family pixel suite draws its cases on; see
//			family_devices.h.
//
//=============================================================================//

#include "family_devices.h"

#include "kvtext/keyvalues.h"

#if defined( RENDERTEST_FAMILY_GL ) || defined( RENDERTEST_FAMILY_CROSS )
#include "render/device/gl/provider.h"
#endif
#if !defined( RENDERTEST_FAMILY_GL )
#include "render/device/vulkan/provider.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace rendertest::families
{

namespace
{

const char *const kCrossFixture = "quality/fixtures/render-families/cross-backend-v1.vdf";

} // namespace

CaseDevices::CaseDevices( testing::Checks &checks )
{
#if !defined( RENDERTEST_FAMILY_GL )
	{
		namespace vulkan = render::device::vulkan;
		m_VulkanLayer = vulkan::ValidationLayerAvailable();
		vulkan::VulkanAdapterOptions options;
		options.validation = m_VulkanLayer;
		options.validationCounter = &m_VulkanMessages;
		if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
			options.adapterIndex = std::atoi( adapter );
		auto created = vulkan::Create( options );
		if ( checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			entries.push_back( { "vulkan", std::move( created ).Value() } );
	}
#endif
#if defined( RENDERTEST_FAMILY_GL ) || defined( RENDERTEST_FAMILY_CROSS )
	{
		namespace gl = render::device::gl;
		gl::GlAdapterOptions options;
		options.validation = true;
		options.validationCounter = &m_GlMessages;
#if defined( RENDERTEST_FAMILY_SEEDED_GL_LOWER_LEFT )
		// Sensitivity row: a GL adapter that keeps GL's own lower-left
		// origin draws every case upside down against Vulkan.
		options.sensitivity.lowerLeftOrigin = true;
#endif
		auto created = gl::Create( options );
		if ( checks.That( created.HasValue(), "device.a-gl-device-is-created" ) )
		{
			m_GlDebug = true;
			entries.push_back( { "gl", std::move( created ).Value() } );
		}
	}
#endif
#if defined( RENDERTEST_FAMILY_CROSS )
	std::ifstream file( kCrossFixture, std::ios::binary );
	std::stringstream text;
	text << file.rdbuf();
	const kvtext::ParseResult fixture = kvtext::ParseKeyValues( text.str() );
	if ( checks.That( fixture.ok && !fixture.root.children.empty(), "setup.cross-fixture-parses" ) )
	{
		for ( const kvtext::KeyValueNode &node : fixture.root.children[0].children )
		{
			const std::string *name = node.Find( "name" );
			const std::string *limit = node.Find( "limit" );
			const std::string *outliers = node.Find( "outliers" );
			if ( node.name == "case" && name && limit )
				m_Limits[*name] = { std::atoi( limit->c_str() ),
				    outliers ? std::size_t( std::atoi( outliers->c_str() ) ) : 0 };
		}
	}
#endif
}

CaseDevices::~CaseDevices() = default;

void CaseDevices::Compare(
    testing::Checks &checks, const Entry &entry, const std::string &caseName, const Drawn &drawn )
{
	if ( entries.size() < 2 )
		return;
	if ( &entry == &entries.front() )
	{
		m_Reference[caseName] = drawn;
		return;
	}
	const auto reference = m_Reference.find( caseName );
	if ( !checks.That( reference != m_Reference.end(),
	         "pixels." + caseName + "." + entry.name + "-has-a-reference-frame" ) )
		return;
	const auto limit = m_Limits.find( caseName );
	if ( !That( checks, limit != m_Limits.end(),
	         "fixture." + caseName + ".records-a-cross-backend-limit",
	         std::string( "add it to " ) + kCrossFixture ) )
		return;
	// Every pixel of the frame, not only the port's samples.
	int worst = 0;
	std::size_t over = 0;
	std::string worstAt;
	const std::vector<std::uint8_t> &a = reference->second.rgba;
	const std::vector<std::uint8_t> &b = drawn.rgba;
	if ( !checks.That(
	         a.size() == b.size() && !a.empty(), "pixels." + caseName + ".frames-match-in-size" ) )
		return;
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		const int difference = std::abs( int( a[i] ) - int( b[i] ) );
		over += difference > limit->second.levels ? 1 : 0;
		if ( difference > worst )
		{
			worst = difference;
			const std::size_t texel = i / 4;
			worstAt = std::to_string( texel % kSize ) + "," + std::to_string( texel / kSize ) +
			          " channel " + std::to_string( i % 4 ) + ": vulkan " + std::to_string( a[i] ) +
			          ", " + entry.name + " " + std::to_string( b[i] );
		}
	}
	std::printf( "INFO %s: %s against %s, worst difference %d, %zu channels over %d (allowed "
	             "%zu)\n",
	    caseName.c_str(), entry.name.c_str(), entries.front().name.c_str(), worst, over,
	    limit->second.levels, limit->second.outliers );
	const bool within = over <= limit->second.outliers;
	++m_Compared;
	// RENDER_FAMILY_DUMP_DIR: both frames of a case over its limit, for
	// diagnosis, as <case>.<device>.ppm.
	const char *directory = std::getenv( "RENDER_FAMILY_DUMP_DIR" );
	const Drawn *const frames[] = { &reference->second, &drawn };
	for ( const Drawn *frame : frames )
	{
		if ( !directory || within )
			break;
		const std::string &device = frame == &drawn ? entry.name : entries.front().name;
		std::ofstream file(
		    std::string( directory ) + "/" + caseName + "." + device + ".ppm", std::ios::binary );
		file << "P6\n" << kSize << " " << kSize << "\n255\n";
		for ( std::size_t i = 0; i < std::size_t( kSize ) * kSize; ++i )
			file.write( reinterpret_cast<const char *>( &frame->rgba[i * 4] ), 3 );
	}
	That( checks, within,
	    "pixels." + caseName + "." + entry.name + "-within-cross-backend-tolerance-of-" +
	        entries.front().name,
	    worstAt + " (" + std::to_string( over ) + " channels over)" );
}

int CaseDevices::Finish( testing::Checks &checks )
{
	for ( Entry &entry : entries )
		(void)entry.device->WaitIdle();
	entries.clear();
#if !defined( RENDERTEST_FAMILY_GL )
	if ( m_VulkanLayer )
		checks.Equal( m_VulkanMessages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
#endif
	if ( m_GlDebug )
		checks.Equal( m_GlMessages.load(), std::uint64_t( 0 ), "gl.debug-output.no-messages" );
#if defined( RENDERTEST_FAMILY_CROSS )
	checks.That( m_Compared > 0 && std::size_t( m_Compared ) == m_Reference.size(),
	    "cross.every-reference-case-was-compared" );
#endif
	return checks.Report();
}

} // namespace rendertest::families
