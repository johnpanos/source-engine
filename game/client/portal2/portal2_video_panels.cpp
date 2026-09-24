//========= Portal 2 port ======================================================//
//
// Purpose: The Portal 2 movie console commands the server's logic_playmovie and
//          CPortal_Player send (playvideo_nointerrupt,
//          playvideo_exitcommand_nointerrupt, playvideo_end_level_transition,
//          stopvideos, stopvideos_fadeout, stop_transition_videos_fadeout).
//
//          CS:GO's client vgui_video.cpp implements them on its own VideoPanel.
//          This client keeps the SDK 2013 VideoPanel (vgui_video.cpp, which
//          already provides playvideo and playvideo_exitcommand), so the Portal 2
//          playback options live in a subclass here: no interruption, looping,
//          fades and level-transition movies. Command behavior follows the CS:GO
//          source drop; the repository's provenance and distribution warning
//          applies.
//
//          A movie that cannot be opened (no video provider for its format)
//          still runs its exit command, so the map's OnPlaybackFinished output
//          fires as it would after the movie.
//
//=============================================================================//

#include "cbase.h"
#include "vgui_video.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum Portal2VideoInterrupt_t
{
	PORTAL2_VIDEO_NO_INTERRUPT = 0,
	PORTAL2_VIDEO_ALLOW_INTERRUPT,
	PORTAL2_VIDEO_ALLOW_INTERRUPT_DEV_ONLY,
};

class CPortal2VideoPanel;
static CUtlVector<CPortal2VideoPanel *> s_Portal2VideoPanels;

//-----------------------------------------------------------------------------
// A full-screen VideoPanel with the Portal 2 playback options.
//-----------------------------------------------------------------------------
class CPortal2VideoPanel : public VideoPanel
{
	DECLARE_CLASS_SIMPLE( CPortal2VideoPanel, VideoPanel );

public:
	CPortal2VideoPanel( unsigned int nWidth, unsigned int nHeight,
	    Portal2VideoInterrupt_t eInterrupt, float flFadeInTime, bool bLoop,
	    bool bIsTransitionVideo )
	    : BaseClass( 0, 0, nHeight, nWidth, true ), m_eInterrupt( eInterrupt ), m_bLoop( bLoop ),
	      m_bIsTransitionVideo( bIsTransitionVideo ), m_flFadeInTime( flFadeInTime ),
	      m_flFadeOutTime( 0.0f ), m_flFadeStart( 0.0f ), m_bFadingOut( false )
	{
		s_Portal2VideoPanels.AddToTail( this );
	}

	virtual ~CPortal2VideoPanel() { s_Portal2VideoPanels.FindAndRemove( this ); }

	bool IsTransitionVideo() const { return m_bIsTransitionVideo; }

	bool Start( const char *pFilename )
	{
		if ( !BeginPlayback( pFilename ) )
			return false;

		if ( m_bLoop && m_VideoMaterial )
		{
			m_VideoMaterial->SetLooping( true );
		}

		m_flFadeStart = gpGlobals->realtime;
		SetFadeAlpha( m_flFadeInTime > 0.0f ? 0 : 255 );
		vgui::ivgui()->AddTickSignal( GetVPanel() );
		DoModal();
		return true;
	}

	// Stops the movie now; OnClose runs the exit command, as in CS:GO.
	void StopPlayback()
	{
		if ( IsVisible() )
		{
			OnClose();
		}
	}

	void SetFadeOutTime( float flTime )
	{
		if ( m_bFadingOut )
			return;

		m_bFadingOut = true;
		m_flFadeOutTime = flTime;
		m_flFadeStart = gpGlobals->realtime;
	}

	virtual void OnKeyCodeTyped( vgui::KeyCode code )
	{
		if ( CanInterrupt() )
		{
			BaseClass::OnKeyCodeTyped( code );
		}
	}

	virtual void OnKeyCodePressed( vgui::KeyCode code )
	{
		if ( CanInterrupt() )
		{
			BaseClass::OnKeyCodePressed( code );
		}
	}

protected:
	virtual void OnTick()
	{
		BaseClass::OnTick();

		if ( !IsVisible() )
			return;

		float flElapsed = gpGlobals->realtime - m_flFadeStart;
		if ( m_bFadingOut )
		{
			float flFraction = ( m_flFadeOutTime > 0.0f ) ? flElapsed / m_flFadeOutTime : 1.0f;
			if ( flFraction >= 1.0f )
			{
				OnClose();
				return;
			}
			SetFadeAlpha( (int)( 255.0f * ( 1.0f - flFraction ) ) );
		}
		else if ( m_flFadeInTime > 0.0f )
		{
			SetFadeAlpha( (int)( 255.0f * MIN( 1.0f, flElapsed / m_flFadeInTime ) ) );
		}
	}

private:
	bool CanInterrupt() const
	{
		switch ( m_eInterrupt )
		{
		case PORTAL2_VIDEO_ALLOW_INTERRUPT:
			return true;
		case PORTAL2_VIDEO_ALLOW_INTERRUPT_DEV_ONLY:
		{
			static ConVarRef developer( "developer" );
			return developer.IsValid() && developer.GetInt() > 0;
		}
		default:
			return false;
		}
	}

	// VideoPanel::Paint draws the movie with the foreground alpha; the panel
	// alpha fades its black background with it.
	void SetFadeAlpha( int nAlpha )
	{
		nAlpha = clamp( nAlpha, 0, 255 );
		Color fg = GetFgColor();
		fg[3] = nAlpha;
		SetFgColor( fg );
		SetAlpha( nAlpha );
	}

	Portal2VideoInterrupt_t m_eInterrupt;
	bool m_bLoop;
	bool m_bIsTransitionVideo;
	float m_flFadeInTime;
	float m_flFadeOutTime;
	float m_flFadeStart;
	bool m_bFadingOut;
};

//-----------------------------------------------------------------------------
// Purpose: Take a raw filename and ensure it points to the correct directory and file extension
//-----------------------------------------------------------------------------
static void ComposeBinkFilename( const char *lpszFilename, char *lpszOut, int nOutSize )
{
	Q_strncpy( lpszOut, "media/", nOutSize ); // Assume we must play out of the media directory
	char strFilename[MAX_PATH];
	Q_StripExtension( lpszFilename, strFilename, MAX_PATH );
	Q_strncat( lpszOut, strFilename, nOutSize );
	Q_strncat( lpszOut, ".bik", nOutSize ); // Assume we're a .bik extension type
}

//-----------------------------------------------------------------------------
// Purpose: Create a video panel with the supplied commands
//-----------------------------------------------------------------------------
static void CreateVideoPanel( const char *lpszFilename, const char *lpszExitCommand, int nWidth,
    int nHeight, Portal2VideoInterrupt_t eInterrupt, float flFadeTime = 0, bool bLoop = false,
    bool bIsTransitionVideo = false )
{
	char strFullpath[MAX_PATH];
	ComposeBinkFilename( lpszFilename, strFullpath, sizeof( strFullpath ) );

	// Use the full screen size if they haven't specified an override
	unsigned int nScreenWidth = ( nWidth != 0 ) ? nWidth : ScreenWidth();
	unsigned int nScreenHeight = ( nHeight != 0 ) ? nHeight : ScreenHeight();

	CPortal2VideoPanel *pVideoPanel = new CPortal2VideoPanel(
	    nScreenWidth, nScreenHeight, eInterrupt, flFadeTime, bLoop, bIsTransitionVideo );
	pVideoPanel->SetExitCommand( lpszExitCommand );
	if ( pVideoPanel->Start( strFullpath ) )
		return;

	delete pVideoPanel;
	Warning( "Unable to play video: %s\n", strFullpath );

	// Portal 2 port: finish immediately so the server's logic_playmovie still
	// fires OnPlaybackFinished (the SDK 2013 playvideo_exitcommand does the same).
	if ( lpszExitCommand && lpszExitCommand[0] )
	{
		engine->ClientCmd( lpszExitCommand );
	}
}

CON_COMMAND(
    playvideo_nointerrupt, "Plays a video without ability to skip: <filename> [width height]" )
{
	if ( args.ArgC() < 2 )
		return;

	unsigned int nScreenWidth = Q_atoi( args[2] );
	unsigned int nScreenHeight = Q_atoi( args[3] );

	CreateVideoPanel( args[1], NULL, nScreenWidth, nScreenHeight, PORTAL2_VIDEO_NO_INTERRUPT );
}

CON_COMMAND( playvideo_end_level_transition, "Plays a video fullscreen without ability to skip "
                                             "(unless dev 1) and fades in: <filename> <time>" )
{
	if ( args.ArgC() < 2 )
		return;

	float flTime = Q_atoi( args[2] );
	if ( flTime <= 0 )
	{
		Warning( "Fade time needs to be greater than zero! Setting to 0.1f\n" );
		flTime = 0.1f;
	}

	FOR_EACH_VEC( s_Portal2VideoPanels, itr )
	{
		if ( s_Portal2VideoPanels[itr]->IsTransitionVideo() )
		{
			// We're already playing a transition video... don't start another
			return;
		}
	}

	// CS:GO overlays the co-op partner's avatar ("partner in science") on these
	// transition movies; this client has no such overlay.
	static bool s_bWarned = false;
	if ( !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "Portal 2: the co-op partner overlay on transition movies is not supported by "
		            "this engine\n" );
	}

	CreateVideoPanel(
	    args[1], NULL, 0, 0, PORTAL2_VIDEO_ALLOW_INTERRUPT_DEV_ONLY, flTime, true, true );
}

CON_COMMAND( playvideo_exitcommand_nointerrupt,
    "Plays a video (without interruption) and fires and exit command when it is stopped or "
    "finishes: <filename> <exit command>" )
{
	if ( args.ArgC() < 2 )
		return;

	// Pull out the exit command we want to use
	const char *pExitCommand =
	    ( args.ArgC() > 2 ) ? Q_strstr( args.GetCommandString(), args[2] ) : NULL;

	CreateVideoPanel( args[1], pExitCommand, 0, 0, PORTAL2_VIDEO_NO_INTERRUPT );
}

CON_COMMAND( stopvideos, "Stops all videos playing to the screen" )
{
	for ( int i = s_Portal2VideoPanels.Count() - 1; i >= 0; --i )
	{
		s_Portal2VideoPanels[i]->StopPlayback();
	}
}

CON_COMMAND( stopvideos_fadeout, "Fades out all videos playing to the screen: <time>" )
{
	if ( args.ArgC() < 1 )
		return;

	float flTime = Q_atoi( args[1] );
	if ( flTime <= 0 )
	{
		Warning( "Fade time needs to be greater than zero!  Setting to 0.1f\n" );
		flTime = 0.1f;
	}

	FOR_EACH_VEC( s_Portal2VideoPanels, itr )
	{
		s_Portal2VideoPanels[itr]->SetFadeOutTime( flTime );
	}
}

CON_COMMAND( stop_transition_videos_fadeout,
    "Fades out all transition videos playing to the screen: <time>" )
{
	if ( args.ArgC() < 1 )
		return;

	float flTime = Q_atoi( args[1] );
	if ( flTime <= 0 )
	{
		Warning( "Fade time needs to be greater than zero!  Setting to 0.1f\n" );
		flTime = 0.1f;
	}

	FOR_EACH_VEC( s_Portal2VideoPanels, itr )
	{
		if ( s_Portal2VideoPanels[itr]->IsTransitionVideo() )
			s_Portal2VideoPanels[itr]->SetFadeOutTime( flTime );
	}
}

//-----------------------------------------------------------------------------
// Stops transition movies (CS:GO's VGui_ClearTransitionVideoPanels, called when
// the client disconnects).
//-----------------------------------------------------------------------------
void Portal2_ClearTransitionVideoPanels()
{
	for ( int i = s_Portal2VideoPanels.Count() - 1; i >= 0; --i )
	{
		if ( s_Portal2VideoPanels[i]->IsTransitionVideo() )
		{
			s_Portal2VideoPanels[i]->StopPlayback();
		}
	}
}
