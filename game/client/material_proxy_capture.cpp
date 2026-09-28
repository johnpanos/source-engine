//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy side of the material proxy corpus (RFC 0016 K4
//			"Proxy corpus", tools/render/proxy_corpus.py). mat_proxy_capture
//			writes, as JSON lines, the material proxies this client registers
//			(the factory the engine's CMaterialProxyFactory asks) and, for
//			each material matching a pattern, every material variable after its
//			proxies bind: once with no proxy data (as world surfaces bind) and
//			once with the local player's renderable (as models bind). The
//			render core's frontend will run the same materials at the same
//			time and must write the same values (K3).
//
//=============================================================================//

#include "cbase.h"
#include "c_baseplayer.h"
#include "filesystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "tier1/interface.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlstring.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void WriteJsonString( CUtlBuffer &out, const char *pText )
{
	out.PutChar( '"' );
	for ( const unsigned char *p = (const unsigned char *)( pText ? pText : "" ); *p; ++p )
	{
		if ( *p == '"' || *p == '\\' )
		{
			out.PutChar( '\\' );
			out.PutChar( *p );
		}
		else if ( *p < 0x20 || *p >= 0x7f )
		{
			out.Printf( "\\u%04x", *p );
		}
		else
		{
			out.PutChar( *p );
		}
	}
	out.PutChar( '"' );
}

static void WriteVariable( CUtlBuffer &out, IMaterialVar *pVar )
{
	out.PutString( "{\"name\":" );
	WriteJsonString( out, pVar->GetName() );
	switch ( pVar->GetType() )
	{
	case MATERIAL_VAR_TYPE_FLOAT:
		out.Printf( ",\"type\":\"float\",\"value\":%.9g}", pVar->GetFloatValue() );
		return;
	case MATERIAL_VAR_TYPE_INT:
		out.Printf( ",\"type\":\"int\",\"value\":%d}", pVar->GetIntValue() );
		return;
	case MATERIAL_VAR_TYPE_VECTOR:
	{
		float values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		const int nComps = clamp( pVar->VectorSize(), 1, 4 );
		pVar->GetVecValue( values, nComps );
		out.PutString( ",\"type\":\"vector\",\"value\":[" );
		for ( int i = 0; i < nComps; ++i )
			out.Printf( "%s%.9g", i ? "," : "", values[i] );
		out.PutString( "]}" );
		return;
	}
	case MATERIAL_VAR_TYPE_MATRIX:
	{
		const VMatrix &matrix = pVar->GetMatrixValue();
		out.PutString( ",\"type\":\"matrix\",\"value\":[" );
		for ( int i = 0; i < 16; ++i )
			out.Printf( "%s%.9g", i ? "," : "", matrix.m[i / 4][i % 4] );
		out.PutString( "]}" );
		return;
	}
	case MATERIAL_VAR_TYPE_TEXTURE:
	{
		ITexture *pTexture = pVar->GetTextureValue();
		out.PutString( ",\"type\":\"texture\",\"value\":" );
		WriteJsonString( out, pTexture ? pTexture->GetName() : "" );
		out.Printf( ",\"frame\":%d}", pVar->GetIntValue() );
		return;
	}
	case MATERIAL_VAR_TYPE_MATERIAL:
	{
		IMaterial *pMaterial = pVar->GetMaterialValue();
		out.PutString( ",\"type\":\"material\",\"value\":" );
		WriteJsonString( out, pMaterial ? pMaterial->GetName() : "" );
		out.PutChar( '}' );
		return;
	}
	default:
		out.PutString( ",\"type\":\"string\",\"value\":" );
		WriteJsonString( out, pVar->GetStringValue() );
		out.PutChar( '}' );
		return;
	}
}

static void WriteMaterial( CUtlBuffer &out, IMaterial *pMaterial, const char *pPass )
{
	out.PutString( "{\"kind\":\"material\",\"name\":" );
	WriteJsonString( out, pMaterial->GetName() );
	out.PutString( ",\"pass\":" );
	WriteJsonString( out, pPass );
	out.Printf( ",\"has_proxy\":%s,\"vars\":[", pMaterial->HasProxy() ? "true" : "false" );
	// Undefined variables are left out: the comparator reads absence as undefined.
	IMaterialVar **ppVars = pMaterial->GetShaderParams();
	const int nVars = pMaterial->ShaderParamCount();
	bool bFirst = true;
	for ( int i = 0; i < nVars; ++i )
	{
		if ( !ppVars[i]->IsDefined() )
			continue;
		if ( !bFirst )
			out.PutChar( ',' );
		bFirst = false;
		WriteVariable( out, ppVars[i] );
	}
	out.PutString( "]}\n" );
}

static int CompareNames( const CUtlString *pA, const CUtlString *pB )
{
	return Q_strcmp( pA->Get(), pB->Get() );
}

CON_COMMAND( mat_proxy_capture,
    "mat_proxy_capture <output file> <materials/dir/*.vmt>: writes the registered material "
    "proxies and, for each matching material, its variables after its proxies bind with no "
    "proxy data and with the local player (RFC 0016 K4 proxy corpus)" )
{
	if ( args.ArgC() != 3 || Q_strnicmp( args[2], "materials/", 10 ) )
	{
		Warning( "usage: mat_proxy_capture <output file> <materials/dir/*.vmt>\n" );
		return;
	}
	// The matching materials, sorted so every capture lists them in one order.
	char directory[MAX_PATH];
	Q_strncpy( directory, args[2] + 10, sizeof( directory ) );
	char *pSlash = Q_strrchr( directory, '/' );
	if ( pSlash )
		pSlash[1] = 0;
	else
		directory[0] = 0;
	CUtlVector<CUtlString> names;
	FileFindHandle_t handle;
	for ( const char *pFile = g_pFullFileSystem->FindFirstEx( args[2], "GAME", &handle ); pFile;
	    pFile = g_pFullFileSystem->FindNext( handle ) )
	{
		char name[MAX_PATH];
		Q_snprintf( name, sizeof( name ), "%s%s", directory, pFile );
		Q_StripExtension( name, name, sizeof( name ) );
		Q_strlower( name );
		if ( names.Find( CUtlString( name ) ) == names.InvalidIndex() )
			names.AddToTail( CUtlString( name ) );
	}
	g_pFullFileSystem->FindClose( handle );
	names.Sort( CompareNames );
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	CUtlBuffer out( 0, 0, CUtlBuffer::TEXT_BUFFER );
	out.Printf( "{\"kind\":\"header\",\"schema\":\"source-proxy-capture/v1\",\"curtime\":%.9g,"
	            "\"frametime\":%.9g,\"tickcount\":%d,\"framecount\":%d,\"player\":%s}\n",
	    gpGlobals->curtime, gpGlobals->frametime, gpGlobals->tickcount, gpGlobals->framecount,
	    pPlayer ? "true" : "false" );

	const int nSuffix = Q_strlen( IMATERIAL_PROXY_INTERFACE_VERSION );
	for ( InterfaceReg *pReg = InterfaceReg::s_pInterfaceRegs; pReg; pReg = pReg->m_pNext )
	{
		const int nName = Q_strlen( pReg->m_pName );
		if ( nName <= nSuffix ||
		     Q_strcmp( pReg->m_pName + nName - nSuffix, IMATERIAL_PROXY_INTERFACE_VERSION ) )
			continue;
		char name[128];
		Q_strncpy( name, pReg->m_pName, MIN( (int)sizeof( name ), nName - nSuffix + 1 ) );
		out.PutString( "{\"kind\":\"proxy\",\"name\":" );
		WriteJsonString( out, name );
		out.PutString( "}\n" );
	}

	for ( int i = 0; i < names.Count(); ++i )
	{
		const char *pName = names[i].Get();
		IMaterial *pMaterial = materials->FindMaterial( pName, TEXTURE_GROUP_OTHER, false );
		if ( !pMaterial || pMaterial->IsErrorMaterial() )
		{
			out.PutString( "{\"kind\":\"missing\",\"name\":" );
			WriteJsonString( out, pName );
			out.PutString( "}\n" );
			continue;
		}
		// A fixture names the proxy data its proxy accepts ($proxycapturepasses
		// "none", "player", "none player", or "unbound" for a proxy whose data
		// is a structure of its own, such as a shadow handle): a proxy that
		// casts its data to one entity class is bound only with none.
		bool bFound = false;
		IMaterialVar *pPasses = pMaterial->FindVar( "$proxycapturepasses", &bFound, false );
		const char *pPassList = bFound && pPasses ? pPasses->GetStringValue() : "none player";
		if ( Q_strstr( pPassList, "none" ) )
		{
			Msg( "mat_proxy_capture: %s (none)\n", pName );
			pMaterial->CallBindProxy( NULL );
			WriteMaterial( out, pMaterial, "none" );
		}
		if ( pPlayer && Q_strstr( pPassList, "player" ) )
		{
			Msg( "mat_proxy_capture: %s (player)\n", pName );
			pMaterial->CallBindProxy( pPlayer->GetClientRenderable() );
			WriteMaterial( out, pMaterial, "player" );
		}
		if ( Q_strstr( pPassList, "unbound" ) )
			WriteMaterial( out, pMaterial, "unbound" );
	}
	if ( !g_pFullFileSystem->WriteFile( args[1], "DEFAULT_WRITE_PATH", out ) )
		Warning( "mat_proxy_capture: cannot write %s\n", args[1] );
	else
		Msg( "mat_proxy_capture: wrote %s\n", args[1] );
}
