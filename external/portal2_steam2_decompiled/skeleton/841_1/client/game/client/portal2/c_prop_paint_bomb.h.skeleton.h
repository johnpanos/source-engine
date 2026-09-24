// DWARF declaration skeleton for game/client/portal2/c_prop_paint_bomb.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_prop_paint_bomb.h:11 sizeof=0xe30 (i386)
struct C_PropPaintBomb : public C_PhysicsProp
{
public:
	C_PropPaintBomb();  // line 15
	virtual ~C_PropPaintBomb();  // line 16
	virtual void Precache();  // line 18
	virtual void Spawn();  // line 19
	virtual void UpdateOnRemove();  // line 20
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 22
	virtual void OnDataChanged( DataUpdateType_t );  // line 23
	virtual void PostDataUpdate( DataUpdateType_t );  // line 25
	virtual bool ShouldDraw();  // line 27
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 28
	virtual void ClientThink();  // line 30
private:
	void UpdateBlobs( float );  // line 34
	void CreatePaintBombPaintBlobs();  // line 35
	void CleansePaintPower();  // line 36
	int m_nPaintPowerType; // +0xda0  // line 38
	int m_nPrevPaintPowerType; // +0xda4  // line 39
	PaintBlobVector_t m_PaintBlobs; // +0xda8  // line 41
	Vector4D m_lightingBoxColors[6]; // +0xdbc  // line 42
	Vector m_vPrevPosition; // +0xe1c  // line 44
	float m_flAccumulatedTime; // +0xe28  // line 45
	float m_flLastUpdateTime; // +0xe2c  // line 46
};

// game/client/portal2/c_prop_paint_bomb.h:27 @0x539e20 _ZN15C_PropPaintBomb10ShouldDrawEv
bool C_PropPaintBomb::ShouldDraw()
{
}
