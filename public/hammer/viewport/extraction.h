//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render-snapshot extraction for the Hammer viewports (RFC 0002,
//			hammer.viewport; "Rendering and host contracts"). A RenderSnapshot
//			is what a renderer or presenter consumes: stable ids, derived
//			geometry, material names, colors and selection flags, as values.
//			It holds no widget, GL or mutable scene pointer, so it may be handed
//			to another thread or kept while the document changes.
//
//			Content. One SolidDraw per shown solid with faces (id order): its
//			face polygons (scene::BuildGeometry, vertices counter-clockwise
//			from outside) with the side's VMF id, material, texture axes and
//			outward normal.
//			One EntityDraw per shown point entity with a marker (id order). A
//			brush entity has no EntityDraw: its solids carry it as 'owner'.
//			Visibility, marker boxes and colors follow view_policy.h.
//
//			Selection is input, as values: the selected object ids and the
//			selected faces. An object is drawn selected when its id or the id
//			of any container (owning entity, enclosing groups; see
//			scene::ContainerOf) is selected; a face when its FaceRef is
//			selected (face selection does not select the solid). Ids that name
//			nothing are ignored.
//
//			Hidden objects. By default an object the predicate hides is left
//			out. With keepHidden it is included with 'hidden' set (for a ghosted
//			presentation) and still excluded from 'bounds'.
//
//			SnapshotCache keeps a snapshot current across committed change sets
//			and rebuilds only the objects a change can affect: the changed
//			objects; the solids of a changed entity (color, visibility,
//			selection); the leaves of a changed group (visibility, selection);
//			the owners before and after of a changed solid (brush-entity
//			status); and the leaves of every id whose selection changed.
//			Document settings are not drawn, so a settings-only change
//			rebuilds nothing. Its snapshot always equals a full
//			Extract of the same inputs. It is keyed by a caller-supplied
//			revision: an update whose base revision is not the cache's rebuilds
//			everything. A custom visibility predicate must be a pure function
//			of document content for incremental updates to stay exact.
//
//=============================================================================//

#ifndef HAMMER_VIEWPORT_EXTRACTION_H
#define HAMMER_VIEWPORT_EXTRACTION_H

#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/map_document.h"
#include "hammer/scene/displacement_geometry.h"
#include "hammer/scene/solid_geometry.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/view_policy.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hammer::viewport
{

struct FaceDraw
{
	std::uint32_t side = 0; // the side's persistent VMF id
	std::string material;
	scene::TextureAxis uAxis; // the side's texture axes (texels = dot(p, axis) / scale + shift)
	scene::TextureAxis vAxis;
	std::vector<mapgeometry::Vec3d> vertices; // counter-clockwise from outside
	mapgeometry::Vec3d normal;                // outward
	// A displaced side draws this mesh in place of its flat polygon (legacy);
	// absent for ordinary faces and for malformed displacements.
	std::optional<mapgeometry::DisplacementSurface> displacement;
	bool selected = false;

	friend bool operator==( const FaceDraw &, const FaceDraw & ) = default;
};

struct SolidDraw
{
	scene::ObjectId id;
	scene::ObjectId owner; // owning brush entity; invalid for world solids
	std::vector<FaceDraw> faces;
	scene::Box bounds;
	scene::Rgb color;
	bool selected = false;
	bool hidden = false; // only with ExtractOptions::keepHidden

	friend bool operator==( const SolidDraw &, const SolidDraw & ) = default;
};

struct EntityDraw
{
	scene::ObjectId id;
	std::string classname;
	mapgeometry::Vec3d origin;
	mapgeometry::Vec3d angles; // pitch yaw roll; zero when absent
	mapgeometry::Vec3d mins;   // world-space marker box
	mapgeometry::Vec3d maxs;
	scene::Rgb color;
	std::string model;  // the "model" key, else the catalog model
	std::string sprite; // the catalog sprite
	bool selected = false;
	bool hidden = false;

	friend bool operator==( const EntityDraw &, const EntityDraw & ) = default;
};

struct RenderSnapshot
{
	std::vector<SolidDraw> solids;    // id order
	std::vector<EntityDraw> entities; // id order
	std::optional<scene::Box> bounds; // of everything not hidden

	friend bool operator==( const RenderSnapshot &, const RenderSnapshot & ) = default;
};

struct ExtractOptions
{
	const ports::IEntityCatalog *catalog = nullptr;
	VisibilityPredicate visible; // empty = scene::IsVisible
	bool keepHidden = false;
	double pointHalfSize = kDefaultPointHalfSize;
};

struct SelectionInput
{
	std::vector<scene::ObjectId> objects; // any order; duplicates allowed
	std::vector<scene::FaceRef> faces;
};

RenderSnapshot Extract( const scene::DocumentReader &doc, const SelectionInput &selection,
    const ExtractOptions &options = {} );

class SnapshotCache
{
public:
	explicit SnapshotCache( ExtractOptions options = {} );

	// Discards everything and extracts 'doc' (a new or replaced document).
	void Rebuild(
	    const scene::DocumentReader &doc, const SelectionInput &selection, std::uint64_t revision );

	enum class UpdateKind
	{
		Incremental,
		Full, // the base revision did not match: everything was rebuilt
	};
	// Brings the snapshot from 'baseRevision' to 'revision': 'doc' is the
	// document after 'changes' (a committed change set, forward or backward)
	// and 'selection' the current selection.
	UpdateKind Update( const scene::DocumentReader &doc, const scene::ChangeSet &changes,
	    const SelectionInput &selection, std::uint64_t baseRevision, std::uint64_t revision );
	// A selection-only change (the revision is unchanged).
	void SetSelection( const scene::DocumentReader &doc, const SelectionInput &selection );

	const RenderSnapshot &Snapshot() const { return m_snapshot; }
	std::uint64_t Revision() const { return m_revision; }
	bool Built() const { return m_built; }
	// How many objects the last Rebuild/Update/SetSelection re-extracted.
	std::size_t LastRebuiltCount() const { return m_lastRebuilt; }

private:
	void RefreshObjects( const scene::DocumentReader &doc, std::vector<scene::ObjectId> ids );
	void RecomputeBounds();

	ExtractOptions m_options;
	RenderSnapshot m_snapshot;
	std::vector<scene::ObjectId> m_selected;     // sorted, unique
	std::vector<scene::FaceRef> m_selectedFaces; // sorted, unique
	std::uint64_t m_revision = 0;
	bool m_built = false;
	std::size_t m_lastRebuilt = 0;
};

// A segment in screen pixels.
struct ScreenSegment
{
	ScreenPoint a;
	ScreenPoint b;
};

// The 2D wireframe of a solid: its unique edges (shared edges once) projected
// through 'camera', without edges that project to a point and without repeats
// of a segment already listed (within 1e-6 pixels, either direction).
std::vector<ScreenSegment> ProjectEdges2D( const SolidDraw &solid, const Camera2D &camera );

} // namespace hammer::viewport

#endif // HAMMER_VIEWPORT_EXTRACTION_H
