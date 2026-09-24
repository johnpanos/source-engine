//========= Portal 2 reconstruction ============================================//
//
// Purpose: Playtest survey panel and its question widgets.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef SURVEYPANEL_H
#define SURVEYPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>

namespace vgui
{
	class PanelListPanel;
	class Button;
}

//-----------------------------------------------------------------------------
// Purpose: Base class for one survey question
//-----------------------------------------------------------------------------
class CSurveyQuestion : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CSurveyQuestion, vgui::EditablePanel );

	CSurveyQuestion( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName ) {}

	enum questiontype_t
	{
		SVQ_ONE_TO_TEN = 0,
		SVQ_ESSAY,
		SVQ_RADIOCHOICES,

		SVQ_COUNT,
	};

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void GetQuestionText( char *pBuffOut, int iBuffLen ) {}
	virtual void GetAnswerText( char *pBuffOut, int iBuffLen ) {}

protected:
	questiontype_t m_questiontype;
};

//-----------------------------------------------------------------------------
// Purpose: Frame that shows a list of survey questions and records the answers
//-----------------------------------------------------------------------------
class CSurveyPanel : public vgui::Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CSurveyPanel, vgui::Frame );

public:
	CSurveyPanel( IViewPort *pViewport );
	virtual ~CSurveyPanel();

	// IViewPortPanel
	virtual const char *GetName( void ) { return PANEL_SURVEY; }
	virtual void SetData( KeyValues *data );
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	virtual bool WantsBackgroundBlurred( void ) { return false; }
	virtual GameActionSet_t GetPreferredActionSet() { return GAME_ACTION_SET_MENUCONTROLS; }

	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	virtual void OnCommand( const char *command );

	EHANDLE m_hPointSurveyEntity;
	void SendSurveyDoneCmd( void );

protected:
	bool WriteSurveyDataToFile( const char *file );

	IViewPort		*m_pViewPort;

	// Name of the survey script (scripts/surveys/<name>.txt)
	char			m_szSurveyName[MAX_PATH];

	// Survey id and version from the survey script; both must be present
	// before answers are written out.
	int id;
	int version;

	vgui::PanelListPanel *m_pQuestionsList;

	vgui::Button	*m_pSubmitButton;
};

// Loads scripts/surveys/<pszSurveyName>.txt into the survey panel and shows it.
// Returns false when surveys are disabled or the panel/script is unavailable.
bool ShowSurveyPanel( const char *pszSurveyName );

#endif // SURVEYPANEL_H
