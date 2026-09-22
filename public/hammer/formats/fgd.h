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
//=============================================================================//

#ifndef HAMMER_FORMATS_FGD_H
#define HAMMER_FORMATS_FGD_H

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
	Other,
};

// One entry in a choices/flags enumeration: the stored value and its label.
struct FgdChoice
{
	std::string value;
	std::string label;
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
};

// One entity class declared in the FGD.
struct EntityClass
{
	EntityKind kind = EntityKind::Other;
	std::string name;                    // classname (after '=')
	std::string description;             // trailing ": <desc>"
	std::vector<std::string> bases;      // base(...) class names, in order
	std::vector<FgdProperty> properties; // this class's OWN properties (not merged)
};

struct FgdParseResult
{
	bool ok = false;
	std::vector<EntityClass> classes;
	std::string error;         // diagnostic when !ok
	std::size_t errorLine = 0; // 1-based line of the error
};

// Parses FGD text into a class table. Unknown top-level directives (@mapsize,
// @include, @MaterialExclusion, @AutoVisGroup, ...) are skipped, not errors.
// Returns ok == false with a diagnostic on malformed class syntax.
FgdParseResult ParseFgd( const std::string &text );

// Returns 'name' with its base-class properties merged in: base properties first
// (recursively, in base() order), then the class's own, with a later property of
// the same key overriding an earlier one. Returns nullopt if 'name' is unknown.
// Cyclic base references are broken safely (each class visited once).
std::optional<EntityClass> ResolveClass(
    const std::vector<EntityClass> &classes, const std::string &name );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_FGD_H
