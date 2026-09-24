//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "OptionsSubTouch.h"
//#include "CommandCheckButton.h"
#include "KeyToggleCheckButton.h"
#include "CvarNegateCheckButton.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "filesystem.h"

#include "EngineInterface.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include "tier1/convar.h"
#include <stdio.h>
#include <vgui_controls/TextEntry.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

COptionsSubTouch::COptionsSubTouch(vgui::Panel *parent) : PropertyPage(parent, NULL)
{
	m_pTouchEnableCheckBox = new CCvarToggleCheckButton(this,
		"EnableTouch",
		"Enable touch",
		"touch_enable");

	m_pTouchDrawCheckBox = new CCvarToggleCheckButton(this,
		"DrawTouch",
		"Draw touch",
		"touch_draw");

	m_pReverseTouchCheckBox = new CCvarToggleCheckButton(
		this,
		"ReverseTouch",
		"Reverse touch",
		"touch_reverse" );

	m_pTouchFilterCheckBox = new CCvarToggleCheckButton(
		this,
		"TouchFilter",
		"Touch filter",
		"touch_filter" );

	m_pTouchAccelerationCheckBox = new CCvarToggleCheckButton(
		this,
		"TouchAccelerationCheckbox",
		"Touch acceleration",
		"touch_enable_accel" );

	m_pTouchSensitivitySlider = new CCvarSlider( this, "Slider", "Touch sensitivity",
		0.1f, 6.0f, "touch_sensitivity", true );

	m_pTouchSensitivityLabel = new TextEntry(this, "SensitivityLabel");
	m_pTouchSensitivityLabel->AddActionSignalTarget(this);

	m_pTouchAccelExponentSlider = new CCvarSlider( this, "TouchAccelerationSlider", "Touch acceleration",
		1.0f, 1.5f, "touch_accel", true );

	m_pTouchAccelExponentLabel = new TextEntry(this, "TouchAccelerationLabel");
	m_pTouchAccelExponentLabel->AddActionSignalTarget(this);

	m_pTouchYawSensitivitySlider = new CCvarSlider( this, "TouchYawSlider", "#GameUI_JoystickYawSensitivity",
		50.f, 300.f, "touch_yaw", true );
	m_pTouchYawSensitivityPreLabel = new Label(this, "TouchYawSensitivityPreLabel", "#GameUI_JoystickLookSpeedYaw" );
	m_pTouchYawSensitivityLabel = new TextEntry(this, "TouchYawSensitivityLabel");
	m_pTouchYawSensitivityLabel->AddActionSignalTarget(this);

	m_pTouchPitchSensitivitySlider = new CCvarSlider( this, "TouchPitchSlider", "#GameUI_JoystickPitchSensitivity",
		50.f, 300.f, "touch_pitch", true );
	m_pTouchPitchSensitivityPreLabel = new Label(this, "TouchPitchSensitivityPreLabel", "#GameUI_JoystickLookSpeedPitch" );
	m_pTouchPitchSensitivityLabel = new TextEntry(this, "TouchPitchSensitivityLabel");
	m_pTouchPitchSensitivityLabel->AddActionSignalTarget(this);

	m_pTouchSensitivityPreLabel =
	    new Label( this, "TouchSensitivityPreLabel", "Touch sensitivity" );
	m_pTouchAccelExponentPreLabel =
	    new Label( this, "TouchAccelerationPreLabel", "Touch acceleration" );

	// Gyro aiming (touch_gyro* in game/client/touch.cpp)
	m_pGyroModePreLabel = new Label( this, "GyroModePreLabel", "Gyro aiming" );
	m_pGyroModeCombo = new CLabeledCommandComboBox( this, "GyroMode" );
	m_pGyroModeCombo->AddItem( "Off", "touch_gyro 0" );
	m_pGyroModeCombo->AddItem( "While touching the look area", "touch_gyro 1" );
	m_pGyroModeCombo->AddItem( "Always", "touch_gyro 2" );
	m_pGyroModeCombo->AddActionSignalTarget( this );

	m_pGyroAxisPreLabel = new Label( this, "GyroAxisPreLabel", "Gyro turning" );
	m_pGyroAxisCombo = new CLabeledCommandComboBox( this, "GyroAxis" );
	m_pGyroAxisCombo->AddItem( "Turn the device", "touch_gyro_axis 0" );
	m_pGyroAxisCombo->AddItem( "Steer like a wheel", "touch_gyro_axis 1" );
	m_pGyroAxisCombo->AddItem( "Turn and steer", "touch_gyro_axis 2" );
	m_pGyroAxisCombo->AddItem( "Player space (recommended)", "touch_gyro_axis 3" );
	m_pGyroAxisCombo->AddActionSignalTarget( this );

	m_pGyroSensitivityPreLabel = new Label( this, "GyroSensitivityPreLabel", "Gyro sensitivity" );
	m_pGyroSensitivitySlider = new CCvarSlider( this, "GyroSensitivitySlider", "Gyro sensitivity",
	    0.25f, 4.0f, "touch_gyro_sensitivity", true );
	m_pGyroSensitivityLabel = new TextEntry( this, "GyroSensitivityLabel" );
	m_pGyroSensitivityLabel->AddActionSignalTarget( this );

	m_pGyroInvertPitchCheckBox = new CCvarToggleCheckButton(
	    this, "GyroInvertPitch", "Invert gyro pitch", "touch_gyro_invert_pitch" );

	LoadControlSettings("Resource\\OptionsSubTouch.res");
	if ( !g_pFullFileSystem->FileExists( "resource/OptionsSubTouch.res" ) )
		LayoutWithoutResource();

	UpdateLabel(m_pTouchSensitivitySlider, m_pTouchSensitivityLabel);
	UpdateLabel(m_pTouchAccelExponentSlider, m_pTouchAccelExponentLabel);
	UpdateLabel(m_pTouchYawSensitivitySlider, m_pTouchYawSensitivityLabel);
	UpdateLabel(m_pTouchPitchSensitivitySlider, m_pTouchPitchSensitivityLabel);
	UpdateLabel( m_pGyroSensitivitySlider, m_pGyroSensitivityLabel );
}

static void PlaceControl( Panel *pPanel, int x, int y, int wide, int tall )
{
	pPanel->SetBounds( x, y, wide, tall );
	// Without a pin offset, a page resize moves code-placed children to (0,0).
	pPanel->SetPinCorner( Panel::PIN_TOPLEFT, x, y );
}

// A caption, the slider, and the value entry to its right. Returns the next row.
static int PlaceSliderRow( Panel *pCaption, Panel *pSlider, Panel *pValue, int x, int y )
{
	PlaceControl( pCaption, x, y, 220, 16 );
	PlaceControl( pSlider, x, y + 16, 164, 26 );
	PlaceControl( pValue, x + 172, y + 18, 48, 22 );
	return y + 46;
}

static int PlaceComboRow( Panel *pCaption, Panel *pCombo, int x, int y )
{
	PlaceControl( pCaption, x, y, 220, 16 );
	PlaceControl( pCombo, x, y + 16, 220, 24 );
	return y + 46;
}

//-----------------------------------------------------------------------------
// Purpose: No game ships Resource/OptionsSubTouch.res, so without one the page
//			places its own controls: touch options in the left column, look
//			speeds and gyro aiming in the right one.
//-----------------------------------------------------------------------------
void COptionsSubTouch::LayoutWithoutResource()
{
	const int nLeftX = 16;
	const int nRightX = 256;

	int y = 8;
	Panel *checkBoxes[] = { m_pTouchEnableCheckBox, m_pTouchDrawCheckBox, m_pReverseTouchCheckBox,
	    m_pTouchFilterCheckBox, m_pTouchAccelerationCheckBox };
	for ( int i = 0; i < ARRAYSIZE( checkBoxes ); ++i )
	{
		PlaceControl( checkBoxes[i], nLeftX, y, 220, 22 );
		y += 22;
	}
	y += 6;
	y = PlaceSliderRow( m_pTouchSensitivityPreLabel, m_pTouchSensitivitySlider,
	    m_pTouchSensitivityLabel, nLeftX, y );
	PlaceSliderRow( m_pTouchAccelExponentPreLabel, m_pTouchAccelExponentSlider,
	    m_pTouchAccelExponentLabel, nLeftX, y );

	y = 8;
	y = PlaceSliderRow( m_pTouchYawSensitivityPreLabel, m_pTouchYawSensitivitySlider,
	    m_pTouchYawSensitivityLabel, nRightX, y );
	y = PlaceSliderRow( m_pTouchPitchSensitivityPreLabel, m_pTouchPitchSensitivitySlider,
	    m_pTouchPitchSensitivityLabel, nRightX, y );
	y += 6;
	y = PlaceComboRow( m_pGyroModePreLabel, m_pGyroModeCombo, nRightX, y );
	y = PlaceComboRow( m_pGyroAxisPreLabel, m_pGyroAxisCombo, nRightX, y );
	y = PlaceSliderRow(
	    m_pGyroSensitivityPreLabel, m_pGyroSensitivitySlider, m_pGyroSensitivityLabel, nRightX, y );
	PlaceControl( m_pGyroInvertPitchCheckBox, nRightX, y, 220, 22 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
COptionsSubTouch::~COptionsSubTouch()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubTouch::OnResetData()
{
	m_pReverseTouchCheckBox->Reset();
	m_pTouchFilterCheckBox->Reset();
	m_pTouchSensitivitySlider->Reset();
	m_pTouchAccelExponentSlider->Reset();
	m_pTouchYawSensitivitySlider->Reset();
	m_pTouchPitchSensitivitySlider->Reset();
	m_pTouchAccelerationCheckBox->Reset();

	ConVarRef touch_gyro( "touch_gyro" );
	ConVarRef touch_gyro_axis( "touch_gyro_axis" );
	m_pGyroModeCombo->SetInitialItem( clamp( touch_gyro.GetInt(), 0, 2 ) );
	m_pGyroAxisCombo->SetInitialItem( clamp( touch_gyro_axis.GetInt(), 0, 3 ) );
	m_pGyroSensitivitySlider->Reset();
	m_pGyroInvertPitchCheckBox->Reset();
	UpdateLabel( m_pGyroSensitivitySlider, m_pGyroSensitivityLabel );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubTouch::OnApplyChanges()
{
	m_pReverseTouchCheckBox->ApplyChanges();
	m_pTouchFilterCheckBox->ApplyChanges();
	m_pTouchSensitivitySlider->ApplyChanges();
	m_pTouchAccelExponentSlider->ApplyChanges();
	m_pTouchYawSensitivitySlider->ApplyChanges();
	m_pTouchPitchSensitivitySlider->ApplyChanges();
	m_pTouchEnableCheckBox->ApplyChanges();
	m_pTouchDrawCheckBox->ApplyChanges();
	m_pTouchAccelerationCheckBox->ApplyChanges();

	m_pGyroModeCombo->ApplyChanges();
	m_pGyroAxisCombo->ApplyChanges();
	m_pGyroSensitivitySlider->ApplyChanges();
	m_pGyroInvertPitchCheckBox->ApplyChanges();
}

//-----------------------------------------------------------------------------
// Purpose: sets background color & border
//-----------------------------------------------------------------------------
void COptionsSubTouch::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubTouch::OnControlModified(Panel *panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));

	// the HasBeenModified() check is so that if the value is outside of the range of the
	// slider, it won't use the slider to determine the display value but leave the
	// real value that we determined in the constructor
	if (panel == m_pTouchSensitivitySlider && m_pTouchSensitivitySlider->HasBeenModified())
		UpdateLabel( m_pTouchSensitivitySlider, m_pTouchSensitivityLabel );
	else if (panel == m_pTouchAccelExponentSlider && m_pTouchAccelExponentSlider->HasBeenModified())
		UpdateLabel( m_pTouchAccelExponentSlider, m_pTouchAccelExponentLabel );
	else if (panel == m_pTouchYawSensitivitySlider && m_pTouchYawSensitivitySlider->HasBeenModified())
		UpdateLabel( m_pTouchYawSensitivitySlider, m_pTouchYawSensitivityLabel );
	else if (panel == m_pTouchPitchSensitivitySlider && m_pTouchPitchSensitivitySlider->HasBeenModified())
		UpdateLabel( m_pTouchPitchSensitivitySlider, m_pTouchPitchSensitivityLabel );
	else if ( panel == m_pGyroSensitivitySlider && m_pGyroSensitivitySlider->HasBeenModified() )
		UpdateLabel( m_pGyroSensitivitySlider, m_pGyroSensitivityLabel );
	else if (panel == m_pTouchAccelerationCheckBox)
	{
		m_pTouchAccelExponentSlider->SetEnabled(m_pTouchAccelerationCheckBox->IsSelected());
		m_pTouchAccelExponentLabel->SetEnabled(m_pTouchAccelerationCheckBox->IsSelected());
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubTouch::OnTextChanged(Panel *panel)
{
	if ( panel == m_pTouchSensitivityLabel )
	{
		char buf[64];
		m_pTouchSensitivityLabel->GetText(buf, 64);

		float fValue;
		int numParsed = sscanf(buf, "%f", &fValue);
		if ( ( numParsed == 1 ) && ( fValue >= 0.0f ) )
		{
			m_pTouchSensitivitySlider->SetSliderValue(fValue);
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
	}
	else if ( panel == m_pTouchAccelExponentLabel )
	{
		char buf[64];
		m_pTouchAccelExponentLabel->GetText(buf, 64);

		float fValue = (float) atof(buf);
		if (fValue >= 1.0)
		{
			m_pTouchAccelExponentSlider->SetSliderValue(fValue);
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
	}
	else if( panel == m_pTouchPitchSensitivityLabel )
	{
		char buf[64];
		m_pTouchPitchSensitivityLabel->GetText(buf, 64);

		float fValue = (float) atof(buf);
		if (fValue >= 1.0)
		{
			m_pTouchPitchSensitivitySlider->SetSliderValue(fValue);
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
	}
	else if ( panel == m_pGyroSensitivityLabel )
	{
		char buf[64];
		m_pGyroSensitivityLabel->GetText( buf, 64 );

		float fValue = (float)atof( buf );
		if ( fValue > 0.0f )
		{
			m_pGyroSensitivitySlider->SetSliderValue( fValue );
			PostActionSignal( new KeyValues( "ApplyButtonEnable" ) );
		}
	}
	else if( panel == m_pTouchYawSensitivityLabel )
	{
		char buf[64];
		m_pTouchYawSensitivityLabel->GetText(buf, 64);

		float fValue = (float) atof(buf);
		if (fValue >= 1.0)
		{
			m_pTouchYawSensitivitySlider->SetSliderValue(fValue);
			PostActionSignal(new KeyValues("ApplyButtonEnable"));
		}
	}
}

void COptionsSubTouch::UpdateLabel(CCvarSlider *slider, vgui::TextEntry *label)
{
	char buf[64];
	Q_snprintf(buf, sizeof( buf ), " %.2f", slider->GetSliderValue());
	label->SetText(buf);
}
