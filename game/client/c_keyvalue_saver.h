//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lazily reads and writes named client KeyValues save files.
//
//===========================================================================//

#ifndef C_KEYVALUE_SAVER_H
#define C_KEYVALUE_SAVER_H

#include "igamesystem.h"
#include "GameEventListener.h"
#include "tier1/utlvector.h"

class KeyValues;

typedef void ( *KeyValueBuilder )( KeyValues *pKeyValues );

struct KeyValueSaverData
{
	char szFileName[260];
	bool bDirtySaveData;
	KeyValues *pKeyValues;
	KeyValueBuilder funcKeyValueBuilder;
};

class C_KeyValueSaver : public CAutoGameSystemPerFrame, public CGameEventListener
{
public:
	C_KeyValueSaver();

	void SetSlot( int nSlot );

	bool Init() OVERRIDE;
	void Shutdown() OVERRIDE;
	void Update( float frametime ) OVERRIDE;
	void FireGameEvent( IGameEvent *pEvent ) OVERRIDE;

	bool InitKeyValues( const char *pchFileName, KeyValueBuilder funcKeyValueBuilder );
	bool WriteDirtyKeyValues( const char *pchFileName, bool bForceWrite = false );
	KeyValues *GetKeyValues( const char *pchFileName, bool bForceReread = false );
	void MarkKeyValuesDirty( const char *pchFileName );

private:
	bool ReadKeyValues( KeyValueSaverData *pKeyValueData );
	bool WriteDirtyKeyValues( KeyValueSaverData *pKeyValueData, bool bForceWrite );
	void WriteAllDirtyKeyValues();
	KeyValueSaverData *FindKeyValueData( const char *pchFileName );

	CUtlVector<KeyValueSaverData> m_KeyValueData;
	int m_nSplitScreenSlot;
};

C_KeyValueSaver &KeyValueSaver();

#endif // C_KEYVALUE_SAVER_H
