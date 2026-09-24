// DWARF declaration skeleton for game/server/portal2/paint_sprayer.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/paint_sprayer.h:11 sizeof=0x40c (i386)
struct CPaintSprayer : public CBaseEntity
{
public:
	CPaintSprayer();  // line 17
	virtual ~CPaintSprayer();  // line 18
	virtual void Precache();  // line 20
	virtual void Spawn();  // line 21
	virtual void Activate();  // line 22
	virtual int UpdateTransmitState();  // line 23
	virtual void UpdateOnRemove();  // line 24
	void StartPaintSpray();  // line 26
	void StopPaintSpray();  // line 27
	void InputStart( inputdata_t & );  // line 29
	void InputStop( inputdata_t & );  // line 30
	void InputChangePaintType( inputdata_t & );  // line 31
	void PaintSprayerThink();  // line 33
	void SetPaintSprayerData( bool, float, float, float, float, float, float, float, float, float, float );  // line 36
	void SetPaintPowerType( PaintPowerType );  // line 38
private:
	void SprayPaint( float );  // line 42
	bool m_bStartActive; // +0x3c0  // line 44
	float m_flLastThinkTime; // +0x3c4  // line 45
	float m_flAccumulatedTime; // +0x3c8  // line 46
	int m_nBlobSpawned; // +0x3cc  // line 47
	int m_nRenderMode; // +0x3d0  // line 48
	CNetworkVar( bool, m_bActive ); // +0x3d4  // line 50
	CNetworkVar( bool, m_bSilent ); // +0x3d5  // line 51
	CNetworkVar( int, m_PaintPowerType ); // +0x3d8  // line 53
	CNetworkVar( float, m_flBlobsPerSecond ); // +0x3dc  // line 54
	CNetworkVar( float, m_flBlobSpreadRadius ); // +0x3e0  // line 55
	CNetworkVar( float, m_flBlobSpreadAngle ); // +0x3e4  // line 56
	CNetworkVar( float, m_flMinSpeed ); // +0x3e8  // line 57
	CNetworkVar( float, m_flMaxSpeed ); // +0x3ec  // line 58
	CNetworkVar( float, m_flStreakPercentage ); // +0x3f0  // line 61
	CNetworkVar( float, m_flMinStreakTime ); // +0x3f4  // line 62
	CNetworkVar( float, m_flMaxStreakTime ); // +0x3f8  // line 63
	CNetworkVar( float, m_flMinStreakSpeedDampen ); // +0x3fc  // line 64
	CNetworkVar( float, m_flMaxStreakSpeedDampen ); // +0x400  // line 65
	CNetworkVar( float, m_flStartTime ); // +0x404  // line 67
	CNetworkHandle( CPaintStream, m_hPaintStream ); // +0x408  // line 69
};
