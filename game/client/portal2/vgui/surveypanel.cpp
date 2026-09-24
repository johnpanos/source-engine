//========= Portal 2 reconstruction ============================================//
//
// Purpose: Playtest survey panel and its question widgets.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "surveypanel.h"
#include "baseviewport.h"
#include "clientmode_shared.h"
#include "iclientmode.h"
#include "filesystem.h"
#include "c_portal_player.h"
#include "c_portal_gamestats.h"
#include <time.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/PanelListPanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static const char * const g_szUnansweredQuestion = "unanswered";

ConVar cl_disable_survey_panel( "cl_disable_survey_panel", "0", FCVAR_REPLICATED | FCVAR_ARCHIVE );
ConVar cl_survey_panel_dont_submit_answers( "cl_survey_panel_dont_submit_answers", "0", 0 );

//-----------------------------------------------------------------------------
// Purpose: Tracks how long the local client has been in the current map
//-----------------------------------------------------------------------------
class CTimeInMapHelper : public CAutoGameSystem
{
	virtual const char *Name( void ) { return "TimeInMapHelper"; }

	virtual bool Init( void )
	{
		m_szCurMapName[0] = '\0';
		return true;
	}

	virtual void LevelInitPreEntity( void )
	{
		// Only restart the clock when the map actually changes (not on reloads)
		if ( V_strncmp( engine->GetLevelName(), m_szCurMapName, sizeof( m_szCurMapName ) ) )
		{
			m_flTimeMapStart = Plat_FloatTime();
			V_strncpy( m_szCurMapName, engine->GetLevelName(), sizeof( m_szCurMapName ) );
		}
	}

public:
	float TimeSpentInCurrentMap( void ) { return Plat_FloatTime() - m_flTimeMapStart; }

protected:
	float m_flTimeMapStart;
	char m_szCurMapName[MAX_PATH];
};

static CTimeInMapHelper g_TimeInMapHelper;

void CSurveyQuestion::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBgColor( Color( 50, 50, 50, 255 ) );
}

//-----------------------------------------------------------------------------
// Purpose: Free-form text answer
//-----------------------------------------------------------------------------
class CEssayQuestion : public CSurveyQuestion
{
	static const int ESSAY_QUESTION_MAX_CHARS = 8192;

	DECLARE_CLASS_SIMPLE( CEssayQuestion, CSurveyQuestion );

	CEssayQuestion( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
	{
		m_pQuestionText = new Label( this, "EssayQuestionText", "Essay Question Text" );
		m_pAnswer = new TextEntry( this, "Answer" );
		m_pAnswer->SetWrap( true );
		m_pAnswer->SetMultiline( true );
		m_pAnswer->SetCatchEnterKey( true );
		m_pAnswer->SetMaximumCharCount( ESSAY_QUESTION_MAX_CHARS );

		m_questiontype = SVQ_ESSAY;

		LoadControlSettings( "resource/UI/essayquestion.res" );
	}

	virtual ~CEssayQuestion()
	{
		delete m_pQuestionText;
		delete m_pAnswer;
	}

	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		m_pAnswer->SetFgColor( pScheme->GetColor( "DullWhite", Color( 255, 255, 255, 255 ) ) );
		m_pAnswer->SetBgColor( pScheme->GetColor( "Black", Color( 0, 0, 0, 255 ) ) );

		BaseClass::ApplySchemeSettings( pScheme );
	}

	bool SetData( KeyValues *pKeys );

	virtual void GetQuestionText( char *pBuffOut, int iBuffLen ) { m_pQuestionText->GetText( pBuffOut, iBuffLen ); }
	virtual void GetAnswerText( char *pBuffOut, int iBuffLen ) { m_pAnswer->GetText( pBuffOut, iBuffLen ); }

protected:
	Label *m_pQuestionText;
	TextEntry *m_pAnswer;
};

bool CEssayQuestion::SetData( KeyValues *pKeys )
{
	for ( KeyValues *pQuestionData = pKeys->GetFirstSubKey(); pQuestionData; pQuestionData = pQuestionData->GetNextKey() )
	{
		if ( !V_stricmp( "text", pQuestionData->GetName() ) )
		{
			m_pQuestionText->SetText( pQuestionData->GetString() );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Pick one of a list of choices
//-----------------------------------------------------------------------------
class CRadioChoicesQuestion : public CSurveyQuestion
{
	static const int MAX_RADIO_BUTTONS = 10;
	static const int RADIO_BUTTON_CHOICE_HEIGHT = 24;

	DECLARE_CLASS_SIMPLE( CRadioChoicesQuestion, CSurveyQuestion );

	CRadioChoicesQuestion( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
	{
		m_pQuestionText = new Label( this, "RadioChoicesQuestionText", "Radio Choices Question Text" );
		SetSize( parent->GetWide(), 64 );
		LoadControlSettings( "resource/UI/radiochoicesquestion.res" );

		m_questiontype = SVQ_RADIOCHOICES;
		m_iNumRadioButtons = 0;
		V_memset( m_pRadioButtons, 0, sizeof( m_pRadioButtons ) );
	}

	virtual ~CRadioChoicesQuestion()
	{
		delete m_pQuestionText;

		for ( int i = 0; i < MAX_RADIO_BUTTONS; ++i )
		{
			delete m_pRadioButtons[i];
		}
	}

	bool SetData( KeyValues *pKeys );

	virtual void GetQuestionText( char *pBuffOut, int iBuffLen ) { m_pQuestionText->GetText( pBuffOut, iBuffLen ); }
	virtual void GetAnswerText( char *pBuffOut, int iBuffLen );

protected:
	Label *m_pQuestionText;
	RadioButton *m_pRadioButtons[MAX_RADIO_BUTTONS];
	int m_iNumRadioButtons;
};

bool CRadioChoicesQuestion::SetData( KeyValues *pKeys )
{
	for ( KeyValues *pQuestionData = pKeys->GetFirstSubKey(); pQuestionData; pQuestionData = pQuestionData->GetNextKey() )
	{
		if ( !V_stricmp( "text", pQuestionData->GetName() ) )
		{
			m_pQuestionText->SetText( pQuestionData->GetString() );
		}
		else if ( !V_stricmp( "choice", pQuestionData->GetName() ) && m_iNumRadioButtons < MAX_RADIO_BUTTONS )
		{
			// Line the choices up under the question text
			int iLeftPadding = 16, iDummy;
			m_pQuestionText->GetPos( iLeftPadding, iDummy );

			m_pRadioButtons[m_iNumRadioButtons] = new RadioButton( this, "radiobutton", pQuestionData->GetString() );
			m_pRadioButtons[m_iNumRadioButtons]->SetPos( iLeftPadding, ( m_iNumRadioButtons + 1 ) * RADIO_BUTTON_CHOICE_HEIGHT );
			m_pRadioButtons[m_iNumRadioButtons]->SetSize( GetParent()->GetWide(), RADIO_BUTTON_CHOICE_HEIGHT );
			++m_iNumRadioButtons;
		}
	}

	SetTall( m_iNumRadioButtons * RADIO_BUTTON_CHOICE_HEIGHT + 48 );
	InvalidateLayout( true, true );

	return true;
}

void CRadioChoicesQuestion::GetAnswerText( char *pBuffOut, int iBuffLen )
{
	for ( int i = 0; i < m_iNumRadioButtons; ++i )
	{
		if ( m_pRadioButtons[i] && m_pRadioButtons[i]->IsSelected() )
		{
			m_pRadioButtons[i]->GetText( pBuffOut, iBuffLen );
			return;
		}
	}

	V_strncpy( pBuffOut, g_szUnansweredQuestion, iBuffLen );
}

//-----------------------------------------------------------------------------
// Purpose: Rate something from one to ten
//-----------------------------------------------------------------------------
class COneToTenQuestion : public CSurveyQuestion
{
	static const int MAX_RADIO_BUTTONS = 10;
	static const int RADIO_BUTTON_TALL = 48;

	DECLARE_CLASS_SIMPLE( COneToTenQuestion, CSurveyQuestion );

	COneToTenQuestion( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
	{
		m_pQuestionText = new Label( this, "OneToTenQuestionText", "Question Text" );
		m_pLabelAtOne = new Label( this, "LabelAtOne", "least label" );
		m_pLabelAtTen = new Label( this, "LabelAtTen", "most label" );

		m_questiontype = SVQ_ONE_TO_TEN;

		LoadControlSettings( "resource/UI/onetotenquestion.res" );

		// Spread the buttons evenly across the list width, along the bottom
		int iWide = parent->GetWide();
		int xpos = 0;
		for ( int i = 0; i < MAX_RADIO_BUTTONS; ++i )
		{
			char szNum[3];
			V_snprintf( szNum, sizeof( szNum ), "%d", i + 1 );
			m_pRadioButtons[i] = new RadioButton( this, "radiobutton", szNum );
			m_pRadioButtons[i]->SetPos( xpos, GetTall() - RADIO_BUTTON_TALL );
			xpos += iWide / MAX_RADIO_BUTTONS;
		}
	}

	virtual ~COneToTenQuestion()
	{
		delete m_pQuestionText;
		delete m_pLabelAtOne;
		delete m_pLabelAtTen;

		for ( int i = 0; i < MAX_RADIO_BUTTONS; ++i )
		{
			delete m_pRadioButtons[i];
		}
	}

	bool SetData( KeyValues *pKeys );

	virtual void GetQuestionText( char *pBuffOut, int iBuffLen ) { m_pQuestionText->GetText( pBuffOut, iBuffLen ); }
	virtual void GetAnswerText( char *pBuffOut, int iBuffLen );

protected:
	RadioButton *m_pRadioButtons[MAX_RADIO_BUTTONS];
	Label *m_pQuestionText;
	Label *m_pLabelAtOne;
	Label *m_pLabelAtTen;
};

bool COneToTenQuestion::SetData( KeyValues *pKeys )
{
	for ( KeyValues *pQuestionData = pKeys->GetFirstSubKey(); pQuestionData; pQuestionData = pQuestionData->GetNextKey() )
	{
		if ( !V_stricmp( "text", pQuestionData->GetName() ) )
		{
			m_pQuestionText->SetText( pQuestionData->GetString() );
		}
		else if ( !V_stricmp( "labelatone", pQuestionData->GetName() ) )
		{
			m_pLabelAtOne->SetText( pQuestionData->GetString() );
		}
		else if ( !V_stricmp( "labelatten", pQuestionData->GetName() ) )
		{
			m_pLabelAtTen->SetText( pQuestionData->GetString() );
		}
	}

	return true;
}

void COneToTenQuestion::GetAnswerText( char *pBuffOut, int iBuffLen )
{
	bool bFound = false;
	for ( int i = 0; i < MAX_RADIO_BUTTONS; ++i )
	{
		if ( m_pRadioButtons[i] && m_pRadioButtons[i]->IsSelected() )
		{
			m_pRadioButtons[i]->GetText( pBuffOut, iBuffLen );
			bFound = true;
			break;
		}
	}

	if ( !bFound )
	{
		V_strncpy( pBuffOut, g_szUnansweredQuestion, iBuffLen );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSurveyPanel::CSurveyPanel( IViewPort *pViewport ) : Frame( NULL, PANEL_SURVEY )
{
	id = -1;
	version = -1;

	m_pViewPort = pViewport;

	m_pSubmitButton = new Button( this, "submit", "" );
	m_pQuestionsList = new PanelListPanel( this, "questionlist" );
	//m_pQuestionsList->SetShowScrollBar( false );
	m_pQuestionsList->SetFirstColumnWidth( 0 );
	m_pQuestionsList->RequestFocus( 0 );

	SetMoveable( false );
	SetSizeable( false );
	SetTitleBarVisible( false );
	SetProportional( true );

	LoadControlSettings( "resource/UI/surveypanel.res" );
}

CSurveyPanel::~CSurveyPanel()
{
	m_pQuestionsList->DeleteAllItems();
	delete m_pSubmitButton;
}

//-----------------------------------------------------------------------------
// Purpose: Build the question widgets from a survey script
//-----------------------------------------------------------------------------
void CSurveyPanel::SetData( KeyValues *data )
{
	V_strncpy( m_szSurveyName, data->GetName(), sizeof( m_szSurveyName ) );
	id = data->GetInt( "id", -1 );
	version = data->GetInt( "version", -1 );

	for ( KeyValues *pQuestionEntry = data->GetFirstTrueSubKey(); pQuestionEntry; pQuestionEntry = pQuestionEntry->GetNextTrueSubKey() )
	{
		if ( !V_stricmp( "onetoten", pQuestionEntry->GetName() ) )
		{
			COneToTenQuestion *pOneToTen = new COneToTenQuestion( m_pQuestionsList, "onetotenquestion" );
			pOneToTen->SetData( pQuestionEntry );
			m_pQuestionsList->AddItem( NULL, pOneToTen );
		}
		else if ( !V_stricmp( "essay", pQuestionEntry->GetName() ) )
		{
			CEssayQuestion *pEssayQuestion = new CEssayQuestion( m_pQuestionsList, "essayquestion" );
			pEssayQuestion->SetData( pQuestionEntry );
			m_pQuestionsList->AddItem( NULL, pEssayQuestion );
		}
		else if ( !V_stricmp( "radiochoices", pQuestionEntry->GetName() ) )
		{
			// Reconstruction note: both 2010 builds name this panel "essayquestion".
			CRadioChoicesQuestion *pRadioChoices = new CRadioChoicesQuestion( m_pQuestionsList, "essayquestion" );
			pRadioChoices->SetData( pQuestionEntry );
			m_pQuestionsList->AddItem( NULL, pRadioChoices );
		}
	}

	InvalidateLayout( true, true );
}

void CSurveyPanel::Reset()
{
	m_pQuestionsList->DeleteAllItems();
}

void CSurveyPanel::Update()
{
}

bool CSurveyPanel::NeedsUpdate( void )
{
	return false;
}

void CSurveyPanel::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Update();
		SetVisible( true );
		MoveToFront();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		Reset();

		// Let the point_survey entity fire its completion output
		SendSurveyDoneCmd();
	}

	m_pSubmitButton->SetVisible( bShow );
	m_pQuestionsList->SetVisible( bShow );
}

void CSurveyPanel::SendSurveyDoneCmd( void )
{
	CUtlString strCmd;
	int nEntityIndex = m_hPointSurveyEntity.Get() ? m_hPointSurveyEntity->entindex() : -1;
	strCmd.Format( "survey_done %i", nEntityIndex );
	engine->ClientCmd( strCmd );
}

//-----------------------------------------------------------------------------
// Purpose: Append one CSV row with the answers to the given file
//-----------------------------------------------------------------------------
bool CSurveyPanel::WriteSurveyDataToFile( const char *file )
{
	if ( !file )
		return false;

	FileHandle_t handle = g_pFullFileSystem->Open( file, "a" );
	if ( !g_pFullFileSystem->IsOk( handle ) )
	{
		Warning( "Failed to write to survey file '%s'\n", file );
		return false;
	}

	tm t;
	Plat_GetLocalTime( &t );

	char pszMap[64];
	V_FileBase( engine->GetLevelName(), pszMap, sizeof( pszMap ) );

	if ( id == -1 || version == -1 )
	{
		// Reconstruction note: both 2010 builds return here without closing the handle.
		Warning( "No survey id or version information specified for survey '%s'. Not submitting data.\n", m_szSurveyName );
		return false;
	}

	g_pFullFileSystem->FPrintf( handle, "%i,%i,", id, version );

	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
	g_pFullFileSystem->FPrintf( handle, "\"%s\",\"%s\",%i-%02i-%02i,%02i:%02i:%02i,%f",
		pPlayer->GetPlayerName(), pszMap, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
		g_TimeInMapHelper.TimeSpentInCurrentMap() );

	for ( int i = m_pQuestionsList->FirstItem(); i != m_pQuestionsList->InvalidItemID(); i = m_pQuestionsList->NextItem( i ) )
	{
		CSurveyQuestion *pQuestion = static_cast< CSurveyQuestion * >( m_pQuestionsList->GetItemPanel( i ) );

		char szAnswerBuff[MAX_PATH];
		pQuestion->GetAnswerText( szAnswerBuff, sizeof( szAnswerBuff ) );

		char szOutputBuff[8192];
		V_snprintf( szOutputBuff, sizeof( szOutputBuff ), ",\"%s\"", szAnswerBuff );
		Msg( "%s", szOutputBuff );

		// Reconstruction note: the 2010 builds pass szOutputBuff as the format
		// string; answers are free text, so it is written through "%s".
		g_pFullFileSystem->FPrintf( handle, "%s", szOutputBuff );
	}

	g_pFullFileSystem->FPrintf( handle, "\n" );
	g_pFullFileSystem->Close( handle );

	return true;
}

void CSurveyPanel::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "submit" ) )
	{
		if ( !cl_survey_panel_dont_submit_answers.GetInt() )
		{
			// The standard playtest survey is difficulty, enjoyment, then comments
			if ( m_pQuestionsList->GetItemCount() > 2 )
			{
				char szDifficulty[MAX_PATH];
				char szEnjoyment[MAX_PATH];
				char szEssay[MAX_PATH];
				V_memset( szDifficulty, 0, sizeof( szDifficulty ) );
				V_memset( szEnjoyment, 0, sizeof( szEnjoyment ) );
				V_memset( szEssay, 0, sizeof( szEssay ) );

				static_cast< CSurveyQuestion * >( m_pQuestionsList->GetItemPanel( 0 ) )->GetAnswerText( szDifficulty, sizeof( szDifficulty ) );
				static_cast< CSurveyQuestion * >( m_pQuestionsList->GetItemPanel( 1 ) )->GetAnswerText( szEnjoyment, sizeof( szEnjoyment ) );
				static_cast< CSurveyQuestion * >( m_pQuestionsList->GetItemPanel( 2 ) )->GetAnswerText( szEssay, sizeof( szEssay ) );

#if !defined( _GAMECONSOLE ) && !defined( NO_STEAM )
				g_PortalGameStats.Event_AddSurveyData( atoi( szDifficulty ), atoi( szEnjoyment ), szEssay );
#endif
			}

			char pszFileName[256];
			char pszMap[64];
			tm t;
			Plat_GetLocalTime( &t );
			V_FileBase( engine->GetLevelName(), pszMap, sizeof( pszMap ) );

			V_snprintf( pszFileName, sizeof( pszFileName ), "%s_%s_%s_%i_%02i_%02i_%02i_%02i_%02i.csv",
				m_szSurveyName, pszMap, C_BasePlayer::GetLocalPlayer()->GetPlayerName(),
				t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );

			// One file per submission on the playtest share...
			char pszFilePath[512];
			V_snprintf( pszFilePath, sizeof( pszFilePath ), "\\\\fileserver\\user\\portal2\\surveys\\%s\\", m_szSurveyName );
			g_pFullFileSystem->CreateDirHierarchy( pszFilePath, NULL );
			V_strncat( pszFilePath, pszFileName, sizeof( pszFilePath ) );
			WriteSurveyDataToFile( pszFilePath );

			// ...and a running local log per survey
			V_snprintf( pszFileName, sizeof( pszFileName ), "%s.csv", m_szSurveyName );
			V_snprintf( pszFilePath, sizeof( pszFilePath ), "scripts\\surveys\\" );
			g_pFullFileSystem->CreateDirHierarchy( pszFilePath, NULL );
			V_strncat( pszFilePath, pszFileName, sizeof( pszFilePath ) );
			WriteSurveyDataToFile( pszFilePath );
		}
	}
	else if ( V_stricmp( command, "close" ) )
	{
		return;
	}

	gViewPortInterface->ShowPanel( PANEL_SURVEY, false );
}

CON_COMMAND( cl_show_portal_puzzle_survey, "Shows the survey panel.\nNote that filename must be located in the 'scripts\\surveys' folder and the .txt extention is added automatically\n" )
{
	if ( args.ArgC() != 2 )
	{
		Msg( "Usage: cl_show_portal_puzzle_survey <filename>\n" );
		return;
	}

	const char *pszSurveyName = args[1];
	ShowSurveyPanel( pszSurveyName );
}

bool ShowSurveyPanel( const char *pszSurveyName )
{
	if ( !gViewPortInterface || cl_disable_survey_panel.GetInt() )
		return false;

	CBaseViewport *pViewport = dynamic_cast< CBaseViewport * >( GetClientMode()->GetViewport() );
	if ( pViewport )
	{
		IViewPortPanel *pSurveyPanel = pViewport->FindPanelByName( PANEL_SURVEY );
		if ( !pSurveyPanel )
		{
			pSurveyPanel = pViewport->CreatePanelByName( PANEL_SURVEY );
			pViewport->AddNewPanel( pSurveyPanel, "PANEL_SURVEY" );
		}

		if ( pSurveyPanel )
		{
			char pszFilePath[MAX_PATH];
			V_snprintf( pszFilePath, sizeof( pszFilePath ), "scripts/surveys/%s.txt", pszSurveyName );

			KeyValues *pKV = new KeyValues( pszSurveyName );
			if ( pKV->LoadFromFile( g_pFullFileSystem, pszFilePath, "GAME" ) )
			{
				pSurveyPanel->Reset();
				pSurveyPanel->SetData( pKV );
				pKV->deleteThis();

				gViewPortInterface->ShowPanel( PANEL_SURVEY, true );
				return true;
			}

			// Reconstruction note: both 2010 builds leak pKV on this path.
			Warning( "cl_show_portal_puzzle_survey: Error loading '%s.txt' from GAME\\scripts\\surveys directory\n", pszSurveyName );
			return false;
		}
	}

	Warning( "cl_show_portal_puzzle_survey: Failed to create CSurveyPanel!\n" );
	return false;
}
