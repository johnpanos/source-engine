// RFC 0010 V0: the VGUI fixture scheme. Repository-owned: its one font is
// the packaged resource/DejaVuSans.ttf, registered under a fixture-only name
// so no system font can stand in for it.
Scheme
{
	Colors
	{
		"White"			"255 255 255 255"
		"Black"			"0 0 0 255"
		"FixtureGray"		"96 96 96 255"
		"FixtureDark"		"32 32 40 255"
		"FixtureLight"		"200 200 210 255"
		"FixtureAccent"		"220 140 30 255"
	}

	BaseSettings
	{
		"FgColor"			"White"
		"BgColor"			"FixtureDark"
		"Label.TextColor"		"White"
		"Label.BgColor"			"0 0 0 0"
		"Label.SelectedTextColor"	"White"
		"Label.DisabledFgColor1"	"FixtureGray"
		"Label.DisabledFgColor2"	"FixtureGray"
		"Button.TextColor"		"White"
		"Button.BgColor"		"FixtureGray"
		"Button.ArmedTextColor"		"White"
		"Button.ArmedBgColor"		"FixtureAccent"
		"Button.DepressedTextColor"	"Black"
		"Button.DepressedBgColor"	"FixtureLight"
		"CheckButton.TextColor"		"White"
		"CheckButton.SelectedTextColor"	"White"
		"CheckButton.BgColor"		"FixtureDark"
		"CheckButton.Border1"		"FixtureLight"
		"CheckButton.Border2"		"FixtureGray"
		"CheckButton.Check"		"White"
		"TextEntry.TextColor"		"White"
		"TextEntry.BgColor"		"Black"
		"TextEntry.CursorColor"		"White"
		"TextEntry.SelectedTextColor"	"Black"
		"TextEntry.SelectedBgColor"	"FixtureAccent"
		"Frame.BgColor"			"FixtureDark"
		"Frame.OutOfFocusBgColor"	"FixtureDark"
		"Frame.TitleTextInsetX"		"8"
		"FrameTitleBar.TextColor"	"White"
		"FrameTitleBar.BgColor"		"FixtureGray"
		"FrameTitleBar.DisabledTextColor"	"FixtureLight"
		"FrameTitleBar.DisabledBgColor"	"FixtureGray"
		"Border.Bright"			"FixtureLight"
		"Border.Dark"			"Black"
		"Border.Selection"		"FixtureAccent"
		"FrameSystemButton.Icon"	"fixture/titlebar_icon"
		"FrameSystemButton.DisabledIcon"	"fixture/titlebar_icon_disabled"
	}

	CustomFontFiles
	{
		"1"
		{
			"font"	"resource/DejaVuSans.ttf"
			"name"	"VGUIFixtureSans"
		}
	}

	Fonts
	{
		"Default"
		{
			"1"
			{
				"name"		"VGUIFixtureSans"
				"tall"		"16"
				"weight"	"500"
				"antialias"	"1"
			}
		}
		"DefaultLarge"
		{
			"1"
			{
				"name"		"VGUIFixtureSans"
				"tall"		"28"
				"weight"	"500"
				"antialias"	"1"
			}
		}
		"DefaultOutline"
		{
			"1"
			{
				"name"		"VGUIFixtureSans"
				"tall"		"20"
				"weight"	"500"
				"antialias"	"1"
				"outline"	"1"
			}
		}
	}

	Borders
	{
		"BaseBorder"
		{
			"inset"	"0 0 0 0"
			Left	{ "1" { "color" "Border.Dark" "offset" "0 0" } }
			Right	{ "1" { "color" "Border.Bright" "offset" "0 0" } }
			Top	{ "1" { "color" "Border.Dark" "offset" "0 0" } }
			Bottom	{ "1" { "color" "Border.Bright" "offset" "0 0" } }
		}
		"ButtonBorder"
		{
			"inset"	"0 0 0 0"
			Left	{ "1" { "color" "Border.Bright" "offset" "0 0" } }
			Right	{ "1" { "color" "Border.Dark" "offset" "0 0" } }
			Top	{ "1" { "color" "Border.Bright" "offset" "0 0" } }
			Bottom	{ "1" { "color" "Border.Dark" "offset" "0 0" } }
		}
		"FrameBorder"	"ButtonBorder"
		"DepressedBorder"	"BaseBorder"
		"ButtonDepressedBorder"	"BaseBorder"
		"ButtonKeyFocusBorder"	"ButtonBorder"
		"RaisedBorder"	"ButtonBorder"
		"TitleButtonBorder"	"ButtonBorder"
	}
}
