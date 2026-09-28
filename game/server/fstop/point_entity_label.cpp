//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Persistent world-space labels for the F-Stop mechanics map.
//
//=============================================================================//

#include "cbase.h"
#include "ndebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CPointEntityLabel : public CPointEntity
{
	DECLARE_CLASS( CPointEntityLabel, CPointEntity );
	DECLARE_DATADESC();

public:
	void Spawn() override;
	void Activate() override;

private:
	void ResolveTarget();
	void LabelThink();

	string_t m_iszLabel;
	string_t m_iszTargetName;
	EHANDLE m_hTarget;
	Vector m_vecOffset;
};

LINK_ENTITY_TO_CLASS( point_fstop_label, CPointEntityLabel );

BEGIN_DATADESC( CPointEntityLabel )
	DEFINE_KEYFIELD( m_iszLabel, FIELD_STRING, "label" ),
	DEFINE_KEYFIELD( m_iszTargetName, FIELD_STRING, "label_target" ),
	DEFINE_KEYFIELD( m_vecOffset, FIELD_VECTOR, "offset" ),
	DEFINE_FIELD( m_hTarget, FIELD_EHANDLE ),
	DEFINE_THINKFUNC( LabelThink ),
END_DATADESC()

void CPointEntityLabel::Spawn()
{
	SetThink( &CPointEntityLabel::LabelThink );
	SetNextThink( gpGlobals->curtime );
}

void CPointEntityLabel::Activate()
{
	BaseClass::Activate();
	ResolveTarget();
}

void CPointEntityLabel::ResolveTarget()
{
	if ( m_iszTargetName != NULL_STRING )
		m_hTarget = gEntList.FindEntityByName( NULL, STRING( m_iszTargetName ), this );
}

void CPointEntityLabel::LabelThink()
{
	if ( !m_hTarget )
		ResolveTarget();
	if ( m_hTarget && m_iszLabel != NULL_STRING )
		NDebugOverlay::Text( m_hTarget->GetAbsOrigin() + m_vecOffset,
			STRING( m_iszLabel ), true, 0.2f );
	SetNextThink( gpGlobals->curtime + 0.1f );
}
