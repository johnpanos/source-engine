//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity class schema port (RFC 0002, hammer.ports). The catalog
//			answers what the editor knows about an entity class: its kind, its
//			resolved keys (base classes merged, own declarations winning) with
//			types, defaults and choices, its inputs and outputs, and the display
//			hints the viewport uses (box size, color, model, sprite).
//
//			hammer.formats provides the FGD-backed catalog; tests use an
//			in-memory one. Consumers: entity placement defaults and class
//			changes (hammer.app), the map check, the class palette and entity
//			inspector (hammer.presenters), entity markers (hammer.viewport).
//
//			Contract: lookups are case-insensitive on the class name, results
//			stay valid for the catalog's lifetime, and the catalog is immutable
//			after construction (safe to share across threads).
//
//=============================================================================//

#ifndef HAMMER_PORTS_ENTITY_CATALOG_H
#define HAMMER_PORTS_ENTITY_CATALOG_H

#include "mapgeometry/brush.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::ports
{

enum class EntityClassKind
{
	Point,
	Solid, // a brush entity
	Other, // filter, keyframe, move, NPC: point-like for placement
};

enum class KeyType
{
	String,
	Integer,
	Float,
	Boolean,
	Choices,
	Flags,
	TargetSource,      // the entity's own name
	TargetDestination, // names another entity
	Color255,          // "r g b [brightness]"
	Color1,            // "r g b" in 0..1
	Angle,             // "pitch yaw roll"
	Vector,            // a point or direction "x y z"
	Studio,            // model path
	Sprite,
	Sound,
	Material,
	Decal,
	Scene,
	FilterClass,
	SideList, // side ids
	Other,
};

// Maps an FGD type word ("string", "target_destination", "studio", ...) to a
// KeyType; unknown words map to Other.
KeyType KeyTypeFromName( std::string_view typeName );

struct KeyChoice
{
	std::string value;
	std::string label;
	bool defaultOn = false; // flags only: set by default
};

struct KeyDefinition
{
	std::string key;
	std::string displayName;
	std::string help;
	std::string defaultValue;
	KeyType type = KeyType::String;
	std::string typeName; // the declared FGD type word
	std::vector<KeyChoice> choices;
	bool readOnly = false;
};

struct IoDefinition
{
	std::string name;
	std::string type; // "void", "integer", "float", "string", "bool", ...
	std::string help;
};

struct EntityClassInfo
{
	std::string name;
	std::string description;
	EntityClassKind kind = EntityClassKind::Point;
	std::vector<KeyDefinition> keys; // resolved: bases first, own overriding in place
	std::vector<IoDefinition> inputs;
	std::vector<IoDefinition> outputs;
	// Display hints; absent when the schema declares none.
	std::optional<mapgeometry::Vec3d> boxMins;
	std::optional<mapgeometry::Vec3d> boxMaxs;
	std::optional<mapgeometry::Vec3d> color; // 0..255 components
	std::string model;                       // studio("...") or a studio key default
	std::string sprite;                      // iconsprite("...")

	const KeyDefinition *FindKey( std::string_view key ) const;
	bool HasInput( std::string_view name ) const;
	bool HasOutput( std::string_view name ) const;
};

class IEntityCatalog
{
public:
	virtual ~IEntityCatalog() = default;

	// The class named 'name' (case-insensitive), or nullptr.
	virtual const EntityClassInfo *Find( std::string_view name ) const = 0;

	// Every non-base class name, sorted case-insensitively.
	virtual std::vector<std::string> ClassNames() const = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_ENTITY_CATALOG_H
