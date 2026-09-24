// DWARF declaration skeleton for game/client/portal/c_func_noportal_volume.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal/c_func_noportal_volume.h:16 sizeof=0xa64 (i386)
struct C_FuncNoPortalVolume : public C_BaseEntity
{
public:
	C_FuncNoPortalVolume();  // line 21
	virtual ~C_FuncNoPortalVolume();  // line 22
	bool m_bActive; // +0xa5d  // line 24
	bool IsActive();  // line 25
	static C_FuncNoPortalVolume *s_pClassList;  // line 26
	C_FuncNoPortalVolume *m_pNext; // +0xa60  // line 27
};

// game/client/portal/c_func_noportal_volume.h:30
typedef C_FuncNoPortalVolume CFuncNoPortalVolume;
