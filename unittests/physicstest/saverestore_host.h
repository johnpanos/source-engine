//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: In-memory ISave/IRestore for the VPhysics conformance suite.
//
//          The game hands a provider its save stream and treats what the
//          provider writes as opaque, reading it back in the same order. This
//          host is that stream: a byte buffer, plus a datadesc walker for the
//          WriteAll/ReadAll path IVP uses (plain fields, arrays, embedded
//          tables and custom save/restore ops). Entity-typed and engine-string
//          fields are not physics data and are rejected.
//
//=============================================================================//
#ifndef SAVERESTORE_HOST_H
#define SAVERESTORE_HOST_H

#include <string.h>

#include "isaverestore.h"
#include "tier1/utlvector.h"

class CSaveRestoreBuffer
{
public:
	CSaveRestoreBuffer() : m_readPos( 0 ), m_errors( 0 ) {}

	void Write( const void *pData, int size )
	{
		if ( size <= 0 )
			return;
		int start = m_bytes.AddMultipleToTail( size );
		memcpy( m_bytes.Base() + start, pData, size );
	}
	bool Read( void *pData, int size )
	{
		if ( size < 0 || m_readPos + size > m_bytes.Count() )
		{
			m_errors++;
			memset( pData, 0, size > 0 ? size : 0 );
			return false;
		}
		memcpy( pData, m_bytes.Base() + m_readPos, size );
		m_readPos += size;
		return true;
	}
	int Size() const { return m_bytes.Count(); }
	int ReadPos() const { return m_readPos; }
	void SetReadPos( int pos ) { m_readPos = pos; }
	int Errors() const { return m_errors; }
	void Error() { m_errors++; }
	unsigned char *Base() { return m_bytes.Base(); }

private:
	CUtlVector<unsigned char> m_bytes;
	int m_readPos;
	int m_errors;
};

// Walks a datamap (base maps first) calling pVisit( field, pFieldData ).
template <typename FN>
inline void VisitDataMap( datamap_t *pMap, void *pBase, FN pVisit )
{
	if ( !pMap )
		return;
	VisitDataMap( pMap->baseMap, pBase, pVisit );
	for ( int i = 0; i < pMap->dataNumFields; i++ )
	{
		typedescription_t *pField = &pMap->dataDesc[i];
		if ( pField->fieldType == FIELD_VOID )
			continue;
		pVisit( pField, (char *)pBase + pField->fieldOffset[TD_OFFSET_NORMAL] );
	}
}

class CHostSave : public ISave
{
public:
	explicit CHostSave( CSaveRestoreBuffer *pBuffer ) : m_pBuffer( pBuffer ) {}

	virtual void StartLogging( const char * ) {}
	virtual void EndLogging( void ) {}
	virtual bool IsAsync() { return false; }
	virtual int GetWritePos() const { return m_pBuffer->Size(); }
	virtual void SetWritePos( int ) { m_pBuffer->Error(); }

	virtual int WriteAll( const void *pLeafObject, datamap_t *pLeafMap )
	{
		int before = m_pBuffer->Size();
		VisitDataMap( pLeafMap, const_cast<void *>( pLeafObject ), [this]( typedescription_t *pField, void *pData )
		{
			WriteField( pField, pData );
		} );
		return m_pBuffer->Size() - before;
	}
	virtual int WriteFields( const char *, const void *pBaseData, datamap_t *, typedescription_t *pFields, int fieldCount )
	{
		for ( int i = 0; i < fieldCount; i++ )
			WriteField( &pFields[i], (char *)pBaseData + pFields[i].fieldOffset[TD_OFFSET_NORMAL] );
		return fieldCount;
	}

	virtual void StartBlock( const char * ) {}
	virtual void StartBlock() {}
	virtual void EndBlock() {}

	virtual void WriteShort( const short *value, int count = 1 ) { m_pBuffer->Write( value, sizeof( short ) * count ); }
	virtual void WriteInt( const int *value, int count = 1 ) { m_pBuffer->Write( value, sizeof( int ) * count ); }
	virtual void WriteBool( const bool *value, int count = 1 ) { m_pBuffer->Write( value, sizeof( bool ) * count ); }
	virtual void WriteFloat( const float *value, int count = 1 ) { m_pBuffer->Write( value, sizeof( float ) * count ); }
	virtual void WriteData( const char *pdata, int size ) { m_pBuffer->Write( pdata, size ); }
	virtual void WriteString( const char *pstring ) { m_pBuffer->Write( pstring, (int)strlen( pstring ) + 1 ); }
	virtual void WriteString( const string_t *, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteVector( const Vector &value ) { m_pBuffer->Write( &value, sizeof( Vector ) ); }
	virtual void WriteVector( const Vector *value, int count = 1 ) { m_pBuffer->Write( value, sizeof( Vector ) * count ); }
	virtual void WriteQuaternion( const Quaternion &value ) { m_pBuffer->Write( &value, sizeof( Quaternion ) ); }
	virtual void WriteQuaternion( const Quaternion *value, int count = 1 ) { m_pBuffer->Write( value, sizeof( Quaternion ) * count ); }

	virtual void WriteShort( const char *, const short *value, int count = 1 ) { WriteShort( value, count ); }
	virtual void WriteInt( const char *, const int *value, int count = 1 ) { WriteInt( value, count ); }
	virtual void WriteBool( const char *, const bool *value, int count = 1 ) { WriteBool( value, count ); }
	virtual void WriteFloat( const char *, const float *value, int count = 1 ) { WriteFloat( value, count ); }
	virtual void WriteData( const char *, int size, const char *pdata ) { WriteData( pdata, size ); }
	virtual void WriteString( const char *, const char *pstring ) { WriteString( pstring ); }
	virtual void WriteString( const char *, const string_t *, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteVector( const char *, const Vector &value ) { WriteVector( value ); }
	virtual void WriteVector( const char *, const Vector *value, int count = 1 ) { WriteVector( value, count ); }
	virtual void WriteQuaternion( const char *, const Quaternion &value ) { WriteQuaternion( value ); }
	virtual void WriteQuaternion( const char *, const Quaternion *value, int count = 1 ) { WriteQuaternion( value, count ); }

	virtual void WriteTime( const char *, const float *value, int count = 1 ) { WriteFloat( value, count ); }
	virtual void WriteTick( const char *, const int *value, int count = 1 ) { WriteInt( value, count ); }
	virtual void WritePositionVector( const char *, const Vector &value ) { WriteVector( value ); }
	virtual void WritePositionVector( const char *, const Vector *value, int count = 1 ) { WriteVector( value, count ); }
	virtual void WriteFunction( datamap_t *, const char *, inputfunc_t **, int = 1 ) { m_pBuffer->Error(); }

	virtual void WriteTime( const float *value, int count = 1 ) { WriteFloat( value, count ); }
	virtual void WriteTick( const int *value, int count = 1 ) { WriteInt( value, count ); }
	virtual void WritePositionVector( const Vector &value ) { WriteVector( value ); }
	virtual void WritePositionVector( const Vector *value, int count = 1 ) { WriteVector( value, count ); }

	virtual void WriteEntityPtr( const char *, CBaseEntity **, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteEdictPtr( const char *, edict_t **, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteEHandle( const char *, const EHANDLE *, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteEntityPtr( CBaseEntity **, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteEdictPtr( edict_t **, int = 1 ) { m_pBuffer->Error(); }
	virtual void WriteEHandle( const EHANDLE *, int = 1 ) { m_pBuffer->Error(); }

	virtual CGameSaveRestoreInfo *GetGameSaveRestoreInfo() { return NULL; }

private:
	void WriteField( typedescription_t *pField, void *pData )
	{
		switch ( pField->fieldType )
		{
		case FIELD_EMBEDDED:
		{
			int stride = pField->fieldSize > 0 ? pField->fieldSizeInBytes / pField->fieldSize : 0;
			for ( int i = 0; i < pField->fieldSize; i++ )
				WriteAll( (char *)pData + i * stride, pField->td );
			break;
		}
		case FIELD_CUSTOM:
		{
			SaveRestoreFieldInfo_t info = { pData, NULL, pField };
			pField->pSaveRestoreOps->Save( info, this );
			break;
		}
		case FIELD_STRING:
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
		case FIELD_EHANDLE:
		case FIELD_CLASSPTR:
		case FIELD_EDICT:
		case FIELD_FUNCTION:
		case FIELD_INPUT:
			m_pBuffer->Error();
			break;
		default:
			m_pBuffer->Write( pData, pField->fieldSizeInBytes );
			break;
		}
	}

	CSaveRestoreBuffer *m_pBuffer;
};

class CHostRestore : public IRestore
{
public:
	explicit CHostRestore( CSaveRestoreBuffer *pBuffer ) : m_pBuffer( pBuffer ) {}

	virtual int GetReadPos() const { return m_pBuffer->ReadPos(); }
	virtual void SetReadPos( int pos ) { m_pBuffer->SetReadPos( pos ); }

	virtual int ReadAll( void *pLeafObject, datamap_t *pLeafMap )
	{
		VisitDataMap( pLeafMap, pLeafObject, [this]( typedescription_t *pField, void *pData )
		{
			ReadField( pField, pData );
		} );
		return 1;
	}
	virtual int ReadFields( const char *, void *pBaseData, datamap_t *, typedescription_t *pFields, int fieldcount = 1 )
	{
		for ( int i = 0; i < fieldcount; i++ )
			ReadField( &pFields[i], (char *)pBaseData + pFields[i].fieldOffset[TD_OFFSET_NORMAL] );
		return fieldcount;
	}
	virtual void EmptyFields( void *, typedescription_t *, int = 1 ) {}

	virtual void StartBlock( SaveRestoreRecordHeader_t * ) {}
	virtual void StartBlock( char[SIZE_BLOCK_NAME_BUF] ) {}
	virtual void StartBlock() {}
	virtual void EndBlock() {}

	virtual void ReadHeader( SaveRestoreRecordHeader_t * ) { m_pBuffer->Error(); }
	virtual int SkipHeader() { m_pBuffer->Error(); return 0; }
	virtual const char *StringFromHeaderSymbol( int ) { return ""; }

	virtual short ReadShort( void ) { short value = 0; m_pBuffer->Read( &value, sizeof( value ) ); return value; }
	virtual int ReadShort( short *pValue, int count = 1, int = 0 ) { return m_pBuffer->Read( pValue, sizeof( short ) * count ) ? count : 0; }
	virtual int ReadInt( int *pValue, int count = 1, int = 0 ) { return m_pBuffer->Read( pValue, sizeof( int ) * count ) ? count : 0; }
	virtual int ReadInt( void ) { int value = 0; m_pBuffer->Read( &value, sizeof( value ) ); return value; }
	virtual int ReadBool( bool *pValue, int count = 1, int = 0 ) { return m_pBuffer->Read( pValue, sizeof( bool ) * count ) ? count : 0; }
	virtual int ReadFloat( float *pValue, int count = 1, int = 0 ) { return m_pBuffer->Read( pValue, sizeof( float ) * count ) ? count : 0; }
	virtual int ReadData( char *pData, int size, int ) { return m_pBuffer->Read( pData, size ) ? size : 0; }
	virtual void ReadString( char *pDest, int nSizeDest, int nBytesAvailable )
	{
		// A known length reads exactly that many bytes; otherwise up to NUL.
		int i = 0;
		for ( ;; )
		{
			char c = 0;
			if ( nBytesAvailable > 0 && i >= nBytesAvailable )
				break;
			if ( !m_pBuffer->Read( &c, 1 ) )
				break;
			if ( i < nSizeDest )
				pDest[i] = c;
			i++;
			if ( nBytesAvailable <= 0 && c == 0 )
				break;
		}
		if ( nSizeDest > 0 )
			pDest[MIN( i, nSizeDest - 1 )] = 0;
	}
	virtual int ReadString( string_t *, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }
	virtual int ReadVector( Vector *pValue ) { return ReadVector( pValue, 1, 0 ); }
	virtual int ReadVector( Vector *pValue, int count = 1, int = 0 ) { return m_pBuffer->Read( pValue, sizeof( Vector ) * count ) ? count : 0; }
	virtual int ReadQuaternion( Quaternion *pValue ) { return ReadQuaternion( pValue, 1, 0 ); }
	virtual int ReadQuaternion( Quaternion *pValue, int count = 1, int = 0 ) { return m_pBuffer->Read( pValue, sizeof( Quaternion ) * count ) ? count : 0; }
	virtual int ReadTime( float *pValue, int count = 1, int = 0 ) { return ReadFloat( pValue, count ); }
	virtual int ReadTick( int *pValue, int count = 1, int = 0 ) { return ReadInt( pValue, count ); }
	virtual int ReadPositionVector( Vector *pValue ) { return ReadVector( pValue, 1, 0 ); }
	virtual int ReadPositionVector( Vector *pValue, int count = 1, int = 0 ) { return ReadVector( pValue, count ); }
	virtual int ReadFunction( datamap_t *, inputfunc_t **, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }

	virtual int ReadEntityPtr( CBaseEntity **, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }
	virtual int ReadEdictPtr( edict_t **, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }
	virtual int ReadEHandle( EHANDLE *, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }
	virtual int ReadVMatrix( VMatrix *, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }
	virtual int ReadVMatrixWorldspace( VMatrix *, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }
	virtual int ReadMatrix3x4Worldspace( matrix3x4_t *, int = 1, int = 0 ) { m_pBuffer->Error(); return 0; }

	virtual bool GetPrecacheMode( void ) { return false; }
	virtual CGameSaveRestoreInfo *GetGameSaveRestoreInfo() { return NULL; }

private:
	void ReadField( typedescription_t *pField, void *pData )
	{
		switch ( pField->fieldType )
		{
		case FIELD_EMBEDDED:
		{
			int stride = pField->fieldSize > 0 ? pField->fieldSizeInBytes / pField->fieldSize : 0;
			for ( int i = 0; i < pField->fieldSize; i++ )
				ReadAll( (char *)pData + i * stride, pField->td );
			break;
		}
		case FIELD_CUSTOM:
		{
			SaveRestoreFieldInfo_t info = { pData, NULL, pField };
			pField->pSaveRestoreOps->Restore( info, this );
			break;
		}
		case FIELD_STRING:
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
		case FIELD_EHANDLE:
		case FIELD_CLASSPTR:
		case FIELD_EDICT:
		case FIELD_FUNCTION:
		case FIELD_INPUT:
			m_pBuffer->Error();
			break;
		default:
			m_pBuffer->Read( pData, pField->fieldSizeInBytes );
			break;
		}
	}

	CSaveRestoreBuffer *m_pBuffer;
};

#endif // SAVERESTORE_HOST_H
