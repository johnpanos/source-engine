// DWARF declaration skeleton for game/client/portal2/c_info_placement_helper.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_info_placement_helper.h:16 sizeof=0xa74 (i386)
struct C_InfoPlacementHelper : public C_BaseEntity
{
public:
	C_InfoPlacementHelper();  // line 22
	virtual void Spawn();  // line 23
	virtual void UpdateOnRemove();  // line 24
	float GetTargetRadius();  // line 26
	C_BaseEntity *GetTargetOverride();  // line 29
	const Vector &GetTargetOrigin();  // line 38
	const QAngle &GetTargetAngles();  // line 55
	bool ShouldUseHelperAngles();  // line 73
	bool ShouldForcePlacement();  // line 74
	void Disable( float );  // line 76
	bool IsEnabled();  // line 77
	void BindToPortal( C_Portal_Base2D * );  // line 78
private:
	string_t m_strTargetProxy; // +0xa5c  // line 81
	string_t m_strTargetEntity; // +0xa60  // line 82
	float m_flRadius; // +0xa64  // line 83
	bool m_bSnapToHelperAngles; // +0xa68  // line 84
	bool m_bForcePlacement; // +0xa69  // line 85
	bool m_bDisabled; // +0xa6a  // line 86
	float m_flDisableTime; // +0xa6c  // line 87
	bool m_bDeferringToPortal; // +0xa70  // line 88
};

// game/client/portal2/c_info_placement_helper.h:16 (declaration)
~C_InfoPlacementHelper();

// game/client/portal2/c_info_placement_helper.h:16 @0x521920 _ZN21C_InfoPlacementHelperD0Ev
C_InfoPlacementHelper::~C_InfoPlacementHelper()
{
}

// game/client/portal2/c_info_placement_helper.h:16 @0x521990 _ZN21C_InfoPlacementHelperD1Ev
C_InfoPlacementHelper::~C_InfoPlacementHelper()
{
}
