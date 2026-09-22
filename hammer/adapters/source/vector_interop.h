//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.source -- the single named owner of the boundary
//			policy that marshals Source engine value types (Vector, BoundBox)
//			to and from the strict, MFC-free Hammer geometry library
//			(hammer::geometry::Vec3 / AxisAlignedBox).
//
//			This is a NON-STRICT adapter: it deliberately includes the Source
//			platform layer (mathlib/vector.h) and the legacy BoundBox, which the
//			strict libraries never may. It exists so that a thin UI sibling --
//			the MFC shell (hammer.adapters.mfc) or the GTK shell
//			(hammer.adapters.gtk) -- can drive the reusable geometry library on
//			Source's own data without duplicating the conversion rule. Per the
//			project DRY rule, the Vector<->Vec3 conversion has exactly one owner:
//			here. Callers route through it; they do not open-code component copies.
//
//			Both Source Vector and Vec3 store three float components in x,y,z
//			order, so the conversion is exact (no precision or ordering change).
//			This module is compiled only on profiles that provide the Source
//			platform layer (the engine build, or the MSVC/Wine parity lane); it is
//			absent from the linux-headless-core profile by design.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_SOURCE_VECTOR_INTEROP_H
#define HAMMER_ADAPTERS_SOURCE_VECTOR_INTEROP_H

#include "mathlib/vector.h"          // Source Vector (non-strict dependency)
#include "boundbox.h"                // legacy BoundBox (non-strict dependency)
#include "hammer/geometry/aabb.h"    // strict reusable library

namespace hammer::adapters::source
{

//-----------------------------------------------------------------------------
// Purpose: Convert a Source Vector to the strict geometry Vec3. Exact: same
//			float components, same x/y/z order.
//-----------------------------------------------------------------------------
inline hammer::geometry::Vec3 ToVec3( const Vector &v )
{
	return hammer::geometry::Vec3( v.x, v.y, v.z );
}

//-----------------------------------------------------------------------------
// Purpose: Convert a strict geometry Vec3 back to a Source Vector. Exact inverse
//			of ToVec3 for finite components.
//-----------------------------------------------------------------------------
inline Vector ToVector( const hammer::geometry::Vec3 &v )
{
	return Vector( v.x, v.y, v.z );
}

//-----------------------------------------------------------------------------
// Purpose: Read a legacy BoundBox's extent into a strict AxisAlignedBox without
//			mutating the legacy object. This lets a UI sibling run a reusable
//			geometry operation on data that currently lives in an MFC-era object.
//-----------------------------------------------------------------------------
inline hammer::geometry::AxisAlignedBox ToAABB( const BoundBox &box )
{
	return hammer::geometry::AxisAlignedBox( ToVec3( box.bmins ), ToVec3( box.bmaxs ) );
}

//-----------------------------------------------------------------------------
// Purpose: Write a strict AxisAlignedBox's extent back onto a legacy BoundBox.
//			Only the mins/maxs are copied; the caller owns the BoundBox lifetime
//			and any derived legacy state. This is the commit half of the seam:
//			gather/compute on the reusable library, then commit onto the live
//			legacy object.
//-----------------------------------------------------------------------------
inline void FromAABB( const hammer::geometry::AxisAlignedBox &box, BoundBox &out )
{
	out.bmins = ToVector( box.mins );
	out.bmaxs = ToVector( box.maxs );
}

} // namespace hammer::adapters::source

#endif // HAMMER_ADAPTERS_SOURCE_VECTOR_INTEROP_H
