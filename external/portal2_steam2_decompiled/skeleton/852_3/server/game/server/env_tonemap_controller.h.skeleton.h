// DWARF declaration skeleton for game/server/env_tonemap_controller.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/env_tonemap_controller.h:11 sizeof=0x4f0 (i386)
struct CTonemapTrigger : public CBaseTrigger
{
public:
	virtual void Spawn();  // line 16
	virtual void StartTouch( CBaseEntity * );  // line 17
	virtual void EndTouch( CBaseEntity * );  // line 18
	CBaseEntity *GetTonemapController() const;  // line 20
private:
	string_t m_tonemapControllerName; // +0x4e8  // line 23
	EHANDLE m_hTonemapController; // +0x4ec  // line 24
};

// game/server/env_tonemap_controller.h:38 sizeof=0x10 (i386)
struct CTonemapSystem : public CAutoGameSystem
{
public:
	CTonemapSystem( const char * );  // line 42
	virtual ~CTonemapSystem();  // line 47
	virtual void LevelInitPreEntity();  // line 52
	virtual void LevelInitPostEntity();  // line 53
	CBaseEntity *GetMasterTonemapController() const;  // line 54
private:
	EHANDLE m_hMasterController; // +0xc  // line 58
};
