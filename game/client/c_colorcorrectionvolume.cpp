//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Color correction entity.
//
 // $NoKeywords: $
//===========================================================================//
#include "cbase.h"

#include "filesystem.h"
#include "cdll_client_int.h"
#include "materialsystem/MaterialSystemUtil.h"
#include "colorcorrectionmgr.h"
#ifdef PORTAL2
#include "c_triggers.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Purpose : Shadow control entity
//------------------------------------------------------------------------------
#ifdef PORTAL2
// The server volume is a networked CBaseTrigger in Portal 2 (DT_BaseTrigger).
class C_ColorCorrectionVolume : public C_BaseTrigger
{
public:
	DECLARE_CLASS( C_ColorCorrectionVolume, C_BaseTrigger );
#else
class C_ColorCorrectionVolume : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_ColorCorrectionVolume, C_BaseEntity );
#endif

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_ColorCorrectionVolume();
	virtual ~C_ColorCorrectionVolume();

	void OnDataChanged(DataUpdateType_t updateType);
	bool ShouldDraw();

	void ClientThink();

private:
	float	m_Weight;
	char	m_lookupFilename[MAX_PATH];

	ClientCCHandle_t m_CCHandle;
};

IMPLEMENT_CLIENTCLASS_DT(C_ColorCorrectionVolume, DT_ColorCorrectionVolume, CColorCorrectionVolume)
	RecvPropFloat( RECVINFO(m_Weight) ),
	RecvPropString( RECVINFO(m_lookupFilename) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ColorCorrectionVolume )
	DEFINE_PRED_FIELD( m_Weight, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()


//------------------------------------------------------------------------------
// Constructor, destructor
//------------------------------------------------------------------------------
C_ColorCorrectionVolume::C_ColorCorrectionVolume()
{
	m_CCHandle = INVALID_CLIENT_CCHANDLE;
}

C_ColorCorrectionVolume::~C_ColorCorrectionVolume()
{
	g_pColorCorrectionMgr->RemoveColorCorrection( m_CCHandle );
}


//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void C_ColorCorrectionVolume::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		if ( m_CCHandle == INVALID_CLIENT_CCHANDLE )
		{
			char filename[MAX_PATH];
			Q_strncpy( filename, m_lookupFilename, MAX_PATH );

			m_CCHandle = g_pColorCorrectionMgr->AddColorCorrection( filename );
			SetNextClientThink( ( m_CCHandle != INVALID_CLIENT_CCHANDLE ) ? CLIENT_THINK_ALWAYS : CLIENT_THINK_NEVER );
		}
	}
}

//------------------------------------------------------------------------------
// We don't draw...
//------------------------------------------------------------------------------
bool C_ColorCorrectionVolume::ShouldDraw()
{
	return false;
}

void C_ColorCorrectionVolume::ClientThink()
{
	Vector entityPosition = GetAbsOrigin();
	g_pColorCorrectionMgr->SetColorCorrectionWeight( m_CCHandle, m_Weight );
}













