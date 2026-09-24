//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side copy of func_noportal_volume for predicted portal placement
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_FUNC_NOPORTAL_VOLUME_H
#define C_FUNC_NOPORTAL_VOLUME_H
#ifdef _WIN32
#pragma once
#endif

class C_FuncNoPortalVolume : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_FuncNoPortalVolume, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_FuncNoPortalVolume();
	virtual ~C_FuncNoPortalVolume();

	bool m_bActive;
	bool IsActive( void ) { return m_bActive; }	// is this area currently blocking portals
	static C_FuncNoPortalVolume *s_pClassList;	// Every volume on the client, linked through m_pNext
	C_FuncNoPortalVolume *m_pNext;
};

typedef C_FuncNoPortalVolume CFuncNoPortalVolume;

CFuncNoPortalVolume *GetNoPortalVolumeList();

#endif // C_FUNC_NOPORTAL_VOLUME_H
