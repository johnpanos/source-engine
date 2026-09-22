//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the strict Hammer geometry AABB (RFC 0002). Ported
//			from hammer/BoundBox.cpp with identical float arithmetic and operation
//			order. No PCH, no tier0, no MFC: this translation unit compiles on the
//			Linux headless core profile with only the C++ standard library.
//
//=============================================================================//

#include "hammer/geometry/aabb.h"

#include "hammer/geometry/rounding.h"

namespace hammer::geometry
{

namespace
{

// Preserves BoundBox.cpp's file-local Snap(): round to nearest grid multiple.
// The rounding rule is owned by RoundHalfAwayFromZero (hammer_mathlib V_rint
// semantics: halves away from zero), not std::rint. The intermediate int cast is
// preserved exactly.
int Snap( float value, int gridSize )
{
	return static_cast<int>( RoundHalfAwayFromZero( value / gridSize ) * gridSize );
}

} // namespace

AxisAlignedBox::AxisAlignedBox()
{
	ResetBounds();
}

AxisAlignedBox::AxisAlignedBox( const Vec3 &boxMins, const Vec3 &boxMaxs )
{
	mins = boxMins;
	maxs = boxMaxs;
}

void AxisAlignedBox::ResetBounds()
{
	mins[0] = mins[1] = mins[2] = kUninitialized;
	maxs[0] = maxs[1] = maxs[2] = -kUninitialized;
}

void AxisAlignedBox::SetBounds( const Vec3 &boxMins, const Vec3 &boxMaxs )
{
	mins = boxMins;
	maxs = boxMaxs;
}

void AxisAlignedBox::GetBounds( Vec3 &boxMins, Vec3 &boxMaxs ) const
{
	boxMins = mins;
	boxMaxs = maxs;
}

void AxisAlignedBox::UpdateBounds( const Vec3 &point )
{
	if ( point[0] < mins[0] )
		mins[0] = point[0];
	if ( point[1] < mins[1] )
		mins[1] = point[1];
	if ( point[2] < mins[2] )
		mins[2] = point[2];

	if ( point[0] > maxs[0] )
		maxs[0] = point[0];
	if ( point[1] > maxs[1] )
		maxs[1] = point[1];
	if ( point[2] > maxs[2] )
		maxs[2] = point[2];
}

void AxisAlignedBox::UpdateBounds( const Vec3 &boxMins, const Vec3 &boxMaxs )
{
	if ( boxMins[0] < mins[0] )
		mins[0] = boxMins[0];
	if ( boxMins[1] < mins[1] )
		mins[1] = boxMins[1];
	if ( boxMins[2] < mins[2] )
		mins[2] = boxMins[2];

	if ( boxMaxs[0] > maxs[0] )
		maxs[0] = boxMaxs[0];
	if ( boxMaxs[1] > maxs[1] )
		maxs[1] = boxMaxs[1];
	if ( boxMaxs[2] > maxs[2] )
		maxs[2] = boxMaxs[2];
}

void AxisAlignedBox::UpdateBounds( const AxisAlignedBox &other )
{
	UpdateBounds( other.mins, other.maxs );
}

void AxisAlignedBox::GetBoundsCenter( Vec3 &center ) const
{
	center[0] = ( mins[0] + maxs[0] ) / 2.0f;
	center[1] = ( mins[1] + maxs[1] ) / 2.0f;
	center[2] = ( mins[2] + maxs[2] ) / 2.0f;
}

void AxisAlignedBox::GetBoundsSize( Vec3 &size ) const
{
	size[0] = maxs[0] - mins[0];
	size[1] = maxs[1] - mins[1];
	size[2] = maxs[2] - mins[2];
}

bool AxisAlignedBox::IsIntersectingBox( const Vec3 &boxMins, const Vec3 &boxMaxs ) const
{
	if ( ( mins[0] >= boxMaxs[0] ) || ( maxs[0] <= boxMins[0] ) )
	{
		return false;
	}
	if ( ( mins[1] >= boxMaxs[1] ) || ( maxs[1] <= boxMins[1] ) )
	{
		return false;
	}
	if ( ( mins[2] >= boxMaxs[2] ) || ( maxs[2] <= boxMins[2] ) )
	{
		return false;
	}
	return true;
}

bool AxisAlignedBox::IsInsideBox( const Vec3 &boxMins, const Vec3 &boxMaxs ) const
{
	if ( ( mins[0] < boxMins[0] ) || ( maxs[0] > boxMaxs[0] ) )
	{
		return false;
	}
	if ( ( mins[1] < boxMins[1] ) || ( maxs[1] > boxMaxs[1] ) )
	{
		return false;
	}
	if ( ( mins[2] < boxMins[2] ) || ( maxs[2] > boxMaxs[2] ) )
	{
		return false;
	}
	return true;
}

bool AxisAlignedBox::ContainsPoint( const Vec3 &point ) const
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( point[i] < mins[i] || point[i] > maxs[i] )
		{
			return false;
		}
	}
	return true;
}

bool AxisAlignedBox::IsValidBox() const
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( mins[i] > maxs[i] )
		{
			return false;
		}
	}
	return true;
}

void AxisAlignedBox::SnapToGrid( int gridSize )
{
	// Declared behavior refinement over the legacy code: a non-positive grid is a
	// degenerate input (the legacy Snap divided by it). Reject it and leave the
	// box unchanged rather than divide by zero.
	if ( gridSize <= 0 )
	{
		return;
	}

	// Does not alter the size of the box; snaps its minimum corner to the grid.
	Vec3 size;
	GetBoundsSize( size );

	for ( int i = 0; i < 3; i++ )
	{
		mins[i] = static_cast<float>( Snap( mins[i], gridSize ) );
		maxs[i] = mins[i] + size[i];
	}
}

void AxisAlignedBox::Rotate90( int axis )
{
	int e1 = kAxisX;
	int e2 = kAxisY;

	Vec3 center;
	GetBoundsCenter( center );

	switch ( axis )
	{
	case kAxisZ:
		e1 = kAxisX;
		e2 = kAxisY;
		break;
	case kAxisX:
		e1 = kAxisY;
		e2 = kAxisZ;
		break;
	case kAxisY:
		e1 = kAxisX;
		e2 = kAxisZ;
		break;
	}

	float tmp1 = mins[e1] - center[e1] + center[e2];
	float tmp2 = maxs[e1] - center[e1] + center[e2];
	mins[e1] = mins[e2] - center[e2] + center[e1];
	maxs[e1] = maxs[e2] - center[e2] + center[e1];
	mins[e2] = tmp1;
	maxs[e2] = tmp2;
}

} // namespace hammer::geometry
