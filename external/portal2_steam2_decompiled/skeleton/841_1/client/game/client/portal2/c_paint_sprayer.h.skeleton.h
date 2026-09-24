// DWARF declaration skeleton for game/client/portal2/c_paint_sprayer.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_paint_sprayer.h:10 sizeof=0xaa4 (i386)
struct C_PaintSprayer : public C_BaseEntity
{
public:
	C_PaintSprayer();  // line 15
	virtual ~C_PaintSprayer();  // line 16
	virtual void Precache();  // line 18
	virtual void Spawn();  // line 19
	virtual void PreDataUpdate( DataUpdateType_t );  // line 21
	virtual void PostDataUpdate( DataUpdateType_t );  // line 22
	virtual void ClientThink();  // line 23
private:
	void SprayPaint( float );  // line 27
	bool m_bOldActive; // +0xa5d  // line 29
	float m_flLastThinkTime; // +0xa60  // line 30
	float m_flAccumulatedTime; // +0xa64  // line 31
	int m_nBlobSpawned; // +0xa68  // line 32
	bool m_bActive; // +0xa6c  // line 35
	bool m_bSilent; // +0xa6d  // line 36
	int m_PaintPowerType; // +0xa70  // line 38
	float m_flBlobsPerSecond; // +0xa74  // line 39
	float m_flBlobSpreadRadius; // +0xa78  // line 40
	float m_flBlobSpreadAngle; // +0xa7c  // line 41
	float m_flMinSpeed; // +0xa80  // line 42
	float m_flMaxSpeed; // +0xa84  // line 43
	float m_flStreakPercentage; // +0xa88  // line 45
	float m_flMinStreakTime; // +0xa8c  // line 46
	float m_flMaxStreakTime; // +0xa90  // line 47
	float m_flMinStreakSpeedDampen; // +0xa94  // line 48
	float m_flMaxStreakSpeedDampen; // +0xa98  // line 49
	float m_flStartTime; // +0xa9c  // line 51
	CNetworkHandle( C_PaintStream, m_hPaintStream ); // +0xaa0  // line 53
};
