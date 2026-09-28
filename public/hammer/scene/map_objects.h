//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editable map objects of a Hammer document (RFC 0002,
//			hammer.scene): solids and their sides, entities and their output
//			connections, and groups, each with its editor metadata. These are
//			plain values: an object is copied into a change set before an edit
//			and restored by undo, so every field that must survive a load/save
//			or an undo is a member here, including data the editor does not
//			interpret (kept verbatim in 'extra' fields).
//
//			Identity. ObjectId is the runtime identity inside one document: its
//			high 32 bits are the document serial and it is never reused within
//			that document, so a stale or foreign id resolves to nothing. The VMF
//			'id' numbers (vmfId) are persistent identifiers, kept separately
//			because other content refers to them (overlay "sides" keys name side
//			ids). The two are never interchangeable.
//
//			Authority. A side's plane is its three authored points (exactly as a
//			VMF stores them); Plane() derives the half-space. An entity's origin,
//			angles and every other key live in its ordered key list; the typed
//			accessors parse and format through that one list.
//
//			No escape hatches: every piece of map state is a typed field here or
//			in map_document.h. There is no verbatim block, no "extra" key list
//			and no opaque node, so a codec either models what it reads or
//			refuses it; nothing is carried without meaning. (Entity and world
//			key/value lists are the entity data model itself: Source entities
//			are defined by their keys, which the class schema types.)
//
//=============================================================================//

#ifndef HAMMER_SCENE_MAP_OBJECTS_H
#define HAMMER_SCENE_MAP_OBJECTS_H

#include "foundation/strong_id.h"
#include "kvtext/keyvalues.h" // kvtext::KeyValue: an ordered entity key
#include "mapgeometry/brush.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::scene
{

struct ObjectIdTag;
using ObjectId = foundation::StrongId<ObjectIdTag, std::uint64_t>;

// The document serial an id was issued by (its high 32 bits).
constexpr std::uint32_t DocumentSerialOf( ObjectId id )
{
	return static_cast<std::uint32_t>( id.value >> 32 );
}

// One face of a solid, addressed by the solid's runtime id and the side's
// persistent VMF id (unique within a document; see MapDocument).
struct FaceRef
{
	ObjectId solid;
	std::uint32_t side = 0;

	friend bool operator==( const FaceRef &, const FaceRef & ) = default;
	friend auto operator<=>( const FaceRef &, const FaceRef & ) = default;
};

enum class ObjectKind
{
	Solid,
	Entity,
	Group,
};

// A texture axis: world-space direction, texel shift and world units per texel.
struct TextureAxis
{
	mapgeometry::Vec3d axis;
	double shift = 0.0;
	double scale = 0.25;

	friend bool operator==( const TextureAxis &, const TextureAxis & ) = default;
};

struct FaceTexture
{
	std::string material;
	TextureAxis u;
	TextureAxis v;
	double rotation = 0.0;
	double lightmapScale = 16.0;
	std::uint32_t smoothingGroups = 0;

	friend bool operator==( const FaceTexture &, const FaceTexture & ) = default;
};

struct Rgb
{
	int r = 0;
	int g = 0;
	int b = 0;

	friend bool operator==( const Rgb &, const Rgb & ) = default;
};

// The VMF 'editor' block. Group membership is not here: it is the object's
// 'group' field (a runtime id), which the codec maps to and from 'groupid'.
struct EditorInfo
{
	std::optional<Rgb> color;
	std::vector<int> visgroupIds; // persistent visgroup ids, in authored order
	bool visgroupShown = true;
	bool visgroupAutoShown = true;
	// Position in the logical (entity I/O) view, "[x y]"; absent when unset.
	std::optional<std::array<int, 2>> logicalPos;
	// The object's free-text editor comment; absent when unset.
	std::optional<std::string> comments;

	friend bool operator==( const EditorInfo &, const EditorInfo & ) = default;
};

// A displaced face (the VMF 'dispinfo' block). Per-vertex arrays are row-major
// over a (2^power + 1)^2 grid; triangle tags have two per quad over a
// (2^power)^2 grid. Each array is absent when the file omits its block, so an
// authored file round-trips exactly. The pre-release keys (mintess, smooth,
// alpha) that legacy Hammer still reads are modeled too.
struct Displacement
{
	int power = 2;
	mapgeometry::Vec3d startPosition;
	int flags = 0;
	double elevation = 0.0;
	bool subdivided = false;
	std::optional<int> minTess;
	std::optional<double> smoothingAngle;
	std::optional<std::array<double, 4>> cornerAlphas;

	std::optional<std::vector<mapgeometry::Vec3d>> normals;
	std::optional<std::vector<double>> distances;
	std::optional<std::vector<mapgeometry::Vec3d>> offsets;
	std::optional<std::vector<mapgeometry::Vec3d>> offsetNormals;
	std::optional<std::vector<double>> alphas;
	std::optional<std::vector<int>> triangleTags;
	std::optional<std::vector<std::int64_t>> allowedVerts; // the "allowed_verts" words

	// Vertices per row ((2^power) + 1) and quads per row (2^power).
	int VertsPerRow() const { return ( 1 << power ) + 1; }
	int QuadsPerRow() const { return 1 << power; }

	friend bool operator==( const Displacement &, const Displacement & ) = default;
};

struct Side
{
	std::uint32_t vmfId = 0;
	std::array<mapgeometry::Vec3d, 3> points; // the authored plane points
	FaceTexture texture;
	std::optional<Displacement> displacement;

	// The half-space of 'points' (VMF winding: normal = (p0-p1) x (p2-p1)
	// normalized, pointing out of the solid). A degenerate triple gives a zero
	// normal.
	mapgeometry::Plane Plane() const;

	friend bool operator==( const Side &, const Side & ) = default;
};

struct Solid
{
	ObjectId id;
	std::uint32_t vmfId = 0;
	ObjectId owner;      // the brush entity that owns this solid; invalid = world
	ObjectId group;      // the group this solid belongs to; invalid = none
	bool hidden = false; // quick-hidden (the VMF 'hidden' wrapper)
	std::vector<Side> sides;
	EditorInfo editor;

	const Side *FindSide( std::uint32_t sideVmfId ) const;
	Side *FindSide( std::uint32_t sideVmfId );

	friend bool operator==( const Solid &, const Solid & ) = default;
};

// One entity output: "<output>" "<target>,<input>,<parameter>,<delay>,<times>".
// 'separator' is the field separator the VMF used (',' or the newer 0x1B).
struct Connection
{
	std::string output;
	std::string target;
	std::string input;
	std::string parameter;
	double delay = 0.0;
	int timesToFire = -1; // -1 = unlimited
	char separator = ',';

	friend bool operator==( const Connection &, const Connection & ) = default;
};

// Parses a connection value; nothing when it has fewer than five fields or a
// non-numeric delay/count.
std::optional<Connection> ParseConnection( std::string_view output, std::string_view value );
// The VMF value text for a connection (the output name is the key).
std::string FormatConnectionValue( const Connection &connection );

struct Entity
{
	ObjectId id;
	std::uint32_t vmfId = 0;
	ObjectId group;
	bool hidden = false;
	std::string classname;
	std::vector<kvtext::KeyValue> keys; // every key except "id" and "classname"
	std::vector<Connection> connections;
	EditorInfo editor;

	// The first value of 'key', or nullptr.
	const std::string *Key( std::string_view key ) const;
	// Sets the first 'key' (appending it when absent). Returns whether it changed.
	bool SetKey( std::string_view key, std::string_view value );
	// Removes every 'key'. Returns whether one was present.
	bool RemoveKey( std::string_view key );

	// "origin" parsed as three numbers; nothing when absent or malformed.
	std::optional<mapgeometry::Vec3d> Origin() const;
	void SetOrigin( const mapgeometry::Vec3d &origin );
	// "angles" as (pitch, yaw, roll); nothing when absent or malformed.
	std::optional<mapgeometry::Vec3d> Angles() const;
	void SetAngles( const mapgeometry::Vec3d &pitchYawRoll );
	// "targetname", or empty.
	std::string_view Name() const;

	friend bool operator==( const Entity &, const Entity & ) = default;
};

struct Group
{
	ObjectId id;
	std::uint32_t vmfId = 0;
	ObjectId group; // enclosing group; invalid = top level
	bool hidden = false;
	EditorInfo editor;

	friend bool operator==( const Group &, const Group & ) = default;
};

// Formats a number the way the VMF writer does ("%.10g"; integers without a
// point, negative zero as "0").
std::string FormatNumber( double value );
// "x y z" with FormatNumber components.
std::string FormatVec3( const mapgeometry::Vec3d &v );

} // namespace hammer::scene

#endif // HAMMER_SCENE_MAP_OBJECTS_H
