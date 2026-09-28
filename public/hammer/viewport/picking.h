//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hit testing for the Hammer viewports (RFC 0002, hammer.viewport):
//			what lies under a 2D click, along a 3D ray, and inside a 2D
//			marquee. Pure functions of a DocumentReader and a camera; they know
//			nothing of selection, tools or widgets.
//
//			What can be hit. Solids and point entities that the visibility
//			predicate shows (view_policy.h; default scene::IsVisible). A brush
//			entity is hit through its solids: each such hit names the solid
//			('object') and its owning entity ('owner'); SelectionTarget() gives
//			the id a click selects (the owner when there is one). Point entities
//			are hit through their marker box (view_policy.h EntityMarkerBox).
//			Groups are never hit directly.
//
//			2D (legacy rule). A solid is hit when the pointer is within
//			edgeTolerancePixels of one of its projected edges (SolidEdge) or
//			within centerHandlePixels of its projected bounds centre, the
//			legacy centre handle (SolidCenter); when both apply the nearer one
//			is reported. A point entity is hit when the pointer is inside its
//			projected marker or within edgeTolerancePixels of it (distance 0
//			inside). Faces are not picked in 2D.
//
//			Ordering policy (the one owner of hit order; tools reuse it):
//				2D:  distance in pixels ascending, then projected bounds area
//				     ascending (a small object in front of a large one), then
//				     'object' id ascending.
//				Ray: t ascending, then 'object' id ascending.
//			Each object is reported at most once.
//
//			3D. PickRay intersects the ray with each solid's convex geometry
//			(scene::BuildGeometry) and each marker box. A hit is where the ray
//			ENTERS the volume: its t, point, outward normal and, for solids, the
//			entered face (FaceRef with the side's VMF id). A ray that starts
//			inside a volume does not hit it (its faces face away), and hits
//			beyond maxDistance are dropped.
//
//			Marquee. A screen rectangle selects solids and point entities by
//			their projected bounds: Inside = bounds wholly inside the rectangle
//			(edges inclusive), Touching = bounds overlap it. With
//			ownersForBrushEntities, brush-entity solids contribute their owner:
//			Inside needs every shown solid of the entity inside, Touching any.
//			Results are unique ids in id order.
//
//			Invalid input (a non-finite pixel, rectangle or ray, a zero ray
//			direction, a camera without a viewport) produces no hits; negative
//			tolerances count as zero.
//
//=============================================================================//

#ifndef HAMMER_VIEWPORT_PICKING_H
#define HAMMER_VIEWPORT_PICKING_H

#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/map_document.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/view_policy.h"

#include <limits>
#include <vector>

namespace hammer::viewport
{

enum class HitKind
{
	SolidEdge,    // 2D: near a projected edge
	SolidCenter,  // 2D: on the centre handle
	SolidFace,    // ray: entered a face
	EntityMarker, // 2D or ray: a point entity's marker box
};

struct PickOptions
{
	const ports::IEntityCatalog *catalog = nullptr; // marker sizes; optional
	VisibilityPredicate visible;                    // empty = scene::IsVisible
	double pointHalfSize = kDefaultPointHalfSize;
	double edgeTolerancePixels = 3.0;
	double centerHandlePixels = 4.0;
	double maxDistance = std::numeric_limits<double>::infinity(); // ray only
	bool solids = true;
	bool entities = true;
};

struct Hit2D
{
	scene::ObjectId object; // the solid or point entity hit
	scene::ObjectId owner;  // the brush entity owning 'object'; invalid otherwise
	HitKind kind = HitKind::SolidEdge;
	double distance = 0.0; // pixels
	double area = 0.0;     // projected bounds area, square pixels
};

struct RayHit
{
	scene::ObjectId object;
	scene::ObjectId owner;
	HitKind kind = HitKind::SolidFace;
	double t = 0.0; // distance along the unit direction
	mapgeometry::Vec3d point;
	mapgeometry::Vec3d normal; // outward normal of the entered face or box side
	scene::FaceRef face;       // solids only; invalid solid id for markers
};

// The id a click on this hit selects: the owner when there is one.
scene::ObjectId SelectionTarget( const Hit2D &hit );
scene::ObjectId SelectionTarget( const RayHit &hit );

std::vector<Hit2D> Pick2D( const scene::DocumentReader &doc, const Camera2D &camera, double px,
    double py, const PickOptions &options = {} );

// 'direction' need not be unit length (it is normalized; t is in world units).
std::vector<RayHit> PickRay( const scene::DocumentReader &doc, const mapgeometry::Vec3d &origin,
    const mapgeometry::Vec3d &direction, const PickOptions &options = {} );

// A screen rectangle between two corners in any order.
struct ScreenRect
{
	ScreenPoint a;
	ScreenPoint b;
};

enum class MarqueeMode
{
	Inside,
	Touching,
};

struct MarqueeOptions
{
	const ports::IEntityCatalog *catalog = nullptr;
	VisibilityPredicate visible;
	double pointHalfSize = kDefaultPointHalfSize;
	bool ownersForBrushEntities = true;
	bool solids = true;
	bool entities = true;
};

std::vector<scene::ObjectId> MarqueeSelect( const scene::DocumentReader &doc,
    const Camera2D &camera, const ScreenRect &rect, MarqueeMode mode,
    const MarqueeOptions &options = {} );

} // namespace hammer::viewport

#endif // HAMMER_VIEWPORT_PICKING_H
