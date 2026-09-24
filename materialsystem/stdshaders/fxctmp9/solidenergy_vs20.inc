#include "shaderlib/cshader.h"
class solidenergy_vs20_Static_Index
{
private:
	int m_nDETAIL1;
#ifdef _DEBUG
	bool m_bDETAIL1;
#endif
public:
	void SetDETAIL1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAIL1 = i;
#ifdef _DEBUG
		m_bDETAIL1 = true;
#endif
	}
	void SetDETAIL1( bool i )
	{
		m_nDETAIL1 = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAIL1 = true;
#endif
	}
private:
	int m_nDETAIL2;
#ifdef _DEBUG
	bool m_bDETAIL2;
#endif
public:
	void SetDETAIL2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAIL2 = i;
#ifdef _DEBUG
		m_bDETAIL2 = true;
#endif
	}
	void SetDETAIL2( bool i )
	{
		m_nDETAIL2 = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAIL2 = true;
#endif
	}
private:
	int m_nTANGENTTOPACITY;
#ifdef _DEBUG
	bool m_bTANGENTTOPACITY;
#endif
public:
	void SetTANGENTTOPACITY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTANGENTTOPACITY = i;
#ifdef _DEBUG
		m_bTANGENTTOPACITY = true;
#endif
	}
	void SetTANGENTTOPACITY( bool i )
	{
		m_nTANGENTTOPACITY = i ? 1 : 0;
#ifdef _DEBUG
		m_bTANGENTTOPACITY = true;
#endif
	}
private:
	int m_nTANGENTSOPACITY;
#ifdef _DEBUG
	bool m_bTANGENTSOPACITY;
#endif
public:
	void SetTANGENTSOPACITY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTANGENTSOPACITY = i;
#ifdef _DEBUG
		m_bTANGENTSOPACITY = true;
#endif
	}
	void SetTANGENTSOPACITY( bool i )
	{
		m_nTANGENTSOPACITY = i ? 1 : 0;
#ifdef _DEBUG
		m_bTANGENTSOPACITY = true;
#endif
	}
private:
	int m_nFRESNELOPACITY;
#ifdef _DEBUG
	bool m_bFRESNELOPACITY;
#endif
public:
	void SetFRESNELOPACITY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFRESNELOPACITY = i;
#ifdef _DEBUG
		m_bFRESNELOPACITY = true;
#endif
	}
	void SetFRESNELOPACITY( bool i )
	{
		m_nFRESNELOPACITY = i ? 1 : 0;
#ifdef _DEBUG
		m_bFRESNELOPACITY = true;
#endif
	}
private:
	int m_nVERTEXCOLOR;
#ifdef _DEBUG
	bool m_bVERTEXCOLOR;
#endif
public:
	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif
	}
	void SetVERTEXCOLOR( bool i )
	{
		m_nVERTEXCOLOR = i ? 1 : 0;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif
	}
private:
	int m_nFLOWMAP;
#ifdef _DEBUG
	bool m_bFLOWMAP;
#endif
public:
	void SetFLOWMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLOWMAP = i;
#ifdef _DEBUG
		m_bFLOWMAP = true;
#endif
	}
	void SetFLOWMAP( bool i )
	{
		m_nFLOWMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLOWMAP = true;
#endif
	}
private:
	int m_nMODELFORMAT;
#ifdef _DEBUG
	bool m_bMODELFORMAT;
#endif
public:
	void SetMODELFORMAT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMODELFORMAT = i;
#ifdef _DEBUG
		m_bMODELFORMAT = true;
#endif
	}
	void SetMODELFORMAT( bool i )
	{
		m_nMODELFORMAT = i ? 1 : 0;
#ifdef _DEBUG
		m_bMODELFORMAT = true;
#endif
	}
public:
	solidenergy_vs20_Static_Index( )
	{
#ifdef _DEBUG
		m_bDETAIL1 = false;
#endif // _DEBUG
		m_nDETAIL1 = 0;
#ifdef _DEBUG
		m_bDETAIL2 = false;
#endif // _DEBUG
		m_nDETAIL2 = 0;
#ifdef _DEBUG
		m_bTANGENTTOPACITY = false;
#endif // _DEBUG
		m_nTANGENTTOPACITY = 0;
#ifdef _DEBUG
		m_bTANGENTSOPACITY = false;
#endif // _DEBUG
		m_nTANGENTSOPACITY = 0;
#ifdef _DEBUG
		m_bFRESNELOPACITY = false;
#endif // _DEBUG
		m_nFRESNELOPACITY = 0;
#ifdef _DEBUG
		m_bVERTEXCOLOR = false;
#endif // _DEBUG
		m_nVERTEXCOLOR = 0;
#ifdef _DEBUG
		m_bFLOWMAP = false;
#endif // _DEBUG
		m_nFLOWMAP = 0;
#ifdef _DEBUG
		m_bMODELFORMAT = false;
#endif // _DEBUG
		m_nMODELFORMAT = 0;
	}
	int GetIndex()
	{
#ifdef _DEBUG
		bool bAllDefined = m_bDETAIL1 && m_bDETAIL2 && m_bTANGENTTOPACITY && m_bTANGENTSOPACITY && m_bFRESNELOPACITY && m_bVERTEXCOLOR && m_bFLOWMAP && m_bMODELFORMAT;
		Assert( bAllDefined );
#endif // _DEBUG
		return ( 16 * m_nDETAIL1 ) + ( 32 * m_nDETAIL2 ) + ( 64 * m_nTANGENTTOPACITY ) + ( 128 * m_nTANGENTSOPACITY ) + ( 256 * m_nFRESNELOPACITY ) + ( 512 * m_nVERTEXCOLOR ) + ( 1024 * m_nFLOWMAP ) + ( 2048 * m_nMODELFORMAT ) + 0;
	}
};
#define shaderStaticTest_solidenergy_vs20 vsh_forgot_to_set_static_DETAIL1 + vsh_forgot_to_set_static_DETAIL2 + vsh_forgot_to_set_static_TANGENTTOPACITY + vsh_forgot_to_set_static_TANGENTSOPACITY + vsh_forgot_to_set_static_FRESNELOPACITY + vsh_forgot_to_set_static_VERTEXCOLOR + vsh_forgot_to_set_static_FLOWMAP + vsh_forgot_to_set_static_MODELFORMAT + 0
class solidenergy_vs20_Dynamic_Index
{
private:
	int m_nCOMPRESSED_VERTS;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS;
#endif
public:
	void SetCOMPRESSED_VERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMPRESSED_VERTS = i;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif
	}
	void SetCOMPRESSED_VERTS( bool i )
	{
		m_nCOMPRESSED_VERTS = i ? 1 : 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif
	}
private:
	int m_nSKINNING;
#ifdef _DEBUG
	bool m_bSKINNING;
#endif
public:
	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif
	}
	void SetSKINNING( bool i )
	{
		m_nSKINNING = i ? 1 : 0;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif
	}
private:
	int m_nVORTEX1;
#ifdef _DEBUG
	bool m_bVORTEX1;
#endif
public:
	void SetVORTEX1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVORTEX1 = i;
#ifdef _DEBUG
		m_bVORTEX1 = true;
#endif
	}
	void SetVORTEX1( bool i )
	{
		m_nVORTEX1 = i ? 1 : 0;
#ifdef _DEBUG
		m_bVORTEX1 = true;
#endif
	}
private:
	int m_nVORTEX2;
#ifdef _DEBUG
	bool m_bVORTEX2;
#endif
public:
	void SetVORTEX2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVORTEX2 = i;
#ifdef _DEBUG
		m_bVORTEX2 = true;
#endif
	}
	void SetVORTEX2( bool i )
	{
		m_nVORTEX2 = i ? 1 : 0;
#ifdef _DEBUG
		m_bVORTEX2 = true;
#endif
	}
public:
	solidenergy_vs20_Dynamic_Index( )
	{
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
#endif // _DEBUG
		m_nCOMPRESSED_VERTS = 0;
#ifdef _DEBUG
		m_bSKINNING = false;
#endif // _DEBUG
		m_nSKINNING = 0;
#ifdef _DEBUG
		m_bVORTEX1 = false;
#endif // _DEBUG
		m_nVORTEX1 = 0;
#ifdef _DEBUG
		m_bVORTEX2 = false;
#endif // _DEBUG
		m_nVORTEX2 = 0;
	}
	int GetIndex()
	{
#ifdef _DEBUG
		bool bAllDefined = m_bCOMPRESSED_VERTS && m_bSKINNING && m_bVORTEX1 && m_bVORTEX2;
		Assert( bAllDefined );
#endif // _DEBUG
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nVORTEX1 ) + ( 8 * m_nVORTEX2 ) + 0;
	}
};
#define shaderDynamicTest_solidenergy_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_VORTEX1 + vsh_forgot_to_set_dynamic_VORTEX2 + 0
