//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "icvar.h"
#include "tier1/tier1.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ------------------------------------------------------------------------------------------- //
// ConVar stuff.
// ------------------------------------------------------------------------------------------- //
class CShaderLibConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool	RegisterConCommandBase( ConCommandBase *pCommand )
	{
		// Link to engine's list instead
		g_pCVar->RegisterConCommand( pCommand );

		char const *pValue = g_pCVar->GetCommandLineValue( pCommand->GetName() );
		if( pValue && !pCommand->IsCommand() )
		{
			( ( ConVar * )pCommand )->SetValue( pValue );
		}
		return true;
	}
};

CShaderLibConVarAccessor g_ConVarAccessor;


void InitShaderLibCVars( CreateInterfaceFn cvarFactory )
{
	if ( g_pCVar )
	{
		ConVar_Register( FCVAR_MATERIAL_SYSTEM_THREAD, &g_ConVarAccessor );
	}
}

// A linked standard library remains resident between material-system sessions.
// ConVar_Register consumes its constructor list, so this accessor retains the
// module-owned static registrations for reconnect. It is not used by mod DLLs.
class CBuiltinShaderConVarAccessor : public CShaderLibConVarAccessor
{
public:
	bool RegisterConCommandBase( ConCommandBase *command )
	{
		if ( m_Commands.Find( command ) == m_Commands.InvalidIndex() )
			m_Commands.AddToTail( command );
		return CShaderLibConVarAccessor::RegisterConCommandBase( command );
	}

	void Connect()
	{
		int existing = m_Commands.Count();
		ConVar_Register( FCVAR_MATERIAL_SYSTEM_THREAD, this );
		for ( int i = 0; i < existing; ++i )
			RegisterConCommandBase( m_Commands[i] );
	}

	void Disconnect()
	{
		// Individual removal also clears duplicate-name alias registrations that
		// are not present in ICvar's public linked list.
		for ( int i = m_Commands.Count(); --i >= 0; )
			g_pCVar->UnregisterConCommand( m_Commands[i] );
		ConVar_Unregister();
	}

private:
	CUtlVector<ConCommandBase *> m_Commands;
};

static CBuiltinShaderConVarAccessor s_BuiltinAccessor;

void InitBuiltinShaderLibCVars()
{
	s_BuiltinAccessor.Connect();
}

void ShutdownBuiltinShaderLibCVars()
{
	s_BuiltinAccessor.Disconnect();
}
