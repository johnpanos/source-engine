// DWARF declaration skeleton for game/shared/portal/paint_power_info.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_power_info.h:13
enum PaintPowerState
{
	ACTIVATING_PAINT_POWER = 0,
	ACTIVE_PAINT_POWER = 1,
	DEACTIVATING_PAINT_POWER = 2,
	INACTIVE_PAINT_POWER = 3,
};

// game/shared/portal/paint_power_info.h:28 sizeof=0x2c (i386)
struct PaintPowerInfo_t
{
public:
	int (**_vptr$PaintPowerInfo_t)(); // +0x0  // line 0
	static typedescription_t m_PredDesc[];  // line 30
	Vector m_SurfaceNormal; // +0x4  // line 34
	Vector m_ContactPoint; // +0x10  // line 35
	PaintPowerType m_PaintPowerType; // +0x1c  // line 36
	CBaseHandle m_HandleToOther; // +0x20  // line 37
	PaintPowerState m_State; // +0x24  // line 38
	bool m_IsOnThinSurface; // +0x28  // line 39
	PaintPowerInfo_t();  // line 41
	PaintPowerInfo_t( const Vector &, const Vector &, C_BaseEntity *, PaintPowerType, bool );  // line 46
};
