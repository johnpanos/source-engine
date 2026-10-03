//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "networkstringtable_clientdll.h"
#include <KeyValues.h>
#include "panelmetaclassmgr.h"
#include <vgui_controls/Controls.h>
#include "mathlib/vmatrix.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "view.h"
#include "collisionutils.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IVGui.h>
#include "ienginevgui.h"
#include "in_buttons.h"
#include <vgui/MouseCode.h>
#include "materialsystem/imesh.h"
#include "clienteffectprecachesystem.h"
#include "c_vguiscreen.h"
#include "iclientmode.h"
#include "vgui_bitmapbutton.h"
#include "vgui_bitmappanel.h"
#include "filesystem.h"
#include "iinput.h"
#include "emissive_area_lights.h"
#include "engine/iworldpanels.h"
#include "materialsystem/itexture.h"
#include "vgui/IWorldPanelRecorder.h"
#include "view_scene.h"
#include "tier1/fmtstr.h"

#include <vgui/IInputInternal.h>
extern vgui::IInputInternal *g_InputInternal;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define VGUI_SCREEN_MODE_RADIUS	80

//Precache the materials
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectVGuiScreen )
CLIENTEFFECT_MATERIAL( "engine/writez" )
CLIENTEFFECT_REGISTER_END()


// ----------------------------------------------------------------------------- //
// This is a cache of preloaded keyvalues.
// ----------------------------------------------------------------------------- // 

CUtlDict<KeyValues*, int> g_KeyValuesCache;

KeyValues* CacheKeyValuesForFile( const char *pFilename )
{
	MEM_ALLOC_CREDIT();
	int i = g_KeyValuesCache.Find( pFilename );
	if ( i == g_KeyValuesCache.InvalidIndex() )
	{
		KeyValues *rDat = new KeyValues( pFilename );
		rDat->LoadFromFile( filesystem, pFilename, NULL );
		g_KeyValuesCache.Insert( pFilename, rDat );
		return rDat;		
	}
	else
	{
		return g_KeyValuesCache[i];
	}
}

void ClearKeyValuesCache()
{
	MEM_ALLOC_CREDIT();
	for ( int i=g_KeyValuesCache.First(); i != g_KeyValuesCache.InvalidIndex(); i=g_KeyValuesCache.Next( i ) )
	{
		g_KeyValuesCache[i]->deleteThis();
	}
	g_KeyValuesCache.Purge();
}


IMPLEMENT_CLIENTCLASS_DT(C_VGuiScreen, DT_VGuiScreen, CVGuiScreen)
RecvPropFloat( RECVINFO( m_flWidth ) ), RecvPropFloat( RECVINFO( m_flHeight ) ),
    RecvPropInt( RECVINFO( m_fScreenFlags ) ), RecvPropInt( RECVINFO( m_nPanelName ) ),
    RecvPropInt( RECVINFO( m_nAttachmentIndex ) ), RecvPropInt( RECVINFO( m_nOverlayMaterial ) ),
    RecvPropEHandle( RECVINFO( m_hPlayerOwner ) ),
    END_RECV_TABLE()

    //-----------------------------------------------------------------------------
    // Constructor
    //-----------------------------------------------------------------------------
    IEngineWorldPanels *g_pEngineWorldPanels = NULL;
IWorldPanelRecorder *g_pWorldPanelRecorder = NULL;

C_VGuiScreen::C_VGuiScreen()
{
	m_PanelResolution = world_panel::Resolution();
	m_RecordedResolution = world_panel::Resolution();
	m_nRecordedFrame = -1;
	m_bRecordingValid = false;
	m_bRecordingRefusalLogged = false;
	m_bSubmittedToCore = false;
	m_nOldPanelName = m_nPanelName = -1;
	m_nOldOverlayMaterial = m_nOverlayMaterial = -1;
	m_nOldPx = m_nOldPy = -1;
	m_nButtonState = 0;
	m_bLoseThinkNextFrame = false;
	m_bAcceptsInput = true;

	m_WriteZMaterial.Init( "engine/writez", TEXTURE_GROUP_VGUI );
	m_OverlayMaterial.Init( m_WriteZMaterial );
}

C_VGuiScreen::~C_VGuiScreen()
{
	DestroyVguiScreen();
}

//-----------------------------------------------------------------------------
// Network updates
//-----------------------------------------------------------------------------
void C_VGuiScreen::PreDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PreDataUpdate( updateType );
	m_nOldPanelName = m_nPanelName;
	m_nOldOverlayMaterial = m_nOverlayMaterial;
}

void C_VGuiScreen::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ((type == DATA_UPDATE_CREATED) || (m_nPanelName != m_nOldPanelName))
	{
		CreateVguiScreen( PanelName() );
		m_nButtonState = 0;
	}

	// Set up the overlay material
	if (m_nOldOverlayMaterial != m_nOverlayMaterial)
	{
		m_OverlayMaterial.Shutdown();

		const char *pMaterialName = GetMaterialNameFromIndex(m_nOverlayMaterial);
		if (pMaterialName)
		{
			m_OverlayMaterial.Init( pMaterialName, TEXTURE_GROUP_VGUI );
		}
		else
		{
			m_OverlayMaterial.Init( m_WriteZMaterial );
		}
	}
}

void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

//-----------------------------------------------------------------------------
// Returns the attachment render origin + origin
//-----------------------------------------------------------------------------
void C_VGuiScreen::GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pOrigin, QAngle *pAngles )
{
	C_BaseEntity *pEnt = pAttachedTo->GetBaseEntity();
	if (pEnt && (m_nAttachmentIndex > 0))
	{
		{
			C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, true );
			pEnt->GetAttachment( m_nAttachmentIndex, *pOrigin, *pAngles );
		}
		
		if ( IsAttachedToViewModel() )
		{
			FormatViewModelAttachment( *pOrigin, true );
		}
	}
	else
	{
		BaseClass::GetAimEntOrigin( pAttachedTo, pOrigin, pAngles );
	}
}

//-----------------------------------------------------------------------------
// Create, destroy vgui panels...
//-----------------------------------------------------------------------------
void C_VGuiScreen::CreateVguiScreen( const char *pTypeName )
{
	// Clear out any old screens.
	DestroyVguiScreen();

	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

	// Create the new screen...
	VGuiScreenInitData_t initData( this );
	m_PanelWrapper.Activate( pTypeName, NULL, 0, &initData );

	// Retrieve the panel dimensions
	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
	if (pPanel)
	{
		int x, y;
		pPanel->GetBounds( x, y, m_nPixelWidth, m_nPixelHeight );
	}
	else
	{
		m_nPixelWidth = m_nPixelHeight = 0;
	}
}

void C_VGuiScreen::DestroyVguiScreen( )
{
	if ( m_bSubmittedToCore && g_pEngineWorldPanels )
		g_pEngineWorldPanels->RemovePanel( PanelId() );
	m_bSubmittedToCore = false;
	m_nRecordedFrame = -1;
	m_RecordedQuads.RemoveAll();
	m_RecordedTextures.RemoveAll();
	m_PanelWrapper.Deactivate();
}

//-----------------------------------------------------------------------------
// RFC 0016 render.pass.panels: the screen as an emissive surface
//-----------------------------------------------------------------------------

// The recording's sink: quads in paint order, each texture indexed once.
class C_VGuiScreen::CPanelRecording final : public IWorldPanelRecording
{
public:
	CPanelRecording( C_VGuiScreen &screen, const CVGuiScreenPanel *pScreenPanel )
	    : m_Screen( screen ), m_pScreenPanel( pScreenPanel )
	{
	}
	void AddQuad( const world_panel::Quad &quad, ITexture *pTexture ) override
	{
		world_panel::Quad recorded = quad;
		recorded.texture = world_panel::kWhite;
		recorded.layer = pTexture && m_pScreenPanel && m_pScreenPanel->PaintsCoating( pTexture )
		                     ? world_panel::kLayerCoating
		                     : world_panel::kLayerEmissive;
		if ( pTexture )
		{
			int nIndex = m_Screen.m_RecordedTextures.Find( pTexture );
			if ( nIndex < 0 )
				nIndex = m_Screen.m_RecordedTextures.AddToTail( pTexture );
			recorded.texture = unsigned( nIndex );
		}
		m_Screen.m_RecordedQuads.AddToTail( recorded );
	}

private:
	C_VGuiScreen &m_Screen;
	const CVGuiScreenPanel *m_pScreenPanel;
};

unsigned long long C_VGuiScreen::PanelId() const
{
	// The entity's handle: stable for the entity's life, never reused while
	// the core holds its image (RemovePanel on destruction).
	return (unsigned long long)(unsigned int)GetRefEHandle().ToInt();
}

world_panel::Placement C_VGuiScreen::PanelPlacement()
{
	Vector upperLeft, upperRight, lowerLeft;
	ComputeEdges( &upperLeft, &upperRight, &lowerLeft );
	world_panel::Placement placement;
	for ( int k = 0; k < 3; ++k )
	{
		placement.origin[k] = upperLeft[k];
		placement.right[k] = upperRight[k] - upperLeft[k];
		placement.down[k] = lowerLeft[k] - upperLeft[k];
	}
	return placement;
}

// The panel painted once this frame into its draw list, at the resolution the
// views last asked for: every view of the frame and the lights it casts take
// this one list, so they show and cast the same state.
bool C_VGuiScreen::RecordFrame()
{
	if ( m_nRecordedFrame == gpGlobals->framecount )
		return m_bRecordingValid;
	m_nRecordedFrame = gpGlobals->framecount;
	m_bRecordingValid = false;
	m_RecordedQuads.RemoveAll();
	m_RecordedTextures.RemoveAll();
	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
	if ( !pPanel || !g_pWorldPanelRecorder || m_nPixelWidth <= 0 || m_nPixelHeight <= 0 )
		return false;
	// The resolution the main view needs (its last world-to-screen transform):
	// at least one texel per screen pixel where the screen is densest on it.
	{
		int nScreenWide = 0, nScreenTall = 0;
		engine->GetScreenSize( nScreenWide, nScreenTall );
		const VMatrix &worldToScreen = engine->WorldToScreenMatrix();
		float toClip[16];
		for ( int r = 0; r < 4; ++r )
			for ( int c = 0; c < 4; ++c )
				toClip[r * 4 + c] = worldToScreen.m[r][c];
		const float viewport[4] = { 0.0f, 0.0f, float( nScreenWide ), float( nScreenTall ) };
		const float flPixelsPerUnit = world_panel::PixelsPerUnit(
		    PanelPlacement(), float( m_nPixelWidth ), float( m_nPixelHeight ), toClip, viewport );
		m_PanelResolution = world_panel::ChooseResolution(
		    float( m_nPixelWidth ), float( m_nPixelHeight ), flPixelsPerUnit, m_PanelResolution );
	}
	m_RecordedResolution = m_PanelResolution;
#ifdef PORTAL2
	// As DrawModel: the panel sees the screen's active state as its enabled state.
	pPanel->SetEnabled( IsActive() );
#endif
	CPanelRecording recording( *this, dynamic_cast<const CVGuiScreenPanel *>( pPanel ) );
	char szWhy[128] = {};
	m_bRecordingValid = g_pWorldPanelRecorder->RecordPanel( pPanel->GetVPanel(), m_nPixelWidth,
	    m_nPixelHeight, m_RecordedResolution.texelsPerUnit, &recording, szWhy, sizeof( szWhy ) );
	if ( m_bRecordingValid )
	{
		for ( ITexture *pTexture : m_RecordedTextures )
		{
			if ( pTexture->IsProcedural() && !pTexture->IsTranslucent() )
			{
				float rgba[4];
				if ( !EmissiveAreaLights_SampleTexture( pTexture, 0.5f, 0.5f, rgba ) )
				{
					m_bRecordingValid = false;
					V_strncpy( szWhy, "an opaque procedural image has no current CPU samples",
					    sizeof( szWhy ) );
					break;
				}
			}
		}
	}
	if ( !m_bRecordingValid && !m_bRecordingRefusalLogged )
	{
		m_bRecordingRefusalLogged = true;
		DevWarning(
		    "vgui_screen %s: its panel cannot be recorded (%s); it draws through the legacy "
		    "2D path and casts no light\n",
		    PanelName(), szWhy );
	}
	return m_bRecordingValid;
}

static ConVar cl_world_panel_report( "cl_world_panel_report", "0", FCVAR_CHEAT,
    "RFC 0016 render.pass.panels: each frame, print every lit screen's recorded list (its "
    "textures with their quad counts and highest alpha), its screen corners, and its tile "
    "lights with and without the quads of textures cl_world_panel_report_exclude names" );
static ConVar cl_world_panel_report_exclude( "cl_world_panel_report_exclude",
    "elevator_video_overlay", FCVAR_CHEAT,
    "The texture name part whose quads cl_world_panel_report leaves out of its "
    "second set of tile lights (the chamber sign's grime)" );

int C_VGuiScreen::EmissiveAreaLights( area_light::AreaLight *pLights, int *pKeys, int nMax )
{
	CVGuiScreenPanel *pScreenPanel = dynamic_cast<CVGuiScreenPanel *>( m_PanelWrapper.GetPanel() );
	if ( nMax < 1 || !pScreenPanel || IsDormant() || !IsActive() || IsEffectActive( EF_NODRAW ) ||
	     !RecordFrame() )
		return 0;
	const float flUnitsWide = float( m_nPixelWidth ), flUnitsTall = float( m_nPixelHeight );
	int nAcross = 1, nDown = 1;
	world_panel::Tiles( flUnitsWide, flUnitsTall, nAcross, nDown );
	if ( nAcross * nDown > nMax )
		nAcross = nDown = 1;
	// The list's texture keys are indices into m_RecordedTextures.
	CUtlVector<int> keys;
	for ( int i = 0; i < m_RecordedTextures.Count(); ++i )
		keys.AddToTail( i );
	const world_panel::DrawListView list = { flUnitsWide, flUnitsTall, m_RecordedQuads.Base(),
	    unsigned( m_RecordedQuads.Count() ), keys.Base(), unsigned( keys.Count() ) };
	float radiance[world_panel::kMaxTiles][3];
	std::vector<std::uint64_t> revisions;
	for ( ITexture *texture : m_RecordedTextures )
		revisions.push_back( EmissiveAreaLights_SampleRevision( texture ) );
	m_TileRadiance.Evaluate(
	    list, pScreenPanel->EmissionScale(), nAcross, nDown, 16, revisions,
	    [&]( int nTexture, float s, float t, float, float rgba[4] )
	    {
		    return EmissiveAreaLights_SampleTexture( m_RecordedTextures[nTexture], s, t, rgba );
	    },
	    radiance );

	const world_panel::Placement placement = PanelPlacement();
	if ( cl_world_panel_report.GetBool() )
	{
		// The list as recorded this frame, its screen corners (the main view,
		// -1 to 1, y up), and its lights with and without the excluded quads.
		CUtlString line;
		line.Format( "cl_world_panel_report: frame %d ent %d res %ux%u quads %d textures",
		    gpGlobals->framecount, entindex(), m_RecordedResolution.width,
		    m_RecordedResolution.height, m_RecordedQuads.Count() );
		const char *pszExclude = cl_world_panel_report_exclude.GetString();
		CUtlVector<world_panel::Quad> kept;
		for ( int t = 0; t < m_RecordedTextures.Count(); ++t )
		{
			int nQuads = 0, nAlpha = 0;
			for ( const world_panel::Quad &quad : m_RecordedQuads )
			{
				if ( quad.texture == unsigned( t ) )
				{
					++nQuads;
					nAlpha = MAX( nAlpha, int( quad.color[3] ) );
				}
			}
			line += CFmtStr( " %s:%d:%d", m_RecordedTextures[t]->GetName(), nQuads, nAlpha ).Get();
		}
		for ( const world_panel::Quad &quad : m_RecordedQuads )
		{
			if ( !pszExclude[0] || quad.texture == world_panel::kWhite ||
			     !V_stristr( m_RecordedTextures[quad.texture]->GetName(), pszExclude ) )
				kept.AddToTail( quad );
		}
		line += CFmtStr( " placement %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f units %d %d",
		    placement.origin[0], placement.origin[1], placement.origin[2], placement.right[0],
		    placement.right[1], placement.right[2], placement.down[0], placement.down[1],
		    placement.down[2], m_nPixelWidth, m_nPixelHeight )
		            .Get();
		line += " screen";
		for ( int c = 0; c < 4; ++c )
		{
			Vector corner;
			for ( int k = 0; k < 3; ++k )
				corner[k] = placement.origin[k] + ( c == 1 || c == 2 ? placement.right[k] : 0.0f ) +
				            ( c >= 2 ? placement.down[k] : 0.0f );
			Vector screen;
			ScreenTransform( corner, screen );
			line += CFmtStr( " %.4f %.4f", screen.x, screen.y ).Get();
		}
		float without[world_panel::kMaxTiles][3];
		const world_panel::DrawListView keptList = { flUnitsWide, flUnitsTall, kept.Base(),
		    unsigned( kept.Count() ), keys.Base(), unsigned( keys.Count() ) };
		world_panel::TileRadiance(
		    keptList, pScreenPanel->EmissionScale(), nAcross, nDown, 16,
		    [&]( int nTexture, float s, float t, float, float rgba[4] )
		    {
			    return EmissiveAreaLights_SampleTexture( m_RecordedTextures[nTexture], s, t, rgba );
		    },
		    without );
		line += CFmtStr( " tiles %dx%d", nAcross, nDown ).Get();
		for ( int i = 0; i < nAcross * nDown; ++i )
			line +=
			    CFmtStr( " %.5f %.5f %.5f", radiance[i][0], radiance[i][1], radiance[i][2] ).Get();
		line += " without";
		for ( int i = 0; i < nAcross * nDown; ++i )
			line += CFmtStr( " %.5f %.5f %.5f", without[i][0], without[i][1], without[i][2] ).Get();
		Msg( "%s\n", line.Get() );
	}
	// The panel is one emitter: every tile reaches as far as the whole panel
	// at its mean radiance does (a tile's own reach, a fraction of the
	// panel's power, would cut the sum's far field short).
	float flPanelReach = 0.0f;
	{
		area_light::Rect whole;
		float mean[3] = { 0.0f, 0.0f, 0.0f };
		for ( int k = 0; k < 3; ++k )
		{
			whole.center[k] =
			    placement.origin[k] + 0.5f * ( placement.right[k] + placement.down[k] );
			whole.halfU[k] = 0.5f * placement.right[k];
			whole.halfV[k] = -0.5f * placement.down[k];
			for ( int t = 0; t < nAcross * nDown; ++t )
				mean[k] += radiance[t][k] / float( nAcross * nDown );
		}
		flPanelReach = area_light::Reach( whole, mean );
	}
	int nCount = 0;
	for ( int ty = 0; ty < nDown; ++ty )
	{
		for ( int tx = 0; tx < nAcross; ++tx )
		{
			area_light::AreaLight &light = pLights[nCount];
			light = area_light::AreaLight();
			// Front (halfU x halfV) on the drawn side: halfV points up the panel.
			for ( int k = 0; k < 3; ++k )
			{
				light.rect.center[k] = placement.origin[k] +
				                       placement.right[k] * ( tx + 0.5f ) / nAcross +
				                       placement.down[k] * ( ty + 0.5f ) / nDown;
				light.rect.halfU[k] = 0.5f * placement.right[k] / nAcross;
				light.rect.halfV[k] = -0.5f * placement.down[k] / nDown;
			}
			for ( int k = 0; k < 3; ++k )
				light.radiance[k] = radiance[ty * nAcross + tx][k];
			light.reach =
			    area_light::Reach( light.rect, light.radiance ) > 0.0f ? flPanelReach : 0.0f;
			if ( !( light.reach > 0.0f ) )
				continue;
			pKeys[nCount] = EmissiveAreaLights_PanelTileKey( entindex(), ty * nAcross + tx );
			++nCount;
		}
	}
	return nCount;
}

// DrawModel's path through the render core: in a view the core draws, this
// frame's list as an emissive surface; false leaves the view to the legacy
// 2D path.
bool C_VGuiScreen::DrawOnRenderCore()
{
	CVGuiScreenPanel *pScreenPanel = dynamic_cast<CVGuiScreenPanel *>( m_PanelWrapper.GetPanel() );
	if ( !pScreenPanel || !pScreenPanel->DrawsAsEmissiveSurface() ||
	     static_cast<IMaterial *>( m_OverlayMaterial ) !=
	         static_cast<IMaterial *>( m_WriteZMaterial ) ||
	     !g_pEngineWorldPanels || !g_pWorldPanelRecorder ||
	     !g_pEngineWorldPanels->CoreDrawsPanels() )
		return false;
	const world_panel::Placement placement = PanelPlacement();
	if ( !RecordFrame() )
		return false;
	RenderCorePanel panel;
	panel.id = PanelId();
	panel.placement = placement;
	panel.unitsWide = float( m_nPixelWidth );
	panel.unitsTall = float( m_nPixelHeight );
	panel.quads = m_RecordedQuads.Base();
	panel.quadCount = unsigned( m_RecordedQuads.Count() );
	panel.textures = m_RecordedTextures.Base();
	panel.textureCount = unsigned( m_RecordedTextures.Count() );
	panel.resolution = m_RecordedResolution;
	panel.emissionScale = pScreenPanel->EmissionScale();
	// The legacy flag selects sorting. Keep opaque physical faces opaque;
	// otherwise the core uses the same material family's alpha surface.
	panel.transparent = IsTransparent() && !world_panel::CoversFace( m_RecordedQuads.Base(),
	                                           unsigned( m_RecordedQuads.Count() ),
	                                           float( m_nPixelWidth ), float( m_nPixelHeight ) );
	// The scene's light at the screen's face, which its coatings reflect: the
	// engine's lighting cube a model there would take.
	{
		Vector center, normal, color, box[6];
		for ( int k = 0; k < 3; ++k )
			center[k] = placement.origin[k] + 0.5f * ( placement.right[k] + placement.down[k] );
		const Vector right( placement.right[0], placement.right[1], placement.right[2] );
		const Vector down( placement.down[0], placement.down[1], placement.down[2] );
		normal = CrossProduct( down, right );
		VectorNormalize( normal );
		engine->ComputeLighting( center + normal * 4.0f, &normal, false, color, box );
		for ( int f = 0; f < 6; ++f )
			for ( int k = 0; k < 3; ++k )
				panel.ambientCube[f][k] = box[f][k];
	}
	if ( !g_pEngineWorldPanels->DrawPanel( panel ) )
		return false;
	m_bSubmittedToCore = true;
	return true;
}

//-----------------------------------------------------------------------------
// Is the screen active?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsActive() const
{
	return (m_fScreenFlags & VGUI_SCREEN_ACTIVE) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsAttachedToViewModel() const
{
	return (m_fScreenFlags & VGUI_SCREEN_ATTACHED_TO_VIEWMODEL) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_VGuiScreen::AcceptsInput() const
{
	return m_bAcceptsInput;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : acceptsinput - 
//-----------------------------------------------------------------------------
void C_VGuiScreen::SetAcceptsInput( bool acceptsinput )
{
	m_bAcceptsInput = acceptsinput;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_VGuiScreen::GetRenderGroup()
{
	if ( IsAttachedToViewModel() )
		return RENDER_GROUP_VIEW_MODEL_TRANSLUCENT;

	return BaseClass::GetRenderGroup();
}

//-----------------------------------------------------------------------------
// Are we only visible to teammates?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsVisibleOnlyToTeammates() const
{
	return (m_fScreenFlags & VGUI_SCREEN_VISIBLE_TO_TEAMMATES) != 0;
}

//-----------------------------------------------------------------------------
// Are we visible to someone on this team?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsVisibleToTeam( int nTeam )
{
	// FIXME: Should this maybe go into a derived class of some sort?
	// Don't bother with screens on the wrong team
	if (IsVisibleOnlyToTeammates() && (nTeam > 0))
	{
		// Hmmm... sort of a hack...
		C_BaseEntity *pOwner = GetOwnerEntity();
		if ( pOwner && (nTeam != pOwner->GetTeamNumber()) )
			return false;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Activate, deactivate the view screen
//-----------------------------------------------------------------------------
void C_VGuiScreen::GainFocus( )
{
	SetNextClientThink( CLIENT_THINK_ALWAYS );
	m_bLoseThinkNextFrame = false;
	m_nOldButtonState = 0;
}

void C_VGuiScreen::LoseFocus()
{
	m_bLoseThinkNextFrame = true;
	m_nOldButtonState = 0;
}

void C_VGuiScreen::SetButtonState( int nButtonState )
{
	m_nButtonState = nButtonState;
}



//-----------------------------------------------------------------------------
// Returns the panel name 
//-----------------------------------------------------------------------------
const char *C_VGuiScreen::PanelName() const
{
	return g_StringTableVguiScreen->GetString( m_nPanelName );
}

//--------------------------------------------------------------------------
// Purpose:
// Given a field of view and mouse/screen positions as well as the current
// render origin and angles, returns a unit vector through the mouse position
// that can be used to trace into the world under the mouse click pixel.
// Input : 
// mousex -
// mousey -
// fov -
// vecRenderOrigin - 
// vecRenderAngles -
// Output :
// vecPickingRay
//--------------------------------------------------------------------------
void ScreenToWorld( int mousex, int mousey, float fov,
					const Vector& vecRenderOrigin,
					const QAngle& vecRenderAngles,
					Vector& vecPickingRay )
{
	float dx, dy;
	float c_x, c_y;
	float dist;
	Vector vpn, vup, vright;

	float scaled_fov = ScaleFOVByWidthRatio( fov, engine->GetScreenAspectRatio() * 0.75f );

	c_x = ScreenWidth() / 2;
	c_y = ScreenHeight() / 2;

	dx = (float)mousex - c_x;
	// Invert Y
	dy = c_y - (float)mousey;

	// Convert view plane distance
	dist = c_x / tan( M_PI * scaled_fov / 360.0 );

	// Decompose view angles
	AngleVectors( vecRenderAngles, &vpn, &vright, &vup );

	// Offset forward by view plane distance, and then by pixel offsets
	vecPickingRay = vpn * dist + vright * ( dx ) + vup * ( dy );

	// Convert to unit vector
	VectorNormalize( vecPickingRay );
} 

//-----------------------------------------------------------------------------
// Purpose: Deal with input
//-----------------------------------------------------------------------------
void C_VGuiScreen::ClientThink( void )
{
	int nButtonsChanged = m_nOldButtonState ^ m_nButtonState;

	m_nOldButtonState = m_nButtonState;

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_nButtonPressed =  nButtonsChanged & m_nButtonState;		// The changed ones still down are "pressed"
	m_nButtonReleased = nButtonsChanged & (~m_nButtonState);	// The ones not down are "released"

	BaseClass::ClientThink();

	// FIXME: We should really be taking bob, shake, and roll into account
	// but if we did, then all the inputs would be generated multiple times
	// if the world was rendered multiple times (for things like water, etc.)

	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
	if (!pPanel)
		return;
	
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pLocalPlayer)
		return;

	// Generate a ray along the view direction
	Vector vecEyePosition = pLocalPlayer->EyePosition();
	
	QAngle viewAngles = pLocalPlayer->EyeAngles( );

	// Compute cursor position...
	Ray_t lookDir;
	Vector endPos;
	
	float u, v;

	// Viewmodel attached screens that take input need to have a moving cursor
	// Do a pick under the cursor as our selection
	Vector viewDir;
	AngleVectors( viewAngles, &viewDir );
	VectorMA( vecEyePosition, 1000.0f, viewDir, endPos );
	lookDir.Init( vecEyePosition, endPos );

	if (!IntersectWithRay( lookDir, &u, &v, NULL ))
		return;

	if ( ((u < 0) || (v < 0) || (u > 1) || (v > 1)) && !m_bLoseThinkNextFrame)
		return;

	// This will cause our panel to grab all input!
	g_pClientMode->ActivateInGameVGuiContext( pPanel );

	// Convert (u,v) into (px,py)
	int px = (int)(u * m_nPixelWidth + 0.5f);
	int py = (int)(v * m_nPixelHeight + 0.5f);

	// Generate mouse input commands
	if ((px != m_nOldPx) || (py != m_nOldPy))
	{
		g_InputInternal->InternalCursorMoved( px, py );
		m_nOldPx = px;
		m_nOldPy = py;
	}

	if (m_nButtonPressed & IN_ATTACK)
	{
		g_InputInternal->SetMouseCodeState( MOUSE_LEFT, vgui::BUTTON_PRESSED );
		g_InputInternal->InternalMousePressed(MOUSE_LEFT);
	}
	if (m_nButtonPressed & IN_ATTACK2)
	{
		g_InputInternal->SetMouseCodeState( MOUSE_RIGHT, vgui::BUTTON_PRESSED );
		g_InputInternal->InternalMousePressed( MOUSE_RIGHT );
	}
	if ( (m_nButtonReleased & IN_ATTACK) || m_bLoseThinkNextFrame) // for a button release on loosing focus
	{
		g_InputInternal->SetMouseCodeState( MOUSE_LEFT, vgui::BUTTON_RELEASED );
		g_InputInternal->InternalMouseReleased( MOUSE_LEFT );
	}
	if (m_nButtonReleased & IN_ATTACK2)
	{
		g_InputInternal->SetMouseCodeState( MOUSE_RIGHT, vgui::BUTTON_RELEASED );
		g_InputInternal->InternalMouseReleased( MOUSE_RIGHT );
	}

	if ( m_bLoseThinkNextFrame == true )
	{
		m_bLoseThinkNextFrame = false;
		SetNextClientThink( CLIENT_THINK_NEVER );
	}

	g_pClientMode->DeactivateInGameVGuiContext( );
}


//-----------------------------------------------------------------------------
// Computes control points of the quad describing the screen
//-----------------------------------------------------------------------------
void C_VGuiScreen::ComputeEdges( Vector *pUpperLeft, Vector *pUpperRight, Vector *pLowerLeft )
{
	Vector vecOrigin = GetAbsOrigin();
	Vector xaxis, yaxis;
	AngleVectors( GetAbsAngles(), &xaxis, &yaxis, NULL );

	// NOTE: Have to multiply by -1 here because yaxis goes out the -y axis in AngleVectors actually...
	yaxis *= -1.0f;

	VectorCopy( vecOrigin, *pLowerLeft );
	VectorMA( vecOrigin, m_flHeight, yaxis, *pUpperLeft );
	VectorMA( *pUpperLeft, m_flWidth, xaxis, *pUpperRight );
}

void C_VGuiScreen::GetWorldQuad(
    Vector *pLowerLeft, Vector *pWidth, Vector *pHeight, Vector *pNormal )
{
	Vector upperLeft, upperRight;
	ComputeEdges( &upperLeft, &upperRight, pLowerLeft );
	*pWidth = upperRight - upperLeft;
	*pHeight = upperLeft - *pLowerLeft;
	GetVectors( NULL, NULL, pNormal );
}

//-----------------------------------------------------------------------------
// Return intersection point of ray with screen in barycentric coords
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IntersectWithRay( const Ray_t &ray, float *u, float *v, float *t )
{
	// Perform a raycast to see where in barycentric coordinates the ray hits
	// the viewscreen; if it doesn't hit it, you're not in the mode
	Vector origin, upt, vpt;
	ComputeEdges( &origin, &upt, &vpt );
	return ComputeIntersectionBarycentricCoordinates( ray, origin, upt, vpt, *u, *v, t );
}


//-----------------------------------------------------------------------------
// Is the vgui screen backfacing?
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsBackfacing( const Vector &viewOrigin )
{
	// Compute a ray from camera to center of the screen..
	Vector cameraToScreen;
	VectorSubtract( GetAbsOrigin(), viewOrigin, cameraToScreen );

	// Figure out the face normal
	Vector zaxis;
	GetVectors( NULL, NULL, &zaxis );

	// The actual backface cull
	return (DotProduct( zaxis, cameraToScreen ) > 0.0f);
}


//-----------------------------------------------------------------------------
//  Computes the panel center to world transform
//-----------------------------------------------------------------------------
void C_VGuiScreen::ComputePanelToWorld()
{
	// The origin is at the upper-left corner of the screen
	Vector vecOrigin, vecUR, vecLL;
	ComputeEdges( &vecOrigin, &vecUR, &vecLL );
	m_PanelToWorld.SetupMatrixOrgAngles( vecOrigin, GetAbsAngles() );
}


//-----------------------------------------------------------------------------
// a pass to set the z buffer...
//-----------------------------------------------------------------------------
void C_VGuiScreen::DrawScreenOverlay()
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PushMatrix();
	pRenderContext->LoadMatrix( m_PanelToWorld );

	unsigned char pColor[4] = {255, 255, 255, 255};

	CMeshBuilder meshBuilder;
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_OverlayMaterial );
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Position3f( 0.0f, 0.0f, 0 );
	meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( m_flWidth, 0.0f, 0 );
	meshBuilder.TexCoord2f( 0, 1.0f, 0.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( m_flWidth, -m_flHeight, 0 );
	meshBuilder.TexCoord2f( 0, 1.0f, 1.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( 0.0f, -m_flHeight, 0 );
	meshBuilder.TexCoord2f( 0, 0.0f, 1.0f );
	meshBuilder.Color4ubv( pColor );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->PopMatrix();
}


//-----------------------------------------------------------------------------
// Draws the panel using a 3D transform...
//-----------------------------------------------------------------------------
int	C_VGuiScreen::DrawModel( int flags )
{
	vgui::Panel *pPanel = m_PanelWrapper.GetPanel();
#ifdef PORTAL2
	// Portal 2 port: panels see the screen's active state as their enabled
	// state; the lightboards flicker on when it changes.
	if ( pPanel )
	{
		pPanel->SetEnabled( IsActive() );
	}
#endif
	if (!pPanel || !IsActive())
		return 0;

	// Don't bother drawing stuff not visible to me...
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pLocalPlayer || !IsVisibleToTeam(pLocalPlayer->GetTeamNumber()) )
		return 0;

	if ( !IsVisibleToPlayer( pLocalPlayer ) )
	{
		return 0;
	}
	
	// Backface cull the entire panel here...
	if (IsBackfacing(CurrentViewOrigin()))
		return 0;

	// Recompute the panel-to-world center
	// FIXME: Can this be cached off?
	ComputePanelToWorld();

	// The core's physical surface owns its depth too. Keep the legacy overlay
	// only with a view the legacy panel path draws.
	if ( DrawOnRenderCore() )
		return 1;
	g_pMatSystemSurface->DrawPanelIn3DSpace(
	    pPanel->GetVPanel(), m_PanelToWorld, m_nPixelWidth, m_nPixelHeight, m_flWidth, m_flHeight );
	DrawScreenOverlay();

	return 1;
}

bool C_VGuiScreen::ShouldDraw( void )
{
	return !IsEffectActive(EF_NODRAW);
}


//-----------------------------------------------------------------------------
// Purpose: Hook for vgui screens to determine visibility
//-----------------------------------------------------------------------------
bool C_VGuiScreen::IsVisibleToPlayer( C_BasePlayer *pViewingPlayer )
{
	return true;
}

bool C_VGuiScreen::IsTransparent( void )
{
	return (m_fScreenFlags & VGUI_SCREEN_TRANSPARENT) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Sometimes we only want a specific player to be able to input to a panel
//-----------------------------------------------------------------------------
C_BasePlayer *C_VGuiScreen::GetPlayerOwner( void )
{
	return ( C_BasePlayer * )( m_hPlayerOwner.Get() );
}

bool C_VGuiScreen::IsInputOnlyToOwner( void )
{
	return (m_fScreenFlags & VGUI_SCREEN_ONLY_USABLE_BY_OWNER) != 0;
}

//-----------------------------------------------------------------------------
//
// Enumator class for finding vgui screens close to the local player
//
//-----------------------------------------------------------------------------
class CVGuiScreenEnumerator : public IPartitionEnumerator
{
	DECLARE_CLASS_GAMEROOT( CVGuiScreenEnumerator, IPartitionEnumerator );
public:
	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity );

	int	GetScreenCount();
	C_VGuiScreen *GetVGuiScreen( int index );

private:
	CUtlVector< CHandle< C_VGuiScreen > > m_VguiScreens;
};

IterationRetval_t CVGuiScreenEnumerator::EnumElement( IHandleEntity *pHandleEntity )
{
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
	if ( pEnt == NULL )
		return ITERATION_CONTINUE;

	// FIXME.. pretty expensive...
	C_VGuiScreen *pScreen = dynamic_cast<C_VGuiScreen*>(pEnt); 
	if ( pScreen )
	{
		int i = m_VguiScreens.AddToTail( );
		m_VguiScreens[i].Set( pScreen );
	}

	return ITERATION_CONTINUE;
}

int	CVGuiScreenEnumerator::GetScreenCount()
{
	return m_VguiScreens.Count();
}

C_VGuiScreen *CVGuiScreenEnumerator::GetVGuiScreen( int index )
{
	return m_VguiScreens[index].Get();
}	


//-----------------------------------------------------------------------------
//
// Look for vgui screens, returns true if it found one ...
//
//-----------------------------------------------------------------------------
C_BaseEntity *FindNearbyVguiScreen( const Vector &viewPosition, const QAngle &viewAngle, int nTeam )
{
	

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	Assert( pLocalPlayer );

	if ( !pLocalPlayer )
		return NULL;

	// Get the view direction...
	Vector lookDir;
	AngleVectors( viewAngle, &lookDir );

	// Create a ray used for raytracing 
	Vector lookEnd;
	VectorMA( viewPosition, 2.0f * VGUI_SCREEN_MODE_RADIUS, lookDir, lookEnd );

	Ray_t lookRay;
	lookRay.Init( viewPosition, lookEnd );

	// Look for vgui screens that are close to the player
	CVGuiScreenEnumerator localScreens;
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_NON_STATIC_EDICTS, viewPosition, VGUI_SCREEN_MODE_RADIUS, false, &localScreens );

	Vector vecOut, vecViewDelta;

	float flBestDist = 2.0f;
	C_VGuiScreen *pBestScreen = NULL;
	for (int i = localScreens.GetScreenCount(); --i >= 0; )
	{
		C_VGuiScreen *pScreen = localScreens.GetVGuiScreen(i);

		if ( pScreen->IsAttachedToViewModel() )
			continue;

		// Don't bother with screens I'm behind...
		// Hax - don't cancel backfacing with viewmodel attached screens.
		// we can get prediction bugs that make us backfacing for one frame and
		// it resets the mouse position if we lose focus.
		if ( pScreen->IsBackfacing(viewPosition) )
			continue;

		// Don't bother with screens that are turned off
		if (!pScreen->IsActive())
			continue;

		// FIXME: Should this maybe go into a derived class of some sort?
		// Don't bother with screens on the wrong team
		if (!pScreen->IsVisibleToTeam(nTeam))
			continue;

		if ( !pScreen->AcceptsInput() )
			continue;

		if ( pScreen->IsInputOnlyToOwner() && pScreen->GetPlayerOwner() != pLocalPlayer )
			continue;

		// Test perpendicular distance from the screen...
		pScreen->GetVectors( NULL, NULL, &vecOut );
		VectorSubtract( viewPosition, pScreen->GetAbsOrigin(), vecViewDelta );
		float flPerpDist = DotProduct(vecViewDelta, vecOut);
		if ( (flPerpDist < 0) || (flPerpDist > VGUI_SCREEN_MODE_RADIUS) )
			continue;

		// Perform a raycast to see where in barycentric coordinates the ray hits
		// the viewscreen; if it doesn't hit it, you're not in the mode
		float u, v, t;
		if (!pScreen->IntersectWithRay( lookRay, &u, &v, &t ))
			continue;

		// Barycentric test
		if ((u < 0) || (v < 0) || (u > 1) || (v > 1))
			continue;

		if ( t < flBestDist )
		{
			flBestDist = t;
			pBestScreen = pScreen;
		}
	}
	
	return pBestScreen;
}

void ActivateVguiScreen( C_BaseEntity *pVguiScreenEnt )
{
	if (pVguiScreenEnt)
	{
		Assert( dynamic_cast<C_VGuiScreen*>(pVguiScreenEnt) );
		C_VGuiScreen *pVguiScreen = static_cast<C_VGuiScreen*>(pVguiScreenEnt);
		pVguiScreen->GainFocus( );
	}
}

void SetVGuiScreenButtonState( C_BaseEntity *pVguiScreenEnt, int nButtonState )
{
	if (pVguiScreenEnt)
	{
		Assert( dynamic_cast<C_VGuiScreen*>(pVguiScreenEnt) );
		C_VGuiScreen *pVguiScreen = static_cast<C_VGuiScreen*>(pVguiScreenEnt);
		pVguiScreen->SetButtonState( nButtonState );
	}
}

void DeactivateVguiScreen( C_BaseEntity *pVguiScreenEnt )
{
	if (pVguiScreenEnt)
	{
		Assert( dynamic_cast<C_VGuiScreen*>(pVguiScreenEnt) );
		C_VGuiScreen *pVguiScreen = static_cast<C_VGuiScreen*>(pVguiScreenEnt);
		pVguiScreen->LoseFocus( );
	}
}

CVGuiScreenPanel::CVGuiScreenPanel( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_hEntity = NULL;
	EmissiveAreaLights_AddSource( this );
}

CVGuiScreenPanel::CVGuiScreenPanel( vgui::Panel *parent, const char *panelName, vgui::HScheme hScheme )
	: BaseClass( parent, panelName, hScheme )
{
	m_hEntity = NULL;
	EmissiveAreaLights_AddSource( this );
}

CVGuiScreenPanel::~CVGuiScreenPanel()
{
	EmissiveAreaLights_RemoveSource( this );
}

int CVGuiScreenPanel::GetAreaLights( area_light::AreaLight *pLights, int *pKeys, int nMax )
{
	C_VGuiScreen *pScreen = dynamic_cast<C_VGuiScreen *>( GetEntity() );
	return DrawsAsEmissiveSurface() && pScreen ? pScreen->EmissiveAreaLights( pLights, pKeys, nMax )
	                                           : 0;
}

bool CVGuiScreenPanel::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	const char *pResFile = pKeyValues->GetString( "resfile" );
	if (pResFile[0] != 0)
	{
		KeyValues *pCachedKeyValues = CacheKeyValuesForFile( pResFile );
		LoadControlSettings( pResFile, NULL, pCachedKeyValues );
	}

	// Dimensions in pixels
	int nWidth, nHeight;
	nWidth = pKeyValues->GetInt( "pixelswide", 240 );
	nHeight = pKeyValues->GetInt( "pixelshigh", 160 );
	if ((nWidth <= 0) || (nHeight <= 0))
		return false;

	// If init data isn't specified, then we're just precaching.
	if ( pInitData )
	{
		m_hEntity.Set( pInitData->m_pEntity );

		C_VGuiScreen *screen = dynamic_cast< C_VGuiScreen * >( pInitData->m_pEntity );
		if ( screen )
		{
			bool acceptsInput = pKeyValues->GetInt( "acceptsinput", 1 ) ? true : false;
			screen->SetAcceptsInput( acceptsInput );
		}
	}

	SetBounds( 0, 0, nWidth, nHeight );

	return true;
}

vgui::Panel *CVGuiScreenPanel::CreateControlByName(const char *controlName)
{
	// Check the panel metaclass manager to make these controls...
	if (!Q_strncmp(controlName, "MaterialImage", 20))
	{
		return new CBitmapPanel(NULL, "BitmapPanel");
	}

	if (!Q_strncmp(controlName, "MaterialButton", 20))
	{
		return new CBitmapButton(NULL, "BitmapButton", "");
	}

	// Didn't find it? Just use the default stuff
	return BaseClass::CreateControlByName( controlName );
}

//-----------------------------------------------------------------------------
// Purpose: Called when the user presses a button
//-----------------------------------------------------------------------------
void CVGuiScreenPanel::OnCommand( const char *command)
{
	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( const_cast<char *>( command ) );
	}

	BaseClass::OnCommand(command);
}

DECLARE_VGUI_SCREEN_FACTORY( CVGuiScreenPanel, "vgui_screen_panel" );