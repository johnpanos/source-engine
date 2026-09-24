// DWARF declaration skeleton for game/server/postprocesscontroller.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/postprocesscontroller.h:24 sizeof=0x3f0 (i386)
struct CPostProcessController : public CBaseEntity
{
public:
	CPostProcessController();  // line 30
	virtual ~CPostProcessController();  // line 31
	virtual void Activate();  // line 34
	virtual int UpdateTransmitState();  // line 35
	void InputSetFadeTime( inputdata_t & );  // line 38
	void InputSetLocalContrastStrength( inputdata_t & );  // line 39
	void InputSetLocalContrastEdgeStrength( inputdata_t & );  // line 40
	void InputSetVignetteStart( inputdata_t & );  // line 41
	void InputSetVignetteEnd( inputdata_t & );  // line 42
	void InputSetVignetteBlurStrength( inputdata_t & );  // line 43
	void InputSetFadeToBlackStrength( inputdata_t & );  // line 44
	void InputSetDepthBlurFocalDistance( inputdata_t & );  // line 45
	void InputSetDepthBlurStrength( inputdata_t & );  // line 46
	void InputSetScreenBlurStrength( inputdata_t & );  // line 47
	void InputSetFilmGrainStrength( inputdata_t & );  // line 48
	void InputTurnOn( inputdata_t & );  // line 50
	void InputTurnOff( inputdata_t & );  // line 51
	virtual void Spawn();  // line 53
	bool IsMaster() const;  // line 55
	CPostProcessController::NetworkVar_m_flPostProcessParameters m_flPostProcessParameters; // +0x3c0  // line 58
	CNetworkVar( bool, m_bMaster ); // +0x3ec  // line 60
};
