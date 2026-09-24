//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side copy of func_noportal_volume for predicted portal placement
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_func_noportal_volume.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_FuncNoPortalVolume *C_FuncNoPortalVolume::s_pClassList = NULL;

IMPLEMENT_CLIENTCLASS_DT( C_FuncNoPortalVolume, DT_FuncNoPortalVolume, CFuncNoPortalVolume )
	RecvPropBool( RECVINFO( m_bActive ) ),
END_RECV_TABLE()


C_FuncNoPortalVolume::C_FuncNoPortalVolume()
{
	// Add ourselves to the head of the list
	m_pNext = s_pClassList;
	s_pClassList = this;
}


C_FuncNoPortalVolume::~C_FuncNoPortalVolume()
{
	// Unlink ourselves from the list
	if ( s_pClassList == this )
	{
		s_pClassList = m_pNext;
	}
	else
	{
		C_FuncNoPortalVolume *pWalk = s_pClassList;
		while ( pWalk->m_pNext != this )
		{
			pWalk = pWalk->m_pNext;
		}

		pWalk->m_pNext = m_pNext;
	}
}


CFuncNoPortalVolume *GetNoPortalVolumeList()
{
	return C_FuncNoPortalVolume::s_pClassList;
}
