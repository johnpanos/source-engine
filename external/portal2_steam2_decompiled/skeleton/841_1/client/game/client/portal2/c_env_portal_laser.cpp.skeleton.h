// DWARF declaration skeleton for game/client/portal2/c_env_portal_laser.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x73400 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 40
}

// game/client/portal2/c_env_portal_laser.cpp:10
const char *LASER_SPARK_EFFECT_NAME;

// game/client/portal2/c_env_portal_laser.cpp:13 sizeof=0xdb8 (i386)
struct C_PortalLaser : public C_BaseAnimating
{
public:
	C_PortalLaser();  // line 18
	virtual void Precache();  // line 20
	virtual void Spawn();  // line 21
	virtual void UpdateOnRemove();  // line 22
	virtual void OnPreDataChanged( DataUpdateType_t );  // line 24
	virtual void OnDataChanged( DataUpdateType_t );  // line 25
private:
	void CreateSparkEffect();  // line 28
	void UpdateSparkEffect();  // line 29
	CUtlReference<CNewParticleEffect> m_pSparkEffect; // +0xd84  // line 31
	Vector m_vecLaserEndPos; // +0xd90  // line 32
	Vector m_vecOldLaserEndPos; // +0xd9c  // line 33
	Vector m_vecLaserImpactNormal; // +0xda8  // line 34
	bool m_bShouldSpark; // +0xdb4  // line 35
	bool m_bOldShouldSpark; // +0xdb5  // line 36
};

// game/client/portal2/c_env_portal_laser.cpp:13 (declaration)
~C_PortalLaser();

// game/client/portal2/c_env_portal_laser.cpp:13 @0x51b040 _ZN13C_PortalLaserD0Ev
C_PortalLaser::~C_PortalLaser()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 13
}

// game/client/portal2/c_env_portal_laser.cpp:13 @0x51b120 _ZN13C_PortalLaserD1Ev
C_PortalLaser::~C_PortalLaser()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 13
}

// game/client/portal2/c_env_portal_laser.cpp:15 @0x51a770 _ZN13C_PortalLaser40YouForgotToImplementOrDeclareClientClassEv
int C_PortalLaser::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_env_portal_laser.cpp:15 @0x51a790 _ZN13C_PortalLaser14GetClientClassEv
ClientClass *C_PortalLaser::GetClientClass()
{
}

// game/client/portal2/c_env_portal_laser.cpp:18 @0x51a8a0 _ZN13C_PortalLaserC2Ev
C_PortalLaser::C_PortalLaser()
{
	// inlined CUtlReference<CNewParticleEffect>::CUtlReference() at line 53
}

// game/client/portal2/c_env_portal_laser.cpp:18 @0x51aa90 _ZN13C_PortalLaserC1Ev
C_PortalLaser::C_PortalLaser()
{
	// inlined C_PortalLaser::C_PortalLaser() at line 55
}

// game/client/portal2/c_env_portal_laser.cpp:20 @0x51a860 _ZN13C_PortalLaser8PrecacheEv
void C_PortalLaser::Precache()
{
}

// game/client/portal2/c_env_portal_laser.cpp:21 @0x51a830 _ZN13C_PortalLaser5SpawnEv
void C_PortalLaser::Spawn()
{
}

// game/client/portal2/c_env_portal_laser.cpp:22 @0x51a980 _ZN13C_PortalLaser14UpdateOnRemoveEv
void C_PortalLaser::UpdateOnRemove()
{
	// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 77
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 80
}

// game/client/portal2/c_env_portal_laser.cpp:24 @0x51aa50 _ZN13C_PortalLaser16OnPreDataChangedE16DataUpdateType_t
void C_PortalLaser::OnPreDataChanged( DataUpdateType_t updateType )
{
	// inlined Vector::operator=() at line 88
}

// game/client/portal2/c_env_portal_laser.cpp:25 @0x51ad80 _ZN13C_PortalLaser13OnDataChangedE16DataUpdateType_t
void C_PortalLaser::OnDataChanged( DataUpdateType_t updateType )
{
	// inlined Vector::operator!=() at line 102
	// inlined C_PortalLaser::UpdateSparkEffect() at line 104
	// inlined CUtlReference<CNewParticleEffect>::IsValid() at line 121
	// inlined CUtlReference<CNewParticleEffect>::IsValid() at line 114
}

// game/client/portal2/c_env_portal_laser.cpp:28 @0x51ab70 _ZN13C_PortalLaser17CreateSparkEffectEv
void C_PortalLaser::CreateSparkEffect()
{
	// inlined CUtlReference<CNewParticleEffect>::IsValid() at line 133
	// inlined C_PortalLaser::UpdateSparkEffect() at line 139
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 135
}

// game/client/portal2/c_env_portal_laser.cpp:29 @0x51a7a0 _ZN13C_PortalLaser17UpdateSparkEffectEv
void C_PortalLaser::UpdateSparkEffect()
{
	// inlined CUtlReference<CNewParticleEffect>::IsValid() at line 145
	{
		Vector vecRight;  // line 151
		Vector vecUp;  // line 151
	}
}

// game/client/portal2/c_env_portal_laser.cpp:40 @0x73220 _Z15ClientClassInitIN14DT_PortalLaser7ignoredEEiPT_
int ClientClassInit<DT_PortalLaser::ignored>( DT_PortalLaser::ignored * )
{
	char *pRecvTableName;  // line 40
	RecvTable &RecvTable;  // line 40
	RecvProp RecvProps[5];  // line 40
}

// game/client/portal2/c_env_portal_laser.cpp:40
RecvTable g_RecvTable;

// game/client/portal2/c_env_portal_laser.cpp:40
int g_RecvTableInit;

// game/client/portal2/c_env_portal_laser.cpp:40 @0x51af20 _ZL27_C_PortalLaser_CreateObjectii
IClientNetworkable *_C_PortalLaser_CreateObject( int entnum, int serialNum )
{
	C_PortalLaser *pRet;  // line 40
	// inlined C_PortalLaser::C_PortalLaser() at line 40
}

// game/client/portal2/c_env_portal_laser.cpp:40
ClientClass __g_C_PortalLaserClientClass;
