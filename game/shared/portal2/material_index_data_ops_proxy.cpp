//========= Portal 2 reconstruction ============================================//
//
// Purpose: Persist paint props' original surface material across save/restore.
//
// The retail server names MaterialIndexDataOpsProxy. The source consumer
// stores a physics surface index, so this implementation stores its name and
// resolves it through the active surface properties table on restore. The
// retail proxy's save format is not yet verified. Not original Valve source;
// the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "material_index_data_ops_proxy.h"
#include "isaverestore.h"
#include "vphysics_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class MaterialIndexDataOpsProxy : public CDefSaveRestoreOps
{
public:
	void Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave ) OVERRIDE
	{
		const int nIndex = *static_cast<const int *>( fieldInfo.pField );
		const char *pName = physprops && nIndex >= 0 && nIndex < physprops->SurfacePropCount()
		                        ? physprops->GetPropName( nIndex )
		                        : NULL;
		pSave->WriteString( pName ? pName : "" );
	}

	void Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore ) OVERRIDE
	{
		char szName[256];
		pRestore->ReadString( szName, sizeof( szName ), 0 );
		int *pIndex = static_cast<int *>( fieldInfo.pField );
		*pIndex = -1;
		if ( szName[0] && physprops )
		{
			*pIndex = physprops->GetSurfaceIndex( szName );
			if ( *pIndex < 0 )
			{
				Warning( "GetMaterialIndex: couldn't find material %s\n", szName );
			}
		}
	}

	bool IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo ) OVERRIDE
	{
		return *static_cast<const int *>( fieldInfo.pField ) < 0;
	}

	void MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo ) OVERRIDE
	{
		*static_cast<int *>( fieldInfo.pField ) = -1;
	}
};

ISaveRestoreOps &GetMaterialIndexDataOpsProxy()
{
	static MaterialIndexDataOpsProxy s_DataOps;
	return s_DataOps;
}
