// DWARF declaration skeleton for game/client/portal2/c_paint_sprayer.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_paint_sprayer.h:8 sizeof=0xa9c (i386)
struct C_PaintSprayer : public C_BaseEntity
{
public:
	C_PaintSprayer();  // line 13
	virtual ~C_PaintSprayer();  // line 14
	virtual void Precache();  // line 16
	virtual void Spawn();  // line 17
	virtual void PreDataUpdate( DataUpdateType_t );  // line 19
	virtual void PostDataUpdate( DataUpdateType_t );  // line 20
	virtual void ClientThink();  // line 21
private:
	void SprayPaint( float );  // line 25
	bool m_bOldActive; // +0xa59  // line 27
	float m_flLastThinkTime; // +0xa5c  // line 28
	float m_flAccumulatedTime; // +0xa60  // line 29
	int m_nBlobSpawned; // +0xa64  // line 30
	bool m_bActive; // +0xa68  // line 33
	bool m_bSilent; // +0xa69  // line 34
	int m_PaintPowerType; // +0xa6c  // line 36
	float m_flBlobsPerSecond; // +0xa70  // line 37
	float m_flBlobSpreadRadius; // +0xa74  // line 38
	float m_flBlobSpreadAngle; // +0xa78  // line 39
	float m_flMinSpeed; // +0xa7c  // line 40
	float m_flMaxSpeed; // +0xa80  // line 41
	float m_flStreakPercentage; // +0xa84  // line 43
	float m_flMinStreakTime; // +0xa88  // line 44
	float m_flMaxStreakTime; // +0xa8c  // line 45
	float m_flMinStreakSpeedDampen; // +0xa90  // line 46
	float m_flMaxStreakSpeedDampen; // +0xa94  // line 47
	float m_flStartTime; // +0xa98  // line 49
};
