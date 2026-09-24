// DWARF declaration skeleton for game/shared/portal/paint_power_user_interface.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_power_user_interface.h:14
typedef const PaintPowerInfo_t *PaintPowerConstIter;

// game/shared/portal/paint_power_user_interface.h:16
typedef std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> PaintPowerConstRange;

// game/shared/portal/paint_power_user_interface.h:19 sizeof=0x4 (i386)
struct IPaintPowerUser
{
public:
	int (**_vptr$IPaintPowerUser)(); // +0x0  // line 0
	virtual ~IPaintPowerUser();  // line 24
	virtual const PaintPowerConstRange GetPaintPowers() const;  // line 29
	virtual const PaintPowerInfo_t &GetPaintPower( unsigned int ) const;  // line 30
	virtual const PaintPowerInfo_t *FindHighestPriorityActivePaintPower() const;  // line 31
	virtual void AddSurfacePaintPowerInfo( const PaintPowerInfo_t &, const char * );  // line 36
	virtual void UpdatePaintPowers();  // line 37
};
