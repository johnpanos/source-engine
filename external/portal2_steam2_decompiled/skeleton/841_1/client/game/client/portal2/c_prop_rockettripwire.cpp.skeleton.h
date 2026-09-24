// DWARF declaration skeleton for game/client/portal2/c_prop_rockettripwire.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x791f0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 49
}

// game/client/portal2/c_prop_rockettripwire.cpp:22 sizeof=0xdac (i386)
struct C_PropRocketTripwire : public C_BaseAnimating
{
public:
	C_PropRocketTripwire();  // line 27
	virtual ~C_PropRocketTripwire();  // line 28
	virtual void Spawn();  // line 30
	virtual void ClientThink();  // line 31
	virtual ITraceFilter *GetBeamTraceFilter();  // line 33
private:
	C_Beam *m_pBeam; // +0xd84  // line 37
	int m_nBeamEndpointTexture; // +0xd88  // line 38
	int m_iMuzzleAttachment; // +0xd8c  // line 39
	bool m_bDrawBeams; // +0xd90  // line 40
	EHANDLE m_hRocket; // +0xd94  // line 42
	CTraceFilterSkipClassname m_filterBeams; // +0xd98  // line 45
};

// game/client/portal2/c_prop_rockettripwire.cpp:25 @0x539e30 _ZN20C_PropRocketTripwire40YouForgotToImplementOrDeclareClientClassEv
int C_PropRocketTripwire::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:25 @0x539e50 _ZN20C_PropRocketTripwire14GetClientClassEv
ClientClass *C_PropRocketTripwire::GetClientClass()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:27 @0x53a0b0 _ZN20C_PropRocketTripwireC1Ev
C_PropRocketTripwire::C_PropRocketTripwire()
{
	// inlined CHandle<C_BaseEntity>::CHandle() at line 60
}

// game/client/portal2/c_prop_rockettripwire.cpp:27 @0x53a170 _ZN20C_PropRocketTripwireC2Ev
C_PropRocketTripwire::C_PropRocketTripwire()
{
	// inlined CHandle<C_BaseEntity>::CHandle() at line 60
}

// game/client/portal2/c_prop_rockettripwire.cpp:28 @0x539ed0 _ZN20C_PropRocketTripwireD0Ev
C_PropRocketTripwire::~C_PropRocketTripwire()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:28 @0x539f70 _ZN20C_PropRocketTripwireD1Ev
C_PropRocketTripwire::~C_PropRocketTripwire()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:28 @0x53a010 _ZN20C_PropRocketTripwireD2Ev
C_PropRocketTripwire::~C_PropRocketTripwire()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:30 @0x539e70 _ZN20C_PropRocketTripwire5SpawnEv
void C_PropRocketTripwire::Spawn()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:31 @0x53a240 _ZN20C_PropRocketTripwire11ClientThinkEv
void C_PropRocketTripwire::ClientThink()
{
	Vector vecMuzzle;  // line 88
	QAngle angMuzzleDir;  // line 89
	Vector vecAimDir;  // line 92
	Vector vEndPoint;  // line 129
	float fEndFraction;  // line 130
	Ray_t rayPath;  // line 131
	// inlined Vector::operator=() at line 145
	// inlined Vector::operator+() at line 145
	// inlined Vector::operator*() at line 145
	// inlined CTraceFilterSimple::SetPassEntity() at line 140
	// inlined CHandle<C_BaseEntity>::operator C_BaseEntity*() at line 134
	// inlined Ray_t::Init() at line 132
	// inlined Vector::operator+() at line 132
	// inlined Vector::operator*() at line 132
	// inlined C_BaseEntity::RemoveEffects() at line 120
	// inlined Vector::operator*() at line 147
	// inlined Vector::operator*() at line 147
	// inlined Vector::operator+() at line 147
	// inlined Vector::operator=() at line 147
	// inlined C_Beam::SetHaloTexture() at line 98
	// inlined C_Beam::SetColor() at line 99
	// inlined C_Beam::SetBrightness() at line 100
	// inlined C_Beam::SetNoise() at line 101
	// inlined C_Beam::SetWidth() at line 102
	// inlined C_Beam::SetEndWidth() at line 103
	// inlined C_Beam::SetScrollRate() at line 104
	// inlined C_Beam::SetFadeLength() at line 105
	// inlined C_Beam::SetHaloScale() at line 106
	// inlined Vector::operator+() at line 109
	// inlined Vector::operator VectorByValue&() at line 109
	// inlined C_Beam::SetWidth() at line 111
	// inlined C_Beam::SetScrollRate() at line 112
	// inlined C_Beam::SetHaloScale() at line 113
}

// game/client/portal2/c_prop_rockettripwire.cpp:33 @0x539e60 _ZN20C_PropRocketTripwire18GetBeamTraceFilterEv
ITraceFilter *C_PropRocketTripwire::GetBeamTraceFilter()
{
}

// game/client/portal2/c_prop_rockettripwire.cpp:49 @0x79000 _Z15ClientClassInitIN21DT_PropRocketTripwire7ignoredEEiPT_
int ClientClassInit<DT_PropRocketTripwire::ignored>( DT_PropRocketTripwire::ignored * )
{
	char *pRecvTableName;  // line 49
	RecvTable &RecvTable;  // line 49
	RecvProp RecvProps[6];  // line 49
}

// game/client/portal2/c_prop_rockettripwire.cpp:49
RecvTable g_RecvTable;

// game/client/portal2/c_prop_rockettripwire.cpp:49
int g_RecvTableInit;

// game/client/portal2/c_prop_rockettripwire.cpp:49 @0x53a7e0 _ZL34_C_PropRocketTripwire_CreateObjectii
IClientNetworkable *_C_PropRocketTripwire_CreateObject( int entnum, int serialNum )
{
	C_PropRocketTripwire *pRet;  // line 49
	// inlined C_PropRocketTripwire::C_PropRocketTripwire() at line 49
}

// game/client/portal2/c_prop_rockettripwire.cpp:49
ClientClass __g_C_PropRocketTripwireClientClass;
