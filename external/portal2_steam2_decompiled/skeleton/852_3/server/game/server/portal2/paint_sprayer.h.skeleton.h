// DWARF declaration skeleton for game/server/portal2/paint_sprayer.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/paint_sprayer.h:10 sizeof=0x404 (i386)
struct CPaintSprayer : public CBaseEntity
{
public:
	CPaintSprayer();  // line 16
	virtual ~CPaintSprayer();  // line 17
	virtual void Precache();  // line 19
	virtual void Spawn();  // line 20
	virtual void Activate();  // line 21
	virtual int UpdateTransmitState();  // line 22
	void StartPaintSpray();  // line 24
	void StopPaintSpray();  // line 25
	void InputStart( inputdata_t & );  // line 27
	void InputStop( inputdata_t & );  // line 28
	void InputChangePaintType( inputdata_t & );  // line 29
	void PaintSprayerThink();  // line 31
	void SetPaintSprayerData( bool, float, float, float, float, float, float, float, float, float, float );  // line 34
	void SetPaintPowerType( PaintPowerType );  // line 36
private:
	void SprayPaint( float );  // line 40
	bool m_bStartActive; // +0x3c0  // line 42
	float m_flLastThinkTime; // +0x3c4  // line 43
	float m_flAccumulatedTime; // +0x3c8  // line 44
	int m_nBlobSpawned; // +0x3cc  // line 45
	CNetworkVar( bool, m_bActive ); // +0x3d0  // line 47
	CNetworkVar( bool, m_bSilent ); // +0x3d1  // line 48
	CNetworkVar( int, m_PaintPowerType ); // +0x3d4  // line 50
	CNetworkVar( float, m_flBlobsPerSecond ); // +0x3d8  // line 51
	CNetworkVar( float, m_flBlobSpreadRadius ); // +0x3dc  // line 52
	CNetworkVar( float, m_flBlobSpreadAngle ); // +0x3e0  // line 53
	CNetworkVar( float, m_flMinSpeed ); // +0x3e4  // line 54
	CNetworkVar( float, m_flMaxSpeed ); // +0x3e8  // line 55
	CNetworkVar( float, m_flStreakPercentage ); // +0x3ec  // line 58
	CNetworkVar( float, m_flMinStreakTime ); // +0x3f0  // line 59
	CNetworkVar( float, m_flMaxStreakTime ); // +0x3f4  // line 60
	CNetworkVar( float, m_flMinStreakSpeedDampen ); // +0x3f8  // line 61
	CNetworkVar( float, m_flMaxStreakSpeedDampen ); // +0x3fc  // line 62
	CNetworkVar( float, m_flStartTime ); // +0x400  // line 64
};
