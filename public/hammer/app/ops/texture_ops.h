//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Face texture operations (RFC 0002, hammer.app): applying materials,
//			setting shift/scale/rotation/lightmap scale, justify and fit,
//			world/face alignment, material replacement, applying one face's
//			texture to others, and the texture-lock rule that transforms use.
//
//			Texture space follows legacy Hammer: a texel coordinate is
//			  u = dot( p, u.axis ) / u.scale + u.shift
//			and likewise for v. Justify, fit and shift normalization need the
//			material's mapping size, which comes from the IMaterialInfo port; a
//			missing material is an explicit refusal, never a guessed size.
//
//			Every operation is a pure function over a DocumentEdit, so it is
//			tested without a session and committed by EditSession::Execute.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_TEXTURE_OPS_H
#define HAMMER_APP_OPS_TEXTURE_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/ports/material_info.h"
#include "hammer/scene/change_set.h"
#include "mapgeometry/transform.h"

#include <optional>
#include <string>
#include <vector>

namespace hammer::app::ops
{

// --- Texture math (values) -------------------------------------------------------

// Texel coordinates of 'p' under 'texture' (shift included).
double TexelU( const scene::FaceTexture &texture, const mapgeometry::Vec3d &p );
double TexelV( const scene::FaceTexture &texture, const mapgeometry::Vec3d &p );

// Texture lock: the texture after moving its face by 'xf', such that every
// point keeps its texel (u(xf(p)) == u(p)). Exact for any invertible map
// (translation, rotation, scale, mirror); returns 'texture' unchanged for a
// singular map.
scene::FaceTexture LockTexture( const scene::FaceTexture &texture, const mapgeometry::Affine &xf );

// Legacy axis initialization for a face with 'normal': World keeps the
// world-aligned axes; Face projects them into the face plane. Scales,
// material and lightmap scale are kept; shifts and rotation reset to zero.
enum class TextureAlignment
{
	World,
	Face,
};
scene::FaceTexture AlignedTexture( const scene::FaceTexture &texture,
    const mapgeometry::Vec3d &normal, TextureAlignment alignment );

// Rotates the texture axes 'degrees' counterclockwise about the texture normal
// (cross(v, u)) and adds 'degrees' to the rotation field.
scene::FaceTexture RotatedTexture( const scene::FaceTexture &texture, double degrees );

// Wraps shifts into [0, size) per axis (legacy NormalizeTextureShifts) and
// rounds axis components within 0.001 of an integer.
scene::FaceTexture NormalizedShifts(
    const scene::FaceTexture &texture, const ports::MaterialSize &size );

// --- Operations -------------------------------------------------------------------

// Sets the material of the given faces. Refuses unknown faces.
EditResult ApplyMaterial( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    const std::string &material );

// Sets the material of every face of the solids the ids stand for (groups and
// brush entities expand to their solids).
EditResult ApplyMaterialToObjects( scene::DocumentEdit &edit,
    const std::vector<scene::ObjectId> &ids, const std::string &material );

// Absolute values to set; an absent field is left alone. Rotation changes the
// axes by the difference to the current rotation (RotatedTexture).
struct TextureValues
{
	std::optional<double> shiftU;
	std::optional<double> shiftV;
	std::optional<double> scaleU;
	std::optional<double> scaleV;
	std::optional<double> rotation;
	std::optional<double> lightmapScale;
};
// Refuses zero scales and non-positive lightmap scales.
EditResult SetTextureValues( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    const TextureValues &values );

// Adds to the shifts (the texture nudge).
EditResult ShiftTexture( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    double deltaU, double deltaV );

// Sets or clears smoothing group 'group' (1..32) on the faces.
EditResult SetSmoothingGroup(
    scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces, int group, bool on );

enum class Justification
{
	Left,
	Right,
	Top,
	Bottom,
	Center,
	Fit,
};
// Legacy justify: aligns the texture edge with the face's texture-space
// extent; Fit also scales so the texture spans the extent 'fitU' x 'fitV'
// times. With 'treatAsOne' the union of all faces' extents is used.
EditResult JustifyTexture( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    Justification justification, const ports::IMaterialInfo &materials, bool treatAsOne = false,
    int fitU = 1, int fitV = 1 );

EditResult AlignTexture( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    TextureAlignment alignment );

// Replaces 'find' with 'replace' on every face of the given objects (all
// solids when 'ids' is empty). Matching is case-insensitive; with
// 'substring' a face matches when its material contains 'find' and only that
// part is replaced. Writes the number of faces changed; Nothing when zero.
EditResult ReplaceMaterial( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::string &find, const std::string &replace, bool substring, int &replacedCount );

enum class ApplyTextureMode
{
	MaterialOnly,   // Ctrl-apply: only the material
	MaterialValues, // material, scales, shifts, rotation and lightmap scale
	Projected,      // everything including the axes (the source's projection)
};
EditResult ApplyTextureFrom( scene::DocumentEdit &edit, const scene::FaceTexture &source,
    const std::vector<scene::FaceRef> &targets, ApplyTextureMode mode );

// The side a face reference names, or nullptr.
const scene::Side *FindFace( const scene::DocumentReader &doc, const scene::FaceRef &face );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_TEXTURE_OPS_H
