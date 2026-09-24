// DWARF declaration skeleton for game/client/portal2/c_paint_input.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_paint_input.h:24 sizeof=0x1f0 (i386)
struct C_Paint_Input : public CInput
{
public:
	C_Paint_Input();  // line 27
	~C_Paint_Input();  // line 28
protected:
	virtual void ApplyMouse( int, QAngle &, CUserCmd *, float, float );  // line 31
	virtual void JoyStickTurn( CUserCmd *, float &, float &, float, bool, bool );  // line 32
};
