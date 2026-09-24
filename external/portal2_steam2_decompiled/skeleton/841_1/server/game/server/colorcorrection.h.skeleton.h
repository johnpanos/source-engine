// DWARF declaration skeleton for game/server/colorcorrection.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/colorcorrection.h:31 sizeof=0x4f8 (i386)
struct CColorCorrection : public CBaseEntity
{
public:
	CColorCorrection();  // line 37
	virtual void Spawn();  // line 39
	virtual int UpdateTransmitState();  // line 40
	virtual void Activate();  // line 41
	virtual int ObjectCaps();  // line 43
	bool IsMaster() const;  // line 45
	bool IsClientSide() const;  // line 47
	bool IsExclusive() const;  // line 49
	void InputEnable( inputdata_t & );  // line 52
	void InputDisable( inputdata_t & );  // line 53
	void InputSetFadeInDuration( inputdata_t & );  // line 54
	void InputSetFadeOutDuration( inputdata_t & );  // line 55
private:
	void FadeIn();  // line 58
	void FadeOut();  // line 59
	void FadeInThink();  // line 61
	void FadeOutThink();  // line 62
	CNetworkVar( float, m_flFadeInDuration ); // +0x3c0  // line 66
	CNetworkVar( float, m_flFadeOutDuration ); // +0x3c4  // line 67
	float m_flStartFadeInWeight; // +0x3c8  // line 68
	float m_flStartFadeOutWeight; // +0x3cc  // line 69
	float m_flTimeStartFadeIn; // +0x3d0  // line 70
	float m_flTimeStartFadeOut; // +0x3d4  // line 71
	CNetworkVar( float, m_flMaxWeight ); // +0x3d8  // line 73
	bool m_bStartDisabled; // +0x3dc  // line 75
	CNetworkVar( bool, m_bEnabled ); // +0x3dd  // line 76
	CNetworkVar( bool, m_bMaster ); // +0x3de  // line 77
	CNetworkVar( bool, m_bClientSide ); // +0x3df  // line 78
	CNetworkVar( bool, m_bExclusive ); // +0x3e0  // line 79
	CNetworkVar( float, m_MinFalloff ); // +0x3e4  // line 81
	CNetworkVar( float, m_MaxFalloff ); // +0x3e8  // line 82
	CNetworkVar( float, m_flCurWeight ); // +0x3ec  // line 83
	CColorCorrection::NetworkVar_m_netlookupFilename m_netlookupFilename; // +0x3f0  // line 84
	string_t m_lookupFilename; // +0x4f4  // line 86
};
