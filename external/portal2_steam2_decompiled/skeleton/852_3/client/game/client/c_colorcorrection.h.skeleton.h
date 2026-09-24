// DWARF declaration skeleton for game/client/c_colorcorrection.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/c_colorcorrection.h:20 sizeof=0xbb4 (i386)
struct C_ColorCorrection : public C_BaseEntity
{
public:
	C_ColorCorrection();  // line 26
	virtual ~C_ColorCorrection();  // line 27
	virtual void OnDataChanged( DataUpdateType_t );  // line 29
	virtual bool ShouldDraw();  // line 30
	virtual void Update( C_BasePlayer *, float );  // line 32
	bool IsMaster() const;  // line 34
	bool IsClientSide() const;  // line 35
	bool IsExclusive() const;  // line 36
	void EnableOnClient( bool, bool );  // line 38
	Vector GetOrigin();  // line 40
	float GetMinFalloff();  // line 41
	float GetMaxFalloff();  // line 42
	void SetWeight( float );  // line 44
protected:
	void StartFade( int, float );  // line 47
	float GetFadeRatio( int ) const;  // line 48
	bool IsFadeTimeElapsed( int ) const;  // line 49
	Vector m_vecOrigin; // +0xa5c  // line 51
	float m_minFalloff; // +0xa68  // line 53
	float m_maxFalloff; // +0xa6c  // line 54
	float m_flFadeInDuration; // +0xa70  // line 55
	float m_flFadeOutDuration; // +0xa74  // line 56
	float m_flMaxWeight; // +0xa78  // line 57
	float m_flCurWeight; // +0xa7c  // line 58
	char m_netLookupFilename[260]; // +0xa80  // line 59
	bool m_bEnabled; // +0xb84  // line 61
	bool m_bMaster; // +0xb85  // line 62
	bool m_bClientSide; // +0xb86  // line 63
	bool m_bExclusive; // +0xb87  // line 64
	bool m_bEnabledOnClient[2]; // +0xb88  // line 66
	float m_flCurWeightOnClient[2]; // +0xb8c  // line 67
	bool m_bFadingIn[2]; // +0xb94  // line 68
	float m_flFadeStartWeight[2]; // +0xb98  // line 69
	float m_flFadeStartTime[2]; // +0xba0  // line 70
	float m_flFadeDuration[2]; // +0xba8  // line 71
	ClientCCHandle_t m_CCHandle; // +0xbb0  // line 73
};
