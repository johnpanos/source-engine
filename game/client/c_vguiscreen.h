//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_VGUISCREEN_H
#define C_VGUISCREEN_H

#ifdef _WIN32
#pragma once
#endif


#include <vgui_controls/EditablePanel.h>
#include "c_baseentity.h"
#include "panelmetaclassmgr.h"
#include "emissive_area_lights.h"
#include "render/world_panel.h"

class KeyValues;
class IEngineWorldPanels;
class IWorldPanelRecorder;
class ITexture;

// RFC 0016 render.pass.panels: the engine's in-world panels and the surface's
// recorder (null when a product has neither).
extern IEngineWorldPanels *g_pEngineWorldPanels;
extern IWorldPanelRecorder *g_pWorldPanelRecorder;

//-----------------------------------------------------------------------------
// Helper macro to make overlay factories one line of code. Use like this:
//	DECLARE_VGUI_SCREEN_FACTORY( CVguiScreenPanel, "image" );
//-----------------------------------------------------------------------------
struct VGuiScreenInitData_t
{
	C_BaseEntity *m_pEntity;

	VGuiScreenInitData_t() : m_pEntity(NULL) {}
	VGuiScreenInitData_t( C_BaseEntity *pEntity ) : m_pEntity(pEntity) {}
};

#define DECLARE_VGUI_SCREEN_FACTORY( _PanelClass, _nameString )	\
	DECLARE_PANEL_FACTORY( _PanelClass, VGuiScreenInitData_t, _nameString )


//-----------------------------------------------------------------------------
// Base class for vgui screen panels
//-----------------------------------------------------------------------------
class CVGuiScreenPanel : public vgui::EditablePanel, public IEmissiveAreaLightSource
{
	DECLARE_CLASS_GAMEROOT( CVGuiScreenPanel, vgui::EditablePanel );

public:
	CVGuiScreenPanel( vgui::Panel *parent, const char *panelName );
	CVGuiScreenPanel( vgui::Panel *parent, const char *panelName, vgui::HScheme hScheme );
	virtual ~CVGuiScreenPanel();
	virtual int GetAreaLights( area_light::AreaLight *pLights, int *pKeys, int nMax );
	bool CoreOnly() const override { return true; }
	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	vgui::Panel *CreateControlByName(const char *controlName);
	virtual void OnCommand( const char *command );

	// RFC 0016 render.pass.panels (render.world-panel.v1): a panel that is a
	// lit board draws as an emissive surface on the render core in the views
	// the core draws, and casts the light of its image
	// (C_VGuiScreen::EmissiveAreaLights). Its emission scale is the scene
	// radiance per decoded image value.
	virtual bool DrawsAsEmissiveSurface() const { return true; }
	virtual float EmissionScale() const { return 1.0f; }
	// What of the panel's paint is a coating on its face (grime): it blocks
	// the board's light and reflects the scene's, emitting none
	// (render/world_panel.h kLayerCoating).
	virtual bool PaintsCoating( ITexture *pTexture ) const { return false; }

protected:
	C_BaseEntity *GetEntity() const { return m_hEntity.Get(); }

private:
	EHANDLE	m_hEntity;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_VGuiScreen : public C_BaseEntity
{
	DECLARE_CLASS( C_VGuiScreen, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	C_VGuiScreen();
	~C_VGuiScreen();

	virtual void PreDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual int DrawModel( int flags );
	virtual bool ShouldDraw( void );
	virtual void ClientThink( );
	virtual void GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pOrigin, QAngle *pAngles );
	virtual bool IsVisibleToPlayer( C_BasePlayer *pViewingPlayer );
	virtual bool IsTransparent( void );

	const char *PanelName() const;

	// The view screen has the cursor pointing at it
	void GainFocus( );
	void LoseFocus();

	// Button state...
	void SetButtonState( int nButtonState );

	// Is the screen backfaced given a view position?
	bool IsBackfacing( const Vector &viewOrigin );

	// Return intersection point of ray with screen in barycentric coords
	bool IntersectWithRay( const Ray_t &ray, float *u, float *v, float *t );

	// The screen's quad in the world: its lower-left corner, its full width
	// and height along its axes, and its front normal (the side it is drawn
	// on). For lights it emits (emissive_area_lights.h).
	void GetWorldQuad( Vector *pLowerLeft, Vector *pWidth, Vector *pHeight, Vector *pNormal );

	// Is the screen turned on?
	bool IsActive() const;

	// RFC 0016 render.pass.panels: the lights this frame's image casts
	// (render.world-panel.v1: a grid of the screen's tiles, each with the mean
	// emission of its part of the image, in the frame's one state), at most
	// nMax, with their keys; 0 when the screen is off or its panel cannot be
	// recorded.
	int EmissiveAreaLights( area_light::AreaLight *pLights, int *pKeys, int nMax );

	// Are we only visible to teammates?
	bool IsVisibleOnlyToTeammates() const;

	// Are we visible to someone on this team?
	bool IsVisibleToTeam( int nTeam );

	bool IsAttachedToViewModel() const;

	virtual RenderGroup_t GetRenderGroup();

	bool AcceptsInput() const;
	void SetAcceptsInput( bool acceptsinput );

	C_BasePlayer *GetPlayerOwner( void );
	bool IsInputOnlyToOwner( void );

private:
	// Vgui screen management
	void CreateVguiScreen( const char *pTypeName );
	void DestroyVguiScreen( );

	//  Computes the panel to world transform
	void ComputePanelToWorld();

	// Computes control points of the quad describing the screen
	void ComputeEdges( Vector *pUpperLeft, Vector *pUpperRight, Vector *pLowerLeft );

	// Writes the z buffer
	void DrawScreenOverlay();

	// RFC 0016 render.pass.panels: the frame's recording of the panel (once
	// per frame, at the resolution the views last asked for), and DrawModel's
	// path through the render core.
	class CPanelRecording;
	bool RecordFrame();
	bool DrawOnRenderCore();
	world_panel::Placement PanelPlacement();
	unsigned long long PanelId() const;
	CUtlVector<world_panel::Quad> m_RecordedQuads;
	CUtlVector<ITexture *> m_RecordedTextures;
	world_panel::TileRadianceCache m_TileRadiance;
	world_panel::Resolution m_PanelResolution;    // what the next recording paints at
	world_panel::Resolution m_RecordedResolution; // what this frame's recording painted at
	int m_nRecordedFrame;
	bool m_bRecordingValid;
	bool m_bRecordingRefusalLogged;
	bool m_bSubmittedToCore;

private:
	int m_nPixelWidth; 
	int m_nPixelHeight;
	float m_flWidth; 
	float m_flHeight;
	int m_nPanelName;	// The name of the panel 
	int	m_nButtonState;
	int m_nButtonPressed;
	int m_nButtonReleased;
	int m_nOldPx;
	int m_nOldPy;
	int m_nOldButtonState;
	int m_nAttachmentIndex;
	int m_nOverlayMaterial;
	int m_fScreenFlags;

	int	m_nOldPanelName;
	int m_nOldOverlayMaterial;

	bool m_bLoseThinkNextFrame;

	bool	m_bAcceptsInput;

	CMaterialReference	m_WriteZMaterial;
	CMaterialReference	m_OverlayMaterial;

	VMatrix	m_PanelToWorld;

	CPanelWrapper m_PanelWrapper;

	CHandle<C_BasePlayer> m_hPlayerOwner;
};


//-----------------------------------------------------------------------------
// Returns an entity that is the nearby vgui screen; NULL if there isn't one
//-----------------------------------------------------------------------------
C_BaseEntity *FindNearbyVguiScreen( const Vector &viewPosition, const QAngle &viewAngle, int nTeam = -1 );


//-----------------------------------------------------------------------------
// Activates/Deactivates vgui screen
//-----------------------------------------------------------------------------
void ActivateVguiScreen( C_BaseEntity *pVguiScreen );
void DeactivateVguiScreen( C_BaseEntity *pVguiScreen );


//-----------------------------------------------------------------------------
// Updates vgui screen button state
//-----------------------------------------------------------------------------
void SetVGuiScreenButtonState( C_BaseEntity *pVguiScreen, int nButtonState );


// Called at shutdown.
void ClearKeyValuesCache();


#endif // C_VGUISCREEN_H
  
