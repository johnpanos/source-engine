//========= Copyright Valve Corporation, All rights reserved. ============//

// Execute the production dialog-list handlers with an SDK-free parent fake.
// EditablePanel may repost an unhandled navigation key to its default button;
// the fake supplies that behavior once, just as vgui_nav_lock_default_button does.
// portal2_video_input_test.py generates the include from the current source.
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>
#include <vector>

namespace vgui
{
using KeyCode = int;
}
using vgui::KeyCode;
using DWORD = unsigned int;

enum
{
	KEY_XBUTTON_A,
	KEY_XSTICK1_LEFT,
	KEY_XSTICK2_LEFT,
	KEY_XBUTTON_LEFT,
	KEY_XBUTTON_LEFT_SHOULDER,
	KEY_LEFT,
	KEY_XSTICK1_RIGHT,
	KEY_XSTICK2_RIGHT,
	KEY_XBUTTON_RIGHT,
	KEY_XBUTTON_RIGHT_SHOULDER,
	KEY_RIGHT,
	KEY_UP
};
static int GetJoystickForCode( KeyCode )
{
	return 0;
}
static int GetBaseButtonCode( KeyCode code )
{
	return code;
}
static int XBX_GetUserId( int slot )
{
	return slot;
}
static DWORD XBX_GetPrimaryUserId()
{
	return 0;
}
static int V_stricmp( const char *a, const char *b )
{
	return std::string( a ).compare( b );
}

struct TestString
{
	std::string value;
	TestString &operator=( const char *text )
	{
		value = text;
		return *this;
	}
	const char *Get() const { return value.c_str(); }
};
template <class T> class TestVector : public std::vector<T>
{
public:
	int Count() const { return int( this->size() ); }
	bool IsValidIndex( int index ) const { return index >= 0 && index < Count(); }
	void Purge() { this->clear(); }
	int AddToTail()
	{
		this->emplace_back();
		return Count() - 1;
	}
};

struct KeyValues
{
	std::string command;
	KeyValues( const char *, const char *, const char *value ) : command( value ) {}
	KeyValues( const char *, const char *, int ) {}
};

namespace BaseModUI
{
enum
{
	UISOUND_INVALID,
	UISOUND_CLICK
};
class CBaseModPanel
{
public:
	static CBaseModPanel &GetSingleton()
	{
		static CBaseModPanel panel;
		return panel;
	}
	int GetLastActiveUserId() const { return 0; }
	void SetLastActiveUserId( int ) {}
	void PlayUISound( int ) {}
};

class ParentButton
{
public:
	bool repost = false;
	int baseCalls = 0;
	int parentCalls = 0;
	std::vector<KeyCode> pending;
	void OnKeyCodePressed( KeyCode code )
	{
		++baseCalls;
		if ( repost )
		{
			// The parent's navigation lock allows one repost per input event.
			repost = false;
			pending.push_back( code );
		}
	}
	void CallParentFunction( KeyValues *message )
	{
		++parentCalls;
		delete message;
	}
};

class BaseModHybridButton : public ParentButton
{
public:
	using BaseClass = ParentButton;
	enum
	{
		USE_EVERYBODY,
		USE_PRIMARY,
		USE_SLOT0,
		USE_SLOT1,
		USE_SLOT2,
		USE_SLOT3,
		BUTTON_DEFAULT,
		BUTTON_LEFTINDIALOG,
		BUTTON_DIALOGLIST
	};
	enum ListSelectionChange_t
	{
		SELECT_PREV,
		SELECT_NEXT
	};
	struct DialogListItem_t
	{
		TestString m_String;
		TestString m_CommandString;
		bool m_bEnabled = true;
	};
	bool m_bOnlyActiveUser = false;
	bool m_bIgnoreButtonA = false;
	bool enabled = true;
	int m_iUsablePlayerIndex = USE_EVERYBODY;
	int m_nStyle = BUTTON_DIALOGLIST;
	int m_nDialogListCurrentIndex = 0;
	TestVector<DialogListItem_t> m_DialogListItems;
	std::vector<std::string> commands;
	bool IsEnabled() const { return enabled; }
	void PostActionSignal( KeyValues *message )
	{
		commands.push_back( message->command );
		delete message;
	}
	void OnKeyCodePressed( KeyCode code );
	void ChangeDialogListSelection( ListSelectionChange_t direction );
	void SetCurrentSelection( const char *text );
	const char *GetCurrentSelection();
	void Press( KeyCode code )
	{
		OnKeyCodePressed( code );
		while ( !pending.empty() )
		{
			KeyCode forwarded = pending.back();
			pending.pop_back();
			OnKeyCodePressed( forwarded );
		}
	}
};
}
using namespace BaseModUI;

#include "portal2_dialog_input_methods.inc"

static BaseModHybridButton MakeRow( const char *prefix, int choices, bool frontEnd )
{
	BaseModHybridButton row;
	row.repost = frontEnd;
	for ( int i = 0; i < choices; ++i )
	{
		int index = row.m_DialogListItems.AddToTail();
		std::string text = "choice" + std::to_string( i );
		std::string command = std::string( prefix ) + std::to_string( i );
		row.m_DialogListItems[index].m_String = text.c_str();
		row.m_DialogListItems[index].m_CommandString = command.c_str();
	}
	row.SetCurrentSelection( "choice1" );
	return row;
}

#define CHECK( expression )                                                                        \
	do                                                                                             \
	{                                                                                              \
		++checks;                                                                                  \
		if ( !( expression ) )                                                                     \
		{                                                                                          \
			++failures;                                                                            \
			std::fprintf( stderr, "check failed: %s at %d\n", #expression, __LINE__ );             \
		}                                                                                          \
	} while ( false )

int main()
{
	int checks = 0;
	int failures = 0;
	const struct
	{
		const char *prefix;
		int choices;
	} rows[] = { { "_coreao", 5 }, { "_coreshadows", 4 }, { "_coredepth", 2 }, { "_coremovers", 2 },
	    { "_coredirect", 2 } };
	for ( bool frontEnd : { false, true } )
	{
		for ( const auto &setting : rows )
		{
			auto row = MakeRow( setting.prefix, setting.choices, frontEnd );
			CHECK( row.commands.empty() );
			row.Press( KEY_LEFT );
			CHECK( std::string( row.GetCurrentSelection() ) == "choice0" );
			CHECK( row.commands.size() == 1 );
			CHECK( row.commands.back() == std::string( setting.prefix ) + "0" );
			CHECK( row.baseCalls == 0 );
			row.repost = frontEnd;
			row.Press( KEY_RIGHT );
			CHECK( std::string( row.GetCurrentSelection() ) == "choice1" );
			CHECK( row.commands.size() == 2 );
			CHECK( row.commands.back() == std::string( setting.prefix ) + "1" );
			row.SetCurrentSelection( "choice0" );
			CHECK( row.commands.size() == 2 );
			row.Press( KEY_LEFT );
			CHECK( row.m_nDialogListCurrentIndex == setting.choices - 1 );
		}
	}
	for ( KeyCode code : { KEY_XSTICK1_LEFT, KEY_XSTICK2_LEFT, KEY_XBUTTON_LEFT,
	          KEY_XBUTTON_LEFT_SHOULDER, KEY_XSTICK1_RIGHT, KEY_XSTICK2_RIGHT, KEY_XBUTTON_RIGHT,
	          KEY_XBUTTON_RIGHT_SHOULDER, KEY_XBUTTON_A } )
	{
		auto row = MakeRow( "_coredirect", 2, true );
		row.Press( code );
		CHECK( row.m_nDialogListCurrentIndex == 0 );
		CHECK( row.commands.size() == 1 && row.commands[0] == "_coredirect0" );
		CHECK( row.baseCalls == 0 );
	}
	// Unhandled navigation and the dialog's Apply key retain their parent route.
	auto row = MakeRow( "_coredirect", 2, false );
	row.Press( KEY_UP );
	CHECK( row.baseCalls == 1 && row.commands.empty() );
	row.m_bIgnoreButtonA = true;
	row.Press( KEY_XBUTTON_A );
	CHECK( row.parentCalls == 1 && row.commands.empty() );
	row.m_nStyle = BaseModHybridButton::BUTTON_DEFAULT;
	row.Press( KEY_LEFT );
	CHECK( row.baseCalls == 2 && row.commands.empty() );
	// Skip disabled choices; a list with no enabled alternative emits no command.
	row = MakeRow( "_coreao", 5, true );
	row.m_DialogListItems[0].m_bEnabled = false;
	row.Press( KEY_LEFT );
	CHECK( row.m_nDialogListCurrentIndex == 4 );
	CHECK( row.commands.size() == 1 && row.commands[0] == "_coreao4" );
	row = MakeRow( "_coredirect", 2, true );
	row.m_DialogListItems[0].m_bEnabled = false;
	row.Press( KEY_LEFT );
	CHECK( row.m_nDialogListCurrentIndex == 1 && row.commands.empty() );
	return testing::ReportConformance( checks, failures );
}
