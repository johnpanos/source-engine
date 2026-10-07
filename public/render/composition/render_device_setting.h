//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The saved render device setting (RFC 0016 K10, RFC 0022): the one
//			owner of its file, its names and its text form.
//
//			The Video options write the device the player chose to
//			<game>/cfg/render_device.txt; the launcher reads it before it
//			composes the render core, so a change takes effect at the next
//			launch (switching the device in-process is R97). -render-device
//			on the command line overrides it, and with neither the product
//			profile's default applies. The file holds one line, the device's
//			name; anything else is ignored and the default applies.
//
//			Header-only and C++11 so the launcher, GameUI and tests share it.
//			It uses no C string functions: Source's game code redefines them
//			(tier1/strtools.h).
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_DEVICE_SETTING_H
#define RENDER_COMPOSITION_RENDER_DEVICE_SETTING_H

#include <cstddef>

namespace render_device_setting
{

// Relative to the game directory (the MOD search path's write location).
static const char *const kFile = "cfg/render_device.txt";

struct Choice
{
	const char *name;  // RenderCoreConfig::device
	const char *label; // shown in the Video options
};

// In menu order.
static const Choice kChoices[] = {
    { "vulkan", "Vulkan" },
    { "gl", "OpenGL 4.5" },
    { "gles", "OpenGL ES 3.1" },
};
static const int kChoiceCount = int( sizeof( kChoices ) / sizeof( kChoices[0] ) );

// Whether the length bytes at text are exactly name.
inline bool Is( const char *name, const char *text, std::size_t length )
{
	std::size_t i = 0;
	for ( ; i < length; ++i )
	{
		if ( name[i] != text[i] || !name[i] )
			return false;
	}
	return name[i] == '\0';
}

inline bool IsSpace( char c )
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

// The index of the device named by length bytes at text, or -1.
inline int Find( const char *text, std::size_t length )
{
	for ( int i = 0; text && i < kChoiceCount; ++i )
	{
		if ( Is( kChoices[i].name, text, length ) )
			return i;
	}
	return -1;
}

// The index of a device name in kChoices, or -1.
inline int Find( const char *name )
{
	std::size_t length = 0;
	while ( name && name[length] )
		++length;
	return Find( name, length );
}

// The device named by the file's text (surrounding whitespace ignored), or
// NULL when the text names none of kChoices.
inline const char *Parse( const char *text, std::size_t length )
{
	std::size_t begin = 0;
	while ( begin < length && IsSpace( text[begin] ) )
		++begin;
	std::size_t end = begin;
	while ( end < length && !IsSpace( text[end] ) && text[end] )
		++end;
	const int index = end == begin ? -1 : Find( text + begin, end - begin );
	return index < 0 ? NULL : kChoices[index].name;
}

} // namespace render_device_setting

#endif // RENDER_COMPOSITION_RENDER_DEVICE_SETTING_H
