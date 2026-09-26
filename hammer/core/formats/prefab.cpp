//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats prefab instantiation. See
//			public/hammer/formats/prefab.h. Placement uses the shared VMF transform
//			owner (vmf/vmf_transform), the same one func_instance uses.
//
//=============================================================================//

#include "hammer/formats/prefab.h"

#include "vmf/vmf_transform.h"

namespace hammer::formats
{

PrefabInstance InstantiatePrefab( const kvtext::KeyValueNode &prefabDoc, const Placement &at )
{
	PrefabInstance instance;
	const vmf::Mat3 rotation = vmf::AngleMatrix( at.pitch, at.yaw, at.roll );

	for ( const kvtext::KeyValueNode &block : prefabDoc.children )
	{
		if ( block.name == "world" )
		{
			for ( const kvtext::KeyValueNode &child : block.children )
			{
				if ( child.name == "solid" )
				{
					instance.solids.push_back( vmf::TransformSolid( child, rotation, at.origin ) );
				}
			}
		}
		else if ( block.name == "entity" )
		{
			instance.entities.push_back( vmf::TransformEntity( block, rotation, at.origin ) );
		}
	}

	return instance;
}

} // namespace hammer::formats
