//========= Copyright Valve Corporation, All rights reserved. ============//
// Executes the production resource hook against an owned KeyValues fixture.
#include "testing/conformance_result.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <vector>

class KeyValues
{
public:
	explicit KeyValues( const char *name ) : name( name ) {}
	std::string name;
	std::map<std::string, std::string> fields;
	std::vector<KeyValues *> children;
	KeyValues *parent = nullptr;
	~KeyValues()
	{
		for ( auto *child : children )
			delete child;
	}
	KeyValues *MakeCopy() const
	{
		auto *copy = new KeyValues( name.c_str() );
		copy->fields = fields;
		for ( auto *child : children )
			copy->AddSubKey( child->MakeCopy() );
		return copy;
	}
	void SetName( const char *value ) { name = value; }
	const char *GetName() const { return name.c_str(); }
	void SetString( const char *key, const char *value ) { fields[key] = value; }
	const char *GetString( const char *key ) const
	{
		auto found = fields.find( key );
		return found == fields.end() ? "" : found->second.c_str();
	}
	void SetInt( const char *key, int value ) { fields[key] = std::to_string( value ); }
	int GetInt( const char *key ) const { return std::atoi( GetString( key ) ); }
	void AddSubKey( KeyValues *child )
	{
		child->parent = this;
		children.push_back( child );
	}
	void RemoveSubKey( KeyValues *child )
	{
		children.erase( std::remove( children.begin(), children.end(), child ), children.end() );
		child->parent = nullptr;
	}
	void deleteThis() { delete this; }
	void Clear()
	{
		for ( auto *child : children )
			delete child;
		children.clear();
		fields.clear();
	}
	KeyValues *FindKey( const char *key, bool create = false )
	{
		for ( auto *child : children )
			if ( child->name == key )
				return child;
		if ( !create )
			return nullptr;
		auto *child = new KeyValues( key );
		AddSubKey( child );
		return child;
	}
	KeyValues *GetFirstTrueSubKey() { return children.empty() ? nullptr : children.front(); }
	KeyValues *GetNextTrueSubKey()
	{
		if ( !parent )
			return nullptr;
		auto found = std::find( parent->children.begin(), parent->children.end(), this );
		return ++found == parent->children.end() ? nullptr : *found;
	}
};
int V_stricmp( const char *left, const char *right )
{
	return strcasecmp( left, right );
}
class CFmtStr
{
	char text[80];

public:
	CFmtStr( const char *format, int value )
	{
		std::snprintf( text, sizeof( text ), format, value );
	}
	operator const char *() const { return text; }
};
class HdrVideo
{
public:
	struct Menu
	{
		struct Settings
		{
			int peakNits = 617;
		} settings;
		const Settings &Draft() const { return settings; }
	} m_Menu;
	const char *GetName() const { return "HdrVideo"; }
	void PreApplyControlSettings( KeyValues *resource );
};
#include "portal2_hdr_layout_method.inc"

int main()
{
	int checks = 0, failures = 0;
	const auto check = [&]( bool passed, const char *name )
	{
		++checks;
		if ( !passed )
		{
			++failures;
			std::fprintf( stderr, "HDR layout: %s\n", name );
		}
	};
	KeyValues resource( "Video.res" );
	auto *frame = resource.FindKey( "Video", true );
	frame->SetString( "ControlName", "Frame" );
	frame->SetString( "fieldName", "Video" );
	frame->SetInt( "tall", 4 );
	auto *row = resource.FindKey( "DrpDisplayMode", true );
	row->SetString( "ControlName", "BaseModHybridButton" );
	row->SetString( "style", "DialogListButton" );
	row->SetInt( "ypos", 100 );
	row->FindKey( "?windowed", true )->SetInt( "ypos", 75 );
	row->FindKey( "list", true )->SetString( "Fullscreen", "Fullscreen" );
	auto *button = resource.FindKey( "BtnAdvanced", true );
	button->SetString( "ControlName", "BaseModHybridButton" );
	button->SetString( "style", "LeftDialogButton" );
	button->FindKey( "?windowed", true )->SetInt( "ypos", 150 );
	HdrVideo menu;
	menu.PreApplyControlSettings( &resource );
	frame = resource.FindKey( "HdrVideo" );
	check( frame && frame->GetInt( "tall" ) == 5, "frame identity and extent" );
	check( !resource.FindKey( "Video" ) && !resource.FindKey( "DrpDisplayMode" ) &&
	           !resource.FindKey( "BtnAdvanced" ),
	    "obsolete original controls removed" );
	const char *names[] = { "DrpHdrMode", "DrpHdrExposure", "DrpHdrPeak", "BtnHdrApply" };
	for ( int i = 0; i < 4; ++i )
	{
		auto *control = resource.FindKey( names[i] );
		check( control != nullptr, "control exists" );
		if ( !control )
			continue;
		check( control->GetInt( "ypos" ) == i * 25, "rows do not overlap" );
		check( !control->FindKey( "?windowed" ), "windowed overrides cannot collapse rows" );
		check( std::string( control->GetString( "fieldName" ) ) == names[i],
		    "unique control identity" );
		check( std::string( control->GetString( "navUp" ) ) == names[( i + 3 ) % 4] &&
		           std::string( control->GetString( "navDown" ) ) == names[( i + 1 ) % 4],
		    "controller navigation wraps" );
	}
	auto *peak = resource.FindKey( "DrpHdrPeak" )->FindKey( "list" );
	check( std::string( peak->GetString( "617 nits" ) ) == "HdrPeak617",
	    "exact calibration selectable" );
	check( std::string( resource.FindKey( "BtnHdrApply" )->GetString( "command" ) ) == "ApplyHDR",
	    "apply action" );
	auto *status = resource.FindKey( "LblHdrStatus" );
	check( status && status->GetInt( "ypos" ) >= 100 && status->GetInt( "tall" ) > 0,
	    "status below controls" );
	menu.PreApplyControlSettings( nullptr );
	return testing::ReportConformance( checks, failures );
}
