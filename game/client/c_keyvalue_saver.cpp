//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lazy client-side KeyValues persistence for Portal 2 state.
//
//===========================================================================//

#include "cbase.h"
#include "c_keyvalue_saver.h"

#include <KeyValues.h>
#include "filesystem.h"
#include "tier1/utlbuffer.h"

#include "tier0/memdbgon.h"

namespace
{
const int kMaxKeyValueSaverSlots = 2;
const int kKeyValueSaverFileNameLength = 260;
const int kKeyValueSaverPathLength = 1024;

C_KeyValueSaver g_KeyValueSaver[kMaxKeyValueSaverSlots];

bool IsSafeSaveName( const char *pchFileName )
{
	if ( !pchFileName || !pchFileName[0] ||
	     V_strlen( pchFileName ) >= kKeyValueSaverFileNameLength )
		return false;

	for ( const char *pch = pchFileName; *pch; ++pch )
	{
		if ( *pch == '/' || *pch == '\\' || *pch == ':' )
			return false;
	}

	return V_strcmp( pchFileName, "." ) != 0 && V_strcmp( pchFileName, ".." ) != 0;
}

void BuildSavePath( char *pszPath, int nPathBytes, const char *pchFileName )
{
	V_snprintf( pszPath, nPathBytes, "save/%s", pchFileName );
}
}

C_KeyValueSaver::C_KeyValueSaver()
    : CAutoGameSystemPerFrame( "C_KeyValueSaver" ), m_nSplitScreenSlot( 0 )
{
}

C_KeyValueSaver &KeyValueSaver()
{
	// This engine branch exposes one local-player context. Keep the second
	// historical saver object available for layout/source compatibility, while
	// routing current callers to the supported primary slot.
	return g_KeyValueSaver[0];
}

void C_KeyValueSaver::SetSlot( int nSlot )
{
	m_nSplitScreenSlot = clamp( nSlot, 0, kMaxKeyValueSaverSlots - 1 );
}

bool C_KeyValueSaver::Init()
{
	for ( int nSlot = 0; nSlot < kMaxKeyValueSaverSlots; ++nSlot )
	{
		if ( this == &g_KeyValueSaver[nSlot] )
		{
			SetSlot( nSlot );
			break;
		}
	}

	ListenForGameEvent( "game_newmap" );
	ListenForGameEvent( "round_start" );
	ListenForGameEvent( "server_spawn" );
	return true;
}

void C_KeyValueSaver::Shutdown()
{
	WriteAllDirtyKeyValues();
	StopListeningForAllEvents();

	for ( int i = 0; i < m_KeyValueData.Count(); ++i )
	{
		if ( m_KeyValueData[i].pKeyValues )
			m_KeyValueData[i].pKeyValues->deleteThis();
	}

	m_KeyValueData.RemoveAll();
}

void C_KeyValueSaver::Update( float frametime )
{
	// Writes are event-driven and occur on level transitions and shutdown.
}

void C_KeyValueSaver::FireGameEvent( IGameEvent *pEvent )
{
	if ( !pEvent )
		return;

	const char *pchEventName = pEvent->GetName();
	if ( !V_strcmp( pchEventName, "game_newmap" ) || !V_strcmp( pchEventName, "round_start" ) ||
	     !V_strcmp( pchEventName, "server_spawn" ) )
	{
		WriteAllDirtyKeyValues();
	}
}

bool C_KeyValueSaver::InitKeyValues( const char *pchFileName, KeyValueBuilder funcKeyValueBuilder )
{
	if ( !IsSafeSaveName( pchFileName ) )
		return false;

	KeyValueSaverData *pData = FindKeyValueData( pchFileName );
	if ( pData )
		return false;

	KeyValueSaverData data;
	V_strncpy( data.szFileName, pchFileName, sizeof( data.szFileName ) );
	data.bDirtySaveData = false;
	data.pKeyValues = NULL;
	data.funcKeyValueBuilder = funcKeyValueBuilder;
	m_KeyValueData.AddToTail( data );
	return true;
}

bool C_KeyValueSaver::WriteDirtyKeyValues( const char *pchFileName, bool bForceWrite )
{
	KeyValueSaverData *pData = FindKeyValueData( pchFileName );
	return pData && WriteDirtyKeyValues( pData, bForceWrite );
}

KeyValues *C_KeyValueSaver::GetKeyValues( const char *pchFileName, bool bForceReread )
{
	KeyValueSaverData *pData = FindKeyValueData( pchFileName );
	if ( !pData )
		return NULL;

	if ( pData->pKeyValues && !bForceReread )
		return pData->pKeyValues;

	return ReadKeyValues( pData ) ? pData->pKeyValues : NULL;
}

void C_KeyValueSaver::MarkKeyValuesDirty( const char *pchFileName )
{
	KeyValueSaverData *pData = FindKeyValueData( pchFileName );
	if ( pData )
		pData->bDirtySaveData = true;
}

bool C_KeyValueSaver::ReadKeyValues( KeyValueSaverData *pData )
{
	if ( !pData )
		return false;

	if ( pData->pKeyValues )
	{
		pData->pKeyValues->deleteThis();
		pData->pKeyValues = NULL;
	}

	char szPath[kKeyValueSaverPathLength];
	BuildSavePath( szPath, sizeof( szPath ), pData->szFileName );

	pData->pKeyValues = new KeyValues( "KeyValueSaverData" );
	return pData->pKeyValues->LoadFromFile( filesystem, szPath, NULL );
}

bool C_KeyValueSaver::WriteDirtyKeyValues( KeyValueSaverData *pData, bool bForceWrite )
{
	if ( !pData )
		return false;

	if ( !pData->bDirtySaveData && !bForceWrite )
		return true;

	// A reconstructed caller may own only one subtree of this save. Retain
	// unrelated keys already on disk while its builder replaces that subtree.
	if ( !pData->pKeyValues )
		ReadKeyValues( pData );
	if ( !pData->pKeyValues )
		pData->pKeyValues = new KeyValues( "KeyValueSaverData" );
	pData->bDirtySaveData = false;
	if ( pData->funcKeyValueBuilder )
		pData->funcKeyValueBuilder( pData->pKeyValues );

	CUtlBuffer buffer( 0, 0, CUtlBuffer::TEXT_BUFFER );
	pData->pKeyValues->RecursiveSaveToFile( buffer, 0 );

	char szPath[kKeyValueSaverPathLength];
	BuildSavePath( szPath, sizeof( szPath ), pData->szFileName );
	filesystem->CreateDirHierarchy( "save", "MOD" );
	FileHandle_t hFile = filesystem->Open( szPath, "wb", "MOD" );
	if ( hFile == FILESYSTEM_INVALID_HANDLE )
	{
		pData->bDirtySaveData = true;
		return false;
	}

	const int nBytesWritten = filesystem->Write( buffer.Base(), buffer.TellPut(), hFile );
	filesystem->Close( hFile );
	const bool bWriteSuccess = nBytesWritten == buffer.TellPut();
	if ( !bWriteSuccess )
		pData->bDirtySaveData = true;

	return bWriteSuccess;
}

void C_KeyValueSaver::WriteAllDirtyKeyValues()
{
	for ( int i = 0; i < m_KeyValueData.Count(); ++i )
		WriteDirtyKeyValues( &m_KeyValueData[i], false );
}

KeyValueSaverData *C_KeyValueSaver::FindKeyValueData( const char *pchFileName )
{
	if ( !IsSafeSaveName( pchFileName ) )
		return NULL;

	for ( int i = 0; i < m_KeyValueData.Count(); ++i )
	{
		if ( !V_strcmp( m_KeyValueData[i].szFileName, pchFileName ) )
			return &m_KeyValueData[i];
	}

	return NULL;
}
