// DWARF declaration skeleton for game/client/clientalphaproperty.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/clientalphaproperty.h:23 sizeof=0x20 (i386)
struct CClientAlphaProperty : public IClientAlphaProperty
{
public:
	virtual IClientUnknown *GetIClientUnknown();  // line 26
	virtual void SetAlphaModulation( uint8 );  // line 27
	virtual void SetRenderFX( RenderFx_t, RenderMode_t, float, float );  // line 28
	virtual void SetFade( float, float, float );  // line 29
	virtual void SetDesyncOffset( int );  // line 30
	virtual void EnableAlphaModulationOverride( bool );  // line 31
	virtual void EnableShadowAlphaModulationOverride( bool );  // line 32
	virtual void SetDistanceFadeMode( ClientAlphaDistanceFadeMode_t );  // line 33
	CClientAlphaProperty();  // line 37
	void Init( IClientUnknown * );  // line 38
	void SetShadowHandle( ClientShadowHandle_t );  // line 41
	uint8 GetAlphaModulation() const;  // line 44
	uint8 ComputeRenderAlpha() const;  // line 47
	float GetMinFadeDist() const;  // line 50
	float GetMaxFadeDist() const;  // line 51
	float GetGlobalFadeScale() const;  // line 52
	bool IgnoresZBuffer() const;  // line 55
private:
	IClientUnknown *m_pOuter; // +0x4  // line 62
	ClientShadowHandle_t m_hShadowHandle; // +0x8  // line 64
	uint16 m_nRenderFX : 5; // +0xa  // line 65
	uint16 m_nRenderMode : 4; // +0xa  // line 66
	uint16 m_bAlphaOverride : 1; // +0xa  // line 67
	uint16 m_bShadowAlphaOverride : 1; // +0xa  // line 68
	uint16 m_nDistanceFadeMode : 1; // +0xa  // line 69
	uint16 m_nReserved : 4; // +0xa  // line 70
	uint16 m_nDesyncOffset; // +0xc  // line 72
	uint8 m_nAlpha; // +0xe  // line 73
	uint8 m_nReserved2; // +0xf  // line 74
	uint16 m_nDistFadeStart; // +0x10  // line 76
	uint16 m_nDistFadeEnd; // +0x12  // line 77
	float m_flFadeScale; // +0x14  // line 79
	float m_flRenderFxStartTime; // +0x18  // line 80
	float m_flRenderFxDuration; // +0x1c  // line 81
};
