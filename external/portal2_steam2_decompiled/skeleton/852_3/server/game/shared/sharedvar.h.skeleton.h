// DWARF declaration skeleton for game/shared/sharedvar.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/sharedvar.h:1113 sizeof=0x4 (i386)
struct CSharedVarSaveDataOps : public CDefSaveRestoreOps
{
private:
	virtual void Save( const SaveRestoreFieldInfo_t &, ISave * );  // line 1115
	virtual void Restore( const SaveRestoreFieldInfo_t &, IRestore * );  // line 1122
	virtual bool IsEmpty( const SaveRestoreFieldInfo_t & );  // line 1129
	virtual void MakeEmpty( const SaveRestoreFieldInfo_t & );  // line 1135
};

// game/shared/sharedvar.h:1113 (declaration)
void CSharedVarSaveDataOps();

// game/shared/sharedvar.h:1142
static CSharedVarSaveDataOps g_SharedVarSaveDataOps;
