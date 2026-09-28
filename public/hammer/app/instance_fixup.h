//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The func_instance name and parameter rules (RFC 0002, hammer.app),
//			as small pure functions on strings, so every consumer that merges
//			instance content (collapse in the editor, a compile-path expansion,
//			a preview) applies the one rule. Taken from the compiler that
//			defines what an instance means (utils/vbsp/map.cpp MergeEntities
//			and ReplaceInstancePair; fgdlib GameData::RemapNameField).
//
//			Fixup style ("fixup_style" key): 0 prefix, 1 postfix, 2 none. The
//			fixup text is the func_instance's "targetname", else its "name"
//			key, else an automatic name. A prefixed name is "<fixup>-<name>",
//			a postfixed one "<name>-<fixup>". Empty names and global names
//			are never fixed up: a name starting with '@' (vbsp's rule) or '!'
//			(the engine's special names such as !activator and !player; this
//			tree's vbsp prefixes them, which breaks them, so they are kept as
//			a deliberate correction matching later Valve compilers).
//
//			Parameters: every func_instance key whose name starts with
//			"replace" (case-insensitive; replace01..replace10 in the FGD)
//			holds "<variable> <value>", split at the first space; a value
//			without a space is ignored. Substitution applies the parameters in
//			key order, each replacing every case-insensitive occurrence of its
//			variable text (V_StrSubst): plain substring replacement, so "$a"
//			also rewrites the start of "$ab" (legacy behavior, kept).
//
//=============================================================================//

#ifndef HAMMER_APP_INSTANCE_FIXUP_H
#define HAMMER_APP_INSTANCE_FIXUP_H

#include "kvtext/keyvalues.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::app
{

enum class InstanceFixupStyle
{
	Prefix = 0,
	Postfix = 1,
	None = 2,
};

// The style a "fixup_style" value names: empty (the key's absence, vbsp's
// IntForKey 0) is Prefix; "0", "1" and "2" map to the styles; anything else
// is nothing.
std::optional<InstanceFixupStyle> ParseFixupStyle( std::string_view value );

// True for names the fixup keeps: empty, '@'-prefixed and '!'-prefixed.
bool IsGlobalInstanceName( std::string_view name );

// 'name' after the fixup (vbsp RemapNameField).
std::string FixupInstanceName(
    std::string_view name, std::string_view fixup, InstanceFixupStyle style );

struct InstanceParameter
{
	std::string variable; // as authored, including its '$'
	std::string value;

	friend bool operator==( const InstanceParameter &, const InstanceParameter & ) = default;
};

// The parameters declared by a func_instance's keys, in key order.
std::vector<InstanceParameter> InstanceParameters( const std::vector<kvtext::KeyValue> &keys );

// 'text' with every parameter substituted, in order.
std::string SubstituteInstanceParameters(
    std::string_view text, const std::vector<InstanceParameter> &parameters );

} // namespace hammer::app

#endif // HAMMER_APP_INSTANCE_FIXUP_H
