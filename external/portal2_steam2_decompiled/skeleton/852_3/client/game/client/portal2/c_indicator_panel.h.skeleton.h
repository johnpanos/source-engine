// DWARF declaration skeleton for game/client/portal2/c_indicator_panel.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_indicator_panel.h:12 sizeof=0xa6c (i386)
struct C_LabIndicatorPanel : public C_BaseEntity
{
public:
	bool IsEnabled();  // line 17
	float GetCountdownPercentage();  // line 18
	bool IsCountingDown();  // line 19
	bool IsCountdownTimer();  // line 20
	bool IsPaused();  // line 21
	bool IsChecked();  // line 22
	int GetPlayerPinged();  // line 23
private:
	bool m_bEnabled; // +0xa59  // line 26
	bool m_bStopped; // +0xa5a  // line 27
	bool m_bCountingDown; // +0xa5b  // line 28
	bool m_bIsCountdownTimer; // +0xa5c  // line 29
	bool m_bIsChecked; // +0xa5d  // line 30
	int m_iPlayerPinged; // +0xa60  // line 31
	float m_flTimerDuration; // +0xa64  // line 32
	float m_flTimerStart; // +0xa68  // line 33
};

// game/client/portal2/c_indicator_panel.h:12 (declaration)
~C_LabIndicatorPanel();

// game/client/portal2/c_indicator_panel.h:12 @0x5212b0 _ZN19C_LabIndicatorPanelD0Ev
C_LabIndicatorPanel::~C_LabIndicatorPanel()
{
}

// game/client/portal2/c_indicator_panel.h:12 @0x521320 _ZN19C_LabIndicatorPanelD1Ev
C_LabIndicatorPanel::~C_LabIndicatorPanel()
{
}

// game/client/portal2/c_indicator_panel.h:12 (declaration)
void C_LabIndicatorPanel();
