//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The VGUI fixture host (RFC 0010 V0 fixed-screen corpus). It composes
//          the material system, the render provider this tree links, the input
//          system, vgui2 and vguimatsurface as the launcher does (typed linked
//          factories, no module loading by name), mounts only the
//          repository-owned fixture game (quality/fixtures/vgui-surface), and
//          paints fixed screens through the real VGUI surface:
//
//          - primitives: every ISurface draw kind over the fixture materials
//            (rectangles, alpha, outlines, fades, circles, polylines, lines,
//            textured rectangles, polygons, sub-rectangles, additive, opaque,
//            $color, point sampling, an alpha ramp, $frame frames) and text;
//          - controls: a vgui_controls Frame with a label, button, text entry,
//            check button and image panel, laid out by the fixture scheme;
//          - text: lines in the scheme's three fonts (glyph pages, batches);
//          - empty: nothing but the root panel.
//
//          For each screen it records the surface's own counters
//          (VGuiSurfaceStats001) over the first frame and over steady frames,
//          and reads back the last frame's pixels. This program measures; it
//          does not judge. tools/vgui/vgui_fixture_host.py stages the runtime,
//          runs it and applies the oracle.
//
//          Run inside the staged runtime:
//            vgui_fixture_host -game vguifixture -renderer <id> -out <dir>
//                [-steady <frames>]
//
//=============================================================================//

#include "appframework/AppFramework.h"
#include "appframework/ilaunchermgr.h"
#include "appframework/linked_systems.h"
#include "appframework/window_provider.h"
#include "filesystem.h"
#include "inputsystem/iinputsystem.h"
#include "inputsystem/provider_catalog.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "render/builtin_shader_provider.h"
#include "render/legacy_shader_provider.h"
#if defined( LINKED_NATIVE_VULKAN_BACKEND )
#include "render/device/vulkan/host_binding.h"
#endif
#include "tier0/icommandline.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/TextEntry.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "VGuiMatSurface/IVGuiSurfaceStats.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <string>
#include <vector>

namespace
{
// The back buffer. Screens are laid out for it; the oracle reads its pixels.
const int kWidth = 640;
const int kHeight = 480;
// The clear color under every screen (the oracle's background).
const unsigned char kClear[4] = { 0, 0, 64, 255 };
// Frames before the steady window (the first frame alone, then warm-up).
const int kWarmFrames = 3;

enum FixtureTexture
{
	kQuadrants,
	kAlphaRamp,
	kAdditive,
	kOpaque,
	kChecker,
	kAtlas,
	kFrames,
	kTinted,
	kTextureCount
};
const char *const kTextureFiles[kTextureCount] = { "vgui/fixture/quadrants",
    "vgui/fixture/alpha_ramp", "vgui/fixture/additive", "vgui/fixture/opaque",
    "vgui/fixture/checker", "vgui/fixture/atlas", "vgui/fixture/frames", "vgui/fixture/tinted" };

struct Fonts
{
	vgui::HFont normal = vgui::INVALID_FONT;
	vgui::HFont large = vgui::INVALID_FONT;
	vgui::HFont outline = vgui::INVALID_FONT;
};

// A full-screen panel that draws one screen in Paint.
class CFixtureScreen : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CFixtureScreen, vgui::Panel );

public:
	typedef void ( *PaintFn )( const int *textures, const Fonts &fonts );

	CFixtureScreen( const char *name, PaintFn paint, const int *textures, const Fonts &fonts )
	    : BaseClass( nullptr, name ), m_Paint( paint ), m_Textures( textures ), m_Fonts( fonts )
	{
		// A panel without a parent is an orphan that never paints; screens
		// belong to the surface's embedded (root) panel.
		SetParent( vgui::surface()->GetEmbeddedPanel() );
		SetBounds( 0, 0, kWidth, kHeight );
		SetPaintBackgroundEnabled( false );
		SetPaintBorderEnabled( false );
		SetVisible( false );
	}

	void Paint() override
	{
		if ( m_Paint )
			m_Paint( m_Textures, m_Fonts );
	}

private:
	PaintFn m_Paint;
	const int *m_Textures;
	const Fonts &m_Fonts;
};

void TexturedRect( int texture, int x, int y, int w, int h )
{
	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( texture );
	vgui::surface()->DrawTexturedRect( x, y, x + w, y + h );
}

void Text( vgui::HFont font, int x, int y, const wchar_t *text )
{
	vgui::surface()->DrawSetTextFont( font );
	vgui::surface()->DrawSetTextColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTextPos( x, y );
	vgui::surface()->DrawPrintText( text, int( wcslen( text ) ) );
}

// The positions here are the oracle's: tools/vgui/vgui_fixture_host.py.
void PaintPrimitives( const int *t, const Fonts &fonts )
{
	vgui::ISurface *s = vgui::surface();
	// Row A, y 20..84: untextured.
	s->DrawSetColor( 200, 40, 40, 255 );
	s->DrawFilledRect( 20, 20, 84, 84 );
	s->DrawSetColor( 40, 200, 40, 128 );
	s->DrawFilledRect( 100, 20, 164, 84 );
	s->DrawSetColor( 255, 255, 255, 255 );
	s->DrawOutlinedRect( 180, 20, 244, 84 );
	s->DrawSetColor( 255, 255, 255, 255 );
	s->DrawFilledRectFade( 260, 20, 324, 84, 255, 0, true );
	s->DrawSetColor( 255, 255, 255, 255 );
	s->DrawOutlinedCircle( 372, 52, 28, 32 );
	int px[3] = { 420, 480, 420 };
	int py[3] = { 20, 84, 84 };
	s->DrawSetColor( 255, 255, 0, 255 );
	s->DrawPolyLine( px, py, 3 );

	// Row B, y 100..164: textured, 64 x 64.
	TexturedRect( t[kQuadrants], 20, 100, 64, 64 );
	vgui::Vertex_t quad[4];
	quad[0].Init( Vector2D( 100, 100 ), Vector2D( 0, 0 ) );
	quad[1].Init( Vector2D( 164, 100 ), Vector2D( 1, 0 ) );
	quad[2].Init( Vector2D( 164, 164 ), Vector2D( 1, 1 ) );
	quad[3].Init( Vector2D( 100, 164 ), Vector2D( 0, 1 ) );
	s->DrawSetColor( 255, 255, 255, 255 );
	s->DrawSetTexture( t[kQuadrants] );
	s->DrawTexturedPolygon( 4, quad );
	s->DrawSetTexture( t[kAtlas] );
	s->DrawTexturedSubRect(
	    180, 100, 244, 164, 64.0f / 128, 32.0f / 128, 96.0f / 128, 64.0f / 128 );
	TexturedRect( t[kAdditive], 260, 100, 64, 64 );
	TexturedRect( t[kOpaque], 340, 100, 64, 64 );
	TexturedRect( t[kTinted], 420, 100, 64, 64 );
	TexturedRect( t[kChecker], 500, 100, 64, 64 );

	// Row C, y 180..196.
	TexturedRect( t[kAlphaRamp], 20, 180, 256, 16 );
	// DrawSetTextureFrame caches the $frame variable's lookup in a caller's
	// token, as callers keep one per texture (it must not be null).
	static unsigned int s_FrameVarCache = 0;
	for ( int frame = 0; frame < 3; ++frame )
	{
		s->DrawSetTextureFrame( t[kFrames], frame, &s_FrameVarCache );
		TexturedRect( t[kFrames], 300 + 20 * frame, 180, 16, 16 );
	}
	s->DrawSetColor( 255, 255, 0, 255 );
	s->DrawLine( 380, 188, 620, 188 );

	// Row D: text.
	Text( fonts.normal, 20, 220, L"VGUI fixture: The quick brown fox 0123456789" );
	Text( fonts.large, 20, 250, L"Large 28 px" );
	Text( fonts.outline, 20, 300, L"Outline 20 px" );
}

void PaintText( const int *, const Fonts &fonts )
{
	static const wchar_t *const kLines[] = { L"abcdefghijklmnopqrstuvwxyz",
	    L"ABCDEFGHIJKLMNOPQRSTUVWXYZ", L"0123456789 !\"#$%&'()*+,-./:;<=>?@",
	    L"[\\]^_`{|}~ The quick brown fox jumps over the lazy dog." };
	int y = 10;
	const vgui::HFont fonts3[] = { fonts.normal, fonts.large, fonts.outline };
	for ( vgui::HFont font : fonts3 )
	{
		for ( const wchar_t *line : kLines )
		{
			Text( font, 10, y, line );
			y += vgui::surface()->GetFontTall( font ) + 4;
		}
	}
}

void PaintNothing( const int *, const Fonts & )
{
}

// Pixels of the frame being drawn, top row first, RGBA8888.
bool ReadFrame( std::vector<unsigned char> &pixels )
{
	pixels.assign( size_t( kWidth ) * kHeight * 4, 0 );
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	pRenderContext->ReadPixels( 0, 0, kWidth, kHeight, pixels.data(), IMAGE_FORMAT_RGBA8888 );
	return true;
}

void WriteCounters( FILE *out, const char *name, const VGuiSurfaceStatsPerFrame_t &f )
{
	std::fprintf( out,
	    "\"%s\": {\"frames\": %llu, \"paintPasses\": %.6f, \"paintMilliseconds\": %.6f, "
	    "\"draws\": %.6f, \"textDraws\": %.6f, \"vertices\": %.6f, \"indices\": %.6f, "
	    "\"vertexKiB\": %.6f, \"textureUploads\": %.6f, \"textureUploadKiB\": %.6f, "
	    "\"glyphUploads\": %.6f, \"glyphUploadKiB\": %.6f, \"cpuCopies\": %.6f, "
	    "\"cpuCopyKiB\": %.6f}",
	    name, f.frames, f.paintPasses, f.paintMilliseconds, f.draws, f.textDraws, f.vertices,
	    f.indices, f.vertexKiB, f.textureUploads, f.textureUploadKiB, f.glyphUploads,
	    f.glyphUploadKiB, f.cpuCopies, f.cpuCopyKiB );
}

class CVguiFixtureApp : public CSteamAppSystemGroup
{
public:
	bool Create() override;
	bool PreInit() override;
	int Main() override;
	void PostShutdown() override;
	void Destroy() override {}

private:
	void RunFrame();
	bool m_bReadThisFrame = false;
	std::vector<unsigned char> m_Pixels;
};

bool CVguiFixtureApp::Create()
{
	const WindowProviderDescriptor *window = WindowProvider_Describe();
	const InputProviderDescriptor *input = InputSystem_Describe();
	if ( !AddSystem( window->create(), SDLMGR_INTERFACE_VERSION ) ||
	     !AddSystem( input->create(), INPUTSYSTEM_INTERFACE_VERSION ) ||
	     !AddSystem( MaterialSystem_Create(), MATERIAL_SYSTEM_INTERFACE_VERSION ) ||
	     !AddSystem( VGuiSurface_Create(), VGUI_SURFACE_INTERFACE_VERSION ) ||
	     !AddSystem( VGui_Create(), VGUI_IVGUI_INTERFACE_VERSION ) )
		return false;
	IMaterialSystem *pMaterialSystem =
	    (IMaterialSystem *)FindSystem( MATERIAL_SYSTEM_INTERFACE_VERSION );

	// The render catalog the product's composition root offers.
	const render::LegacyShaderProvider *catalog[] = {
#if defined( LINKED_DX9_BACKEND )
	    Dx9ShaderBackend_Describe(),
#endif
#if defined( LINKED_NATIVE_VULKAN_BACKEND )
	    NativeVulkanShaderBackend_Describe(),
#endif
	};
#if defined( LINKED_NATIVE_VULKAN_BACKEND )
	NativeVulkanShaderBackend_BindDeviceFactory( &render::device::vulkan::HostDeviceFactory() );
#endif
	const char *requested = CommandLine()->ParmValue( "-renderer", "" );
	const render::LegacyShaderProvider *selected = nullptr;
	for ( const render::LegacyShaderProvider *provider : catalog )
	{
		if ( !Q_stricmp( requested, provider->id ) )
			selected = provider;
	}
	if ( !selected || !MaterialSystem_BindShaderProvider( pMaterialSystem, selected ) )
	{
		Warning( "vgui fixture host: render provider '%s' is not linked\n", requested );
		return false;
	}
	return MaterialSystem_BindBuiltinShaderProvider(
	    pMaterialSystem, StandardShaderLibrary_Describe() );
}

bool CVguiFixtureApp::PreInit()
{
	CreateInterfaceFn factory = GetFactory();
	ConnectTier1Libraries( &factory, 1 );
	ConVar_Register();
	ConnectTier2Libraries( &factory, 1 );
	ConnectTier3Libraries( &factory, 1 );
	if ( !g_pFullFileSystem || !g_pMaterialSystem || !g_pInputSystem || !g_pMatSystemSurface )
		return false;
	// Readback resolves through the full-screen texture (as the engine connects it).
	g_pMaterialSystem->SetAdapter( 0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE );
	return SetupSearchPaths( nullptr, false, true );
}

void CVguiFixtureApp::PostShutdown()
{
	if ( g_pMatSystemSurface )
		g_pMatSystemSurface->AttachToWindow( nullptr );
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	ConVar_Unregister();
	DisconnectTier1Libraries();
}

// One UI frame as a VGUI application draws it (utils/modelbrowser).
void CVguiFixtureApp::RunFrame()
{
	g_pInputSystem->PollInputState();
	g_pMaterialSystem->BeginFrame( 0 );
	{
		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->ClearColor4ub( kClear[0], kClear[1], kClear[2], kClear[3] );
		pRenderContext->ClearBuffers( true, true );
	}
	vgui::ivgui()->RunFrame();
	vgui::surface()->PaintTraverseEx( vgui::surface()->GetEmbeddedPanel(), true );
	if ( m_bReadThisFrame )
		ReadFrame( m_Pixels );
	g_pMaterialSystem->EndFrame();
	g_pMaterialSystem->SwapBuffers();
}

int CVguiFixtureApp::Main()
{
	const char *outDir = CommandLine()->ParmValue( "-out", "" );
	const int steadyFrames = CommandLine()->ParmValue( "-steady", 10 );
	if ( !outDir[0] || steadyFrames <= 0 )
	{
		Warning( "vgui fixture host: need -out <dir> (and -steady > 0)\n" );
		return 2;
	}

	g_pMaterialSystem->ModInit();
	ILauncherMgr *pLauncher = (ILauncherMgr *)FindSystem( SDLMGR_INTERFACE_VERSION );
	if ( !pLauncher || !pLauncher->CreateGameWindow( "vgui fixture", true, kWidth, kHeight ) )
	{
		Warning( "vgui fixture host: no window\n" );
		return 3;
	}
	int pixelWidth = 0, pixelHeight = 0;
	SDL_PumpEvents();
	if ( !SDL_GetWindowSizeInPixels(
	         static_cast<SDL_Window *>( pLauncher->GetWindowRef() ), &pixelWidth, &pixelHeight ) ||
	     pixelWidth != kWidth || pixelHeight != kHeight )
	{
		Warning( "vgui fixture host: drawable %dx%d, need %dx%d (run headless: SDL offscreen)\n",
		    pixelWidth, pixelHeight, kWidth, kHeight );
		return 3;
	}
	MaterialSystem_Config_t config = g_pMaterialSystem->GetCurrentConfigForVideoCard();
	config.m_VideoMode.m_Width = kWidth;
	config.m_VideoMode.m_Height = kHeight;
	config.m_VideoMode.m_RefreshRate = 60;
	config.SetFlag( MATSYS_VIDCFG_FLAGS_WINDOWED, true );
	if ( !g_pMaterialSystem->SetMode( pLauncher->GetWindowRef(), config ) )
	{
		Warning( "vgui fixture host: SetMode failed\n" );
		return 3;
	}
	// One sample per pixel, no forced anisotropy: the oracle reads exact pixels.
	MaterialSystem_Config_t pinned = g_pMaterialSystem->GetCurrentConfigForVideoCard();
	pinned.m_nAASamples = 0;
	pinned.m_nForceAnisotropicLevel = 1;
	g_pMaterialSystem->OverrideConfig( pinned, false );
	// Readback must see this frame, so the render context runs on this thread.
	if ( ConVar *pQueueMode = g_pCVar->FindVar( "mat_queue_mode" ) )
		pQueueMode->SetValue( 0 );

	g_pMatSystemSurface->AttachToWindow( pLauncher->GetWindowRef(), true );
	CreateInterfaceFn factory = GetFactory();
	if ( !vgui::VGui_InitInterfacesList( "VGUIFIXTURE", &factory, 1 ) )
	{
		Warning( "vgui fixture host: vgui_controls could not connect\n" );
		return 3;
	}
	IVGuiSurfaceStats *stats = static_cast<IVGuiSurfaceStats *>(
	    g_pMatSystemSurface->QueryInterface( VGUI_SURFACE_STATS_INTERFACE_VERSION ) );
	if ( !stats )
	{
		Warning(
		    "vgui fixture host: the surface serves no %s\n", VGUI_SURFACE_STATS_INTERFACE_VERSION );
		return 3;
	}
	vgui::HScheme scheme =
	    vgui::scheme()->LoadSchemeFromFile( "resource/FixtureScheme.res", "FixtureScheme" );
	if ( !scheme )
	{
		Warning( "vgui fixture host: resource/FixtureScheme.res did not load\n" );
		return 3;
	}
	vgui::ivgui()->Start();

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( scheme );
	Fonts fonts;
	fonts.normal = pScheme->GetFont( "Default" );
	fonts.large = pScheme->GetFont( "DefaultLarge" );
	fonts.outline = pScheme->GetFont( "DefaultOutline" );

	int textures[kTextureCount];
	for ( int i = 0; i < kTextureCount; ++i )
	{
		textures[i] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( textures[i], kTextureFiles[i], true, false );
	}

	// The root panel covers the back buffer.
	const vgui::VPANEL root = vgui::surface()->GetEmbeddedPanel();
	vgui::ipanel()->SetPos( root, 0, 0 );
	vgui::ipanel()->SetSize( root, kWidth, kHeight );
	vgui::ipanel()->SetVisible( root, true );

	struct Screen
	{
		const char *name;
		vgui::Panel *panel;
	};
	std::vector<Screen> screens;
	screens.push_back(
	    { "primitives", new CFixtureScreen( "primitives", &PaintPrimitives, textures, fonts ) } );
	{
		CFixtureScreen *root = new CFixtureScreen( "controls", nullptr, textures, fonts );
		vgui::Frame *frame = new vgui::Frame( root, "FixtureFrame" );
		frame->SetScheme( scheme );
		frame->SetTitle( "Fixture controls", false );
		frame->SetBounds( 40, 40, 400, 300 );
		frame->SetSizeable( false );
		frame->SetMoveable( false );
		frame->SetCloseButtonVisible( false );
		frame->SetVisible( true );
		vgui::Label *label = new vgui::Label( frame, "FixtureLabel", "Label" );
		label->SetBounds( 20, 40, 200, 24 );
		vgui::Button *button = new vgui::Button( frame, "FixtureButton", "Button" );
		button->SetBounds( 20, 80, 120, 30 );
		vgui::TextEntry *entry = new vgui::TextEntry( frame, "FixtureEntry" );
		entry->SetBounds( 20, 120, 200, 24 );
		entry->SetText( "TextEntry" );
		vgui::CheckButton *check = new vgui::CheckButton( frame, "FixtureCheck", "Check" );
		check->SetBounds( 20, 160, 120, 24 );
		check->SetSelected( true );
		vgui::ImagePanel *image = new vgui::ImagePanel( frame, "FixtureImage" );
		image->SetBounds( 260, 40, 64, 64 );
		image->SetImage( "fixture/quadrants" );
		image->SetShouldScaleImage( true );
		screens.push_back( { "controls", root } );
	}
	screens.push_back( { "text", new CFixtureScreen( "text", &PaintText, textures, fonts ) } );
	screens.push_back( { "empty", new CFixtureScreen( "empty", &PaintNothing, textures, fonts ) } );
	for ( Screen &screen : screens )
		screen.panel->SetScheme( scheme );

	std::string summary = std::string( outDir ) + "/frames.json";
	FILE *out = std::fopen( summary.c_str(), "w" );
	if ( !out )
	{
		Warning( "vgui fixture host: cannot write %s\n", summary.c_str() );
		return 3;
	}
	std::fprintf( out,
	    "{\"schema\": \"vgui-fixture-frames/v1\", \"renderer\": \"%s\", \"width\": %d, "
	    "\"height\": %d, \"clear\": [%d, %d, %d, %d], \"warm_frames\": %d, "
	    "\"steady_frames\": %d, \"fonts\": {\"Default\": %d, \"DefaultLarge\": %d, "
	    "\"DefaultOutline\": %d}, \"screens\": [",
	    CommandLine()->ParmValue( "-renderer", "" ), kWidth, kHeight, kClear[0], kClear[1],
	    kClear[2], kClear[3], kWarmFrames, steadyFrames,
	    fonts.normal ? vgui::surface()->GetFontTall( fonts.normal ) : 0,
	    fonts.large ? vgui::surface()->GetFontTall( fonts.large ) : 0,
	    fonts.outline ? vgui::surface()->GetFontTall( fonts.outline ) : 0 );

	for ( size_t i = 0; i < screens.size(); ++i )
	{
		for ( Screen &other : screens )
			other.panel->SetVisible( false );
		screens[i].panel->SetVisible( true );

		VGuiSurfaceStats_t s0, s1, s2, s3;
		stats->GetStats( s0 );
		RunFrame();
		stats->GetStats( s1 );
		for ( int f = 1; f < kWarmFrames; ++f )
			RunFrame();
		stats->GetStats( s2 );
		for ( int f = 0; f < steadyFrames; ++f )
		{
			m_bReadThisFrame = f == steadyFrames - 1;
			RunFrame();
		}
		m_bReadThisFrame = false;
		stats->GetStats( s3 );

		VGuiSurfaceStatsPerFrame_t first, steady;
		const bool haveFirst = VGuiSurfaceStats_PerFrame( s0, s1, first );
		const bool haveSteady = VGuiSurfaceStats_PerFrame( s2, s3, steady );
		const std::string frameFile = std::string( screens[i].name ) + ".rgba";
		FILE *pixels = std::fopen( ( std::string( outDir ) + "/" + frameFile ).c_str(), "wb" );
		const bool wrote =
		    pixels && std::fwrite( m_Pixels.data(), 1, m_Pixels.size(), pixels ) == m_Pixels.size();
		if ( pixels )
			std::fclose( pixels );
		std::fprintf( out, "%s{\"name\": \"%s\", \"frame\": \"%s\", \"frame_written\": %s, ",
		    i ? ", " : "", screens[i].name, frameFile.c_str(), wrote ? "true" : "false" );
		std::fprintf( out, "\"have_first\": %s, \"have_steady\": %s, ",
		    haveFirst ? "true" : "false", haveSteady ? "true" : "false" );
		WriteCounters( out, "first", first );
		std::fprintf( out, ", " );
		WriteCounters( out, "steady", steady );
		std::fprintf( out, "}" );
	}
	std::fprintf( out, "]}\n" );
	std::fclose( out );

	for ( Screen &screen : screens )
		screen.panel->MarkForDeletion();
	vgui::ivgui()->RunFrame();
	return 0;
}
} // namespace

// The file system is linked too, as the launcher links it (launcher.cpp), so
// the staged runtime needs no bin/filesystem_stdio module.
int main( int argc, char **argv )
{
	static CVguiFixtureApp s_App;
	static CSteamApplication s_SteamApp( &s_App, FileSystemStdio_Create() );
	return AppMain( argc, argv, &s_SteamApp );
}
