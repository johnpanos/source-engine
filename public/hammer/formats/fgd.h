//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless FGD (Forge Game Data) entity-schema parser for the Hammer
//			editor (RFC 0002, hammer.formats). An FGD declares the entity classes a
//			game exposes -- their kind (point/solid/base), key/value properties
//			with types and defaults, choices/flags enumerations, and base-class
//			inheritance -- which is what drives the editor's entity property sheet
//			("FGD-driven entity editing"). This is the strict, MFC-free, GPU-free
//			schema core: text in, a queryable class table out. It reads schema; it
//			holds no UI and edits no map.
//
//			Scope of this slice: @PointClass / @SolidClass / @BaseClass /
//			@KeyValueClass / @NPCClass / @FilterClass / @MoveClass declarations,
//			their `base(...)` inheritance, typed key properties (string, integer,
//			float, choices, flags, target_destination, angle, ...), default values,
//			and choices/flags value lists. `ResolveClass` flattens inheritance.
//
//			Also kept (R08-DOMAIN): @KeyFrameClass declarations, every header
//			helper other than base() (size, color, studio, iconsprite, sphere,
//			line, ...) with its comma-separated arguments, input/output
//			declarations, the flags' default field, the `readonly` modifier and
//			@include directives. Includes are recorded, not loaded; the entity
//			catalog (fgd_entity_catalog.h) resolves them through its loader.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_FGD_H
#define HAMMER_FORMATS_FGD_H

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace hammer::formats
{

enum class EntityKind
{
	Point,
	Solid,
	Base,
	KeyValue,
	Npc,
	Filter,
	Move,
	KeyFrame,
	Other,
};

// One entry in a choices/flags enumeration: the stored value and its label.
struct FgdChoice
{
	std::string value;
	std::string label;
	std::string defaultValue; // flags only: the third ": <0|1>" field (empty if absent)
};

// One key/value property a class exposes to the editor.
struct FgdProperty
{
	std::string name;               // the VMF key this sets
	std::string type;               // "string", "integer", "float", "choices", "flags", ...
	std::string displayName;        // editor label (empty if unspecified)
	std::string defaultValue;       // default (empty if unspecified)
	std::string help;               // help text (empty if unspecified)
	std::vector<FgdChoice> choices; // for "choices"/"flags"; empty otherwise
	bool readOnly = false;          // the `readonly` modifier
};

// One class-header helper other than base(): its lower-cased name and its
// arguments split at commas, each argument's words joined by one space and
// quoted strings unquoted. size(-8 -8 -8, 8 8 8) -> { "-8 -8 -8", "8 8 8" };
// studio("models/x.mdl") -> { "models/x.mdl" }; sphere() -> {}. A bare helper
// word without parentheses (e.g. halfgridsnap) has no arguments.
struct FgdHelper
{
	std::string name;
	std::vector<std::string> args;
};

// One input or output declaration: `input Name(type) : "help"`.
struct FgdIo
{
	std::string name;
	std::string type; // "void", "integer", "float", "string", "bool", ... (empty if none)
	std::string help;
};

// One entity class declared in the FGD.
struct EntityClass
{
	EntityKind kind = EntityKind::Other;
	std::string name;                    // classname (after '=')
	std::string description;             // trailing ": <desc>"
	std::vector<std::string> bases;      // base(...) class names, in order
	std::vector<FgdProperty> properties; // this class's OWN properties (not merged)
	std::vector<FgdHelper> helpers;      // header helpers except base(), in order
	std::vector<FgdIo> inputs;           // own input declarations, in order
	std::vector<FgdIo> outputs;          // own output declarations, in order
	std::size_t line = 0;                // 1-based line of the @-class directive
};

struct FgdParseResult
{
	bool ok = false;
	std::vector<EntityClass> classes;
	std::string error;         // diagnostic when !ok
	std::size_t errorLine = 0; // 1-based line of the error
	// @include "file" directives in source order, and the 1-based line of each
	// (parallel to 'includes').
	std::vector<std::string> includes;
	std::vector<std::size_t> includeLines;
};

// Parses FGD text into a class table. @include directives are recorded in
// 'includes'; other unknown top-level directives (@mapsize, @MaterialExclusion,
// @AutoVisGroup, ...) are skipped, not errors.
// Returns ok == false with a diagnostic on malformed class syntax.
FgdParseResult ParseFgd( const std::string &text );

// Returns 'name' with its base-class properties merged in: base properties first
// (recursively, in base() order), then the class's own, with a later property of
// the same key overriding an earlier one in place. Inputs and outputs merge the
// same way by name; helpers too, except that a class's own helpers replace every
// inherited helper of that name (the group takes the first inherited one's
// place), so repeated helpers such as two sphere()s survive. Names of classes,
// keys, I/O and helpers compare case-insensitively (an exact class-name match is
// preferred). Returns nullopt if 'name' is unknown. Cyclic base references are
// broken safely (each class visited once); an unknown base contributes nothing.
std::optional<EntityClass> ResolveClass(
    const std::vector<EntityClass> &classes, const std::string &name );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_FGD_H
