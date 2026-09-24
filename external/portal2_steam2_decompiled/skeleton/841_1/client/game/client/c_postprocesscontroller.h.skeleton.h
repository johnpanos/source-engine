// DWARF declaration skeleton for game/client/c_postprocesscontroller.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/c_postprocesscontroller.h:20 sizeof=0xa90 (i386)
struct C_PostProcessController : public C_BaseEntity
{
public:
	C_PostProcessController();  // line 25
	virtual ~C_PostProcessController();  // line 26
	virtual void PostDataUpdate( DataUpdateType_t );  // line 28
	static C_PostProcessController *GetMasterController();  // line 30
	PostProcessParameters_t m_PostProcessParameters; // +0xa60  // line 32
private:
	bool m_bMaster; // +0xa8c  // line 35
	static C_PostProcessController *ms_pMasterController;  // line 37
};
