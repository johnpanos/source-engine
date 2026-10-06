// DWARF declaration skeleton for game/client/IClientVehicle.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/IClientVehicle.h:26 sizeof=0x4 (i386)
struct IClientVehicle : public IVehicle
{
public:
	virtual void GetVehicleFOV( float & );  // line 29
	virtual void UpdateViewAngles( C_BasePlayer *, CUserCmd * );  // line 32
	virtual void DrawHudElements();  // line 35
	virtual bool IsPredicted() const;  // line 38
	virtual C_BaseEntity *GetVehicleEnt();  // line 41
	virtual void GetVehicleClipPlanes( float &, float & ) const;  // line 44
	virtual int GetJoystickResponseCurve() const;  // line 47
};
