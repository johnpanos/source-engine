//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SOUNDEMITTERSYSTEMBASE_H
#define SOUNDEMITTERSYSTEMBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "SoundEmitterSystem/isoundemittersystementries.h"
#include "soundflags.h"
#include "interval.h"
#include "UtlSortVector.h"
#include <tier1/utlstring.h>
#include <tier1/utlhashtable.h>

soundlevel_t TextToSoundLevel( const char *key );

struct CSoundEntry
{
	CSoundEntry() : m_nScriptFileIndex( 0 ), m_bRemoved( false ), m_bIsOverride( false ),
		m_pOperatorsKV( NULL ), m_nSoundEntryVersion( 1 )
	{
	}
	~CSoundEntry();

	CUtlConstString				m_Name;
	CSoundParametersInternal	m_SoundParams;
	uint16						m_nScriptFileIndex;
	bool						m_bRemoved : 1;
	bool						m_bIsOverride : 1;
	// Owned copy of the entry's "operator_stacks" block (NULL for none).
	KeyValues					*m_pOperatorsKV;
	int							m_nSoundEntryVersion;

	// Replaces the operator stacks and entry version from a script entry.
	void						SetEntryVersionData( KeyValues *pEntryKV );

	bool						IsOverride() const
	{
		return m_bIsOverride;
	}
};

struct CSoundEntryHashFunctor : CaselessStringHashFunctor
{
	using CaselessStringHashFunctor::operator();
	unsigned int operator()( CSoundEntry *e ) const
	{
		return CaselessStringHashFunctor::operator()( e->m_Name.Get() );
	}
};

struct CSoundEntryEqualFunctor : CaselessStringEqualFunctor
{
	using CaselessStringEqualFunctor::operator();
	bool operator()( CSoundEntry *lhs, CSoundEntry *rhs ) const
	{
		return CaselessStringEqualFunctor::operator()( lhs->m_Name.Get(), rhs->m_Name.Get() );
	}
	bool operator()( CSoundEntry *lhs, const char *rhs ) const
	{
		return CaselessStringEqualFunctor::operator()( lhs->m_Name.Get(), rhs );
	}
};



//-----------------------------------------------------------------------------
// Purpose: Base class for sound emitter system handling (can be used by tools)
//-----------------------------------------------------------------------------
class CSoundEmitterSystemBase : public ISoundEmitterSystemBase, public ISoundEmitterSystemEntries
{
public:
	CSoundEmitterSystemBase();
	virtual ~CSoundEmitterSystemBase() { }

	// Methods of IAppSystem
	virtual bool Connect( CreateInterfaceFn factory );
	virtual void Disconnect();
	virtual void *QueryInterface( const char *pInterfaceName );
	virtual InitReturnVal_t Init();
	virtual void Shutdown();

public:

	virtual bool ModInit();
	virtual void ModShutdown();

	virtual int	GetSoundIndex( const char *pName ) const;
	virtual bool IsValidIndex( int index );
	virtual int GetSoundCount( void );

	virtual const char *GetSoundName( int index );
	virtual bool GetParametersForSound( const char *soundname, CSoundParameters& params, gender_t gender, bool isbeingemitted = false );
	virtual const char *GetWaveName( CUtlSymbol& sym );

	virtual CUtlSymbol AddWaveName( const char *name );

	virtual soundlevel_t LookupSoundLevel( const char *soundname );
	virtual const char *GetWavFileForSound( const char *soundname, char const *actormodel );
	virtual const char *GetWavFileForSound( const char *soundname, gender_t gender );

	virtual int		CheckForMissingWavFiles( bool verbose );
	virtual const char *GetSourceFileForSound( int index ) const;
	// Iteration methods
	virtual int		First() const;
	virtual int		Next( int i ) const;
	virtual int		InvalidIndex() const;

	virtual CSoundParametersInternal *InternalGetParametersForSound( int index );


	// The host application is responsible for dealing with dirty sound scripts, etc.
	virtual bool		AddSound( const char *soundname, const char *scriptfile, const CSoundParametersInternal& params );
	virtual void		RemoveSound( const char *soundname );

	virtual void		MoveSound( const char *soundname, const char *newscript );

	virtual void		RenameSound( const char *soundname, const char *newname );
	virtual void		UpdateSoundParameters( const char *soundname, const CSoundParametersInternal& params );

	virtual int			GetNumSoundScripts() const;
	virtual char const	*GetSoundScriptName( int index ) const;
	virtual bool		IsSoundScriptDirty( int index ) const;
	virtual int			FindSoundScript( const char *name ) const;

	virtual void		SaveChangesToSoundScript( int scriptindex );

	virtual void		ExpandSoundNameMacros( CSoundParametersInternal& params, char const *wavename );
	virtual gender_t	GetActorGender( char const *actormodel );
	virtual void		GenderExpandString( char const *actormodel, char const *in, char *out, int maxlen );
	virtual void		GenderExpandString( gender_t gender, char const *in, char *out, int maxlen );
	virtual bool		IsUsingGenderToken( char const *soundname );
	virtual unsigned int GetManifestFileTimeChecksum();

	virtual bool			GetParametersForSoundEx( const char *soundname, HSOUNDSCRIPTHANDLE& handle, CSoundParameters& params, gender_t gender, bool isbeingemitted = false );
	virtual soundlevel_t	LookupSoundLevelByHandle( char const *soundname, HSOUNDSCRIPTHANDLE& handle );


	// Called from both client and server (single player) or just one (server only in dedicated server and client only if connected to a remote server)
	// Called by LevelInitPreEntity to override sound scripts for the mod with level specific overrides based on custom mapnames, etc.
	virtual void			AddSoundOverrides( char const *scriptfile, bool bPreload = false );

	// Called by either client or server in LevelShutdown to clear out custom overrides
	virtual void			ClearSoundOverrides();

	virtual void		ReloadSoundEntriesInList( IFileList *pFilesToReload );

	// Called by either client or server to force ModShutdown and ModInit
	virtual void			Flush();

	// ISoundEmitterSystemEntries (VSoundEmitterEntries001): version 2 entries.
	virtual KeyValues		*GetOperatorKVByHandle( HSOUNDSCRIPTHASH &handle );
	virtual char const		*GetSoundNameForHash( HSOUNDSCRIPTHASH hash ) const;
	virtual int				GetSoundIndexForHash( HSOUNDSCRIPTHASH hash ) const;
	virtual HSOUNDSCRIPTHASH HashSoundName( char const *pchSndName ) const;
	virtual bool			IsValidHash( HSOUNDSCRIPTHASH hash ) const;
	virtual int				GetSoundEntryVersion( int index ) const;

private:
	// The hash map is derived from m_Sounds; every change to the set of
	// entries (or their names) marks it stale and the next lookup rebuilds it.
	void	InvalidateHashMap() { m_bHashMapStale = true; }
	void	RebuildHashMap() const;

	bool InternalModInit();
	void InternalModShutdown();

	void AddSoundsFromFile( const char *filename, bool bPreload, bool bIsOverride = false, bool bRefresh = false );

	bool		InitSoundInternalParameters( const char *soundname, KeyValues *kv, CSoundParametersInternal& params );

	void LoadGlobalActors();

	float	TranslateAttenuation( const char *key );
	soundlevel_t	TranslateSoundLevel( const char *key );
	int TranslateChannel( const char *name );

	int		FindBestSoundForGender( SoundFile *pSoundnames, int c, gender_t gender );
	void	EnsureAvailableSlotsForGender( SoundFile *pSoundnames, int c, gender_t gender );
	void	AddSoundName( CSoundParametersInternal& params, char const *wavename, gender_t gender );

	CUtlHashtable< CUtlConstString, gender_t, CaselessStringHashFunctor, UTLConstStringCaselessStringEqualFunctor<char> > m_ActorGenders;
	CUtlStableHashtable< CSoundEntry*, empty_t, CSoundEntryHashFunctor, CSoundEntryEqualFunctor, uint16, const char* > m_Sounds;

    CUtlVector< CSoundEntry * >			m_SavedOverrides; 
	CUtlVector< FileNameHandle_t >				m_OverrideFiles;

	struct CSoundScriptFile
	{
		FileNameHandle_t hFilename;
		bool		dirty;
	};

	CUtlVector< CSoundScriptFile >	m_SoundKeyValues;
	int					m_nInitCount;
	unsigned int		m_uManifestPlusScriptChecksum;

	CUtlSymbolTable		m_Waves;

	mutable CUtlHashtable< HSOUNDSCRIPTHASH, UtlHashHandle_t > m_HashToSound;
	mutable bool		m_bHashMapStale;
};

#endif // SOUNDEMITTERSYSTEMBASE_H
