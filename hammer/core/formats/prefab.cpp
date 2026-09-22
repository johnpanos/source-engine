//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats prefab instantiation. See
//			public/hammer/formats/prefab.h. Placement uses the shared VMF transform
//			owner (hammer/formats/vmf_transform), the same one func_instance uses.
//
//=============================================================================//

#include "hammer/formats/prefab.h"

#include "hammer/formats/vmf_transform.h"

namespace hammer::formats
{

PrefabInstance InstantiatePrefab( const KeyValueNode &prefabDoc, const Placement &at )
{
	PrefabInstance instance;
	const Mat3 rotation = AngleMatrix( at.pitch, at.yaw, at.roll );

	for ( const KeyValueNode &block : prefabDoc.children )
	{
		if ( block.name == "world" )
		{
			for ( const KeyValueNode &child : block.children )
			{
				if ( child.name == "solid" )
				{
					instance.solids.push_back( TransformSolid( child, rotation, at.origin ) );
				}
			}
		}
		else if ( block.name == "entity" )
		{
			instance.entities.push_back( TransformEntity( block, rotation, at.origin ) );
		}
	}

	return instance;
}

} // namespace hammer::formats
