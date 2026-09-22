//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats cordon filtering. See
//			public/hammer/formats/cordon.h.
//
//=============================================================================//

#include "hammer/formats/cordon.h"

#include <sstream>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

using geometry::Vec3d;

struct Bounds
{
	Vec3d mins;
	Vec3d maxs;
	bool valid = false;

	void Add( const Vec3d &p )
	{
		if ( !valid )
		{
			mins = p;
			maxs = p;
			valid = true;
			return;
		}
		mins.x = p.x < mins.x ? p.x : mins.x;
		mins.y = p.y < mins.y ? p.y : mins.y;
		mins.z = p.z < mins.z ? p.z : mins.z;
		maxs.x = p.x > maxs.x ? p.x : maxs.x;
		maxs.y = p.y > maxs.y ? p.y : maxs.y;
		maxs.z = p.z > maxs.z ? p.z : maxs.z;
	}
};

bool ParseDoubles( const std::string &text, std::vector<double> &out )
{
	out.clear();
	std::string cleaned = text;
	for ( char &ch : cleaned )
	{
		if ( ch == '(' || ch == ')' || ch == '[' || ch == ']' )
		{
			ch = ' ';
		}
	}
	std::istringstream in( cleaned );
	double value = 0.0;
	while ( in >> value )
	{
		out.push_back( value );
	}
	return true;
}

// Accumulates a "side" plane's three points into 'bounds'.
void AddPlaneBounds( const std::string &planeValue, Bounds &bounds )
{
	std::vector<double> vals;
	ParseDoubles( planeValue, vals );
	// Points come in triples; use as many complete triples as present.
	const std::size_t triples = vals.size() / 3;
	for ( std::size_t t = 0; t < triples; ++t )
	{
		bounds.Add( Vec3d( vals[t * 3 + 0], vals[t * 3 + 1], vals[t * 3 + 2] ) );
	}
}

// Bounds of a "solid" block: the union of all its side planes' points.
Bounds SolidBounds( const KeyValueNode &solid )
{
	Bounds bounds;
	for ( const KeyValueNode &side : solid.children )
	{
		if ( side.name != "side" )
		{
			continue;
		}
		if ( const std::string *plane = side.Find( "plane" ) )
		{
			AddPlaneBounds( *plane, bounds );
		}
	}
	return bounds;
}

bool BoxesIntersect( const Vec3d &aMin, const Vec3d &aMax, const Vec3d &bMin, const Vec3d &bMax )
{
	return aMin.x <= bMax.x && aMax.x >= bMin.x && aMin.y <= bMax.y && aMax.y >= bMin.y &&
	       aMin.z <= bMax.z && aMax.z >= bMin.z;
}

bool SolidInBox( const KeyValueNode &solid, const CordonBox &box )
{
	const Bounds b = SolidBounds( solid );
	if ( !b.valid )
	{
		return false; // no geometry to place; excluded
	}
	return BoxesIntersect( b.mins, b.maxs, box.mins, box.maxs );
}

bool PointInBox( const Vec3d &p, const CordonBox &box )
{
	return p.x >= box.mins.x && p.x <= box.maxs.x && p.y >= box.mins.y && p.y <= box.maxs.y &&
	       p.z >= box.mins.z && p.z <= box.maxs.z;
}

bool ParseVec3( const std::string &text, Vec3d &out )
{
	std::vector<double> vals;
	ParseDoubles( text, vals );
	if ( vals.size() != 3 )
	{
		return false;
	}
	out = Vec3d( vals[0], vals[1], vals[2] );
	return true;
}

} // namespace

CordonResult ApplyCordon( const KeyValueNode &root, const CordonBox &box )
{
	CordonResult result;
	result.document.name = root.name;

	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name == "world" )
		{
			// Keep the world's pairs and non-solid children; filter solids.
			KeyValueNode worldOut;
			worldOut.name = block.name;
			worldOut.pairs = block.pairs;
			for ( const KeyValueNode &child : block.children )
			{
				if ( child.name != "solid" )
				{
					worldOut.children.push_back( child );
					continue;
				}
				if ( SolidInBox( child, box ) )
				{
					worldOut.children.push_back( child );
					++result.solidsKept;
				}
				else
				{
					++result.solidsRemoved;
				}
			}
			result.document.children.push_back( std::move( worldOut ) );
		}
		else if ( block.name == "entity" )
		{
			bool keep = true;
			Vec3d origin;
			bool hasSolids = false;
			for ( const KeyValueNode &child : block.children )
			{
				if ( child.name == "solid" )
				{
					hasSolids = true;
					break;
				}
			}
			if ( const std::string *o = block.Find( "origin" );
			    o != nullptr && ParseVec3( *o, origin ) )
			{
				keep = PointInBox( origin, box ); // point entity
			}
			else if ( hasSolids )
			{
				keep = false;
				for ( const KeyValueNode &child : block.children )
				{
					if ( child.name == "solid" && SolidInBox( child, box ) )
					{
						keep = true;
						break;
					}
				}
			}
			// else: positionless entity (no origin, no solids) -> kept.

			if ( keep )
			{
				result.document.children.push_back( block );
				++result.entitiesKept;
			}
			else
			{
				++result.entitiesRemoved;
			}
		}
		else
		{
			result.document.children.push_back( block ); // versioninfo, cameras, ...
		}
	}

	return result;
}

} // namespace hammer::formats
