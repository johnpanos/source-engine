//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#if !defined( VIEW_H )
#define VIEW_H
#ifdef _WIN32
#pragma once
#endif

#if _DEBUG
extern bool g_bRenderingCameraView;		// For debugging (frustum fix for cameras)...
#endif

class VMatrix;
class Vector;
class QAngle;
class VPlane;


// near and far Z it uses to render the world.
#define VIEW_NEARZ	3
//#define VIEW_FARZ	28400


//-----------------------------------------------------------------------------
// There's a difference between the 'current view' and the 'main view'
// The 'main view' is where the player is sitting. Current view is just
// what's currently being rendered, which, owing to monitors or water,
// could be just about anywhere.
//-----------------------------------------------------------------------------
const Vector &MainViewOrigin( int nSlot = -1 );
const QAngle &MainViewAngles( int nSlot = -1 );
const Vector &PrevMainViewOrigin( int nSlot = -1 );
const QAngle &PrevMainViewAngles( int nSlot = -1 );
const VMatrix &MainWorldToViewMatrix( int nSlot = -1 );
const Vector &MainViewForward( int nSlot = -1 );
const Vector &MainViewRight( int nSlot = -1 );
const Vector &MainViewUp( int nSlot = -1 );

// Local split-screen: the part of the window nSlot's view fills (all of it with one local player)
void GetSplitScreenViewRect( int nSlot, const vrect_t &full, vrect_t &out );
bool IsSplitScreenSplitVertical( int nWidth, int nHeight );

const Vector &CurrentViewOrigin();
const QAngle &CurrentViewAngles();
const VMatrix &CurrentWorldToViewMatrix();
const Vector &CurrentViewForward();
const Vector &CurrentViewRight();
const Vector &CurrentViewUp();

void AllowCurrentViewAccess( bool allow );
bool IsCurrentViewAccessAllowed();

// Returns true of the sphere is outside the frustum defined by pPlanes.
// (planes point inwards).
bool R_CullSphere( const VPlane *pPlanes, int nPlanes, const Vector *pCenter, float radius );
float ScaleFOVByWidthRatio( float fovDegrees, float ratio );

extern ConVar mat_wireframe;

extern const ConVar *sv_cheats;


static inline int WireFrameMode( void )
{
	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats && sv_cheats->GetBool() )
		return mat_wireframe.GetInt();
	else
		return 0;
}

static inline bool ShouldDrawInWireFrameMode( void )
{
	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats && sv_cheats->GetBool() )
		return ( mat_wireframe.GetInt() != 0 );
	else
		return false;
}

void ComputeCameraVariables( const Vector &vecOrigin, const QAngle &vecAngles, Vector *pVecForward, Vector *pVecRight, Vector *pVecUp, VMatrix *pMatCamInverse );

#endif // VIEW_H
