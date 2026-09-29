//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Presentation policies picking and extraction share (RFC 0002,
//			hammer.viewport), each with one owner so a click and a drawn frame
//			never disagree about what an object looks like:
//
//			Visibility. The default is scene::IsVisible (not quick-hidden,
//			visgroups shown, every container visible). A caller may substitute
//			a predicate; it must be a pure function of the document content
//			(SnapshotCache relies on this for incremental updates).
//
//			Entity markers. A point entity is drawn and picked as the box its
//			class declares (catalog boxMins/boxMaxs, relative to the origin,
//			unrotated as in legacy Hammer), else +/- pointHalfSize. An entity
//			without a parsable origin has no marker (as scene::ObjectBounds).
//			A brush entity (one that owns solids) is represented by its solids.
//
//			Colors. Solids: the solid's editor color; else, for a brush-entity
//			solid, the owning class's catalog color, else kDefaultEntityColor;
//			else kDefaultWorldColor. Entities: the catalog class color (the
//			FGD color() is the class's display identity; the VMF writer stores
//			a constant editor color for entities), else the entity's editor
//			color, else kDefaultEntityColor. Catalog components are rounded
//			and clamped to 0..255.
//
//			Edges. A solid's wireframe is its faces' polygon edges with shared
//			edges listed once (endpoints equal within kEdgeEpsilon, in either
//			direction).
//
//			Models. A point entity whose "model" (or catalog studio model)
//			names a ".mdl" file (case-insensitive) is drawn as that model
//			(IsStudioModelPath); "skin" (an integer, else 0), "modelscale" (a
//			positive number, else 1) and "rendercolor" ("r g b", 0..255,
//			else none) are read as values (ModelKeys).
//
//			Instances. A func_instance's content draws with the instance
//			tint: legacy Hammer overlays instance content with (128, 128, 0)
//			at alpha 192 (hammer/Render.h InstanceColor), which hides the
//			textures; the tint keeps that hue as a multiply,
//			kInstanceTint (255, 255, 128), so the content stays readable. Its
//			2D edges use kInstanceEdgeColor, the legacy (128, 128, 0). A
//			selected instance draws its content selected instead.
//
//=============================================================================//

#ifndef HAMMER_VIEWPORT_VIEW_POLICY_H
#define HAMMER_VIEWPORT_VIEW_POLICY_H

#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/map_document.h"
#include "hammer/scene/solid_geometry.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace hammer::viewport
{

// Returns whether an object is shown and pickable. Empty = scene::IsVisible.
using VisibilityPredicate = std::function<bool( const scene::DocumentReader &, scene::ObjectId )>;

// Applies 'predicate' (scene::IsVisible when empty).
bool IsShown(
    const VisibilityPredicate &predicate, const scene::DocumentReader &doc, scene::ObjectId id );

// A predicate that shows every object (hidden ones included).
VisibilityPredicate ShowEverything();

// Legacy fallback marker half-size (scene::ObjectBounds' default).
constexpr double kDefaultPointHalfSize = 8.0;

// The world-space marker box of a point entity; nothing without an origin.
std::optional<scene::Box> EntityMarkerBox( const scene::Entity &entity,
    const ports::IEntityCatalog *catalog, double pointHalfSize = kDefaultPointHalfSize );

// Legacy defaults: world brushes get a random (0, 100..255, 100..255) color in
// legacy Hammer; the deterministic midpoint stands in for it. Entities without
// a class color are legacy magenta.
constexpr scene::Rgb kDefaultWorldColor{ 0, 178, 178 };
constexpr scene::Rgb kDefaultEntityColor{ 220, 30, 220 };

constexpr scene::Rgb kInstanceTint{ 255, 255, 128 };
constexpr scene::Rgb kInstanceEdgeColor{ 128, 128, 0 };

// Whether 'path' names a studio model (ends in ".mdl", any case).
bool IsStudioModelPath( std::string_view path );

struct ModelKeys
{
	std::int32_t skin = 0;
	double scale = 1.0;
	std::optional<scene::Rgb> renderColor;
	// The sequence the model is drawn at (its first frame; RFC 0002 R17
	// follow-up, posed models): the label "DefaultAnim" names when it is set
	// (as prop_dynamic starts it), else the "sequence" key (an index, or a
	// label), else sequence 0. Keys match without regard to case, as the
	// engine's do. A label the model lacks falls back to the index, as the
	// engine keeps the current sequence when DefaultAnim names none.
	std::string sequence = {};      // a label; empty: none
	std::int32_t sequenceIndex = 0; // when no label names one

	friend bool operator==( const ModelKeys &, const ModelKeys & ) = default;
};
ModelKeys ReadModelKeys( const scene::Entity &entity );

// The catalog class color of 'classname', if the catalog declares one.
std::optional<scene::Rgb> CatalogColor(
    const ports::IEntityCatalog *catalog, std::string_view classname );

scene::Rgb SolidColor( const scene::DocumentReader &doc, const scene::Solid &solid,
    const ports::IEntityCatalog *catalog );
scene::Rgb EntityColor( const scene::Entity &entity, const ports::IEntityCatalog *catalog );

constexpr double kEdgeEpsilon = 1.0e-6;

struct WorldEdge
{
	mapgeometry::Vec3d a;
	mapgeometry::Vec3d b;
};

// Appends the edges of a closed polygon (consecutive vertices, last to first)
// that 'edges' does not already hold in either direction. Zero-length edges
// are skipped.
void AppendUniqueEdges(
    std::vector<WorldEdge> &edges, const std::vector<mapgeometry::Vec3d> &polygon );

// The unique edges of every face of 'solid'.
std::vector<WorldEdge> UniqueEdges( const mapgeometry::BrushSolid &solid );

} // namespace hammer::viewport

#endif // HAMMER_VIEWPORT_VIEW_POLICY_H
