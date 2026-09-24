//--------------------------------------------------------------------------------------------------------
// Copyright (c) 2007 Turtle Rock Studios, Inc. - All Rights Reserved
//
// Portal 2 port: fog_volume, from the CS:GO source drop; the repository's provenance and distribution warning applies.
// Fog controller switching is applied by CFogVolumeSystem below (CS:GO does it
// in CBasePlayer::UpdateFXVolume). Per-volume post-process and color-correction
// controllers are not applied by this engine's client and are reported once.

#ifndef FOG_VOLUME_H
#define FOG_VOLUME_H

#ifdef _WIN32
#pragma once
#endif


class CFogController;
class CPostProcessController;
class CColorCorrection;


//--------------------------------------------------------------------------------------------------------
// Fog volume entity
class CFogVolume : public CServerOnlyEntity
{
	DECLARE_CLASS( CFogVolume, CServerOnlyEntity );
	DECLARE_DATADESC();

public:
	CFogVolume();
	virtual ~CFogVolume();
	virtual void Spawn( void );
	virtual void Activate();

	static CFogVolume *FindFogVolumeForPosition( const Vector &position );

	const char *GetFogControllerName() const 
	{
		return STRING( m_fogName );
	}

	CFogController* GetFogController( ) const
	{
		return m_hFogController.Get();
	}

	CPostProcessController* GetPostProcessController( ) const
	{
		return m_hPostProcessController.Get();
	}

	CBaseEntity* GetColorCorrectionController( ) const
	{
		return m_hColorCorrectionController.Get();
	}

	void InputEnable( inputdata_t &data );
	void InputDisable( inputdata_t &data );

private:
	string_t m_fogName;
	string_t m_postProcessName;
	string_t m_colorCorrectionName;

	CHandle< CFogController > m_hFogController;
	CHandle< CPostProcessController > m_hPostProcessController;
	// Portal 2 port: CColorCorrection is private to colorcorrection.cpp here.
	EHANDLE m_hColorCorrectionController;

	bool m_bDisabled;
	bool m_bInFogVolumesList;

	void AddToGlobalList();
	void RemoveFromGlobalList();
};

extern CUtlVector< CFogVolume * > TheFogVolumes;


#endif // FOG_VOLUME_H