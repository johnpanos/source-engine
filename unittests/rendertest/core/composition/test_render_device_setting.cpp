//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device-setting: the saved render device's text form
//			(public/render/composition/render_device_setting.h), which the
//			Video options write and the launcher reads before composing the
//			render core. Every listed device parses; whitespace around it is
//			ignored; an empty, unknown, partial or overlong name names none,
//			so the launcher keeps its default.
//
//=============================================================================//

#include "render/composition/render_device_setting.h"
#include "testing/checks.h"

#include <cstring>
#include <string>

namespace
{

const char *Parse( const std::string &text )
{
	return render_device_setting::Parse( text.data(), text.size() );
}

bool Names( const std::string &text, const char *device )
{
	const char *parsed = Parse( text );
	return parsed && device && !std::strcmp( parsed, device );
}

} // namespace

int main()
{
	testing::Checks checks;
	using render_device_setting::kChoiceCount;
	using render_device_setting::kChoices;

	checks.Equal( kChoiceCount, 3, "choices.vulkan-gl-and-gles" );
	for ( int i = 0; i < kChoiceCount; ++i )
	{
		const std::string name = kChoices[i].name;
		checks.Equal( render_device_setting::Find( kChoices[i].name ), i, "find." + name );
		checks.That( Names( name, kChoices[i].name ), "parse." + name );
		checks.That( Names( name + "\n", kChoices[i].name ), "parse." + name + ".newline" );
		checks.That(
		    Names( "  \t" + name + " \r\n", kChoices[i].name ), "parse." + name + ".whitespace" );
		checks.That(
		    Names( name + "\nignored", kChoices[i].name ), "parse." + name + ".first-line" );
	}
	checks.That( Parse( "" ) == nullptr, "parse.empty-names-none" );
	checks.That( Parse( " \n" ) == nullptr, "parse.blank-names-none" );
	checks.That( Parse( "null" ) == nullptr, "parse.null-is-not-a-menu-choice" );
	checks.That( Parse( "d3d9" ) == nullptr, "parse.unknown-names-none" );
	checks.That( Parse( "g" ) == nullptr, "parse.prefix-names-none" );
	checks.That( Parse( "glesx" ) == nullptr, "parse.longer-name-names-none" );
	checks.That( Parse( "Vulkan" ) == nullptr, "parse.label-is-not-a-name" );
	checks.That( Parse( std::string( 64, 'v' ) ) == nullptr, "parse.overlong-names-none" );
	checks.That( Parse( std::string( "gl\0es", 5 ) ) &&
	                 !std::strcmp( Parse( std::string( "gl\0es", 5 ) ), "gl" ),
	    "parse.stops-at-nul" );
	checks.Equal( render_device_setting::Find( nullptr ), -1, "find.null-pointer" );
	checks.That( std::strcmp( render_device_setting::kFile, "cfg/render_device.txt" ) == 0,
	    "file.under-the-game-cfg" );
	return checks.Report();
}
