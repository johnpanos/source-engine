//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OPTIONS_SUB_TOUCH_H
#define OPTIONS_SUB_TOUCH_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CCvarNegateCheckButton;
class CKeyToggleCheckButton;
class CCvarToggleCheckButton;
class CCvarSlider;
class CLabeledCommandComboBox;

namespace vgui
{
    class Label;
    class Panel;
}

//-----------------------------------------------------------------------------
// Purpose: Touch Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubTouch : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COptionsSubTouch, vgui::PropertyPage );

public:
	COptionsSubTouch(vgui::Panel *parent);
	~COptionsSubTouch();

	virtual void OnResetData();
	virtual void OnApplyChanges();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	MESSAGE_FUNC_PTR( OnControlModified, "ControlModified", panel );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel )
	{
		OnControlModified( panel );
	}

	void UpdateLabel(CCvarSlider *slider, vgui::TextEntry *label);
	void LayoutWithoutResource();

private:
	CCvarToggleCheckButton		*m_pReverseTouchCheckBox;
	CCvarToggleCheckButton		*m_pTouchFilterCheckBox;
	CCvarToggleCheckButton		*m_pTouchRawCheckBox;
	CCvarToggleCheckButton		*m_pTouchAccelerationCheckBox;

	CCvarToggleCheckButton		*m_pTouchCheckBox;
	CCvarToggleCheckButton		*m_pTouchSouthpawCheckBox;
	CCvarToggleCheckButton		*m_pQuickInfoCheckBox;
	CCvarToggleCheckButton		*m_pTouchEnableCheckBox;
	CCvarToggleCheckButton		*m_pTouchDrawCheckBox;

	CCvarSlider					*m_pTouchSensitivitySlider;
	vgui::TextEntry             *m_pTouchSensitivityLabel;

	CCvarSlider					*m_pTouchAccelExponentSlider;
	vgui::TextEntry             *m_pTouchAccelExponentLabel;

	CCvarSlider					*m_pTouchYawSensitivitySlider;
	vgui::Label					*m_pTouchYawSensitivityPreLabel;
	CCvarSlider					*m_pTouchPitchSensitivitySlider;
	vgui::Label					*m_pTouchPitchSensitivityPreLabel;
	vgui::TextEntry					*m_pTouchYawSensitivityLabel;
	vgui::TextEntry					*m_pTouchPitchSensitivityLabel;

	vgui::Label *m_pTouchSensitivityPreLabel;
	vgui::Label *m_pTouchAccelExponentPreLabel;

	vgui::Label *m_pGyroModePreLabel;
	CLabeledCommandComboBox *m_pGyroModeCombo;
	vgui::Label *m_pGyroAxisPreLabel;
	CLabeledCommandComboBox *m_pGyroAxisCombo;
	vgui::Label *m_pGyroSensitivityPreLabel;
	CCvarSlider *m_pGyroSensitivitySlider;
	vgui::TextEntry *m_pGyroSensitivityLabel;
	CCvarToggleCheckButton *m_pGyroInvertPitchCheckBox;
};



#endif // OPTIONS_SUB_TOUCH_H
