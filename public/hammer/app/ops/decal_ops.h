//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Decal and overlay placement (RFC 0002, hammer.app; legacy
//			CToolDecal, CToolOverlay and CMapOverlay). Both are entities whose
//			keys the operations write in full, so a placed overlay needs no
//			helper object to be valid.
//
//			Decal (legacy CToolDecal::OnLMouseDown3D): an "infodecal" entity at
//			the picked point with key "texture" = the material. The face only
//			validates the pick; the engine projects the decal at run time.
//
//			Overlay (legacy CToolOverlay::CreateOverlay + CMapOverlay::
//			Basis_Init): an "info_overlay" at the picked point whose basis
//			comes from the FIRST face:
//			  * BasisNormal = the face's outward unit normal;
//			  * the initial U axis is world +X when the normal's major axis
//			    (largest |component|, earlier axis on ties) is Y or Z, and
//			    world +Y when it is X (Basis_SetInitialUAxis);
//			  * BasisV = normalize(N x U), then BasisU = normalize(V x N), so
//			    U, V, N are orthonormal and U, V lie in the face plane;
//			  * BasisOrigin = the picked point projected onto the face plane
//			    (Basis_UpdateOrigin);
//			  * StartU/EndU/StartV/EndV = 0/1/1/0, or 1/0/0/1 when the signs of
//			    U's and V's largest components differ (Material_TexCoordInit);
//			  * uv0..uv3 = the handle corners (-w/2,-h/2), (-w/2,h/2),
//			    (w/2,h/2), (w/2,-h/2), each "u v 0" (Handles_Init; legacy
//			    derives w, h from the texture size / 4, here they are given);
//			  * "sides" = the faces' side VMF ids, space separated;
//			    "RenderOrder" = 0 (the FGD default).
//			Faces must exist; duplicates collapse (first kept); at most 64
//			(the BSP overlay face limit). Legacy adds faces with Shift-click
//			without touching the basis; SetOverlayFaces does the same unless
//			the first face changes (legacy SideList_Init rebuilds the basis
//			from a new first face), keeping the uv corners.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_DECAL_OPS_H
#define HAMMER_APP_OPS_DECAL_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/transform_ops.h"
#include "hammer/scene/change_set.h"
#include "mapgeometry/transform.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::app::ops
{

// The maximum number of faces one overlay may name (OVERLAY_BSP_FACE_COUNT).
constexpr std::size_t kMaxOverlayFaces = 64;

// Places an infodecal. Refuses an unknown face, an empty material and a
// non-finite point.
EditResult PlaceDecal( scene::DocumentEdit &edit, const scene::FaceRef &face,
    const mapgeometry::Vec3d &point, const std::string &material, scene::ObjectId &created );

// Places an info_overlay of 'width' x 'height' world units centered on
// 'point' over 'faces' (see the rule above). Refuses no faces, unknown faces,
// more than kMaxOverlayFaces, an empty material, non-positive or non-finite
// sizes, a non-finite point and a degenerate first face.
EditResult PlaceOverlay( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    const mapgeometry::Vec3d &point, const std::string &material, double width, double height,
    scene::ObjectId &created );

// Replaces the overlay's face list ("sides"). When the first face differs
// from the current first side, the basis is rebuilt from it about the
// entity's origin (the uv corners are kept). Nothing when unchanged.
EditResult SetOverlayFaces(
    scene::DocumentEdit &edit, scene::ObjectId overlay, const std::vector<scene::FaceRef> &faces );

// Resizes the overlay to a centered 'width' x 'height' rectangle in its
// basis (uv0..uv3; each corner's third component, the legacy axis-flip
// bits, is kept). Nothing when unchanged.
EditResult SetOverlaySize(
    scene::DocumentEdit &edit, scene::ObjectId overlay, double width, double height );

struct OverlayBasis
{
	mapgeometry::Vec3d origin;
	mapgeometry::Vec3d u;
	mapgeometry::Vec3d v;
	mapgeometry::Vec3d normal;
	double startU = 0.0;
	double endU = 1.0;
	double startV = 1.0;
	double endV = 0.0;
};

// The legacy basis for a face with 'facePlane' picked at 'point'; nothing
// when the plane's normal is zero or not finite.
std::optional<OverlayBasis> OverlayBasisFor(
    const mapgeometry::Plane &facePlane, const mapgeometry::Vec3d &point );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_DECAL_OPS_H
