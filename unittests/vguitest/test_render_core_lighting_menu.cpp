//========= Copyright Valve Corporation, All rights reserved. ============//

// Execute the Portal 2 advanced video menu's render core lighting rows: exact
// slices of vadvancedvideo.cpp (layout, strings, commands, row state,
// navigation, descriptions) and of vhybridbutton.cpp (the list button's key,
// mouse, hover and selection handlers), against SDK-free fakes of VGUI. Each
// row is laid out from a resource tree, made a list button from its laid-out
// list, and driven by keyboard, controller and mouse as a player does. Also
// checks gameui/render_core_lighting_preset.h against the product profile.
// render_core_lighting_menu_test.py generates the includes.
#include "gameui/render_core_lighting_preset.h"
#include "testing/conformance_result.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

using gameui::kRenderCoreAOChoices;
using gameui::kRenderCoreShadowChoices;
using gameui::kRenderCoreToggleChoices;
using gameui::RenderCoreLighting;
using gameui::RenderCoreLightingPreset;
using gameui::RenderCoreLightingRow;

// ---------------------------------------------------------------------------
// Platform, string and input fakes.

using DWORD = unsigned int;
namespace vgui
{
using KeyCode = int;
using MouseCode = int;
using HFont = int;
constexpr HFont INVALID_FONT = 0;
}
using vgui::KeyCode;
enum
{
	KEY_XBUTTON_A = 1,
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
	KEY_UP,
	KEY_DOWN,
	KEY_ENTER,
};
enum
{
	MOUSE_LEFT = 100,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
};
#define ARRAYSIZE( a ) ( sizeof( a ) / sizeof( ( a )[0] ) )
static bool IsPC()
{
	return true;
}
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
	return strcasecmp( a, b );
}
static int V_strcmp( const char *a, const char *b )
{
	return std::strcmp( a, b );
}
static int V_strlen( const char *text )
{
	return int( std::strlen( text ) );
}
static bool StringHasPrefix( const char *text, const char *prefix )
{
	return std::strncmp( text, prefix, std::strlen( prefix ) ) == 0;
}
static void Q_wcsncpy( wchar_t *out, const wchar_t *in, int bytes )
{
	const int count = int( bytes / sizeof( wchar_t ) );
	int i = 0;
	for ( ; i + 1 < count && in[i]; ++i )
		out[i] = in[i];
	out[i] = 0;
}
// Only a list item with a %s parameter formats; these rows have none.
static int V_snwprintf( wchar_t *out, int count, const wchar_t *format, ... )
{
	Q_wcsncpy( out, format, int( count * sizeof( wchar_t ) ) );
	return int( std::char_traits<wchar_t>::length( out ) );
}
class CFmtStr
{
public:
	CFmtStr( const char *format, ... )
	{
		va_list args;
		va_start( args, format );
		std::vsnprintf( m_Text, sizeof( m_Text ), format, args );
		va_end( args );
	}
	operator const char *() const { return m_Text; }

private:
	char m_Text[256];
};

struct TestString
{
	std::string value;
	TestString &operator=( const char *text )
	{
		value = text ? text : "";
		return *this;
	}
	const char *Get() const { return value.c_str(); }
	bool IsEmpty() const { return value.empty(); }
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

// A resource tree: the subset of KeyValues the layout and the buttons use.
class KeyValues
{
public:
	explicit KeyValues( const char *name ) : m_Name( name ) {}
	KeyValues( const char *name, const char *key, const char *value ) : m_Name( name )
	{
		SetString( key, value );
	}
	KeyValues( const char *name, const char *key, int value ) : m_Name( name )
	{
		SetInt( key, value );
	}
	~KeyValues()
	{
		for ( KeyValues *child : m_Children )
			delete child;
	}
	const char *GetName() const { return m_Name.c_str(); }
	void SetName( const char *name ) { m_Name = name; }
	KeyValues *FindKey( const char *name, bool create = false )
	{
		for ( KeyValues *child : m_Children )
		{
			if ( !V_stricmp( child->GetName(), name ) )
				return child;
		}
		if ( !create )
			return nullptr;
		KeyValues *made = new KeyValues( name );
		AddSubKey( made );
		return made;
	}
	const char *GetString( const char *name = nullptr, const char *fallback = "" )
	{
		KeyValues *key = name ? FindKey( name ) : this;
		return key && key->m_HasValue ? key->m_Value.c_str() : fallback;
	}
	int GetInt( const char *name, int fallback = 0 )
	{
		KeyValues *key = FindKey( name );
		return key && key->m_HasValue ? std::atoi( key->m_Value.c_str() ) : fallback;
	}
	void SetString( const char *name, const char *value )
	{
		KeyValues *key = FindKey( name, true );
		key->m_Value = value;
		key->m_HasValue = true;
	}
	void SetInt( const char *name, int value )
	{
		SetString( name, std::to_string( value ).c_str() );
	}
	void AddSubKey( KeyValues *child )
	{
		child->m_Parent = this;
		m_Children.push_back( child );
	}
	void RemoveSubKey( KeyValues *child )
	{
		for ( auto it = m_Children.begin(); it != m_Children.end(); ++it )
		{
			if ( *it == child )
			{
				m_Children.erase( it );
				child->m_Parent = nullptr;
				return;
			}
		}
	}
	void deleteThis() { delete this; }
	KeyValues *MakeCopy() const
	{
		KeyValues *copy = new KeyValues( m_Name.c_str() );
		copy->m_Value = m_Value;
		copy->m_HasValue = m_HasValue;
		for ( KeyValues *child : m_Children )
			copy->AddSubKey( child->MakeCopy() );
		return copy;
	}
	KeyValues *GetFirstSubKey() { return m_Children.empty() ? nullptr : m_Children.front(); }
	KeyValues *GetNextKey() { return Sibling( false ); }
	KeyValues *GetFirstTrueSubKey()
	{
		for ( KeyValues *child : m_Children )
		{
			if ( !child->m_Children.empty() )
				return child;
		}
		return nullptr;
	}
	KeyValues *GetNextTrueSubKey() { return Sibling( true ); }

private:
	KeyValues *Sibling( bool withChildren )
	{
		if ( !m_Parent )
			return nullptr;
		std::vector<KeyValues *> &siblings = m_Parent->m_Children;
		bool after = false;
		for ( KeyValues *sibling : siblings )
		{
			if ( after && ( !withChildren || !sibling->m_Children.empty() ) )
				return sibling;
			after = after || sibling == this;
		}
		return nullptr;
	}
	std::string m_Name;
	std::string m_Value;
	bool m_HasValue = false;
	KeyValues *m_Parent = nullptr;
	std::vector<KeyValues *> m_Children;
};

// Localization: tokens with or without '#', English text the menu adds.
class FakeLocalize
{
public:
	wchar_t *Find( const char *token )
	{
		if ( token && token[0] == '#' )
			++token;
		auto found = m_Strings.find( token ? token : "" );
		return found == m_Strings.end() ? nullptr : found->second.data();
	}
	void AddString( const char *token, wchar_t *text, const char * )
	{
		m_Strings[token] = std::wstring( text );
		m_Strings[token].push_back( 0 );
	}
	void ConvertANSIToUnicode( const char *text, wchar_t *out, int bytes )
	{
		const int count = int( bytes / sizeof( wchar_t ) );
		int i = 0;
		for ( ; i + 1 < count && text[i]; ++i )
			out[i] = wchar_t( static_cast<unsigned char>( text[i] ) );
		out[i] = 0;
	}

private:
	std::map<std::string, std::wstring> m_Strings;
};
static FakeLocalize s_Localize;
static FakeLocalize *const g_pVGuiLocalize = &s_Localize;

// Text metrics: every glyph is 10 wide and 20 tall; the arrow glyph 12.
struct FakeSurface
{
	void GetTextSize( vgui::HFont font, const wchar_t *text, int &wide, int &tall )
	{
		wide = font == 2 ? 12 : int( std::char_traits<wchar_t>::length( text ) ) * 10;
		tall = 20;
	}
	int GetCharacterWidth( vgui::HFont, wchar_t ) { return 10; }
};
static FakeSurface s_Surface;
static FakeSurface *surface()
{
	return &s_Surface;
}
namespace vgui
{
using ::surface;
}

// The cursor, in screen coordinates.
struct FakeInput
{
	int x = 0, y = 0;
	void GetCursorPos( int &outX, int &outY )
	{
		outX = x;
		outY = y;
	}
};
static FakeInput s_Input;
static FakeInput *input()
{
	return &s_Input;
}

namespace vgui
{
struct IScheme
{
	const char *GetResourceString( const char *name )
	{
		return !std::strcmp( name, "Dialog.TileHeight" ) ? "10" : "";
	}
};
struct ISchemeManager
{
	IScheme scheme;
	IScheme *GetIScheme( int ) { return &scheme; }
};
static ISchemeManager *scheme()
{
	static ISchemeManager manager;
	return &manager;
}

// A panel: a name, a parent, focus and a screen position.
class Panel
{
public:
	virtual ~Panel() = default;
	const char *GetName() const { return m_PanelName.c_str(); }
	Panel *GetParent() const { return m_pParentPanel; }
	virtual void NavigateToChild( Panel * ) {}
	bool HasFocus() const;
	void RequestFocus( int );
	void ScreenToLocal( int &x, int &y ) const
	{
		x -= m_ScreenX;
		y -= m_ScreenY;
	}
	int GetWide() const { return m_Wide; }
	int GetTall() const { return m_Tall; }
	void OnMousePressed( vgui::MouseCode ) {}
	void OnCursorEntered() {}
	void NavigateTo() { RequestFocus( 0 ); }
	Panel *FindChildByName( const char * ) { return nullptr; }

	std::string m_PanelName;
	Panel *m_pParentPanel = nullptr;
	int m_ScreenX = 0, m_ScreenY = 0, m_Wide = 400, m_Tall = 20;
};
static Panel *s_pFocus = nullptr;
bool Panel::HasFocus() const
{
	return s_pFocus == this;
}
void Panel::RequestFocus( int )
{
	s_pFocus = this;
}

class Label : public Panel
{
public:
	std::string text;
	void SetText( const char *value ) { text = value; }
};
}
using vgui::Panel;

// ---------------------------------------------------------------------------
// The list button: its fields; the production methods are sliced in.

namespace BaseModUI
{
enum
{
	UISOUND_INVALID,
	UISOUND_CLICK,
	UISOUND_FOCUS
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
	void PlayUISound( int sound ) { lastSound = sound; }
	int lastSound = -1;
};

class FlyoutMenu : public Panel
{
public:
	static void CloseActiveMenu( Panel * ) { ++closes; }
	static int closes;
};
int FlyoutMenu::closes = 0;

class BaseModHybridButton;

// The button's parent route for keys a list does not handle: Up and Down
// move focus along the laid-out navUp/navDown names, as vgui::Panel does.
class ParentButton : public Panel
{
public:
	void OnKeyCodePressed( KeyCode code );
	void CallParentFunction( KeyValues *message ) { delete message; }
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
		TestString m_StringParm1;
		TestString m_CommandString;
		bool m_bEnabled = true;
	};
	bool m_bOnlyActiveUser = false;
	bool m_bIgnoreButtonA = false;
	bool m_isOpen = false;
	bool enabled = true;
	int m_iUsablePlayerIndex = USE_EVERYBODY;
	int m_nStyle = BUTTON_DIALOGLIST;
	int m_nDialogListCurrentIndex = 0;
	int m_textInsetX = 8;
	vgui::HFont m_hTextFont = 1;
	vgui::HFont m_hSymbolFont = 2;
	TestVector<DialogListItem_t> m_DialogListItems;
	KeyValues *m_pSettings = nullptr; // its laid-out resource keys
	bool IsEnabled() const { return enabled; }
	void PostActionSignal( KeyValues *message );
	void OnKeyCodePressed( vgui::KeyCode code );
	void ChangeDialogListSelection( ListSelectionChange_t eNext );
	void SetCurrentSelection( const char *pText );
	const char *GetCurrentSelection();
	void OnMousePressed( vgui::MouseCode code );
	bool GetDialogListButtonCenter( int &x, int &y );
	void OnCursorEntered();
};
}
using namespace BaseModUI;

#include "render_core_lighting_button.inc"

// ---------------------------------------------------------------------------
// The menu: its fields; the production methods are sliced in.

static bool HasRenderCoreQuality()
{
	return true;
}

class Menu : public Panel
{
public:
	using BaseClass = Panel;
	BaseModHybridButton *m_drpCorePreset = nullptr;
	BaseModHybridButton *m_drpCoreAO = nullptr;
	BaseModHybridButton *m_drpCoreShadows = nullptr;
	BaseModHybridButton *m_drpCoreDepth = nullptr;
	BaseModHybridButton *m_drpCoreMovers = nullptr;
	BaseModHybridButton *m_drpCorePcss = nullptr;
	BaseModHybridButton *m_drpCoreDirect = nullptr;
	vgui::Label titleLabel, descriptionLabel;
	vgui::Label *m_lblDescriptionTitle = &titleLabel;
	vgui::Label *m_lblDescription = &descriptionLabel;
	Panel *m_pLastDescriptionControl = nullptr;
	Panel *m_ActiveControl = nullptr;
	RenderCoreLighting m_CoreLighting;
	bool m_bDirtyValues = false;
	std::vector<BaseModHybridButton *> buttons;
	int GetScheme() const { return 0; }
	void LayOut( KeyValues *pResourceData );
	void SetRenderCoreQualityState();
	bool OnCommand( const char *command );
	void NavigateToChild( Panel *pNavigateTo ) override;
	void UpdateDescription( Panel *pControl );
	BaseModHybridButton *Find( const char *name )
	{
		for ( BaseModHybridButton *button : buttons )
		{
			if ( !V_stricmp( button->GetName(), name ) )
				return button;
		}
		return nullptr;
	}
};

void BaseModHybridButton::PostActionSignal( KeyValues *message )
{
	// The dialog is the action target: VGUI delivers the command to it.
	static_cast<Menu *>( GetParent() )->OnCommand( message->GetString( "command" ) );
	delete message;
}

void ParentButton::OnKeyCodePressed( KeyCode code )
{
	auto *self = static_cast<BaseModHybridButton *>( this );
	if ( ( code != KEY_UP && code != KEY_DOWN ) || !self->m_pSettings )
		return;
	Menu *menu = static_cast<Menu *>( GetParent() );
	BaseModHybridButton *next =
	    menu->Find( self->m_pSettings->GetString( code == KEY_UP ? "navUp" : "navDown" ) );
	if ( next )
	{
		next->RequestFocus( 0 );
		menu->NavigateToChild( next );
	}
}

#include "render_core_lighting_menu.inc"
#include "render_core_lighting_profile.inc"

// ---------------------------------------------------------------------------
// The test.

static const char *const kCoreRows[] = { "DrpCorePreset", "DrpCoreAO", "DrpCoreShadows",
    "DrpCoreDepth", "DrpCoreMovers", "DrpCorePcss", "DrpCoreDirect" };

// The shipped dialog's rows the layout builds on: two list rows looping
// into each other, the last of them the template, and the frame.
static KeyValues *ShippedResource()
{
	KeyValues *res = new KeyValues( "Resource" );
	KeyValues *frame = new KeyValues( "Frame" );
	frame->SetString( "ControlName", "Frame" );
	frame->SetInt( "tall", 10 );
	res->AddSubKey( frame );
	const char *rows[][3] = { { "DrpAntialias", "DrpModelDetail", "DrpModelDetail" },
	    { "DrpModelDetail", "DrpAntialias", "DrpAntialias" } };
	int y = 50;
	for ( const auto &row : rows )
	{
		KeyValues *key = new KeyValues( row[0] );
		key->SetString( "ControlName", "BaseModHybridButton" );
		key->SetString( "fieldName", row[0] );
		key->SetString( "style", "DialogListButton" );
		key->SetInt( "ypos", y );
		key->SetInt( "tall", 20 );
		key->SetString( "navUp", row[1] );
		key->SetString( "navDown", row[2] );
		KeyValues *list = key->FindKey( "list", true );
		list->SetString( "#GameUI_Low", "_template0" );
		list->SetString( "#GameUI_High", "_template1" );
		res->AddSubKey( key );
		y += 25;
	}
	return res;
}

// A list button made from its laid-out keys, as ApplySettings makes one.
static BaseModHybridButton *MakeButton( Menu &menu, KeyValues *key, int index )
{
	auto *button = new BaseModHybridButton;
	button->m_PanelName = key->GetName();
	button->m_pParentPanel = &menu;
	button->m_pSettings = key;
	button->m_ScreenY = 100 + 25 * index;
	if ( KeyValues *list = key->FindKey( "list" ) )
	{
		for ( KeyValues *item = list->GetFirstSubKey(); item; item = item->GetNextKey() )
		{
			const int i = button->m_DialogListItems.AddToTail();
			button->m_DialogListItems[i].m_String = item->GetName();
			button->m_DialogListItems[i].m_CommandString = item->GetString();
		}
	}
	menu.buttons.push_back( button );
	return button;
}

int main()
{
	int checks = 0, failures = 0;
	const auto check = [&]( bool passed, const std::string &name )
	{
		++checks;
		if ( !passed )
		{
			++failures;
			std::fprintf( stderr, "lighting menu: %s\n", name.c_str() );
		}
	};
	// The shipped localization's quality words; the menu adds the rest.
	for ( const auto &[token, text] : { std::pair{ "GameUI_Low", L"Low" },
	          std::pair{ "GameUI_Medium", L"Medium" }, std::pair{ "GameUI_High", L"High" } } )
		s_Localize.AddString( token, const_cast<wchar_t *>( text ), nullptr );

	const RenderCoreLighting low = gameui::PresetLighting( RenderCoreLightingPreset::kLow );
	const RenderCoreLighting high = gameui::PresetLighting( RenderCoreLightingPreset::kHigh );

	// --- The presets are the product profile's.
	check( low == kProfileLow, "Low preset equals the profile's low settings" );
	check( high == kProfileHigh, "High preset equals the profile's high settings" );
	check( low.runtimeDirect == 0, "Low draws baked direct light" );
	check( low.shadows == 0 && low.shadowMovers == 0, "Low has no shadow atlas" );
	check( high.shadowPcss == 0, "High leaves soft shadows (PCSS) to the user (compiled out)" );
	check( high.runtimeDirect == 1, "High draws runtime direct light" );
	check( gameui::ClassifyPreset( low ) == RenderCoreLightingPreset::kLow, "Low classifies" );
	check( gameui::ClassifyPreset( high ) == RenderCoreLightingPreset::kHigh, "High classifies" );
	check( gameui::ClassifyPreset( RenderCoreLighting{} ) == RenderCoreLightingPreset::kCustom,
	    "the ConVar defaults are Custom" );
	check( gameui::Clamped( RenderCoreLighting{ 9, -3, 7, -1, 3, 5 } ) ==
	           RenderCoreLighting{ 4, 0, 1, 0, 1, 1 },
	    "Clamped" );

	// --- Layout: the production code adds the rows to the shipped resource.
	KeyValues *res = ShippedResource();
	Menu menu;
	menu.LayOut( res );
	std::vector<KeyValues *> laidOut;
	for ( const char *name : kCoreRows )
	{
		KeyValues *key = res->FindKey( name );
		check( key != nullptr, std::string( name ) + " laid out" );
		if ( !key )
			return testing::ReportConformance( checks, failures );
		laidOut.push_back( key );
		check( !V_strcmp( key->GetString( "style" ), "DialogListButton" ),
		    std::string( name ) + " is a list row" );
		check( g_pVGuiLocalize->Find( key->GetString( "labelText" ) ) != nullptr,
		    std::string( name ) + " label is localized" );
	}
	for ( std::size_t i = 1; i < laidOut.size(); ++i )
		check( laidOut[i]->GetInt( "ypos" ) > laidOut[i - 1]->GetInt( "ypos" ),
		    std::string( kCoreRows[i] ) + " below the row before it" );
	check( laidOut[0]->GetInt( "ypos" ) > res->FindKey( "DrpModelDetail" )->GetInt( "ypos" ),
	    "the preset row is below the shipped rows" );
	check( res->FindKey( "Frame" )->GetInt( "tall" ) * 10 >=
	           laidOut.back()->GetInt( "ypos" ) + laidOut.back()->GetInt( "tall" ),
	    "the frame grows to fit the rows" );
	Menu again;
	KeyValues *res2 = ShippedResource();
	again.LayOut( res2 );
	again.LayOut( res2 );
	int presetRows = 0;
	for ( KeyValues *key = res2->GetFirstSubKey(); key; key = key->GetNextKey() )
		presetRows += !V_stricmp( key->GetName(), "DrpCorePreset" );
	check( presetRows == 1, "laying out twice adds the rows once" );
	delete res2;

	// The buttons, from their laid-out lists, with every choice localized.
	MakeButton( menu, res->FindKey( "DrpAntialias" ), 0 );
	MakeButton( menu, res->FindKey( "DrpModelDetail" ), 1 );
	const int expectedChoices[] = { gameui::kRenderCoreLightingPresetChoices, kRenderCoreAOChoices,
	    kRenderCoreShadowChoices, kRenderCoreToggleChoices, kRenderCoreToggleChoices,
	    kRenderCoreToggleChoices, kRenderCoreToggleChoices };
	for ( int i = 0; i < 7; ++i )
	{
		BaseModHybridButton *button = MakeButton( menu, laidOut[i], 2 + i );
		check( button->m_DialogListItems.Count() == expectedChoices[i],
		    std::string( kCoreRows[i] ) + " has its choices" );
		for ( const auto &item : button->m_DialogListItems )
			check( g_pVGuiLocalize->Find( item.m_String.Get() ) != nullptr,
			    std::string( kCoreRows[i] ) + " choice " + item.m_String.Get() + " localized" );
	}
	menu.m_drpCorePreset = menu.Find( "DrpCorePreset" );
	menu.m_drpCoreAO = menu.Find( "DrpCoreAO" );
	menu.m_drpCoreShadows = menu.Find( "DrpCoreShadows" );
	menu.m_drpCoreDepth = menu.Find( "DrpCoreDepth" );
	menu.m_drpCoreMovers = menu.Find( "DrpCoreMovers" );
	menu.m_drpCorePcss = menu.Find( "DrpCorePcss" );
	menu.m_drpCoreDirect = menu.Find( "DrpCoreDirect" );
	BaseModHybridButton *const rows[] = { menu.m_drpCorePreset, menu.m_drpCoreAO,
	    menu.m_drpCoreShadows, menu.m_drpCoreDepth, menu.m_drpCoreMovers, menu.m_drpCorePcss,
	    menu.m_drpCoreDirect };

	// What each row shows, against the menu's value.
	const auto shows = [&]( const RenderCoreLighting &lighting )
	{
		const int preset = int( gameui::ClassifyPreset( lighting ) );
		const char *expected[] = { s_RenderCorePresetNames[preset],
		    s_RenderCoreQualityNames[lighting.ambientOcclusion],
		    s_RenderCoreQualityNames[lighting.shadows],
		    s_RenderCoreToggleNames[lighting.depthPrepass],
		    s_RenderCoreToggleNames[lighting.shadowMovers],
		    s_RenderCoreToggleNames[lighting.shadowPcss],
		    s_RenderCoreToggleNames[lighting.runtimeDirect] };
		for ( int i = 0; i < 7; ++i )
		{
			const char *shown = rows[i]->GetCurrentSelection();
			if ( !shown || V_strcmp( shown, expected[i] ) )
				return false;
		}
		return true;
	};
	const auto reset = [&]( const RenderCoreLighting &lighting )
	{
		menu.m_CoreLighting = lighting;
		menu.m_bDirtyValues = false;
		menu.SetRenderCoreQualityState();
	};
	reset( high );
	check( shows( high ), "opens showing High on every row" );

	// --- Keyboard, controller and mouse on every value row: each input
	// moves its row one choice, the menu takes the value, every row shows
	// the result and the preset row follows.
	struct Input
	{
		const char *name;
		int direction; // +1 next, -1 previous
		void ( *drive )( BaseModHybridButton & );
	};
	static const Input kInputs[] = {
	    { "Right", +1,
	        []( BaseModHybridButton &b )
	        {
		        b.OnKeyCodePressed( KEY_RIGHT );
	        } },
	    { "Left", -1,
	        []( BaseModHybridButton &b )
	        {
		        b.OnKeyCodePressed( KEY_LEFT );
	        } },
	    { "A", +1,
	        []( BaseModHybridButton &b )
	        {
		        b.OnKeyCodePressed( KEY_XBUTTON_A );
	        } },
	    { "dpad right", +1,
	        []( BaseModHybridButton &b )
	        {
		        b.OnKeyCodePressed( KEY_XBUTTON_RIGHT );
	        } },
	    { "stick left", -1,
	        []( BaseModHybridButton &b )
	        {
		        b.OnKeyCodePressed( KEY_XSTICK1_LEFT );
	        } },
	    { "left shoulder", -1,
	        []( BaseModHybridButton &b )
	        {
		        b.OnKeyCodePressed( KEY_XBUTTON_LEFT_SHOULDER );
	        } },
	    { "click right of the choice", +1,
	        []( BaseModHybridButton &b )
	        {
		        int cx = 0, cy = 0;
		        b.GetDialogListButtonCenter( cx, cy );
		        s_Input.x = b.m_ScreenX + cx + 30;
		        s_Input.y = b.m_ScreenY + cy;
		        b.OnMousePressed( MOUSE_LEFT );
	        } },
	    { "click left of the choice", -1,
	        []( BaseModHybridButton &b )
	        {
		        int cx = 0, cy = 0;
		        b.GetDialogListButtonCenter( cx, cy );
		        s_Input.x = b.m_ScreenX + cx - 30;
		        s_Input.y = b.m_ScreenY + cy;
		        b.OnMousePressed( MOUSE_LEFT );
	        } },
	};
	for ( int r = 0; r < 6; ++r )
	{
		const auto row = RenderCoreLightingRow( r );
		BaseModHybridButton &button = *rows[1 + r];
		const int choices = gameui::RenderCoreLightingChoices( row );
		const std::string rowName = kCoreRows[1 + r];
		for ( const Input &in : kInputs )
		{
			for ( const RenderCoreLighting &start : { low, high } )
			{
				reset( start );
				const int before = gameui::Choice( start, row );
				const int after = ( before + in.direction + choices ) % choices;
				in.drive( button );
				const RenderCoreLighting expected = gameui::WithChoice( start, row, after );
				const std::string what =
				    rowName + " " + in.name + " from " + ( start == low ? "Low" : "High" );
				check( menu.m_CoreLighting == expected, what + " sets the value" );
				check( menu.m_bDirtyValues, what + " marks the dialog changed" );
				check( shows( expected ), what + " shows the result" );
			}
		}
		// A full cycle of Right returns to the start; Left past the first
		// choice wraps to the last.
		reset( high );
		for ( int i = 0; i < choices; ++i )
			button.OnKeyCodePressed( KEY_RIGHT );
		check( menu.m_CoreLighting == high, rowName + " cycles back with Right" );
		reset( gameui::WithChoice( high, row, 0 ) );
		button.OnKeyCodePressed( KEY_LEFT );
		check( gameui::Choice( menu.m_CoreLighting, row ) == choices - 1, rowName + " wraps" );
		// The right button, middle button and Up/Down change nothing.
		reset( high );
		const int closes = FlyoutMenu::closes;
		button.OnMousePressed( MOUSE_RIGHT );
		check( FlyoutMenu::closes == closes + 1, rowName + " right click closes flyouts" );
		button.OnKeyCodePressed( KEY_UP );
		button.OnKeyCodePressed( KEY_DOWN );
		check( menu.m_CoreLighting == high && !menu.m_bDirtyValues,
		    rowName + " right click and Up/Down change no value" );
		check( shows( high ), rowName + " still shows High" );
	}

	// --- The preset row, by keyboard and mouse: Low and High set every
	// row; Custom (only reachable by cycling) changes nothing.
	BaseModHybridButton &preset = *menu.m_drpCorePreset;
	reset( high );
	preset.OnKeyCodePressed( KEY_LEFT );
	check( menu.m_CoreLighting == low && shows( low ), "preset Left from High: every row Low" );
	preset.OnKeyCodePressed( KEY_RIGHT );
	check( menu.m_CoreLighting == high && shows( high ), "preset Right from Low: every row High" );
	preset.OnKeyCodePressed( KEY_RIGHT );
	check( menu.m_CoreLighting == high, "preset Right to Custom changes no row" );
	check( !V_strcmp( preset.GetCurrentSelection(), "#GameUI_High" ),
	    "Custom on matching rows still shows High" );
	reset( low );
	kInputs[6].drive( preset );
	check( menu.m_CoreLighting == high && shows( high ), "preset click right: High" );
	kInputs[7].drive( preset );
	check( menu.m_CoreLighting == low && shows( low ), "preset click left: Low" );
	// Editing a row by mouse shows Custom; the preset then restores Low.
	kInputs[6].drive( *menu.m_drpCoreDirect );
	check( !V_strcmp( preset.GetCurrentSelection(), "#GameUI_CoreLightingCustom" ),
	    "a row edit shows Custom" );
	preset.OnKeyCodePressed( KEY_LEFT ); // Custom -> High
	check( menu.m_CoreLighting == high && shows( high ), "preset from Custom: High" );
	preset.OnKeyCodePressed( KEY_LEFT ); // High -> Low
	check( menu.m_CoreLighting == low && shows( low ), "preset from High: Low" );

	// --- Keyboard focus: Down walks the shipped rows into the lighting
	// rows in order and loops; Up walks back. The description follows.
	const char *const kTitles[] = { "#GameUI_CoreLightingPreset", "#GameUI_AmbientOcclusion",
	    "#GameUI_DynamicShadows", "#GameUI_CoreDepthPrepass", "#GameUI_CoreShadowMovers",
	    "#GameUI_CoreShadowPcss", "#GameUI_CoreRuntimeDirect" };
	const char *const kDown[] = { "DrpModelDetail", "DrpCorePreset", "DrpCoreAO", "DrpCoreShadows",
	    "DrpCoreDepth", "DrpCoreMovers", "DrpCorePcss", "DrpCoreDirect", "DrpAntialias" };
	BaseModHybridButton *focus = menu.Find( kDown[0] );
	focus->RequestFocus( 0 );
	for ( int i = 1; i < 9; ++i )
	{
		focus->OnKeyCodePressed( KEY_DOWN );
		focus = static_cast<BaseModHybridButton *>( vgui::s_pFocus );
		check( !V_strcmp( focus->GetName(), kDown[i] ),
		    std::string( "Down from " ) + kDown[i - 1] + " reaches " + kDown[i] );
		if ( V_strcmp( focus->GetName(), kDown[i] ) )
			break;
		if ( i >= 1 && i <= 7 ) // kDown[i] is kCoreRows[i - 1]
		{
			check( menu.titleLabel.text == kTitles[i - 1], std::string( kDown[i] ) + " title" );
			check( menu.descriptionLabel.text == std::string( kTitles[i - 1] ) + "_Info",
			    std::string( kDown[i] ) + " description" );
			check( g_pVGuiLocalize->Find( menu.descriptionLabel.text.c_str() ) != nullptr,
			    std::string( kDown[i] ) + " description localized" );
		}
	}
	focus = menu.Find( "DrpAntialias" );
	focus->RequestFocus( 0 );
	for ( int i = 7; i >= 0; --i )
	{
		focus->OnKeyCodePressed( KEY_UP );
		focus = static_cast<BaseModHybridButton *>( vgui::s_pFocus );
		check( !V_strcmp( focus->GetName(), kDown[i] ), std::string( "Up reaches " ) + kDown[i] );
		if ( V_strcmp( focus->GetName(), kDown[i] ) )
			break;
	}

	// --- Mouse hover: entering a row focuses it and describes it.
	for ( int i = 0; i < 7; ++i )
	{
		menu.Find( "DrpAntialias" )->RequestFocus( 0 );
		rows[i]->OnCursorEntered();
		check( vgui::s_pFocus == rows[i] || menu.m_ActiveControl == rows[i],
		    std::string( kCoreRows[i] ) + " hover makes it the active control" );
		check( menu.titleLabel.text == kTitles[i], std::string( kCoreRows[i] ) + " hover title" );
		check( menu.descriptionLabel.text == std::string( kTitles[i] ) + "_Info",
		    std::string( kCoreRows[i] ) + " hover description" );
	}

	for ( BaseModHybridButton *button : menu.buttons )
		delete button;
	delete res;
	return testing::ReportConformance( checks, failures );
}
