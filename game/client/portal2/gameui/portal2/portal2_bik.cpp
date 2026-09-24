//========= Portal 2 port ======================================================//
//
// Purpose: Bink movie API for the Portal 2 GameUI on IVideoServices; see
//          portal2_bik.h.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_bik.h"
#include "video/ivideoservices.h"
#include "cdll_client_int.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CPortal2BinkPlayer s_Portal2BinkPlayer;
CPortal2BinkPlayer *g_pBIK = &s_Portal2BinkPlayer;

IVideoMaterial *CPortal2BinkPlayer::Lookup( BIKMaterial_t hMaterial ) const
{
	if ( hMaterial == BIKHANDLE_INVALID || !m_Materials.IsValidIndex( hMaterial ) )
		return NULL;
	return m_Materials[hMaterial];
}

BIKMaterial_t CPortal2BinkPlayer::CreateMaterial( const char *pMaterialName, const char *pFileName, const char *pPathID, int nBikFlags )
{
	if ( !g_pVideo )
	{
		static bool s_bWarned = false;
		if ( !s_bWarned )
		{
			s_bWarned = true;
			DevWarning( "Portal 2: movies are not supported without video services\n" );
		}
		return BIKHANDLE_INVALID;
	}

	int nPlaybackFlags = VideoPlaybackFlags::DEFAULT_MATERIAL_OPTIONS;
	if ( nBikFlags & BIK_LOOP )
		nPlaybackFlags |= VideoPlaybackFlags::LOOP_VIDEO;
	if ( nBikFlags & BIK_PRELOAD )
		nPlaybackFlags |= VideoPlaybackFlags::PRELOAD_VIDEO;

	IVideoMaterial *pVideoMaterial = g_pVideo->CreateVideoMaterial( pMaterialName, pFileName, pPathID,
		(VideoPlaybackFlags_t)nPlaybackFlags, VideoSystem::DETERMINE_FROM_FILE_EXTENSION, true );
	if ( !pVideoMaterial )
	{
		DevWarning( "Portal 2: no video provider can play %s\n", pFileName );
		return BIKHANDLE_INVALID;
	}

	int nSlot = m_Materials.Find( NULL );
	if ( nSlot == m_Materials.InvalidIndex() )
	{
		if ( m_Materials.Count() >= BIKHANDLE_INVALID )
		{
			g_pVideo->DestroyVideoMaterial( pVideoMaterial );
			return BIKHANDLE_INVALID;
		}
		nSlot = m_Materials.AddToTail( NULL );
	}
	m_Materials[nSlot] = pVideoMaterial;
	return (BIKMaterial_t)nSlot;
}

void CPortal2BinkPlayer::DestroyMaterial( BIKMaterial_t hMaterial )
{
	IVideoMaterial *pVideoMaterial = Lookup( hMaterial );
	if ( !pVideoMaterial )
		return;

	m_Materials[hMaterial] = NULL;
	if ( g_pVideo )
		g_pVideo->DestroyVideoMaterial( pVideoMaterial );
}

bool CPortal2BinkPlayer::Update( BIKMaterial_t hMaterial )
{
	IVideoMaterial *pVideoMaterial = Lookup( hMaterial );
	if ( !pVideoMaterial )
		return false;

	pVideoMaterial->Update();
	return pVideoMaterial->IsLooping() || !pVideoMaterial->IsFinishedPlaying();
}

bool CPortal2BinkPlayer::ReadyForSwap( BIKMaterial_t hMaterial )
{
	IVideoMaterial *pVideoMaterial = Lookup( hMaterial );
	return pVideoMaterial != NULL && pVideoMaterial->IsVideoPlaying();
}

IMaterial *CPortal2BinkPlayer::GetMaterial( BIKMaterial_t hMaterial )
{
	IVideoMaterial *pVideoMaterial = Lookup( hMaterial );
	return pVideoMaterial ? pVideoMaterial->GetMaterial() : NULL;
}

void CPortal2BinkPlayer::GetTexCoordRange( BIKMaterial_t hMaterial, float *pMaxU, float *pMaxV )
{
	IVideoMaterial *pVideoMaterial = Lookup( hMaterial );
	if ( !pVideoMaterial )
	{
		*pMaxU = *pMaxV = 0.0f;
		return;
	}
	pVideoMaterial->GetVideoTexCoordRange( pMaxU, pMaxV );
}

void CPortal2BinkPlayer::GetFrameSize( BIKMaterial_t hMaterial, int *pWidth, int *pHeight )
{
	IVideoMaterial *pVideoMaterial = Lookup( hMaterial );
	if ( !pVideoMaterial )
	{
		*pWidth = *pHeight = 0;
		return;
	}
	pVideoMaterial->GetVideoImageSize( pWidth, pHeight );
}
