//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats func_instance expansion. See
//			public/hammer/formats/instancing.h. The geometry transform (QAngle
//			rotation matrix + plane/origin placement) is owned by
//			hammer/formats/vmf_transform; this file adds the instance-specific
//			load-recurse-merge logic on top of it.
//
//=============================================================================//

#include "hammer/formats/instancing.h"

#include "hammer/formats/vmf_transform.h" // shared Mat3 + Transform* (DRY)

#include <sstream>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

using geometry::Vec3d;

// Local placement-vector parse (reads a func_instance's "origin"/"angles"). The
// heavy transform math lives in vmf_transform; this is just tokenizing 3 numbers.
bool ParseVec3( const std::string &text, Vec3d &out )
{
	std::string cleaned = text;
	for ( char &ch : cleaned )
	{
		if ( ch == '[' || ch == ']' || ch == '(' || ch == ')' )
		{
			ch = ' ';
		}
	}
	std::istringstream in( cleaned );
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	if ( !( in >> x >> y >> z ) )
	{
		return false;
	}
	out = Vec3d( x, y, z );
	return true;
}

const std::string *FindPair( const KeyValueNode &node, const std::string &key )
{
	return node.Find( key );
}

// Recursively flattens 'doc' under the cumulative placement (r, origin), pushing
// world solids and non-instance entities into the pools. 'identity' skips the
// transform copy at the top level for fidelity.
struct Flattener
{
	ports::IFileStore &store;
	int maxDepth = 16;
	std::vector<KeyValueNode> solids;
	std::vector<KeyValueNode> entities;
	int expanded = 0;
	bool ok = true;
	std::string error;

	void Flatten(
	    const KeyValueNode &doc, const Mat3 &r, const Vec3d &origin, bool identity, int depth )
	{
		if ( !ok )
		{
			return;
		}
		if ( depth > maxDepth )
		{
			ok = false;
			error = "func_instance recursion exceeded maxDepth (cyclic or too deep)";
			return;
		}

		for ( const KeyValueNode &block : doc.children )
		{
			if ( !ok )
			{
				return;
			}
			if ( block.name == "world" )
			{
				for ( const KeyValueNode &child : block.children )
				{
					if ( child.name == "solid" )
					{
						solids.push_back( identity ? child : TransformSolid( child, r, origin ) );
					}
				}
			}
			else if ( block.name == "entity" )
			{
				const std::string *cls = FindPair( block, "classname" );
				if ( cls != nullptr && *cls == "func_instance" )
				{
					ExpandInstance( block, r, origin, depth );
				}
				else
				{
					entities.push_back( identity ? block : TransformEntity( block, r, origin ) );
				}
			}
		}
	}

	void ExpandInstance( const KeyValueNode &inst, const Mat3 &r, const Vec3d &origin, int depth )
	{
		const std::string *file = FindPair( inst, "file" );
		if ( file == nullptr || file->empty() )
		{
			ok = false;
			error = "func_instance has no 'file'";
			return;
		}

		Vec3d instOrigin;
		if ( const std::string *o = FindPair( inst, "origin" ) )
		{
			if ( !ParseVec3( *o, instOrigin ) )
			{
				ok = false;
				error = "func_instance 'origin' is not a vector: " + *o;
				return;
			}
		}
		double pitch = 0.0;
		double yaw = 0.0;
		double roll = 0.0;
		if ( const std::string *a = FindPair( inst, "angles" ) )
		{
			Vec3d ang;
			if ( !ParseVec3( *a, ang ) )
			{
				ok = false;
				error = "func_instance 'angles' is not a vector: " + *a;
				return;
			}
			pitch = ang.x;
			yaw = ang.y;
			roll = ang.z;
		}

		std::string text;
		if ( !store.Read( *file, text ) )
		{
			ok = false;
			error = "cannot read func_instance file: " + *file;
			return;
		}
		ParseResult pr = ParseKeyValues( text );
		if ( !pr.ok )
		{
			ok = false;
			error = "func_instance file failed to parse (" + *file + "): " + pr.error;
			return;
		}

		// Compose: a child point p is first placed by the instance's own
		// (angles, origin), then by the enclosing placement (r, origin):
		//   outer( Ri*p + instOrigin ) = (r*Ri)*p + (r*instOrigin + origin).
		const Mat3 ri = AngleMatrix( pitch, yaw, roll );
		const Mat3 composed = Multiply( r, ri );
		const Vec3d composedOrigin = Place( r, origin, instOrigin );

		++expanded;
		Flatten( pr.root, composed, composedOrigin, false, depth + 1 );
	}
};

} // namespace

InstanceExpandResult ExpandInstances(
    const KeyValueNode &root, ports::IFileStore &store, int maxDepth )
{
	InstanceExpandResult result;

	Flattener flat{ store, maxDepth, {}, {}, 0, true, {} };
	flat.Flatten( root, Mat3(), Vec3d(), true, 0 );
	if ( !flat.ok )
	{
		result.ok = false;
		result.error = flat.error;
		return result;
	}

	// Rebuild the flattened document: copy the root's non-world/non-instance
	// top-level blocks (versioninfo, etc.), then one world with all merged solids,
	// then all collected entities.
	KeyValueNode out;
	out.name = root.name;

	KeyValueNode worldOut;
	worldOut.name = "world";
	bool haveWorld = false;

	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name == "world" )
		{
			if ( !haveWorld )
			{
				worldOut = block;
				worldOut.children.clear();
				for ( const KeyValueNode &child : block.children )
				{
					if ( child.name != "solid" )
					{
						worldOut.children.push_back( child );
					}
				}
				haveWorld = true;
			}
		}
		else if ( block.name == "entity" )
		{
			// Instances are dropped (expanded); non-instance entities come from the
			// collected pool below.
		}
		else
		{
			out.children.push_back( block ); // versioninfo, cameras, cordon, visgroups, ...
		}
	}

	for ( const KeyValueNode &solid : flat.solids )
	{
		worldOut.children.push_back( solid );
	}
	out.children.push_back( worldOut );

	for ( const KeyValueNode &entity : flat.entities )
	{
		KeyValueNode e = entity;
		e.name = "entity";
		out.children.push_back( e );
	}

	result.ok = true;
	result.document = std::move( out );
	result.expanded = flat.expanded;
	return result;
}

} // namespace hammer::formats
